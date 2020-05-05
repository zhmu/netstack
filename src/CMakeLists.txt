add_executable(netstack main.cpp)
target_compile_features(netstack PRIVATE cxx_std_17)
target_link_libraries(netstack PRIVATE quill::quill)