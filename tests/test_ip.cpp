#include "gtest/gtest.h"
#include "protocols/ip.h"
#include "buffer.h"
#include "helpers.h"

#include "range/v3/range/conversion.hpp"
#include "range/v3/view/iota.hpp"
#include "range/v3/view/take.hpp"
#include "range/v3/view/transform.hpp"

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

constexpr std::array headerWithOptions{
    0x4f_b, 0x00_b, 0x00_b, 0x7c_b, 0x80_b, 0xb3_b, 0x40_b, 0x00_b, 0x40_b, 0x01_b, 0xf0_b, 0x5b_b, 0xac_b, 0x1f_b, 0x31_b, 0x01_b,
    0xac_b, 0x1f_b, 0x31_b, 0x02_b, 0x01_b, 0x07_b, 0x27_b, 0x08_b, 0xac_b, 0x1f_b, 0x31_b, 0x01_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b,
    0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b,
    0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x08_b, 0x00_b, 0xc4_b, 0x79_b,
    0x28_b, 0x2b_b, 0x00_b, 0x02_b, 0xad_b, 0x09_b, 0xc8_b, 0x5e_b, 0x00_b, 0x00_b, 0x00_b, 0x00_b, 0xd5_b, 0x1d_b, 0x02_b, 0x00_b,
    0x00_b, 0x00_b, 0x00_b, 0x00_b, 0x10_b, 0x11_b, 0x12_b, 0x13_b, 0x14_b, 0x15_b, 0x16_b, 0x17_b, 0x18_b, 0x19_b, 0x1a_b, 0x1b_b,
    0x1c_b, 0x1d_b, 0x1e_b, 0x1f_b, 0x20_b, 0x21_b, 0x22_b, 0x23_b, 0x24_b, 0x25_b, 0x26_b, 0x27_b, 0x28_b, 0x29_b, 0x2a_b, 0x2b_b,
    0x2c_b, 0x2d_b, 0x2e_b, 0x2f_b, 0x30_b, 0x31_b, 0x32_b, 0x33_b, 0x34_b, 0x35_b, 0x36_b, 0x37_b,
};

TEST(IP, Not_Enough_Data)
{
	const auto data = ranges::views::ints(0, static_cast<int>(protocol::ip::constants::HeaderSize - 1))
					| ranges::views::transform([] (int i) { return std::byte{static_cast<unsigned char>(i)}; });
	Buffer buffer;
	Append(data, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Result>(result));
	EXPECT_EQ(protocol::ip::Result::NotEnoughData, std::get<protocol::ip::Result>(result));
}

TEST(IP, Invalid_Header_Checksum)
{
    auto data = icmpEchoRequest | ranges::to<std::vector>();
    data[10] ^= 1_b; // mess up checksum
    Buffer buffer;
    Append(data, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Result>(result));
	EXPECT_EQ(protocol::ip::Result::ChecksumError, std::get<protocol::ip::Result>(result));
}

TEST(IP, Only_IPv4_Is_Supported)
{
	constexpr std::array headerWithVersion6 {
		0x65_b, 0x00_b, 0x00_b, 0x54_b, 0xf8_b, 0xbe_b, 0x40_b, 0x00_b, 0x40_b, 0x01_b, 0x87_b, 0xa8_b, 0xac_b, 0x1f_b, 0x31_b, 0x01_b,
		0xac_b, 0x1f_b, 0x31_b, 0x02_b
	};
	Buffer buffer;
	Append(headerWithVersion6, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Result>(result));
	EXPECT_EQ(protocol::ip::Result::Unsupported, std::get<protocol::ip::Result>(result));
}

TEST(IP, Reserved_Flag_Is_Rejected)
{
	constexpr std::array headerWithReservedFlag{
		0x45_b, 0x00_b, 0x00_b, 0x54_b, 0xf8_b, 0xbe_b, 0x80_b, 0x00_b, 0x40_b, 0x01_b, 0x87_b, 0xa8_b, 0xac_b, 0x1f_b, 0x31_b, 0x01_b,
		0xac_b, 0x1f_b, 0x31_b, 0x02_b
	};
	Buffer buffer;
	Append(headerWithReservedFlag, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Result>(result));
	EXPECT_EQ(protocol::ip::Result::CorruptHeader, std::get<protocol::ip::Result>(result));
}

TEST(IP, More_Fragments_Flag_Is_Rejected)
{
	constexpr std::array headerWithMoreFragmentsFlag{
		0x45_b, 0x00_b, 0x00_b, 0x54_b, 0xf8_b, 0xbe_b, 0x20_b, 0x00_b, 0x40_b, 0x01_b, 0x87_b, 0xa8_b, 0xac_b, 0x1f_b, 0x31_b, 0x01_b,
		0xac_b, 0x1f_b, 0x31_b, 0x02_b
	};
	Buffer buffer;
	Append(headerWithMoreFragmentsFlag, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Result>(result));
	EXPECT_EQ(protocol::ip::Result::Unsupported, std::get<protocol::ip::Result>(result));
}

TEST(IP, Fragment_Offset_Is_Unsupported)
{
	constexpr std::array headerWithNonZeroFragmentOffset{
		0x45_b, 0x00_b, 0x00_b, 0x54_b, 0xf8_b, 0xbe_b, 0x40_b, 0x02_b, 0x40_b, 0x01_b, 0x87_b, 0xa8_b, 0xac_b, 0x1f_b, 0x31_b, 0x01_b,
		0xac_b, 0x1f_b, 0x31_b, 0x02_b
	};
	Buffer buffer;
	Append(headerWithNonZeroFragmentOffset, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Result>(result));
	EXPECT_EQ(protocol::ip::Result::Unsupported, std::get<protocol::ip::Result>(result));
}

TEST(IP, Valid_ICMP_Packet)
{
    Buffer buffer;
    Append(icmpEchoRequest, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Header>(result));

	const auto& header = std::get<protocol::ip::Header>(result);
	EXPECT_EQ(0, header.tos);
	EXPECT_EQ(84, header.totalLength);
	EXPECT_EQ(63678, header.id);
	EXPECT_EQ(64, header.ttl);
	EXPECT_EQ(protocol::ip::constants::protocol::ICMP, header.protocol);
}

TEST(IP, Options_Are_Processed)
{
	Buffer buffer;
	Append(headerWithOptions, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Header>(result));

	const auto& header = std::get<protocol::ip::Header>(result);
	EXPECT_EQ(60, header.headerSize);
}

TEST(IP, Options_Length_Is_Correctly_Checked)
{
    auto data = headerWithOptions | ranges::views::take(59) | ranges::to<std::vector>();
	Buffer buffer;
	Append(data, buffer);

	const auto result = protocol::ip::ParseHeader(buffer);
	ASSERT_TRUE(std::holds_alternative<protocol::ip::Result>(result));
	EXPECT_EQ(protocol::ip::Result::NotEnoughData, std::get<protocol::ip::Result>(result));
}

TEST(IP, ConstructHeader)
{
	const protocol::ip::Header header{
		.tos = 0,
		.totalLength = 0,
		.id = 12345,
		.flags = 0,
		.frag = 0,
		.ttl = 64,
		.protocol = protocol::ip::constants::protocol::ICMP,
		.checksum = 0,
		.sourceAddr = 0xac100001,
		.destAddr = 0xac100002,
		.headerSize = 20
	};
	Buffer buffer;
	protocol::ip::ConstructHeader(header, buffer);
	EXPECT_EQ(20, buffer.ReadSpan().size());
}

}
}
