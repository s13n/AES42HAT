
cmake_minimum_required(VERSION 3.28)

project(AES42HAT_Firmware CXX)

# Include sodaCat integration
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(download_files)

set(SODACAT_COMMIT "main" CACHE STRING "sodaCat commit/branch to fetch from")
download_files_from_list(
    "${CMAKE_CURRENT_SOURCE_DIR}/manifest.txt"
    "https://raw.githubusercontent.com/s13n/sodaCat/${SODACAT_COMMIT}/"
    "${CMAKE_CURRENT_SOURCE_DIR}/models"
    FALSE
)

# Create the update target
#sodacat_add_update_target()

# Verify local files exist (comment out during initial setup)
#sodacat_verify_local_files()

add_subdirectory(src)
