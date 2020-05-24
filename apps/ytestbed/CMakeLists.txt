add_executable(ytestbed ytestbed.cpp)

set_target_properties(ytestbed PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(ytestbed PUBLIC ${CMAKE_SOURCE_DIR}/libs)
target_link_libraries(ytestbed yocto yocto_pathtrace)
