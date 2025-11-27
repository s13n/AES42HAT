
cmake_minimum_required(VERSION 3.28)

project(AES42HAT_Firmware CXX)

# Include sodaCat integration
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(sodaCat)

download_models(FALSE)

add_subdirectory(src)
