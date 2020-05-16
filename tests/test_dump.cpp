#include "gtest/gtest.h"
#include "dump.h"

#include "range/v3/range/conversion.hpp"
#include "range/v3/view/iota.hpp"
#include "range/v3/view/transform.hpp"

#include "helpers.h"

using namespace netstack::helpers;

namespace netstack {
namespace {

struct Line {
	size_t offset;
	std::string bytes, chars;
};

struct StoreLines {
	StoreLines(std::vector<Line>& lines) : lines(lines) { }
	std::vector<Line>& lines;

	void operator()(size_t offset, std::string_view bytes, std::string_view chars)
	{
		const auto stringViewToString = [](auto sv) { return std::string(sv.data(), sv.size()); };
		lines.emplace_back(Line{ offset, stringViewToString(bytes), stringViewToString(chars) });
	}
};

TEST(Dump, Empty)
{
	const std::array<std::byte, 0> data;
	std::vector<Line> lines;
	dump_buffer::Dump(data, StoreLines(lines));
	EXPECT_EQ(0_sz, lines.size());
}

TEST(Dump, Single_Line)
{
	const auto data = ranges::views::ints(0, static_cast<int>(dump_buffer::DefaultProperties::bytesPerLine))
					| ranges::views::transform([] (int i) { return std::byte{static_cast<unsigned char>(i)}; });
	std::vector<Line> lines;
	dump_buffer::Dump(data, StoreLines(lines));
	ASSERT_EQ(1_sz, lines.size());
	ASSERT_EQ(0_sz, lines[0].offset);
	ASSERT_EQ("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f", lines[0].bytes);
	ASSERT_EQ("................", lines[0].chars);
}

TEST(Dump, Single_Line_Plus_One_Byte)
{
	const auto data = ranges::views::ints(0, static_cast<int>(dump_buffer::DefaultProperties::bytesPerLine + 1))
					| ranges::views::transform([] (int i) { return std::byte{static_cast<unsigned char>(i)}; });
	std::vector<Line> lines;
	dump_buffer::Dump(data, StoreLines(lines));
	ASSERT_EQ(2_sz, lines.size());
	ASSERT_EQ(0_sz, lines[0].offset);
	ASSERT_EQ("00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f", lines[0].bytes);
	ASSERT_EQ("................", lines[0].chars);
	ASSERT_EQ(16, lines[1].offset);
	ASSERT_EQ("10", lines[1].bytes);
	ASSERT_EQ(".", lines[1].chars);
}

}
}