#include "gtest/gtest.h"
#include "buffer.h"
#include "range/v3/range/conversion.hpp"
#include "range/v3/view/iota.hpp"
#include "range/v3/view/transform.hpp"

#include "range/v3/numeric/accumulate.hpp"

#include "helpers.h"

using namespace netstack::helpers;

namespace netstack {
namespace {

constexpr std::array testBytes{ 0_b, 1_b, 2_b, 3_b, 4_b, 5_b, 6_b, 7_b, 8_b, 9_b, 10_b, 11_b, 12_b, 13_b, 14_b, 15_b };

TEST(Buffer, Construction)
{
	Buffer buffer;
}

TEST(Buffer, Empty_When_Created)
{
	Buffer buffer;
	EXPECT_TRUE(buffer.ReadSpan().empty());
	EXPECT_EQ(0_sz, buffer.ReadSpan().size());
	EXPECT_FALSE(buffer.WriteSpan().empty());
	EXPECT_EQ(static_cast<size_t>(Buffer::Size), buffer.WriteSpan().size());
}

TEST(Buffer, Data_Can_Be_Written)
{
	Buffer buffer;
	auto write = buffer.WriteSpan();
	ASSERT_GT(write.size(), testBytes.size());
	Append(testBytes, buffer);
}

TEST(Buffer, Data_Written_Can_Be_Read_Back)
{
	Buffer buffer;
	Append(testBytes, buffer);
	Verify(testBytes, buffer);
}

TEST(Buffer, Entire_Buffer_Can_Be_Used)
{
	Buffer buffer;
	const auto data = ranges::views::ints(0, static_cast<int>(Buffer::Size))
					| ranges::views::transform([] (int i) { return std::byte{static_cast<unsigned char>(i)}; });
	Append(data, buffer);
	Verify(data, buffer);
	EXPECT_TRUE(buffer.WriteSpan().empty());
}

TEST(Buffer, Iterate_Over_Single_Buffer)
{
	Buffer buffer;
	EXPECT_EQ(1_sz, std::distance(buffer.begin(), buffer.end()));
	for(const auto b: buffer) {
		EXPECT_EQ(b, &buffer);
	}
}

TEST(Buffer, Add_Second_Buffer)
{
	Buffer buffer1;
	auto& buffer2 = buffer1.AddBuffer();
	EXPECT_NE(&buffer1, &buffer2);
	EXPECT_EQ(&buffer2, buffer1.next());
	EXPECT_EQ(nullptr, buffer2.next());
}

TEST(Buffer, Chained_Buffers_Can_Be_Iterated)
{
	Buffer buffer1;
	auto& buffer2 = buffer1.AddBuffer();
	auto& buffer3 = buffer2.AddBuffer();

	const auto buffers = ranges::views::all(buffer1) | ranges::to<std::vector>();
	ASSERT_EQ(3_sz, buffers.size());
	EXPECT_EQ(&buffer1, buffers[0]);
	EXPECT_EQ(&buffer2, buffers[1]);
	EXPECT_EQ(&buffer3, buffers[2]);
}

}
}