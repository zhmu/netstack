#include "slipdevice.h"
#include <unistd.h>
#include <fcntl.h>

#include "range/v3/algorithm/copy.hpp"
#include "../buffer.h"
#include "../slip.h"

namespace netstack::devices {

SLIPDevice::~SLIPDevice()
{
	Close();
}

void SLIPDevice::Close()
{
	if (fd >= 0) ::close(fd);
	fd = -1;
}

std::optional<SLIPDevice::ErrorCode> SLIPDevice::Open(std::string_view device)
{
	Close();
	fd = open(device.data(), O_NOCTTY | O_RDWR);
	if (fd < 0)
		return errno;
	return {};
}

std::optional<SLIPDevice::ErrorCode> SLIPDevice::Read(BufferGlue::BufferReceivedCallback&& callback)
{
	for(;;) {
		const auto writeSpan = glue.GetWriteSpan();
		const auto bytesReceived = ::read(fd, writeSpan.data(), writeSpan.size());
		if (bytesReceived < 0)
			return ErrorCode{errno};
		if (bytesReceived == 0)
			break;

		glue.HandleDataReceived(static_cast<size_t>(bytesReceived), [](auto span, auto&& onByte, auto&& onComplete) {
			return slip::Decode(span, onByte, onComplete);
		}, callback);

	}

	return ErrorCode{};
}

}