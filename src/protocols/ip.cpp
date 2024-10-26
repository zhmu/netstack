#include "ip.h"
#include "../buffer.h"
#include "../netorder.h"
#include "ip_checksum.h"

namespace netstack {
namespace protocol {
namespace ip {

std::variant<Result, Header> FillHeaderFromBuffer(Buffer& buffer)
{
	const auto bufferSize = buffer.data().size();
	if (bufferSize < constants::HeaderSize) return Result::NotEnoughData;

	Header header;
	net_order::Consumer consumer(buffer.data().begin());
	uint8_t version_hlen;
	consumer >> version_hlen;
	if (version_hlen >> 4 != constants::Version) return Result::Unsupported;
	header.headerSize = (version_hlen & 0xf) * sizeof(uint32_t);
	if (header.headerSize > bufferSize) return Result::NotEnoughData;

	consumer >> header.tos;
	consumer >> header.totalLength;
	consumer >> header.id;
	uint16_t flags_frag;
	consumer >> flags_frag;
	consumer >> header.ttl;
	consumer >> header.protocol;
	consumer >> header.checksum;
	consumer >> header.sourceAddr;
	consumer >> header.destAddr;

	if (flags_frag & ip::constants::flag::Reserved) return Result::CorruptHeader;
	if (flags_frag & ip::constants::flag::MF) return Result::Unsupported;

	header.flags = flags_frag;
	header.frag = flags_frag & 0x1fff;
	if (header.frag != 0) return Result::Unsupported;
	return header;
}

uint16_t CalculateHeaderChecksum(Buffer& buffer, size_t headerSize)
{
	auto readSpan = buffer.data();
	net_order::Consumer consumer(buffer.data().begin());
	return CalculateChecksum(headerSize, [&]() {
		uint8_t v; consumer >> v; return v;
	});
}

std::variant<Result, Header> ParseHeader(Buffer& buffer)
{
	auto maybe_header = FillHeaderFromBuffer(buffer);
	if (std::holds_alternative<protocol::ip::Result>(maybe_header)) return std::get<Result>(maybe_header);
	Header& header = std::get<Header>(maybe_header);

	if (auto checksum = CalculateHeaderChecksum(buffer, header.headerSize); checksum != 0) return Result::ChecksumError;

	return header;
}

void ConstructHeader(const Header& source, Buffer& buffer)
{
	{
		net_order::Producer producer(buffer.WriteSpan().begin());
		producer << static_cast<uint8_t>(0x40 + (source.headerSize / 4) & 0xf);
		producer << static_cast<uint8_t>(source.tos); // tos
		producer << static_cast<uint16_t>(source.totalLength);
		producer << static_cast<uint16_t>(source.id);
		uint16_t flag_frag = source.flags | source.frag;
		producer << static_cast<uint16_t>(flag_frag); // flags/frag offset
		producer << static_cast<uint8_t>(source.ttl); // ttl
		producer << static_cast<uint8_t>(source.protocol);
		buffer.IncrementFilled(producer.bytesProduced);
	}
	auto checksum_it = buffer.WriteSpan();
	{
		net_order::Producer producer(buffer.WriteSpan().begin());
		producer << static_cast<uint16_t>(0); // checksum
		producer << static_cast<uint32_t>(source.sourceAddr);
		producer << static_cast<uint32_t>(source.destAddr);
		buffer.IncrementFilled(producer.bytesProduced);
	}

	{
		const auto checksum = CalculateHeaderChecksum(buffer, source.headerSize);
		net_order::Producer producer(checksum_it.begin());
		producer << checksum;
	}
}

}
}
}