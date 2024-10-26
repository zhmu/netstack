#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include "nonstd/span.hpp"

#include "range/v3/numeric/accumulate.hpp"
#include "range/v3/view/transform.hpp"

namespace netstack {
	 class Buffer;
	 using BufferPtr = std::unique_ptr<Buffer>;

	 template<typename T>
	 struct BufferChainIterator
	 {
		using iterator_category = std::forward_iterator_tag;
		using value_type = T*;
		using difference_type = std::ptrdiff_t;
		using pointer = T**;
		using reference = T&;

		BufferChainIterator() = default;
		BufferChainIterator(T& buffer) : buffer(&buffer) { }

		BufferChainIterator& operator++();
		BufferChainIterator operator++(int);
		value_type operator*() const { return buffer; }

		friend bool operator==(const BufferChainIterator& a, const BufferChainIterator& b) { return a.buffer == b.buffer; }
		friend bool operator!=(const BufferChainIterator& a, const BufferChainIterator& b) { return !(a == b); }

	 private:
		T* buffer{};
	 };

	 struct BufferDataIterator
	 {
		using iterator_category = std::forward_iterator_tag;
		using value_type = const std::byte;
		using difference_type = std::ptrdiff_t;
		using pointer = value_type*;
		using reference = value_type&;

		BufferDataIterator() = default;
		BufferDataIterator(const Buffer* buffer);

		BufferDataIterator& operator++();
		BufferDataIterator operator++(int);
		BufferDataIterator operator+(size_t amount);
		value_type operator*() const;

		friend bool operator==(const BufferDataIterator& a, const BufferDataIterator& b);
		friend bool operator!=(const BufferDataIterator& a, const BufferDataIterator& b) { return !(a == b); }

	 private:
		 const Buffer* buffer{};
		 nonstd::span<value_type> span;
		 nonstd::span<value_type>::iterator it;
	 };

	 struct BufferData {
		using iterator = BufferDataIterator;
		using const_iterator = iterator;

		 BufferData(const Buffer& buffer) : buffer(buffer) { }

		 iterator begin();
		 iterator end();
		 size_t size() const;

	 private:
		 const Buffer& buffer;
	 };

	 template<typename T>
	 struct BufferChain {
		using iterator = BufferChainIterator<T>;
		using const_iterator = BufferChainIterator<const T>;

		BufferChain(T& buffer) : buffer(buffer) { }

		iterator begin() { return iterator{buffer}; }
		iterator end() { return iterator{}; }
		const_iterator begin() const { return const_iterator{buffer}; }
		const_iterator end() const { return const_iterator{}; }

	 private:
	     T& buffer;
	 };
}

// p2017r1 renames this to borrowed_range; update once C++20 is a thing
template<typename T> inline constexpr bool ranges::enable_safe_range<netstack::BufferChain<T>> = true;

namespace netstack {
	 class Buffer final
	 {
	 public:
		constexpr static inline size_t Size = 1024;

		 nonstd::span<const std::byte> ReadSpan() const { return {&dataBuffer[0], filled}; }
		 nonstd::span<std::byte> WriteSpan() { return {&dataBuffer[filled], dataBuffer.size() - filled}; }

		 void IncrementFilled(const size_t amount) { filled += amount; }

		 Buffer* next() const { return nextBuffer.get(); }

		 BufferChain<Buffer> chain() { return BufferChain{*this}; }
		 BufferChain<const Buffer> chain() const { return BufferChain{*this}; }
		 BufferData data() const { return BufferData{*this}; }


		 Buffer& AddBuffer() {
			 nextBuffer = std::make_unique<Buffer>();
			 return *nextBuffer;
		 }

	 private:
		 BufferPtr nextBuffer;
		 std::array<std::byte, Size> dataBuffer;
		 size_t filled{};
	};

	template<typename T> BufferChainIterator<T>& BufferChainIterator<T>::operator++() {
		buffer = buffer->next();
		return *this;
	}

	template<typename T> BufferChainIterator<T> BufferChainIterator<T>::operator++(int) {
		BufferChainIterator copy{*this};
		operator++();
		return copy;
	}

	inline BufferData::iterator BufferData::begin()
	{
		auto b = &buffer;
		while (b != nullptr && b->ReadSpan().empty())
			b = b->next();
		return iterator{ b };
	}

	inline BufferData::iterator BufferData::end()
	{
		return iterator{ nullptr };
	}

	inline size_t BufferData::size() const
	{
		return ranges::accumulate(buffer.chain() | ranges::views::transform([] (auto b) { return b->ReadSpan().size(); }), size_t(0));
	}

	inline BufferDataIterator::BufferDataIterator(const Buffer* buffer)
		: buffer(buffer)
	{
		if (buffer != nullptr)
			span = buffer->ReadSpan();
		it = span.begin();
	}

	inline BufferDataIterator& BufferDataIterator::operator++()
	{
		++it;
		while (buffer != nullptr && it == span.end()) {
			buffer = buffer->next();
			if (buffer != nullptr)
				span = buffer->ReadSpan();
			else
				span = {};
			it = span.begin();
		}
		return *this;
	}

	inline BufferDataIterator BufferDataIterator::operator++(int)
	{
		auto copy{*this};
		operator++();
		return copy;
	}

	inline BufferDataIterator BufferDataIterator::operator+(size_t amount)
	{
		auto copy{*this};
		for (; amount > 0; --amount)
			copy.operator++();
		return copy;
	}

	inline BufferDataIterator::value_type BufferDataIterator::operator*() const
	{
		return *it;
	}

	inline bool operator==(const BufferDataIterator& a, const BufferDataIterator& b)
	{
		return a.buffer == b.buffer && a.it == b.it;
	}
}
