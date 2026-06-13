#include <iostream>
#include <cmath>
#include <wtts/employeeSystem.hpp>
#include <wtts/employeeData.hpp>

// Validation macro
bool hasErrors = false;
#define ASSERT_APPROX_EQ(expected, actual, testName)                           \
  do {                                                                         \
    if (std::abs((expected) - (actual)) > 0.001) {                             \
      std::cerr << "[ERROR] " << testName << " | Expected: " << (expected)     \
                << ", but got: " << (actual) << "\n";                          \
      hasErrors = true;                                                        \
    } else {                                                                   \
      std::cout << "[SUCCESS] " << testName << "\n";                           \
    }                                                                          \
  } while (0)

int main() {
  es::EmployeeSystem system;
  es::Employee* emp = nullptr;
  es::PersonnelData* pData = nullptr;
  es::AttendanceData* aData = nullptr;

  system.addEmployee(&emp, &pData, &aData);

  pData->setEmployeeHourlyWage(60);

  // Do NOT add any attendance records (Empty report)

  tu::TimePoint startOfMonth{2026, 4, 1, 0, 0};
  double totalPay = emp->calculatePay(startOfMonth);

  ASSERT_APPROX_EQ(0.0, totalPay, "Empty report returns 0 pay");

  return hasErrors ? 1 : 0;
}