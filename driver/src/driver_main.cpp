#include "driver_main.h"

#include <openvr_driver.h>
#include <vector> // Required for GetInterfaceVersions

// Global entry point
vr::IServerTrackedDeviceProvider *g_pMyDriverProvider = nullptr;

// HMD_DLL_EXPORT void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
// {
//     if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName))
//     {
//         if (!g_pMyDriverProvider) {
//             g_pMyDriverProvider = new MyTrackedDeviceProvider();
//         }
//         return g_pMyDriverProvider;
//     }

//     if (pReturnCode)
//         *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

//     return NULL;
// }


// Implementation of IServerTrackedDeviceProvider methods
vr::EVRInitError MyTrackedDeviceProvider::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    // TODO: Initialize your tracked devices here
    return vr::VRInitError_None;
}

void MyTrackedDeviceProvider::Cleanup()
{
    VR_CLEANUP_SERVER_DRIVER_CONTEXT();
    // TODO: Cleanup your tracked devices here
    g_pMyDriverProvider = nullptr; // Ensure we reset the global pointer
}

const char * const *MyTrackedDeviceProvider::GetInterfaceVersions()
{
    static const char * const versions[] = { vr::IServerTrackedDeviceProvider_Version, nullptr };
    return versions;
}

void MyTrackedDeviceProvider::RunFrame()
{
    // TODO: Update device poses or states here
}

bool MyTrackedDeviceProvider::ShouldBlockStandby()
{
    return false;
}

void MyTrackedDeviceProvider::EnterStandby()
{
}

void MyTrackedDeviceProvider::LeaveStandby()
{
}

// Global entry point function
#if defined(_WIN32)
#define HMD_DLL_EXPORT extern "C" __declspec( dllexport )
#elif defined(__GNUC__) || defined(COMPILER_GCC) || defined(__APPLE__)
#define HMD_DLL_EXPORT extern "C" __attribute__((visibility("default")))
#else
#error "Unsupported Platform."
#endif

HMD_DLL_EXPORT void *HmdDriverFactory(const char *pInterfaceName, int *pReturnCode)
{
    if (0 == strcmp(vr::IServerTrackedDeviceProvider_Version, pInterfaceName))
    {
        if (!g_pMyDriverProvider) { // Check if already instantiated
            g_pMyDriverProvider = new MyTrackedDeviceProvider();
        }
        if (pReturnCode)
            *pReturnCode = vr::VRInitError_None; // Set to None explicitly on success
        return g_pMyDriverProvider;
    }

    if (pReturnCode)
        *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

    return NULL;
}
