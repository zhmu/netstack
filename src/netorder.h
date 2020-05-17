#include <cstdint>

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

}
}