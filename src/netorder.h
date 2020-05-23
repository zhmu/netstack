#include <cstdint>
#include <utility>

namespace netstack {
namespace net_order {

template<typename Iterator> uint8_t Consume_u8(Iterator& iterator)
{
	return std::to_integer<uint8_t>(*iterator++);
}

template<typename Iterator> uint16_t Consume_u16(Iterator& iterator)
{
	const auto hi = static_cast<uint16_t>(Consume_u8(iterator));
	const auto lo = static_cast<uint16_t>(Consume_u8(iterator));
	return (hi << 8) | lo;
}

template<typename Iterator> uint32_t Consume_u32(Iterator& iterator)
{
	const auto hi = static_cast<uint32_t>(Consume_u16(iterator));
	const auto lo = static_cast<uint32_t>(Consume_u16(iterator));
	return (hi << 16) | lo;
}

template<typename Iterator> void Produce_u8(Iterator& iterator, const uint8_t v)
{
	*iterator++ = static_cast<std::byte>(v);
}

template<typename Iterator> void Produce_u16(Iterator& iterator, const uint16_t v)
{
	Produce_u8(iterator, static_cast<uint8_t>(v >> 8));
	Produce_u8(iterator, static_cast<uint8_t>(v & 0xff));
}

template<typename Iterator> void Produce_u32(Iterator& iterator, const uint32_t v)
{
	Produce_u16(iterator, static_cast<uint16_t>(v >> 16));
	Produce_u16(iterator, static_cast<uint16_t>(v & 0xffff));
}

template<typename Iterator>
struct Consumer
{
	Consumer(Iterator&& it) : it(std::move(it)) { }

	Consumer& operator>>(uint8_t& v) {
		v = Consume_u8(it);
		return *this;
	}

	Consumer& operator>>(uint16_t& v) {
		v = Consume_u16(it);
		return *this;
	}

	Consumer& operator>>(uint32_t& v) {
		v = Consume_u32(it);
		return *this;
	}

	Iterator it;
};

template<typename Iterator>
struct Producer
{
	Producer(Iterator&& it) : it(std::move(it)) { }

	Producer& operator<<(const uint8_t v) {
		Produce_u8(it, v);
		bytesProduced += sizeof(uint8_t);
		return *this;
	}

	Producer& operator<<(const uint16_t v) {
		Produce_u16(it, v);
		bytesProduced += sizeof(uint16_t);
		return *this;
	}

	Producer& operator<<(const uint32_t v) {
		Produce_u32(it, v);
		bytesProduced += sizeof(uint32_t);
		return *this;
	}

	size_t bytesProduced{};
	Iterator it;
};

}
}