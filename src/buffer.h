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

	 class Buffer final
	 {
	 public:
	 	using value_type = Buffer;
		using iterator = BuffersIterator<Buffer>;
		using const_iterator = BuffersIterator<const Buffer>;
		constexpr static inline size_t Size = 1024;

		 nonstd::span<const std::byte> ReadSpan() const { return {&data[0], filled}; }
		 nonstd::span<std::byte> WriteSpan() { return {&data[filled], data.size() - filled}; }

		 void IncrementFilled(const size_t amount) { filled += amount; }

		 iterator begin() { return iterator{*this}; }
		 iterator end() { return iterator{}; }
		 const_iterator begin() const { return const_iterator{*this}; }
		 const_iterator end() const { return const_iterator{}; }
		 Buffer* next() const { return nextBuffer.get(); }

		 Buffer& AddBuffer() {
			 nextBuffer = std::make_unique<Buffer>();
			 return *nextBuffer;
		 }

	 private:
		 BufferPtr nextBuffer;
		 std::array<std::byte, Size> data;
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
}