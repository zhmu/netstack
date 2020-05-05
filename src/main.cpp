#include "quill/Quill.h"

int main()
{
	quill::start();
	auto dl = quill::get_logger();

	LOG_INFO(dl, "startup");

	return 0;
}