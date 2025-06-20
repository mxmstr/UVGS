#include "my_controller_driver.h"
#include "vrcommon/shared/driverlog.h" // For VRDriverLog
#include "vrcommon/shared/vrmath.h" // For HmdQuaternion_Init_FromMatrix and potentially other math utilities
#include <string> // For std::to_string
#include <vector> // For std::vector

// Define a preprocessor macro for verbose logging control, e.g., in a common header or at the top of the file
// #define ENABLE_VERBOSE_CONTROLLER_LOGGING
// #define ENABLE_VERY_VERBOSE_CONTROLLER_LOGGING // For per-frame pose details

#include <cmath> // For sqrt, used in manual quaternion conversion if needed

namespace vr {

// Helper function to decompose HmdMatrix34_t into position and rotation components
static void DecomposeHmdMatrix34IntoPose(const vr::HmdMatrix34_t& matrix, double outVecPosition[3], vr::HmdQuaternion_t& outQRotation) {
    // Position
    outVecPosition[0] = matrix.m[0][3];
    outVecPosition[1] = matrix.m[1][3];
    outVecPosition[2] = matrix.m[2][3];

    // Rotation
    // Try using HmdQuaternion_Init_FromMatrix first, as it's expected from vrmath.h
    outQRotation = HmdQuaternion_Init_FromMatrix(matrix);

    // Manual implementation as a fallback (example, would need to be uncommented and HmdQuaternion_Init_FromMatrix call removed if it's not available)
    /*
    VRDriverLog()->Log("DecomposeHmdMatrix34IntoPose - Using manual matrix to quaternion conversion."); // Log if manual path is taken
    double trace = matrix.m[0][0] + matrix.m[1][1] + matrix.m[2][2];
    if (trace > 0.0) {
        double s = 0.5 / sqrt(trace + 1.0);
        outQRotation.w = 0.25 / s;
        outQRotation.x = (matrix.m[2][1] - matrix.m[1][2]) * s;
        outQRotation.y = (matrix.m[0][2] - matrix.m[2][0]) * s;
        outQRotation.z = (matrix.m[1][0] - matrix.m[0][1]) * s;
    } else {
        if (matrix.m[0][0] > matrix.m[1][1] && matrix.m[0][0] > matrix.m[2][2]) {
            double s = 2.0 * sqrt(1.0 + matrix.m[0][0] - matrix.m[1][1] - matrix.m[2][2]);
            outQRotation.w = (matrix.m[2][1] - matrix.m[1][2]) / s;
            outQRotation.x = 0.25 * s;
            outQRotation.y = (matrix.m[0][1] + matrix.m[1][0]) / s;
            outQRotation.z = (matrix.m[0][2] + matrix.m[2][0]) / s;
        } else if (matrix.m[1][1] > matrix.m[2][2]) {
            double s = 2.0 * sqrt(1.0 + matrix.m[1][1] - matrix.m[0][0] - matrix.m[2][2]);
            outQRotation.w = (matrix.m[0][2] - matrix.m[2][0]) / s;
            outQRotation.x = (matrix.m[0][1] + matrix.m[1][0]) / s;
            outQRotation.y = 0.25 * s;
            outQRotation.z = (matrix.m[1][2] + matrix.m[2][1]) / s;
        } else {
            double s = 2.0 * sqrt(1.0 + matrix.m[2][2] - matrix.m[0][0] - matrix.m[1][1]);
            outQRotation.w = (matrix.m[1][0] - matrix.m[0][1]) / s;
            outQRotation.x = (matrix.m[0][2] + matrix.m[2][0]) / s;
            outQRotation.y = (matrix.m[1][2] + matrix.m[2][1]) / s;
            outQRotation.z = 0.25 * s;
        }
    }
    // Normalize quaternion if necessary (HmdQuaternion_Init_FromMatrix should handle this)
    // double norm = sqrt(outQRotation.x * outQRotation.x + outQRotation.y * outQRotation.y + outQRotation.z * outQRotation.z + outQRotation.w * outQRotation.w);
    // if (norm > 0) {
    //    outQRotation.x /= norm; outQRotation.y /= norm; outQRotation.z /= norm; outQRotation.w /= norm;
    // }
    */
}


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

#ifdef ENABLE_VERY_VERBOSE_CONTROLLER_LOGGING
  VRDriverLog()->Log("MyControllerDriver::RunFrame - Calling GetRawTrackedDevicePoses for ObjectId: " + std::to_string(m_unObjectId));
#endif
  // Use GetRawTrackedDevicePoses
  std::vector<vr::TrackedDevicePose_t> vecPoseBuffer(vr::k_unMaxTrackedDeviceCount);
  vr::VRServerDriverHost()->GetRawTrackedDevicePoses(0.f, vecPoseBuffer.data(), (uint32_t)vecPoseBuffer.size());

  if (m_unPhysicalControllerIndex != vr::k_unTrackedDeviceIndexInvalid && m_unPhysicalControllerIndex < vr::k_unMaxTrackedDeviceCount) {
    const vr::TrackedDevicePose_t& physicalDevicePose = vecPoseBuffer[m_unPhysicalControllerIndex];

    if (!physicalDevicePose.bDeviceIsConnected) {
#ifdef ENABLE_VERY_VERBOSE_CONTROLLER_LOGGING
        VRDriverLog()->Log("MyControllerDriver::RunFrame - Physical controller (idx " + std::to_string(m_unPhysicalControllerIndex) + ") for ObjectId: " + std::to_string(m_unObjectId) + " reported bDeviceIsConnected=false.");
#endif
    }
    if (!physicalDevicePose.bPoseIsValid) {
#ifdef ENABLE_VERY_VERBOSE_CONTROLLER_LOGGING
        VRDriverLog()->Log("MyControllerDriver::RunFrame - Physical controller (idx " + std::to_string(m_unPhysicalControllerIndex) + ") for ObjectId: " + std::to_string(m_unObjectId) + " reported bPoseIsValid=false.");
#endif
    }

    if (physicalDevicePose.bDeviceIsConnected && physicalDevicePose.bPoseIsValid) {
#ifdef ENABLE_VERY_VERBOSE_CONTROLLER_LOGGING // Changed to VERY_VERBOSE for this level of detail
      VRDriverLog()->Log("MyControllerDriver::RunFrame - Successfully updated m_lastPose from physical controller (idx " + std::to_string(m_unPhysicalControllerIndex) + ") for ObjectId: " + std::to_string(m_unObjectId));
#endif
      m_lastPose.poseIsValid = physicalDevicePose.bPoseIsValid;
      m_lastPose.result = physicalDevicePose.eTrackingResult;
      m_lastPose.deviceIsConnected = true; // Virtual device is active and reflecting a physical one

      // Decompose matrix into position and rotation
      DecomposeHmdMatrix34IntoPose(physicalDevicePose.mDeviceToAbsoluteTracking, m_lastPose.vecPosition, m_lastPose.qRotation);

      // Velocity
      m_lastPose.vecVelocity[0] = physicalDevicePose.vVelocity.v[0];
      m_lastPose.vecVelocity[1] = physicalDevicePose.vVelocity.v[1];
      m_lastPose.vecVelocity[2] = physicalDevicePose.vVelocity.v[2];

      // Angular Velocity
      m_lastPose.vecAngularVelocity[0] = physicalDevicePose.vAngularVelocity.v[0];
      m_lastPose.vecAngularVelocity[1] = physicalDevicePose.vAngularVelocity.v[1];
      m_lastPose.vecAngularVelocity[2] = physicalDevicePose.vAngularVelocity.v[2];

      // Driver-specific pose parameters
      m_lastPose.poseTimeOffset = 0.f;
      m_lastPose.qWorldFromDriverRotation = {1.0, 0.0, 0.0, 0.0};
      m_lastPose.vecWorldFromDriverTranslation[0] = 0.0;
      m_lastPose.vecWorldFromDriverTranslation[1] = 0.0;
      m_lastPose.vecWorldFromDriverTranslation[2] = 0.0;
      m_lastPose.qDriverFromHeadRotation = {1.0, 0.0, 0.0, 0.0};

      physical_pose_retrieved = true;
    }
  } else { // m_unPhysicalControllerIndex is invalid or out of bounds
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
    if (m_unObjectId != vr::k_unTrackedDeviceIndexInvalid) { // Should always be valid here
      if (m_unPhysicalControllerIndex == vr::k_unTrackedDeviceIndexInvalid) {
        // This case is normal if no physical controller is assigned.
        // Might be too spammy to log every frame, consider logging it once or less frequently.
        // For now, let's make it very verbose or remove for this specific sub-condition.
#ifdef ENABLE_VERY_VERBOSE_CONTROLLER_LOGGING
        VRDriverLog()->Log("MyControllerDriver::RunFrame - ObjectId: " + std::to_string(m_unObjectId) + " has no valid physical controller index assigned.");
#endif
      } else { // Index is out of bounds
        VRDriverLog()->Log("MyControllerDriver::RunFrame - ObjectId: " + std::to_string(m_unObjectId) + " - m_unPhysicalControllerIndex " + std::to_string(m_unPhysicalControllerIndex) + " is out of bounds. Cannot use physical pose.");
      }
    }
#endif
  }

  if (!physical_pose_retrieved) {
#ifdef ENABLE_VERBOSE_CONTROLLER_LOGGING
    // Differentiate message based on why pose wasn't retrieved
    if (m_unPhysicalControllerIndex != vr::k_unTrackedDeviceIndexInvalid && m_unPhysicalControllerIndex < vr::k_unMaxTrackedDeviceCount) {
        // We had a valid index, but the pose from it was not usable (disconnected or invalid)
        VRDriverLog()->Log("MyControllerDriver::RunFrame - ObjectId: " + std::to_string(m_unObjectId) + " - Physical controller (idx " + std::to_string(m_unPhysicalControllerIndex) + ") data not used (e.g., disconnected or pose invalid). Falling back to out_of_range pose for virtual controller.");
    } else {
        // The physical index itself was the problem (either k_unTrackedDeviceIndexInvalid or out of bounds)
        // This case is already logged above, so a more generic fallback message here.
        VRDriverLog()->Log("MyControllerDriver::RunFrame - ObjectId: " + std::to_string(m_unObjectId) + " - No valid physical pose retrieved. Falling back to out_of_range pose for virtual controller.");
    }
#endif
    m_lastPose.poseIsValid = false;
    m_lastPose.result = vr::TrackingResult_Running_OutOfRange;
    m_lastPose.deviceIsConnected = true; // Virtual device is still connected
  }

  vr::VRServerDriverHost()->TrackedDevicePoseUpdated(m_unObjectId, m_lastPose, sizeof(vr::DriverPose_t));
}

}  // namespace vr
