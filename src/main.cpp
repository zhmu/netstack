#include "quill/Quill.h"
#include "buffer.h"
#include "dump.h"
#include "drivers/slipdevice.h"
#include "fmt/core.h"

#include "range/v3/view/transform.hpp"
#include "range/v3/numeric/accumulate.hpp"
#include "range/v3/algorithm/fill.hpp"

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
			for(const auto buf: *buffer) {
				netstack::dump_buffer::Dump(buf->ReadSpan(), [](const size_t offset, auto bytes, auto chars) {
					fmt::print("{:4x}: {:48s} {}\n", offset, bytes, chars);
				});
			}
		});
		printf("read done\n");
	}
	return 0;
}