#pragma once

#include <cstddef>
#include <cstdint>

namespace netstack {
namespace protocol {
namespace ip {

template<typename Func>
uint16_t CalculateChecksum(size_t length, Func getByte)
{
	// Algorithm from https://en.wikipedia.org/wiki/IPv4_header_checksum
	uint32_t checksum{};
	for(; length >= sizeof(uint16_t); length -= 2) {
		const auto hi = static_cast<uint16_t>(getByte());
		const auto lo = static_cast<uint16_t>(getByte());
		const auto v = (hi << 8) | lo;
		checksum += v;
	}

	const auto carry_count = checksum >> 16;
	checksum = (checksum & 0xffff) + carry_count;
	if (checksum > 0xffff)
		++checksum;
	return (~checksum) & 0xffff;
}

}
}
}
