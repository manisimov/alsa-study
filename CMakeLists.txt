cmake_minimum_required (VERSION 3.18)
project (alsa-study)

set (CMAKE_BUILD_TYPE Debug)
add_executable (minimal src/minimal.c)
target_link_libraries (minimal PRIVATE asound)
