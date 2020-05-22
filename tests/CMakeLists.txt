project(test)

include_directories(../src)
add_executable(test test_buffer.cpp test_slip.cpp test_bufferglue.cpp test_dump.cpp test_netorder.cpp test_ip_checksum.cpp)
target_link_libraries(test PRIVATE gtest_main)
target_link_libraries(test PRIVATE range-v3)
target_link_libraries(test PRIVATE fmt::fmt)
