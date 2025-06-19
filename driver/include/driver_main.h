#pragma once

#include <openvr_driver.h>
#include <memory> // Required for std::unique_ptr

#include "my_controller_driver.h" // Include the new controller driver header

namespace vr { // Added namespace

class MyTrackedDeviceProvider : public vr::IServerTrackedDeviceProvider { // Added namespace
 public:
  MyTrackedDeviceProvider(); // Added constructor
  virtual ~MyTrackedDeviceProvider(); // Added destructor

  // Member variables to store device indices
  uint32_t m_unLeftControllerDeviceIndex;
  uint32_t m_unRightControllerDeviceIndex;

  virtual vr::EVRInitError Init(vr::IVRDriverContext* pDriverContext) override;
  virtual void Cleanup() override;
  virtual const char* const* GetInterfaceVersions() override;
  virtual void RunFrame() override;
  virtual bool ShouldBlockStandbyMode() override;
  virtual void EnterStandby() override;
  virtual void LeaveStandby() override;

 private: // Added private section
  std::unique_ptr<MyControllerDriver> left_controller_;
  std::unique_ptr<MyControllerDriver> right_controller_;

};

}  // namespace vr // Added namespace
