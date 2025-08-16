cmake_minimum_required(VERSION 3.0)

project(net)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

include_directories(lib)

set(DIRS lib)
foreach(dir IN LISTS DIRS)
    message(STATUS "add_subdirectory ${dir}")
    add_subdirectory(${dir})
endforeach()

set(bins server_select server_poll server_epoll client)
foreach(i IN LISTS bins)
    message(STATUS "build ${i}")
    add_executable(${i} ${i}.c)
    target_link_libraries(${i} wrap)
endforeach()
