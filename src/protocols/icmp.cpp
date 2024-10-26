#include "icmp.h"
#include "../buffer.h"
#include "../netorder.h"
#include "ip.h"
#include "ip_checksum.h"

namespace netstack {
namespace protocol {
namespace icmp {

std::variant<Result, Header> FillHeaderFromBuffer(const ip::Header& ipHeader, Buffer& buffer)
{
	const auto bufferSize = buffer.data().size() - ipHeader.headerSize;
	if (bufferSize < constants::HeaderSize) return Result::NotEnoughData;

	Header header;
	net_order::Consumer consumer(buffer.data().begin() + ipHeader.headerSize);
	consumer >> header.type;
	consumer >> header.code;

	return header;
}

uint16_t CalculateChecksum(const ip::Header& ipHeader, Buffer& buffer)
{
	auto readSpan = buffer.data();
	net_order::Consumer consumer(buffer.data().begin() + ipHeader.headerSize);
	const auto dataSize = ipHeader.totalLength - ipHeader.headerSize;
	return ip::CalculateChecksum(dataSize, [&]() {
		uint8_t v; consumer >> v; 
		return v;
	});
}

std::variant<Result, Header> Parse(const ip::Header& ipHeader, Buffer& buffer)
{
	auto maybe_header = FillHeaderFromBuffer(ipHeader, buffer);
	if (std::holds_alternative<Result>(maybe_header)) return std::get<Result>(maybe_header);
	auto& icmpHeader = std::get<Header>(maybe_header);

	if (auto checksum = CalculateChecksum(ipHeader, buffer); checksum != 0) return Result::ChecksumError;

	return icmpHeader;
}

Buffer CreateEchoResponse(const ip::Header& ipHeader, const Header& icmpHeader, Buffer& buffer)
{
	Buffer response;
	{
		net_order::Producer producer(response.WriteSpan().begin());
		producer << constants::message_type::EchoReply;
		producer << static_cast<uint8_t>(0); // code
		producer << static_cast<uint16_t>(0); // checksum
		response.IncrementFilled(producer.bytesProduced);
	}
	const auto dataOffset = ipHeader.headerSize + 4; // icmp header is 4 bytes
	const auto dataLength = ipHeader.totalLength - dataOffset;
	std::copy(buffer.data().begin() + dataOffset, buffer.data().end(), response.WriteSpan().begin());
	response.IncrementFilled(dataLength);
	return response;
}

std::optional<Buffer> Process(const ip::Header& ipHeader, const Header& icmpHeader, Buffer& buffer)
{
	switch(icmpHeader.type) {
		case constants::message_type::EchoRequest: {
			return CreateEchoResponse(ipHeader, icmpHeader, buffer);
		}
	}
	return {};
}

}
}
}