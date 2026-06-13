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

  // Attempt to add a period with an invalid date (Feb 30th)
  // 2026 is not a leap year, and even if it were, Feb has max 29 days.
  tu::TimePeriod invalidPeriod{{2026, 2, 30, 8, 0}, {2026, 2, 30, 16, 0}, tu::AttendanceType::Work};

  if (aData->addTimePeriod(invalidPeriod) != es::Result::InvalidDate) {
      std::cerr << "[ERROR] System accepted an invalid calendar date (Feb 30th)!\n";
      return 1;
  }

  std::cout << "[SUCCESS] Invalid calendar date rejected\n";
  return 0;
}