#include "gtest/gtest.h"
#include "protocols/icmp.h"
#include "protocols/ip.h"
#include "buffer.h"
#include "helpers.h"

#include "range/v3/range/conversion.hpp"

namespace netstack {

using namespace helpers;

namespace {

constexpr std::array icmpEchoRequest{
    0x45_b, 0x00_b, 0x00_b, 0x54_b, 0xf8_b, 0xbe_b, 0x40_b, 0x00_b, 0x40_b, 0x01_b, 0x87_b, 0xa8_b, 0xac_b, 0x1f_b, 0x31_b, 0x01_b,
    0xac_b, 0x1f_b, 0x31_b, 0x02_b, 0x08_b, 0x00_b, 0x21_b, 0xa3_b, 0xe0_b, 0xec_b, 0x00_b, 0x01_b, 0xe0_b, 0x8a_b, 0xc7_b, 0x5e_b,
    0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x8e_b, 0xb2_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x10_b, 0x11_b, 0x12_b, 0x13_b,
    0x14_b, 0x15_b, 0x16_b, 0x17_b, 0x18_b, 0x19_b, 0x1a_b, 0x1b_b, 0x1c_b, 0x1d_b, 0x1e_b, 0x1f_b, 0x20_b, 0x21_b, 0x22_b, 0x23_b,
    0x24_b, 0x25_b, 0x26_b, 0x27_b, 0x28_b, 0x29_b, 0x2a_b, 0x2b_b, 0x2c_b, 0x2d_b, 0x2e_b, 0x2f_b, 0x30_b, 0x31_b, 0x32_b, 0x33_b,
    0x34_b, 0x35_b, 0x36_b, 0x37_b
};

TEST(ICMP, Invalid_Header_Checksum)
{
    auto data = icmpEchoRequest | ranges::to<std::vector>();
    data[data.size() - 1] ^= 1_b; // mess up the last data byte
    Buffer buffer;
    Append(data, buffer);

	const auto ipResult = protocol::ip::ParseHeader(buffer);
	const auto& ipHeader = std::get<protocol::ip::Header>(ipResult);
	
	const auto icmpResult = protocol::icmp::Parse(ipHeader, buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::icmp::Result>(icmpResult));
	EXPECT_EQ(protocol::icmp::Result::ChecksumError, std::get<protocol::icmp::Result>(icmpResult));
}

TEST(ICMP, Valid_ICMP_Echo_Request_Packet)
{
    Buffer buffer;
    Append(icmpEchoRequest, buffer);

	const auto ipResult = protocol::ip::ParseHeader(buffer);
	const auto& ipHeader = std::get<protocol::ip::Header>(ipResult);
	
	const auto icmpResult = protocol::icmp::Parse(ipHeader, buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::icmp::Header>(icmpResult));
	const auto& icmpHeader = std::get<protocol::icmp::Header>(icmpResult);

	EXPECT_EQ(protocol::icmp::constants::message_type::EchoRequest, icmpHeader.type);
	EXPECT_EQ(0, icmpHeader.code);
}

}
}