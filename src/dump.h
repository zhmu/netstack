#pragma once

#include <array>
#include <string_view>
#include "fmt/core.h"

#include "range/v3/algorithm/fill.hpp"

namespace netstack {
	namespace dump_buffer {
		namespace detail {
			constexpr std::array hexTable{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
		}

		template<int N>
		struct Properties
		{
			constexpr static inline int bytesPerLine = N;
			constexpr static inline int charsPerByte = 3;
		};

		using DefaultProperties = Properties<16>;
		using CallbackFn = std::function<void(size_t offset, std::string_view bytes, std::string_view chars)>;

		template<typename Span, typename Props = DefaultProperties>
		void Dump(Span&& span, CallbackFn callback)
		{
			using namespace detail;
			std::array<char, Props::bytesPerLine * 3> hexByteBuffer;
			std::array<char, Props::bytesPerLine> textBuffer;

			size_t offset{};
			auto hexByteIterator = hexByteBuffer.begin();
			auto textBufferIterator = textBuffer.begin();
			for (auto it = span.begin(); it != span.end(); ++it, ++offset) {
				const auto byte = std::to_integer<uint8_t>(*it);
				*hexByteIterator++ = hexTable[byte >> 4];
				*hexByteIterator++ = hexTable[byte & 15];
				*hexByteIterator++ = ' ';

				*textBufferIterator++ = std::isprint(byte) ? byte : '.';

				if (std::distance(textBuffer.begin(), textBufferIterator) == Props::bytesPerLine) {
					callback(offset - Props::bytesPerLine + 1, std::string_view(hexByteBuffer.data(), Props::bytesPerLine * Props::charsPerByte - 1), std::string_view(textBuffer.data(), Props::bytesPerLine));
					hexByteIterator = hexByteBuffer.begin();
					textBufferIterator = textBuffer.begin();
				}
			}
			if (textBufferIterator != textBuffer.begin()) {
				const auto numberOfCharsForThisLine{std::distance(textBuffer.begin(), textBufferIterator)};
				callback(offset - numberOfCharsForThisLine, std::string_view(hexByteBuffer.data(), numberOfCharsForThisLine * Props::charsPerByte - 1), std::string_view(textBuffer.data(), numberOfCharsForThisLine));
			}
		}
	}
}