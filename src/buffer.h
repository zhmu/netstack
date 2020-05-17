#pragma once

#include <array>
#include <cstddef>
#include <memory>
#include "nonstd/span.hpp"

namespace netstack {
	 class Buffer;
	 using BufferPtr = std::unique_ptr<Buffer>;

	 template<typename T>
	 struct BuffersIterator
	 {
		using iterator_category = std::forward_iterator_tag;
		using value_type = T*;
		using difference_type = std::ptrdiff_t;
		using pointer = T**;
		using reference = T&;

		BuffersIterator() = default;
		BuffersIterator(T& buffer) : buffer(&buffer) { }

		BuffersIterator& operator++();
		BuffersIterator operator++(int);
		value_type operator*() const { return buffer; }

		friend bool operator==(const BuffersIterator& a, const BuffersIterator& b) { return a.buffer == b.buffer; }
		friend bool operator!=(const BuffersIterator& a, const BuffersIterator& b) { return !(a == b); }

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
		value_type operator*() const;

		friend bool operator==(const BufferDataIterator& a, const BufferDataIterator& b);
		friend bool operator!=(const BufferDataIterator& a, const BufferDataIterator& b) { return !(a == b); }

	 private:
		 const Buffer* buffer{};
		 nonstd::span<value_type> span;
		 nonstd::span<value_type>::iterator it;
	 };

	 struct BufferData {
	 	using value_type = BufferData;
		using iterator = BufferDataIterator;
		using const_iterator = iterator;

		 BufferData(const Buffer& buffer) : buffer(buffer) { }

		 iterator begin();
		 iterator end();

	 private:
		 const Buffer& buffer;
	 };

	 class Buffer final
	 {
	 public:
	 	using value_type = Buffer;
		using iterator = BuffersIterator<Buffer>;
		using const_iterator = BuffersIterator<const Buffer>;
		constexpr static inline size_t Size = 1024;

		 nonstd::span<const std::byte> ReadSpan() const { return {&dataBuffer[0], filled}; }
		 nonstd::span<std::byte> WriteSpan() { return {&dataBuffer[filled], dataBuffer.size() - filled}; }

		 void IncrementFilled(const size_t amount) { filled += amount; }

		 iterator begin() { return iterator{*this}; }
		 iterator end() { return iterator{}; }
		 const_iterator begin() const { return const_iterator{*this}; }
		 const_iterator end() const { return const_iterator{}; }
		 Buffer* next() const { return nextBuffer.get(); }

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

	template<typename T> BuffersIterator<T>& BuffersIterator<T>::operator++() {
		buffer = buffer->next();
		return *this;
	}

	template<typename T> BuffersIterator<T> BuffersIterator<T>::operator++(int) {
		BuffersIterator copy{*this};
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

	inline BufferDataIterator::value_type BufferDataIterator::operator*() const
	{
		return *it;
	}

	inline bool operator==(const BufferDataIterator& a, const BufferDataIterator& b)
	{
		return a.buffer == b.buffer && a.it == b.it;
	}
}