#include "my_controller_driver.h"
#include "vrcommon/shared/driverlog.h" // For VRDriverLog
#include <string> // For std::to_string

// Define a preprocessor macro for verbose logging control, e.g., in a common header or at the top of the file
// #define ENABLE_VERBOSE_CONTROLLER_LOGGING


namespace vr {

MyControllerDriver::MyControllerDriver()
    : m_unObjectId(vr::k_unTrackedDeviceIndexInvalid),
      m_unPhysicalControllerIndex(vr::k_unTrackedDeviceIndexInvalid) {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
  VRDriverLog()->Log("MyControllerDriver::MyControllerDriver - Constructor called.");
#endif
  // Initialize m_lastPose to a default disconnected state
  m_lastPose.poseIsValid = false;
  m_lastPose.result = vr::TrackingResult_Uninitialized;
  m_lastPose.deviceIsConnected = false; // Initially false

  m_lastPose.qWorldFromDriverRotation = {1.0, 0.0, 0.0, 0.0};
  m_lastPose.qDriverFromHeadRotation = {1.0, 0.0, 0.0, 0.0};

  for (int i = 0; i < 3; ++i) {
    m_lastPose.vecPosition[i] = 0.0;
    m_lastPose.vecVelocity[i] = 0.0;
    m_lastPose.vecAcceleration[i] = 0.0;
    m_lastPose.vecAngularVelocity[i] = 0.0;
    m_lastPose.vecAngularAcceleration[i] = 0.0;
    m_lastPose.vecWorldFromDriverTranslation[i] = 0.0;
  }
  m_lastPose.qRotation = {1.0, 0.0, 0.0, 0.0};
  m_lastPose.poseTimeOffset = 0.0;
  // m_lastPose.willDriftInYaw = false; // Already defaults to false
  // m_lastPose.shouldApplyHeadModel = false; // Already defaults to false
}

MyControllerDriver::~MyControllerDriver() {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
    VRDriverLog()->Log("MyControllerDriver::~MyControllerDriver - Destructor for ObjectId: " + std::to_string(m_unObjectId));
#endif
}

void MyControllerDriver::SetPhysicalControllerIndex(uint32_t index) {
    VRDriverLog()->Log("MyControllerDriver::SetPhysicalControllerIndex - ObjectId: " + std::to_string(m_unObjectId) + ", PhysicalIndex: " + std::to_string(index));
    m_unPhysicalControllerIndex = index;
}

vr::EVRInitError MyControllerDriver::Activate(uint32_t unObjectId) {
  VRDriverLog()->Log("MyControllerDriver::Activate - Activating controller with ObjectId: " + std::to_string(unObjectId));
  m_unObjectId = unObjectId;

  // Initialize properties or query them from the physical device if necessary
  // Example: vr::VRProperties()->SetStringProperty(m_ulPropertyContainer, Prop_SerialNumber_String, "my-virtual-controller-123");
  // Check errors for property settings.

  // When the device is activated, it's considered connected from the driver's perspective.
  // The actual tracking state (poseIsValid, result) will be updated in RunFrame.
  m_lastPose.deviceIsConnected = true;

  VRDriverLog()->Log("MyControllerDriver::Activate - Controller activated with ObjectId: " + std::to_string(m_unObjectId));
  return vr::VRInitError_None;
}

void MyControllerDriver::Deactivate() {
  VRDriverLog()->Log("MyControllerDriver::Deactivate - Deactivating controller with ObjectId: " + std::to_string(m_unObjectId));
  // Clean up resources, if any were allocated in Activate or during operation
  m_unObjectId = vr::k_unTrackedDeviceIndexInvalid; // Mark as invalid
}

void MyControllerDriver::EnterStandby() {
  VRDriverLog()->Log("MyControllerDriver::EnterStandby - Controller with ObjectId: " + std::to_string(m_unObjectId) + " entering standby.");
  // For a controller, this might mean reducing update rates or preparing for power saving.
  // The pose should reflect that it's in standby (e.g., poseIsValid = false, result = TrackingResult_Uninitialized).
  m_lastPose.poseIsValid = false;
  m_lastPose.result = vr::TrackingResult_Uninitialized;
  // Keep deviceIsConnected true if it's just standby, or false if it should appear disconnected.
  // Let's assume it remains connected but not tracking.
  m_lastPose.deviceIsConnected = true;

  // It's good practice to also inform the host that the pose was updated due to standby.
  if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
      vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, m_lastPose, sizeof(vr::DriverPose_t));
  }
}

void* MyControllerDriver::GetComponent(const char* pchComponentNameAndVersion) {
  // Return a pointer to a component interface, or nullptr if not supported
  return nullptr;
}

void MyControllerDriver::DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) {
  // Handle debug requests
  if (unResponseBufferSize > 0) {
    pchResponseBuffer[0] = '\0';
  }
}

vr::DriverPose_t MyControllerDriver::GetPose() {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
    VRDriverLog()->Log("MyControllerDriver::GetPose - Called for ObjectId: " + std::to_string(m_unObjectId));
#endif
  return m_lastPose;
}

void MyControllerDriver::RunFrame() {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
    if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) { // Avoid logging if not yet activated
        VRDriverLog()->Log("MyControllerDriver::RunFrame - Called for ObjectId: " + std::to_string(m_unObjectId));
    }
#endif

  if (m_unObjectId == vr::k_unTrackedDeviceIndexInvalid) {
    return; // Not activated yet
  }

  bool physical_pose_retrieved = false;
  if (m_unPhysicalControllerIndex != vr::k_unTrackedDeviceIndexInvalid) {
    vr::VRControllerState_t controllerState;
    if (vr::VRServerDriverHost()->GetControllerState(m_unPhysicalControllerIndex, &controllerState, sizeof(controllerState))) {
      // Successfully got controller state
      if (!controllerState.trackedDevicePose.bDeviceIsConnected) {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
          VRDriverLog()->Log("MyControllerDriver::RunFrame - Physical controller (Index: " + std::to_string(m_unPhysicalControllerIndex) + ") for ObjectId: " + std::to_string(m_unObjectId) + " is not connected.");
#endif
      }
      if (!controllerState.trackedDevicePose.bPoseIsValid) {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
          VRDriverLog()->Log("MyControllerDriver::RunFrame - Physical controller (Index: " + std::to_string(m_unPhysicalControllerIndex) + ") for ObjectId: " + std::to_string(m_unObjectId) + " pose is not valid.");
#endif
      }

      if (controllerState.trackedDevicePose.bDeviceIsConnected && controllerState.trackedDevicePose.bPoseIsValid) {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
        VRDriverLog()->Log("MyControllerDriver::RunFrame - Valid physical pose found for ObjectId: " + std::to_string(m_unObjectId) + ". Applying to virtual controller.");
#endif
        // Copy pose from controllerState.trackedDevicePose (vr::TrackedDevicePose_t) to m_lastPose (vr::DriverPose_t)
        m_lastPose.poseIsValid = controllerState.trackedDevicePose.bPoseIsValid;
        m_lastPose.result = controllerState.trackedDevicePose.eTrackingResult;
        // For the virtual device, its 'deviceIsConnected' status is managed by Activate/Deactivate.
        // Here, we are interested in the physical device's pose data.
        // If the physical device disconnects or its pose is invalid, we'll fall into the 'else' block below
        // to set the virtual device's pose to an untracked state.
        // However, the m_lastPose.deviceIsConnected should still reflect the *virtual* device's connection state,
        // which is generally true after Activate() unless Deactivate() or EnterStandby() changes it.
        // So, we don't directly copy controllerState.trackedDevicePose.bDeviceIsConnected to m_lastPose.deviceIsConnected here.
        // We ensure m_lastPose.deviceIsConnected is true because the virtual controller is active.
        m_lastPose.deviceIsConnected = true;


        m_lastPose.vecPosition[0] = controllerState.trackedDevicePose.mDeviceToAbsoluteTracking.m[0][3];
        m_lastPose.vecPosition[1] = controllerState.trackedDevicePose.mDeviceToAbsoluteTracking.m[1][3];
        m_lastPose.vecPosition[2] = controllerState.trackedDevicePose.mDeviceToAbsoluteTracking.m[2][3];

        m_lastPose.qRotation = controllerState.trackedDevicePose.qRotation;

        m_lastPose.vecVelocity[0] = controllerState.trackedDevicePose.vVelocity.v[0];
        m_lastPose.vecVelocity[1] = controllerState.trackedDevicePose.vVelocity.v[1];
        m_lastPose.vecVelocity[2] = controllerState.trackedDevicePose.vVelocity.v[2];

        m_lastPose.vecAngularVelocity[0] = controllerState.trackedDevicePose.vAngularVelocity.v[0];
        m_lastPose.vecAngularVelocity[1] = controllerState.trackedDevicePose.vAngularVelocity.v[1];
        m_lastPose.vecAngularVelocity[2] = controllerState.trackedDevicePose.vAngularVelocity.v[2];

        m_lastPose.poseTimeOffset = 0.f;
        m_lastPose.qWorldFromDriverRotation = {1.0, 0.0, 0.0, 0.0};
        m_lastPose.vecWorldFromDriverTranslation[0] = 0.0;
        m_lastPose.vecWorldFromDriverTranslation[1] = 0.0;
        m_lastPose.vecWorldFromDriverTranslation[2] = 0.0;
        m_lastPose.qDriverFromHeadRotation = {1.0, 0.0, 0.0, 0.0};

        physical_pose_retrieved = true;
      }
    } else {
        VRDriverLog()->Log("MyControllerDriver::RunFrame - GetControllerState failed for physical controller (Index: " + std::to_string(m_unPhysicalControllerIndex) + ") for ObjectId: " + std::to_string(m_unObjectId));
    }
  } else {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
      if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) { // Avoid logging if not yet activated
          VRDriverLog()->Log("MyControllerDriver::RunFrame - ObjectId: " + std::to_string(m_unObjectId) + " has invalid m_unPhysicalControllerIndex. No physical device to track.");
      }
#endif
  }

  if (!physical_pose_retrieved) {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
    VRDriverLog()->Log("MyControllerDriver::RunFrame - ObjectId: " + std::to_string(m_unObjectId) + " - Physical pose not retrieved. Falling back to out_of_range pose.");
#endif
    m_lastPose.poseIsValid = false;
    m_lastPose.result = vr::TrackingResult_Running_OutOfRange;
    m_lastPose.deviceIsConnected = true; // Virtual device is still connected
  }

  vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, m_lastPose, sizeof(vr::DriverPose_t));
}

}  // namespace vr
