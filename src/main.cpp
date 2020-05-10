#include "quill/Quill.h"
#include "buffer.h"
#include "drivers/slipdevice.h"
#include "fmt/core.h"

#include "range/v3/view/transform.hpp"
#include "range/v3/numeric/accumulate.hpp"
#include "range/v3/algorithm/fill.hpp"

namespace {
	constexpr std::array hexTable{ '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
	constexpr int bytesPerLine = 16;
	constexpr int charsPerByte = 3;
	constexpr int hexAsciiSpacer = 4;

	void DumpBuffer(netstack::Buffer& buffers)
	{
		std::array<char, 128> out;
		ranges::fill(out, ' ');

		int currentOffset{};
		for(auto buffer: buffers) {
			const auto readSpan = buffer->ReadSpan();
			fmt::print("buffer size: {}\n", readSpan.size());

			int m = 0;
			auto it = readSpan.begin();
			for (size_t n = 0; n < readSpan.size(); ++n, ++it) {
				const auto byte = std::to_integer<uint8_t>(*it);
				out[charsPerByte * m + 0] = hexTable[byte >> 4];
				out[charsPerByte * m + 1] = hexTable[byte & 0xf];
				out[bytesPerLine * charsPerByte + hexAsciiSpacer + m] = isprint(byte) ? byte : '.';
				out[bytesPerLine * charsPerByte + hexAsciiSpacer + m + 1] = '\0';
				++m;
				if (m == bytesPerLine) {
					fmt::print("{:04x}: {}\n", (n - m) + 1, out.data());
					m = 0;
				}
			}
			if (m > 0) {
				std::fill(out.begin() + m * charsPerByte, out.begin() + bytesPerLine * charsPerByte, ' ');
				fmt::print("{:04x}: {}\n", readSpan.size() - m, out.data());
			}
		}
	}
}

int main(int argc, char* argv[])
{
	quill::start();
	auto dl = quill::get_logger();
	LOG_INFO(dl, "startup");

	if (argc != 2) {
		fmt::print("usage: {} device\n", argv[0]);
		return -1;
	}
	const auto device = argv[1];

	netstack::devices::SLIPDevice slip;
	if (auto result = slip.Open(device); result) {
		fmt::print("cannot open slip device '{}': {}\n", device, strerror(*result));
		return -1;
	}

	while(true)
	{
		printf("read start\n");
		auto result = slip.Read([](std::unique_ptr<netstack::Buffer> buffer)
		{
			//if (buffer->ReadSpan().empty()) return;
			printf("ohai got buffer!\n");
			DumpBuffer(*buffer);
		});
		printf("read done\n");
	}
	return 0;
}