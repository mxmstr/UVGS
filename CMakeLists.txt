cmake_minimum_required(VERSION 3.10)
project(OpenVRDriverExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- OpenVR SDK ---
# Expect OpenVR to be cloned into a subdirectory named 'openvr'
# or provide its location via -DOpenVR_ROOT_DIR=<path_to_openvr_sdk>

set(OpenVR_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openvr) # Define once
set(OpenVR_CMAKE_MODULE_PATH ${OpenVR_SDK_DIR}/cmake)

# Check if the expected FindOpenVR.cmake exists
if(NOT EXISTS "${OpenVR_CMAKE_MODULE_PATH}/FindOpenVR.cmake")
    message(FATAL_ERROR "OpenVR SDK not found or incomplete in the 'openvr' subdirectory. \n"
                        "Please clone the OpenVR SDK from GitHub (https://github.com/ValveSoftware/openvr) \n"
                        "into the 'openvr' directory at the root of this project. \n"
                        "Expected path: ${OpenVR_CMAKE_MODULE_PATH}/FindOpenVR.cmake was not found.")
endif()

# Add OpenVR's cmake directory to the module path
list(APPEND CMAKE_MODULE_PATH "${OpenVR_CMAKE_MODULE_PATH}")
# Set OpenVR_DIR to point to the SDK's root
set(OpenVR_DIR ${OpenVR_SDK_DIR})

find_package(OpenVR REQUIRED)

# This check is somewhat redundant now due to the manual check above and REQUIRED in find_package,
# but it's good practice to keep it.
if(NOT OpenVR_FOUND)
    message(FATAL_ERROR "find_package(OpenVR REQUIRED) failed even after locating FindOpenVR.cmake. Check OpenVR SDK integrity and CMake logs.")
endif()

include_directories(${OpenVR_INCLUDE_DIR})

# --- Driver ---
# Add our driver as a library
add_library(MyDriver SHARED driver/src/driver_main.cpp driver/include/driver_main.h)

# Link our driver against OpenVR
target_link_libraries(MyDriver PRIVATE ${OpenVR_LIBRARIES})

# --- Output ---
# Define where the driver binary should be placed
# For example, in a 'bin' directory within the build folder
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# --- Platform Specifics ---
if(WIN32)
    # Windows specific settings
    target_compile_definitions(MyDriver PRIVATE TDC_WINDOWS)
    # Ensure the output name is just 'driver_mydriver.dll' as SteamVR might expect
    set_target_properties(MyDriver PROPERTIES PREFIX "" OUTPUT_NAME "driver_mydriver")
elseif(LINUX)
    # Linux specific settings
    target_compile_definitions(MyDriver PRIVATE TDC_LINUX)
    set_target_properties(MyDriver PROPERTIES PREFIX "lib" OUTPUT_NAME "driver_mydriver")
    target_link_options(MyDriver PRIVATE "-Wl,-Bsymbolic") # Recommended for OpenVR drivers on Linux
elseif(APPLE)
    # macOS specific settings
    target_compile_definitions(MyDriver PRIVATE TDC_MACOSX)
    set_target_properties(MyDriver PROPERTIES PREFIX "lib" OUTPUT_NAME "driver_mydriver")
    # Add other macOS specific settings if needed
endif()

message(STATUS "OpenVR Include directory: ${OpenVR_INCLUDE_DIR}")
message(STATUS "OpenVR Libraries: ${OpenVR_LIBRARIES}")
message(STATUS "Driver output directory: ${CMAKE_BINARY_DIR}/bin")
