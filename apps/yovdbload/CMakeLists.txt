add_executable(yovdbload yovdbload.cpp)

set_target_properties(yovdbload PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED YES)
target_include_directories(yovdbload PUBLIC ${CMAKE_SOURCE_DIR}/libs)
target_link_libraries(yovdbload yocto)
