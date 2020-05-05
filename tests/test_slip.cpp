#include "gtest/gtest.h"
#include "buffer.h"
#include "slip.h"
#include "helpers.h"

#include "range/v3/algorithm/find.hpp"
#include "range/v3/iterator/insert_iterators.hpp"
#include "range/v3/view/iota.hpp"
#include "range/v3/view/transform.hpp"
#include "range/v3/view/filter.hpp"
#include "range/v3/range/conversion.hpp"

using namespace netstack::helpers;

namespace netstack {
namespace {

auto AsByte(const int v) { return std::byte{static_cast<uint8_t>(v)}; }

template<typename ExpectedContainer, typename DataContainer>
void EncodeDataToExpected(ExpectedContainer& expected, const DataContainer& data)
{
	for(auto b: data) {
		switch(b) {
			case slip::constants::END:
				expected.push_back(slip::constants::ESC);
				expected.push_back(slip::constants::ESC_END);
				break;
			case slip::constants::ESC:
				expected.push_back(slip::constants::ESC);
				expected.push_back(slip::constants::ESC_ESC);
				break;
			default:
				expected.push_back(b);
				break;
		}
	}
}

struct CaptureTransmit
{
	void operator()(const std::byte b)
	{
		data.push_back(b);
	}

	template<typename Expected> void Verify(const Expected& expected)
	{
		EXPECT_EQ(expected.size(), data.size());
		EXPECT_TRUE(ranges::equal(expected, data));
	}

	std::vector<std::byte> data;
};

bool IsSpecialByte(const std::byte b)
{
	constexpr std::array specialBytes{ slip::constants::END, slip::constants::ESC };
	return ranges::find(specialBytes, b) != specialBytes.end();
}

TEST(SLIP, Transmit_Empty_Buffer)
{
	CaptureTransmit capture;
	Buffer empty;
	slip::Transmit(empty, capture);

	constexpr std::array expected{ slip::constants::END, slip::constants::END };
	capture.Verify(expected);
}

TEST(SLIP, Transmit_Non_Special_Bytes_In_Single_Buffer)
{
	Buffer buffer;
	const auto data = ranges::views::ints(0, 256)
					| ranges::views::transform(AsByte)
					| ranges::views::filter(std::not_fn(IsSpecialByte))
					| ranges::to<std::vector>();
	Append(data, buffer);

	const auto expected = [&]() {
		std::vector<std::byte> expected;
		expected.push_back(slip::constants::END);
		ranges::copy(data, ranges::back_inserter(expected));
		expected.push_back(slip::constants::END);
		return expected;
	}();

	CaptureTransmit capture;
	slip::Transmit(buffer, capture);
	capture.Verify(expected);
}

TEST(SLIP, Transmit_Only_Special_Bytes_In_Single_Buffer)
{
	Buffer buffer;
	const auto data = ranges::views::ints(0, 256)
					| ranges::views::transform(AsByte)
					| ranges::views::filter(IsSpecialByte)
					| ranges::to<std::vector>();
	Append(data, buffer);

	const std::vector<std::byte> expected{
		slip::constants::END,
		slip::constants::ESC, slip::constants::ESC_END,
		slip::constants::ESC, slip::constants::ESC_ESC,
		slip::constants::END
	};

	CaptureTransmit capture;
	slip::Transmit(buffer, capture);
	capture.Verify(expected);
}

TEST(SLIP, Transmit_All_Bytes_In_Single_Buffer)
{
	Buffer buffer;
	const auto data = ranges::views::ints(0, 256)
					| ranges::views::transform(AsByte)
					| ranges::to<std::vector>();
	Append(data, buffer);

	const auto expected = [&]() {
		std::vector<std::byte> expected;
		expected.push_back(slip::constants::END);
		EncodeDataToExpected(expected, data);
		expected.push_back(slip::constants::END);
		return expected;
	}();

	CaptureTransmit capture;
	slip::Transmit(buffer, capture);
	capture.Verify(expected);
}

TEST(SLIP, Transmit_No_Special_Bytes_From_Multiple_Buffers)
{
	const auto data1 = ranges::views::ints(0, 128)
					 | ranges::views::transform(AsByte)
					 | ranges::views::filter(std::not_fn(IsSpecialByte))
					 | ranges::to<std::vector>();
	const auto data2 = ranges::views::ints(0, 128)
					 | ranges::views::transform([] (auto i) { return 128 + i; })
					 | ranges::views::transform(AsByte)
					 | ranges::views::filter(std::not_fn(IsSpecialByte))
					 | ranges::to<std::vector>();
	Buffer buffer1;
	Append(data1, buffer1);
	Buffer& buffer2 = buffer1.AddBuffer();
	Append(data2, buffer2);

	const auto expected = [&]() {
		std::vector<std::byte> expected;
		expected.push_back(slip::constants::END);
		ranges::copy(data1, ranges::back_inserter(expected));
		ranges::copy(data2, ranges::back_inserter(expected));
		expected.push_back(slip::constants::END);
		return expected;
	}();

	CaptureTransmit capture;
	slip::Transmit(buffer1, capture);
	capture.Verify(expected);
}

TEST(SLIP, Transmit_All_Bytes_From_Multiple_Buffers)
{
	const auto data = ranges::views::ints(0, 256)
					| ranges::views::transform(AsByte)
					| ranges::to<std::vector>();
	Buffer buffer1;
	Append(data, buffer1);
	Buffer& buffer2 = buffer1.AddBuffer();
	Append(data, buffer2);

	const auto expected = [&]() {
		std::vector<std::byte> expected;
		expected.push_back(slip::constants::END);
		EncodeDataToExpected(expected, data);
		EncodeDataToExpected(expected, data);
		expected.push_back(slip::constants::END);
		return expected;
	}();

	CaptureTransmit capture;
	slip::Transmit(buffer1, capture);
	capture.Verify(expected);
}

TEST(SLIP, Decode_Empty_Range)
{
	constexpr nonstd::span buffer{static_cast<std::byte*>(nullptr), 0_sz};
	auto it = buffer.begin();
	slip::Decode(it, buffer.end(), [](const auto v) {
		ADD_FAILURE() << "called";
	}, []() {
		ADD_FAILURE() << "called";
	});
	EXPECT_EQ(buffer.end(), it);
}

TEST(SLIP, Decode_NonSpecial_Bytes)
{
	const auto data = ranges::views::ints(0, 256)
					| ranges::views::transform(AsByte)
					| ranges::views::filter(std::not_fn(IsSpecialByte))
					| ranges::to<std::vector>();
	auto it = data.begin();
	std::vector<std::byte> decodedBytes;
	slip::Decode(it, data.end(), [&](const auto v) {
		decodedBytes.push_back(v);
	}, [&]() {
		ADD_FAILURE() << "called";
	});
	EXPECT_EQ(data.end(), it);
	ASSERT_EQ(decodedBytes.size(), data.size());
	EXPECT_TRUE(ranges::equal(decodedBytes, data));
}

TEST(SLIP, Decode_End_Character)
{
	constexpr std::array buffer{ slip::constants::END };
	int numberOfEndCalls{};
	auto it = buffer.begin();
	slip::Decode(it, buffer.end(), [](const auto v) {
		ADD_FAILURE() << "called";
	}, [&]() {
		++numberOfEndCalls;
	});
	EXPECT_EQ(buffer.end(), it);
	EXPECT_EQ(1, numberOfEndCalls);
}

TEST(SLIP, Decode_Escaped_Bytes)
{
	constexpr std::array data{
		slip::constants::ESC,
		slip::constants::ESC_END,
		slip::constants::ESC,
		slip::constants::ESC_ESC,
	};
	constexpr std::array expected{ slip::constants::END, slip::constants::ESC };
	auto it = data.begin();
	std::vector<std::byte> decodedBytes;
	slip::Decode(it, data.end(), [&](const auto v) {
		decodedBytes.push_back(v);
	}, [&]() {
		ADD_FAILURE() << "called";
	});
	EXPECT_EQ(data.end(), it);
	ASSERT_EQ(decodedBytes.size(), expected.size());
	EXPECT_TRUE(ranges::equal(decodedBytes, expected));
}

TEST(SLIP, Decode_Single_Escape_Does_Not_Advance_Iterator)
{
	constexpr std::array data{ slip::constants::ESC };
	auto it = data.begin();
	slip::Decode(it, data.end(), [&](const auto v) {
		ADD_FAILURE() << "called";
	}, [&]() {
		ADD_FAILURE() << "called";
	});
	EXPECT_EQ(data.begin(), it);
}

}
}