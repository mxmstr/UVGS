#include "driver_main.h"

#include <openvr_driver.h>
#include <vector> // Required for GetInterfaceVersions
#include "my_controller_driver.h" // Include the controller driver

// Define the vr namespace
namespace vr {

// Global entry point
vr::IServerTrackedDeviceProvider *g_pMyDriverProvider = nullptr;

// MyTrackedDeviceProvider methods
MyTrackedDeviceProvider::MyTrackedDeviceProvider() {} // Constructor
MyTrackedDeviceProvider::~MyTrackedDeviceProvider() {} // Destructor

// Implementation of IServerTrackedDeviceProvider methods
vr::EVRInitError MyTrackedDeviceProvider::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);

    // Initialize your tracked devices here
    left_controller_ = std::make_unique<MyControllerDriver>();
    if (left_controller_) {
        vr::VRServerDriverHost()->TrackedDeviceAdded("my_left_controller_serial", vr::TrackedDeviceClass_Controller, left_controller_.get());
    }

    right_controller_ = std::make_unique<MyControllerDriver>();
    if (right_controller_) {
        vr::VRServerDriverHost()->TrackedDeviceAdded("my_right_controller_serial", vr::TrackedDeviceClass_Controller, right_controller_.get());
    }

    return vr::VRInitError_None;
}

void MyTrackedDeviceProvider::Cleanup()
{
    VR_CLEANUP_SERVER_DRIVER_CONTEXT();
    // Cleanup your tracked devices here
    // unique_ptr will automatically clean up the controller objects
    left_controller_.reset();
    right_controller_.reset();
    // g_pMyDriverProvider = nullptr; // This is handled by the caller or at a higher level
}

const char * const *MyTrackedDeviceProvider::GetInterfaceVersions()
{
    static const char * const versions[] = { vr::IServerTrackedDeviceProvider_Version, vr::IVRDisplayComponent_Version, nullptr }; // Added IVRDisplayComponent_Version for controllers
    return versions;
}

void MyTrackedDeviceProvider::RunFrame()
{
    // Update device poses or states here
    if (left_controller_) {
        left_controller_->RunFrame();
    }
    if (right_controller_) {
        right_controller_->RunFrame();
    }
}

bool MyTrackedDeviceProvider::ShouldBlockStandbyMode()
{
    return false;
}

void MyTrackedDeviceProvider::EnterStandby()
{
}

void MyTrackedDeviceProvider::LeaveStandby()
{
}

} // namespace vr

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
        if (!vr::g_pMyDriverProvider) { // Check if already instantiated
            vr::g_pMyDriverProvider = new vr::MyTrackedDeviceProvider();
        }
        if (pReturnCode)
            *pReturnCode = vr::VRInitError_None; // Set to None explicitly on success
        return vr::g_pMyDriverProvider;
    }

    if (pReturnCode)
        *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

    return NULL;
}
