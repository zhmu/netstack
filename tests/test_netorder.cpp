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

}
}