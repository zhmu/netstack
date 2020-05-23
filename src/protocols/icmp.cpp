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
	auto& header = std::get<Header>(maybe_header);

	if (auto checksum = CalculateChecksum(ipHeader, buffer); checksum != 0) return Result::ChecksumError;

	return header;

}

}
}
}