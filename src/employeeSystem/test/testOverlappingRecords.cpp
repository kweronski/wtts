#include <iostream>
#include <wtts/employeeSystem.hpp>
#include <wtts/employeeData.hpp>
#include <wtts/esResult.hpp>

int main() {
  es::EmployeeSystem system;
  es::Employee* emp = nullptr;
  es::PersonnelData* pData = nullptr;
  es::AttendanceData* aData = nullptr;

  system.addEmployee(&emp, &pData, &aData);

  // 1. Add first record: 08:00 - 12:00 (Work)
  tu::TimePeriod p1{{2026, 6, 1, 8, 0}, {2026, 6, 1, 12, 0}, tu::AttendanceType::Work};
  if (aData->addTimePeriod(p1) != es::Result::Success) {
      std::cerr << "[ERROR] Could not add first record\n";
      return 1;
  }

  // 2. Add overlapping record: 10:00 - 14:00 (Sick)
  // This should trigger an overlap error!
  tu::TimePeriod p2{{2026, 6, 1, 10, 0}, {2026, 6, 1, 14, 0}, tu::AttendanceType::Sick};

  if (aData->addTimePeriod(p2) == es::Result::Success) {
      std::cerr << "[ERROR] System allowed overlapping records! Logic failure.\n";
      return 1; // Test fails because system accepted bad data
  }

  std::cout << "[SUCCESS] Overlapping records correctly rejected\n";
  return 0;
}