# Function to download files listed in a local text file
# Parameters:
#   list_file  - Path to the text file containing relative file paths
#   base_url   - Base URL to prepend to each path
#   output_dir - Directory where files will be downloaded
#   overwrite  - TRUE to overwrite existing files, FALSE to skip
function(download_files_from_list list_file base_url output_dir overwrite)
    # Ensure the output directory exists
    file(MAKE_DIRECTORY "${output_dir}")

    # Read the list of relative paths from the file
    file(READ "${list_file}" path_list_raw)

    # Normalize line endings
    string(REGEX REPLACE "[\r\n]" ";" paths ${path_list_raw})

    # Counters
    set(success_count 0)
    set(skip_count 0)
    set(fail_count 0)

    # Loop over each relative path
    foreach(relpath IN LISTS paths)
        # Trim whitespace
        string(STRIP "${relpath}" relpath)

        # Skip empty lines and comment lines
        if(relpath STREQUAL "" OR relpath MATCHES "^#")
            continue()
        endif()

        # Construct full URL
        string(CONCAT url "${base_url}" "${relpath}")

        # Extract filename (flattened target folder)
        get_filename_component(fname "${relpath}" NAME)
        set(dest "${output_dir}/${fname}")

        # Check if file exists
        if(EXISTS "${dest}" AND NOT overwrite)
            message(STATUS "Skipping existing file: ${dest}")
            math(EXPR skip_count "${skip_count} + 1")
        else()
            message(STATUS "Downloading ${url} -> ${dest}")

            # Attempt download with error handling
            file(DOWNLOAD "${url}" "${dest}" STATUS status)

            list(GET status 0 status_code)
            if(status_code EQUAL 0)
                math(EXPR success_count "${success_count} + 1")
            else()
                list(GET status 1 status_message)
                message(WARNING "Failed to download ${url}: ${status_message}")
                math(EXPR fail_count "${fail_count} + 1")
            endif()
        endif()
    endforeach()

    # Print summary
    message(STATUS "Download summary:")
    message(STATUS "  Downloaded: ${success_count}")
    message(STATUS "  Skipped:    ${skip_count}")
    message(STATUS "  Failed:     ${fail_count}")
endfunction()
