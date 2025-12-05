
cmake_minimum_required(VERSION 3.28)

project(AES42HAT_Firmware CXX)

set(SODACAT_COMMIT "main" CACHE STRING "sodaCat commit/branch to fetch from")
set(SODACAT_URL_BASE "https://raw.githubusercontent.com/s13n/sodaCat/${SODACAT_COMMIT}")

# Include sodaCat integration
if(EXISTS "${CMAKE_SOURCE_DIR}/cmake/sodaCat.cmake")
    message(VERBOSE "Using local sodaCat.cmake")
else()
    message(STATUS "Downloading ${SODACAT_URL_BASE}/cmake/sodaCat.cmake -> cmake/sodaCat.cmake")

    # Attempt download with error handling
    file(DOWNLOAD "${SODACAT_URL_BASE}/cmake/sodaCat.cmake" "${CMAKE_SOURCE_DIR}/cmake/sodaCat.cmake" STATUS status)

    list(GET status 0 status_code)
    if(status_code EQUAL 0)
        message(VERBOSE "sodaCat.cmake downloaded successfully")
    else()
        list(GET status 1 status_message)
        message(WARNING "Failed to download sodaCat.cmake: ${status_message}")
    endif()
endif()

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
include(sodaCat)

download_models(FALSE manifest.txt)

add_subdirectory(src)
