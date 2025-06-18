# OpenVR Driver Example

This repository contains an example of a C++ SteamVR driver.

## Prerequisites

*   **CMake:** Version 3.10 or higher.
*   **C++ Compiler:** A C++17 compatible compiler (e.g., GCC, Clang, MSVC).
*   **Git:** For cloning this repository and the OpenVR SDK.
*   **SteamVR:** Installed and configured.

## Setup and Building

1.  **Clone this Repository:**
    ```bash
    git clone <repository_url> OpenVRDriverExample
    cd OpenVRDriverExample
    ```
    (Replace `<repository_url>` with the actual URL of this repository)

2.  **Clone the OpenVR SDK:**
    The driver requires the OpenVR SDK. Clone it into a subdirectory named `openvr` within the `OpenVRDriverExample` directory:
    ```bash
    git clone https://github.com/ValveSoftware/openvr.git openvr
    ```
    If you clone it to a different location, you will need to specify the path to CMake using `-DOpenVR_ROOT_DIR=<path_to_openvr_sdk>`.

3.  **Configure and Build with CMake:**
    Create a build directory and run CMake:
    ```bash
    mkdir build
    cd build
    cmake ..
    # If OpenVR is not in the 'openvr' subdirectory:
    # cmake .. -DOpenVR_ROOT_DIR=/path/to/your/openvr/sdk
    ```
    Then, compile the driver:
    ```bash
    cmake --build . --config Release
    ```
    The compiled driver binary (e.g., `driver_mydriver.dll` on Windows, `libdriver_mydriver.so` on Linux) will be located in the `build/bin` directory.

## Installation in SteamVR

To use this driver with SteamVR, you need to tell SteamVR where to find it.

1.  **Locate your SteamVR Drivers Directory:**
    This is typically found within your Steam installation:
    *   Windows: `C:\Program Files (x86)\Steam\steamapps\common\SteamVR\drivers`
    *   Linux: `~/.steam/steam/steamapps/common/SteamVR/drivers`
    *   macOS: `~/Library/Application Support/Steam/steamapps/common/SteamVR/drivers` (Path may vary)

2.  **Create a Directory for Your Driver:**
    Inside the SteamVR `drivers` directory, create a new folder for your driver (e.g., `mydriver`).
    ```bash
    # Example for Linux:
    # mkdir -p ~/.steam/steam/steamapps/common/SteamVR/drivers/mydriver/bin
    ```
    You will need to create a `bin` subdirectory inside your driver's folder as specified in the `driver.vrdrivermanifest`.

3.  **Copy Driver Files:**
    *   Copy the `driver.vrdrivermanifest` file from `OpenVRDriverExample/driver/driver.vrdrivermanifest` to the `mydriver` directory you just created (e.g., `~/.steam/steam/steamapps/common/SteamVR/drivers/mydriver/`).
    *   Copy the compiled driver binary (e.g., `driver_mydriver.dll`, `libdriver_mydriver.so`) from your `OpenVRDriverExample/build/bin/` directory to the `bin` subdirectory you created (e.g., `~/.steam/steam/steamapps/common/SteamVR/drivers/mydriver/bin/`).

    Your installed driver structure should look something like this:
    ```
    SteamVR/
    └── drivers/
        └── mydriver/
            ├── driver.vrdrivermanifest
            └── bin/
                └── driver_mydriver.dll  (or .so, .dylib)
    ```

4.  **Restart SteamVR:**
    If SteamVR was running, restart it. It should now attempt to load your driver.

## Driver Details

*   **Driver Name:** `mydriver` (as specified in `driver.vrdrivermanifest`)
*   **Output Binary:** `driver_mydriver` (e.g., `driver_mydriver.dll`, `libdriver_mydriver.so`)

## Troubleshooting

*   Check the SteamVR logs for messages related to driver loading. These can be found in `Steam\logs\vrserver.txt` or `~/.steam/steam/logs/vrserver.txt`.
*   Ensure that the paths in `driver.vrdrivermanifest` correctly point to your driver binary relative to the manifest file's location.
*   Verify that your compiler and CMake are correctly configured and that the OpenVR SDK was found during the CMake configuration step.
