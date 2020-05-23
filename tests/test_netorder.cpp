#include "gtest/gtest.h"
#include "netorder.h"
#include "helpers.h"

namespace netstack {
namespace {

using namespace helpers;

TEST(NetworkByte, Read_u8)
{
	constexpr std::array data{ 0_b, 0x55_b, 0xaa_b, 0xff_b };
	auto it = data.begin();
	EXPECT_EQ(0, net_order::Consume_u8(it));
	EXPECT_EQ(85, net_order::Consume_u8(it));
	EXPECT_EQ(170, net_order::Consume_u8(it));
	EXPECT_EQ(255, net_order::Consume_u8(it));
	EXPECT_EQ(data.end(), it);
}

TEST(NetworkByte, Read_u16)
{
	constexpr std::array data{ 0_b, 1_b, 1_b, 2_b, 0x55_b, 0xaa_b, 0xff_b, 0xff_b };
	auto it = data.begin();
	EXPECT_EQ(0x0001, net_order::Consume_u16(it));
	EXPECT_EQ(0x0102, net_order::Consume_u16(it));
	EXPECT_EQ(0x55aa, net_order::Consume_u16(it));
	EXPECT_EQ(0xffff, net_order::Consume_u16(it));
	EXPECT_EQ(data.end(), it);
}

TEST(NetworkByte, Read_u32)
{
	constexpr std::array data{ 0_b, 1_b, 2_b, 3_b, 0x55_b, 0xaa_b, 0xff_b, 0x00_b };
	auto it = data.begin();
	EXPECT_EQ(0x00010203, net_order::Consume_u32(it));
	EXPECT_EQ(0x55aaff00, net_order::Consume_u32(it));
	EXPECT_EQ(data.end(), it);
}

TEST(NetworkByte, Consumer)
{
	constexpr std::array data{ 0_b, 1_b, 2_b, 3_b, 4_b, 5_b, 6_b };
	net_order::Consumer consumer(data.begin());
	{ uint8_t v; consumer >> v; EXPECT_EQ(0x00, v); }
	{ uint16_t v; consumer >> v; EXPECT_EQ(0x0102, v); }
	{ uint32_t v; consumer >> v; EXPECT_EQ(0x03040506, v); }
}

TEST(NetworkByte, Write_u8)
{
	std::array<std::byte, 2> data{};
	auto it = data.begin();
	net_order::Produce_u8(it, 0x12);
	net_order::Produce_u8(it, 0x34);
	EXPECT_EQ(data.end(), it);
	EXPECT_EQ(0x12_b, data[0]);
	EXPECT_EQ(0x34_b, data[1]);
}

TEST(NetworkByte, Write_u16)
{
	std::array<std::byte, 4> data{};
	auto it = data.begin();
	net_order::Produce_u16(it, 0x55aa);
	net_order::Produce_u16(it, 0xff00);
	EXPECT_EQ(data.end(), it);
	EXPECT_TRUE(ranges::equal(std::array{ 0x55_b, 0xaa_b, 0xff_b, 0x00_b }, data));
}

TEST(NetworkByte, Write_u32)
{
	std::array<std::byte, 8> data{};
	auto it = data.begin();
	net_order::Produce_u32(it, 0x12345678);
	net_order::Produce_u32(it, 0xff55aa00);
	EXPECT_EQ(data.end(), it);
	EXPECT_TRUE(ranges::equal(std::array{ 0x12_b, 0x34_b, 0x56_b, 0x78_b, 0xff_b, 0x55_b, 0xaa_b, 0x00_b }, data));
}

TEST(NetworkByte, Producer)
{
	std::array<std::byte, 8> data;
	net_order::Producer producer(data.begin());
	producer << static_cast<uint8_t>(0x00);
	producer << static_cast<uint16_t>(0x0102);
	producer << static_cast<uint32_t>(0x03040506);
	producer << static_cast<uint8_t>(0x07);
	EXPECT_EQ(producer.bytesProduced, std::distance(data.begin(), data.end()));
	EXPECT_TRUE(ranges::equal(std::array{ 0x00_b, 0x01_b, 0x02_b, 0x03_b, 0x04_b, 0x05_b, 0x06_b, 0x07_b }, data));
}

}
}