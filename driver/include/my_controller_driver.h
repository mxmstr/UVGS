#pragma once

#include <openvr_driver.h>

namespace vr {

class MyControllerDriver : public vr::ITrackedDeviceServerDriver {
 public:
  MyControllerDriver();
  virtual ~MyControllerDriver();

  // Inherited via ITrackedDeviceServerDriver
  virtual vr::EVRInitError Activate(uint32_t unObjectId) override;
  virtual void Deactivate() override;
  virtual void EnterStandby() override;
  virtual void* GetComponent(const char* pchComponentNameAndVersion) override;
  virtual void DebugRequest(const char* pchRequest, char* pchResponseBuffer, uint32_t unResponseBufferSize) override;
  virtual vr::DriverPose_t GetPose() override;

  // Method to update the pose
  void RunFrame();

 private:
  uint32_t m_unObjectId; // Store the object ID
};

}  // namespace vr
