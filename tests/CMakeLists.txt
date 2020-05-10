project(test)

include_directories(../src)
add_executable(test test_buffer.cpp test_slip.cpp test_bufferglue.cpp)
target_link_libraries(test gtest_main range-v3)