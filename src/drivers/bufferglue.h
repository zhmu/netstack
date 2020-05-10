#pragma once

#include <functional>
#include "nonstd/span.hpp"
#include "range/v3/algorithm/copy.hpp"
#include "../buffer.h"

namespace netstack {
class BufferGlue
{
public:
	using BufferReceivedCallback = std::function<void(std::unique_ptr<Buffer>)>;

	auto GetWriteSpan()
	{
		return nonstd::span{ receiveBuffer.data() + receiveBufferFilled, receiveBuffer.size() - receiveBufferFilled };
	}

	template<typename ProcessFn> void HandleDataReceived(const size_t bytesReceived, ProcessFn&& process, BufferReceivedCallback callback)
	{
		const auto bufferSpan = nonstd::span{ receiveBuffer.data(), receiveBufferFilled + bytesReceived };
		const auto it = process(bufferSpan, [&](const std::byte b)
		{
			if (!currentBuffer) {
				currentBuffer = std::make_unique<Buffer>();
				fillingBuffer = currentBuffer.get();
			}

			auto writeSpan = fillingBuffer->WriteSpan();
			if (writeSpan.empty()) {
				fillingBuffer = &fillingBuffer->AddBuffer();
				writeSpan = fillingBuffer->WriteSpan();
			}
			writeSpan.front() = b;
			fillingBuffer->IncrementFilled(1);
		}, [&]() {
			callback(std::move(currentBuffer));
			fillingBuffer = nullptr;
		});

		receiveBufferFilled = std::distance(it, bufferSpan.cend());
		ranges::copy(it, receiveBuffer.cend(), receiveBuffer.begin());
	}

private:
	std::unique_ptr<Buffer> currentBuffer;
	Buffer* fillingBuffer{};
	std::array<std::byte, 1024> receiveBuffer;
	size_t receiveBufferFilled{};
};
}