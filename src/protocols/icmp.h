#pragma once

#include <optional>
#include <variant>
#include <cstdint>

namespace netstack {

class Buffer;
namespace protocol {
namespace ip {
	struct Header;
}
namespace icmp {

namespace constants {
static constexpr inline size_t HeaderSize = 4;

namespace message_type {
	static constexpr inline uint8_t EchoReply = 0;
	static constexpr inline uint8_t EchoRequest = 8;
}
}

struct Header {
	uint8_t type;
	uint8_t code;

	uint16_t headerSize;
};

enum class Result {
	Unsupported,
	NotEnoughData,
	ChecksumError
};

Buffer CreateEchoResponse(const ip::Header& ipHeader, const Header& icmpHeader, Buffer& buffer);

std::variant<Result, Header> Parse(const ip::Header&, Buffer&);
std::optional<Buffer> Process(const ip::Header& ipHeader, const Header& icmpHeader, Buffer& buffer);

}
}
}