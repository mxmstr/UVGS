#include "driver_main.h"

#include <openvr_driver.h>
#include <vector> // Required for GetInterfaceVersions
#include "my_controller_driver.h" // Include the controller driver
#include "vrcommon/shared/driverlog.h" // For VRDriverLog

// Define the vr namespace
namespace vr {

// Global entry point
vr::IServerTrackedDeviceProvider *g_pMyDriverProvider = nullptr;

// MyTrackedDeviceProvider methods
MyTrackedDeviceProvider::MyTrackedDeviceProvider()
    : m_unLeftControllerDeviceIndex(vr::k_unTrackedDeviceIndexInvalid),
      m_unRightControllerDeviceIndex(vr::k_unTrackedDeviceIndexInvalid) {} // Constructor
MyTrackedDeviceProvider::~MyTrackedDeviceProvider() {} // Destructor

// Implementation of IServerTrackedDeviceProvider methods
vr::EVRInitError MyTrackedDeviceProvider::Init(vr::IVRDriverContext *pDriverContext)
{
    VR_INIT_SERVER_DRIVER_CONTEXT(pDriverContext);
    VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Starting initialization");

    uint32_t trackedDeviceCount = vr::VRServerDriverHost()->GetTrackedDeviceCount();
    VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Total tracked devices found by OpenVR: " + std::to_string(trackedDeviceCount));

    // Iterate through connected devices to find controllers
    for (uint32_t i = 0; i < trackedDeviceCount; ++i) {
        vr::PropertyContainerHandle_t container = vr::VRProperties()->TrackedDeviceToPropertyContainer(i);

        ETrackedPropertyError propError; // Renamed to avoid conflict
        int32_t device_class = vr::VRProperties()->GetInt32Property(container, vr::Prop_DeviceClass_Int32, &propError);

        if (propError != vr::TrackedProp_Success) {
            VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Error getting DeviceClass for device index " + std::to_string(i) + ": " + std::to_string(propError));
            continue;
        }

        if (device_class == vr::TrackedDeviceClass_Controller) {
            int32_t controller_role = vr::VRProperties()->GetInt32Property(container, vr::Prop_ControllerRoleHint_Int32, &propError);

            if (propError != vr::TrackedProp_Success) {
                VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Error getting ControllerRoleHint for device index " + std::to_string(i) + ": " + std::to_string(propError));
                continue;
            }

            if (controller_role == vr::TrackedControllerRole_LeftHand) {
                if (!left_controller_) { // Initialize only if not already done
                    VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Found Left Hand controller at OpenVR index: " + std::to_string(i));
                    left_controller_ = std::make_unique<MyControllerDriver>();
                    vr::EVRInitError addError = vr::VRServerDriverHost()->TrackedDeviceAdded("my_left_controller_serial", vr::TrackedDeviceClass_Controller, left_controller_.get());
                    if (addError == vr::VRInitError_None) {
                        VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Successfully added left controller.");
                        m_unLeftControllerDeviceIndex = i; // Store the device index
                        left_controller_->SetPhysicalControllerIndex(i); // Set the physical index
                    } else {
                        VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Error adding left controller: " + std::to_string(addError));
                        left_controller_.reset(); // Nullify if not added
                    }
                }
            } else if (controller_role == vr::TrackedControllerRole_RightHand) {
                if (!right_controller_) { // Initialize only if not already done
                    VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Found Right Hand controller at OpenVR index: " + std::to_string(i));
                    right_controller_ = std::make_unique<MyControllerDriver>();
                    vr::EVRInitError addError = vr::VRServerDriverHost()->TrackedDeviceAdded("my_right_controller_serial", vr::TrackedDeviceClass_Controller, right_controller_.get());
                    if (addError == vr::VRInitError_None) {
                        VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Successfully added right controller.");
                        m_unRightControllerDeviceIndex = i; // Store the device index
                        right_controller_->SetPhysicalControllerIndex(i); // Set the physical index
                    } else {
                        VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Error adding right controller: " + std::to_string(addError));
                        right_controller_.reset(); // Nullify if not added
                    }
                }
            }
        }
    }

    if (!left_controller_) {
        VRDriverLog()->Log("MyTrackedDeviceProvider::Init - No physical left controller found/initialized.");
    }
    if (!right_controller_) {
        VRDriverLog()->Log("MyTrackedDeviceProvider::Init - No physical right controller found/initialized.");
    }

    VRDriverLog()->Log("MyTrackedDeviceProvider::Init - Finished initialization");
    return vr::VRInitError_None;
}

void MyTrackedDeviceProvider::Cleanup()
{
    VRDriverLog()->Log("MyTrackedDeviceProvider::Cleanup - Called");
    VR_CLEANUP_SERVER_DRIVER_CONTEXT();
    // Cleanup your tracked devices here
    // unique_ptr will automatically clean up the controller objects
    left_controller_.reset();
    right_controller_.reset();
    g_pMyDriverProvider = nullptr; // Explicitly nullify the global pointer
    VRDriverLog()->Log("MyTrackedDeviceProvider::Cleanup - Finished");
}

const char * const *MyTrackedDeviceProvider::GetInterfaceVersions()
{
    static const char * const versions[] = { vr::IServerTrackedDeviceProvider_Version, vr::IVRDisplayComponent_Version, nullptr }; // Added IVRDisplayComponent_Version for controllers
    return versions;
}

// Define a preprocessor macro for verbose logging control, e.g., in a common header or at the top of the file
// #define ENABLE_VERBOSE_RUNFRAME_LOGGING

void MyTrackedDeviceProvider::RunFrame()
{
#ifdef ENABLE_VERBOSE_RUNFRAME_LOGGING
    VRDriverLog()->Log("MyTrackedDeviceProvider::RunFrame - Called");
#endif
    // Update device poses or states here
    if (left_controller_) {
        left_controller_->RunFrame();
    }
    if (right_controller_) {
        right_controller_->RunFrame();
    }
    // It might be useful to log if controllers are not present, but could be spammy.
    // Example:
    // if (!left_controller_ && !right_controller_) {
    //     VRDriverLog()->Log("MyTrackedDeviceProvider::RunFrame - No controllers to update.");
    // }
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
            VRDriverLog()->Log("HmdDriverFactory - Creating new MyTrackedDeviceProvider instance.");
            vr::g_pMyDriverProvider = new vr::MyTrackedDeviceProvider();
        } else {
            VRDriverLog()->Log("HmdDriverFactory - Returning existing MyTrackedDeviceProvider instance.");
        }
        if (pReturnCode)
            *pReturnCode = vr::VRInitError_None; // Set to None explicitly on success
        return vr::g_pMyDriverProvider;
    }

    VRDriverLog()->Log("HmdDriverFactory - Interface not found: " + std::string(pInterfaceName));
    if (pReturnCode)
        *pReturnCode = vr::VRInitError_Init_InterfaceNotFound;

    return NULL;
}
