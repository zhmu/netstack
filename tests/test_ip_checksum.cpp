#include "gtest/gtest.h"
#include "protocols/ip_checksum.h"
#include "netorder.h"
#include "helpers.h"
#include <array>

using namespace netstack::helpers;

namespace netstack {
namespace {

template<typename Container>
auto CalculateChecksumFor(const Container& container)
{
	net_order::Consumer consumer(container.begin());
	return protocol::ip::CalculateChecksum(container.size(), [&]() {
		uint8_t v; consumer >> v;
		return v;
	});
}

TEST(IPChecksum, Calculate_Checksum_1)
{
	// Example from https://en.wikipedia.org/wiki/IPv4_header_checksum
	constexpr std::array ipHeader{
        0x45_b, 0x00_b, 0x00_b, 0x73_b, 0x00_b, 0x00_b, 0x40_b, 0x00_b, 0x40_b, 0x11_b, 0x00_b, 0x00_b, 0xc0_b, 0xa8_b, 0x00_b, 0x01_b,
        0xc0_b, 0xa8_b, 0x00_b, 0xc7_b,
	};
	EXPECT_EQ(0xb861, CalculateChecksumFor(ipHeader));
}

TEST(IPChecksum, Calculate_Checksum_2)
{
	// Just some sniffed data when pinging my stack
	constexpr std::array ipHeader{
        0x45_b, 0x00_b, 0x00_b, 0x54_b, 0xf8_b, 0xbe_b, 0x40_b, 0x00_b, 0x40_b, 0x01_b, 0x00_b, 0x00_b, 0xac_b, 0x1f_b, 0x31_b, 0x01_b,
        0xac_b, 0x1f_b, 0x31_b, 0x02_b
	};
	EXPECT_EQ(0x87a8, CalculateChecksumFor(ipHeader));
}

}
}
