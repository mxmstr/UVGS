cmake_minimum_required(VERSION 3.10)
project(OpenVRDriverExample LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- OpenVR SDK ---
# Expect OpenVR to be cloned into a subdirectory named 'openvr'
# or provide its location via -DOpenVR_ROOT_DIR=<path_to_openvr_sdk>

# Add OpenVR's cmake directory to the module path so find_package can locate FindOpenVR.cmake
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/openvr/cmake")
# Also, set OpenVR_DIR to point to the SDK's root, as FindOpenVR.cmake might use it.
# This assumes the user has cloned 'openvr' into the project root.
set(OpenVR_DIR ${CMAKE_CURRENT_SOURCE_DIR}/openvr)

find_package(OpenVR REQUIRED)

if(NOT OpenVR_FOUND)
    message(FATAL_ERROR "OpenVR SDK not found. Please clone it into an 'openvr' subdirectory or specify OpenVR_ROOT_DIR.")
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
