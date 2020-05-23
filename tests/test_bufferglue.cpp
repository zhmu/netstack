#include "gtest/gtest.h"
#include "buffer.h"
#include "drivers/bufferglue.h"
#include "helpers.h"

#include "range/v3/range/conversion.hpp"

using namespace netstack::helpers;

namespace netstack {
namespace {

constexpr std::array testBytes{ 0_b, 1_b, 2_b, 3_b, 4_b, 5_b, 6_b, 7_b, 8_b, 9_b, 10_b, 11_b, 12_b, 13_b, 14_b, 15_b };

namespace constants {
constexpr auto FLUSH = std::byte{0xff};
}

template<typename Container, typename OnByteFn, typename OnEndFn> typename Container::const_iterator Process(const Container& container, OnByteFn&& onByte, OnEndFn&& onEnd)
{
	const auto end{container.end()};
	auto it{container.begin()};
	while(it != end) {
		const auto byte = *it++;
		if (byte == constants::FLUSH) {
			onEnd();
		} else {
			onByte(byte);
		}
	}
	return it;
}


TEST(BufferGlue, Construction)
{
	BufferGlue glue;
}

TEST(BufferGlue, Empty)
{
	BufferGlue glue;
	const auto bufferSize = glue.GetWriteSpan().size();
	glue.HandleDataReceived(0_sz, [](auto span, auto&& onByte, auto&& onComplete) { return Process(span, onByte, onComplete); }, [](auto buffer)
	{
		FAIL() << "unexpected END call";
	});
	EXPECT_EQ(bufferSize, glue.GetWriteSpan().size());
}

TEST(BufferGlue, Filling_Some_Bytes_Without_End_Must_Buffer_Them)
{
	BufferGlue glue;
	auto writeSpan = glue.GetWriteSpan();
	ranges::copy(testBytes, writeSpan.begin());

	const auto bufferSize = glue.GetWriteSpan().size();
	glue.HandleDataReceived(testBytes.size(), [](auto span, auto&& onByte, auto&& onComplete) { return Process(span, onByte, onComplete); }, [](auto buffer)
	{
		FAIL() << "unexpected END call";
	});
	EXPECT_EQ(bufferSize, glue.GetWriteSpan().size());
}

TEST(BufferGlue, Filling_Some_Bytes_With_Flush_Must_Process_Them)
{
	BufferGlue glue;
	auto testBytesWithFlush = testBytes | ranges::to<std::vector>();
	testBytesWithFlush.push_back(constants::FLUSH);
	{
		auto writeSpan = glue.GetWriteSpan();
		ranges::copy(testBytesWithFlush, writeSpan.begin());
	}

	const auto bufferSize = glue.GetWriteSpan().size();
	int numberOfEndCalls{};
	glue.HandleDataReceived(testBytesWithFlush.size(), [](auto span, auto&& onByte, auto&& onComplete) { return Process(span, onByte, onComplete); }, [&](auto buffer)
	{
		Verify(testBytes, *buffer);
		++numberOfEndCalls;
	});
	EXPECT_EQ(bufferSize, glue.GetWriteSpan().size());
	EXPECT_EQ(1, numberOfEndCalls);
}

TEST(BufferGlue, Filling_Some_Bytes_With_Flush_And_Then_Some_Does_Not_Process_The_Unflushed_Ones)
{
	BufferGlue glue;
	auto testBytesWithFlush = testBytes | ranges::to<std::vector>();
	testBytesWithFlush.push_back(constants::FLUSH);

	if (auto writeSpan = glue.GetWriteSpan(); true) ranges::copy(testBytesWithFlush, writeSpan.begin());
	if (auto writeSpan = glue.GetWriteSpan(); true) ranges::copy(testBytes, writeSpan.begin());

	const auto bufferSize = glue.GetWriteSpan().size();
	int numberOfEndCalls{};
	glue.HandleDataReceived(2 * testBytes.size() + 1, [](auto span, auto&& onByte, auto&& onComplete) { return Process(span, onByte, onComplete); }, [&](auto buffer)
	{
		Verify(testBytes, *buffer);
		++numberOfEndCalls;
	});
	EXPECT_EQ(bufferSize, glue.GetWriteSpan().size());
	EXPECT_EQ(1, numberOfEndCalls);
}

TEST(BufferGlue, Empty_Buffers_Are_Never_Complete)
{
	BufferGlue glue;
	const std::vector<std::byte> testFlushBytes{ 10, constants::FLUSH };
	{
		auto writeSpan = glue.GetWriteSpan();
		ranges::copy(testFlushBytes, writeSpan.begin());
	}

	const auto bufferSize = glue.GetWriteSpan().size();
	int numberOfEndCalls{};
	glue.HandleDataReceived(testFlushBytes.size(), [](auto span, auto&& onByte, auto&& onComplete) { return Process(span, onByte, onComplete); }, [&](auto buffer)
	{
		++numberOfEndCalls;
	});
	EXPECT_EQ(bufferSize, glue.GetWriteSpan().size());
	EXPECT_EQ(0, numberOfEndCalls);
}

}
}