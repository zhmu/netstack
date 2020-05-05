cmake_minimum_required(VERSION 3.10)
project(netstack)
set(CMAKE_CXX_STANDARD 17)

include_directories(external/googletest/googletest/include external/span-lite/include)

add_subdirectory(external/googletest)
add_subdirectory(external/range-v3)
add_subdirectory(external/fmt)
add_subdirectory(external/quill)

add_subdirectory(tests)
add_subdirectory(src)
