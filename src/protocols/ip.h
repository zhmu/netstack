#pragma once

#include <variant>
#include <cstdint>

namespace netstack {

class Buffer;
namespace protocol {
namespace ip {

namespace constants {
	static constexpr inline uint8_t Version = 4;
	static constexpr inline size_t HeaderSize = 20;

namespace flag {
	static constexpr inline uint16_t Reserved = (1 << 15);
	static constexpr inline uint16_t DF = (1 << 14);
	static constexpr inline uint16_t MF = (1 << 13);
}
namespace protocol {
	static constexpr inline uint8_t ICMP = 1;
	static constexpr inline uint8_t TCP = 6;
	static constexpr inline uint8_t UDP = 17;
}
}

struct Header {
	uint8_t tos;
	uint16_t totalLength;
	uint16_t id;
	uint16_t flags;
	uint16_t frag;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t checksum;
	uint32_t sourceAddr;
	uint32_t destAddr;

	uint16_t headerSize;
};

enum class Result {
	OK,
	Unsupported,
	NotEnoughData,
	InvalidVersion,
	CorruptHeader,
	ChecksumError,
};

std::variant<Result, Header> ParseHeader(Buffer& buffer);

}
}
}