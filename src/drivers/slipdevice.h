#pragma once

#include <array>
#include <functional>
#include <memory>
#include <string_view>
#include <variant>
#include "bufferglue.h"
#include "nonstd/span.hpp"

namespace netstack { class Buffer; }

namespace netstack::devices {

class SLIPDevice final
{
public:
	static constexpr inline size_t MaxPacketSize = 65536;
	using ErrorCode = int;

	SLIPDevice() = default;
	~SLIPDevice();
	SLIPDevice(const SLIPDevice&) = delete;
	SLIPDevice& operator=(const SLIPDevice&) = delete;

	std::optional<ErrorCode> Open(std::string_view device);
	void Close();

	std::optional<ErrorCode> Read(BufferGlue::BufferReceivedCallback&& callback);

private:
	int fd{-1};
	BufferGlue glue;
};

}