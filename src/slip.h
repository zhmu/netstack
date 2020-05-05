#pragma once

#include "buffer.h"
#include "range/v3/algorithm/for_each.hpp"

namespace netstack::slip {
	namespace constants {
		static constexpr inline std::byte END{0xc0};
		static constexpr inline std::byte ESC{0xdb};
		static constexpr inline std::byte ESC_END{0xdc};
		static constexpr inline std::byte ESC_ESC{0xdd};
	}
	
	template<typename TransmitFn> void Transmit(Buffer& buffer, TransmitFn&& transmit)
	{
		transmit(constants::END);
		ranges::for_each(buffer, [&](const auto buffer)
		{
			ranges::for_each(buffer->ReadSpan(), [&](const auto byte)
			{
				switch(byte) {
					case constants::END:
						transmit(constants::ESC);
						transmit(constants::ESC_END);
						break;
					case constants::ESC:
						transmit(constants::ESC);
						transmit(constants::ESC_ESC);
						break;
					default:
						transmit(byte);
						break;
				}
			});
		});
		transmit(constants::END);
	}

	template<typename Iterator, typename OnByteFn, typename OnEndFn> void Decode(Iterator& it, Iterator end, OnByteFn&& onByte, OnEndFn&& onEnd)
	{
		while(it != end) {
			auto byte = *it;
			switch (byte) {
				case constants::END:
					onEnd();
					++it;
					continue;
				case constants::ESC: {
					if (auto nextIt{it}; ++nextIt == end)
						return;
					++it;
					byte = *it;
					switch(byte) {
						case constants::ESC_END:
							byte = constants::END;
							break;
						case constants::ESC_ESC:
							byte = constants::ESC;
							break;
					}
					break;
				}
			}
			onByte(byte);
			++it;
		}
	}
}