#include <iostream>
#include <wtts/employeeSystem.hpp>
#include <wtts/employeeData.hpp>
#include <wtts/esResult.hpp>

int main() {
  es::EmployeeSystem system;

  es::Driver* driver = nullptr;
  es::PersonnelData* pData = nullptr;
  es::AttendanceData* aData = nullptr;

  if (system.addEmployee(&driver, &pData, &aData) != es::Result::Success) {
      std::cerr << "[ERROR] Failed to add driver to system\n";
      return 1;
  }

  if (driver->logDeliveryBegin() != es::Result::Success) {
      std::cerr << "[ERROR] Initial delivery start failed\n";
      return 1;
  }

  if (driver->logDeliveryBegin() != es::Result::DeliveryAlreadyCheckedIn) {
      std::cerr << "[ERROR] Double delivery start prevention failed\n";
      return 1;
  }

  if (driver->logDeliveryEnd() != es::Result::Success) {
      std::cerr << "[ERROR] Delivery end failed\n";
      return 1;
  }

  if (driver->logDeliveryEnd() != es::Result::DeliveryNotCheckedIn) {
      std::cerr << "[ERROR] Double delivery end prevention failed\n";
      return 1;
  }

  std::cout << "[SUCCESS] Driver delivery state machine tests passed\n";
  return 0;
}