#pragma once

#include "range/v3/algorithm/copy.hpp"
#include "range/v3/algorithm/equal.hpp"
#include "buffer.h"

namespace netstack::helpers {

constexpr std::byte operator "" _b(unsigned long long v) { return std::byte{static_cast<unsigned char>(v)}; }
constexpr std::size_t operator "" _sz(unsigned long long v) { return static_cast<size_t>(v); }

template<typename Source> void Append(const Source& source, Buffer& buffer)
{
	auto write = buffer.WriteSpan();
	ranges::copy(source, write.begin());
	buffer.IncrementFilled(source.size());
}

template<typename Container> void Verify(const Container& expected, Buffer& buffer)
{
	auto read = buffer.ReadSpan();
	ASSERT_EQ(expected.size(), read.size());
	ASSERT_TRUE(ranges::equal(expected, read));
}

}