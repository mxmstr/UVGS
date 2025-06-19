#include "my_controller_driver.h"

namespace vr {

MyControllerDriver::MyControllerDriver() : m_unObjectId(vr::k_unTrackedDeviceIndexInvalid) {}

MyControllerDriver::~MyControllerDriver() {}

vr::EVRInitError MyControllerDriver::Activate(uint32_t unObjectId) {
  // Store the object ID
  m_unObjectId = unObjectId;
  // Initialize properties, etc.
  return vr::VRInitError_None;
}

void MyControllerDriver::Deactivate() {
  // Clean up resources
}

void MyControllerDriver::EnterStandby() {
  // Called when the device enters standby mode
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
  // Return the current pose of the controller
  vr::DriverPose_t pose = {0};
  pose.poseIsValid = true; // Set to true because we are providing a pose
  pose.result = vr::TrackingResult_Running_OK;
  pose.deviceIsConnected = true; // Assume connected

  pose.qWorldFromDriverRotation = {1, 0, 0, 0}; // Identity quaternion
  pose.qDriverFromHeadRotation = {1, 0, 0, 0};  // Identity quaternion
  pose.qRotation = {1, 0, 0, 0}; // Identity quaternion

  // Position (example: 0,0,0)
  pose.vecPosition[0] = 0.0;
  pose.vecPosition[1] = 0.0;
  pose.vecPosition[2] = 0.0;

  // Set a default pose (identity)
  pose.poseTimeOffset = 0.f;
  pose.vecWorldFromDriverTranslation[0] = 0.f;
  pose.vecWorldFromDriverTranslation[1] = 0.f;
  pose.vecWorldFromDriverTranslation[2] = 0.f;

  // Velocity and angular velocity can be zero
  pose.vecVelocity[0] = 0.f;
  pose.vecVelocity[1] = 0.f;
  pose.vecVelocity[2] = 0.f;
  pose.vecAngularVelocity[0] = 0.f;
  pose.vecAngularVelocity[1] = 0.f;
  pose.vecAngularVelocity[2] = 0.f;

  return pose;
}

void MyControllerDriver::RunFrame() {
  if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) {
    vr::DriverPose_t pose = GetPose(); // Get the static pose
    vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, pose, sizeof(vr::DriverPose_t));
  }
}

}  // namespace vr
