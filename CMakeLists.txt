project(mulog)

cmake_minimum_required(VERSION 2.6.2)

set(INCLUDE_INSTALL_DIR
    "${CMAKE_INSTALL_PREFIX}/include/mulog"
    CACHE INTERNAL
    "The directory where we install the header files (internal)"
  )

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")

include_directories(src)

add_subdirectory(src)
add_subdirectory(test test/bin)