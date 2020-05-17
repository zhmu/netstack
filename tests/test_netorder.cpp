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
}
}