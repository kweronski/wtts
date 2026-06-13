#include <iostream>
#include <cmath>
#include <wtts/employeeSystem.hpp>
#include <wtts/employeeData.hpp>

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

// Helper to add a work record for a specific day and calculate pay
double getPayForDay(unsigned int year, unsigned int month, unsigned int day) {
  es::EmployeeSystem system;
  es::Employee* emp = nullptr;
  es::PersonnelData* pData = nullptr;
  es::AttendanceData* aData = nullptr;

  system.addEmployee(&emp, &pData, &aData);
  pData->setEmployeeHourlyWage(60); // 1 unit per minute

  tu::TimePeriod tp{{year, month, day, 8, 0}, {year, month, day, 9, 0}, tu::AttendanceType::Work};
  aData->addTimePeriod(tp);

  tu::TimePoint startOfMonth{year, month, 1, 0, 0};
  return emp->calculatePay(startOfMonth);
}

int main() {

  // Test 28 days (February 2026 - not a leap year)
  ASSERT_APPROX_EQ(60.0, getPayForDay(2026, 2, 28), "Process Feb 28th");

  // Test 30 days (April 2026)
  ASSERT_APPROX_EQ(60.0, getPayForDay(2026, 4, 30), "Process April 30th");

  // Test 31 days (May 2026)
  ASSERT_APPROX_EQ(60.0, getPayForDay(2026, 5, 31), "Process May 31st");

  return hasErrors ? 1 : 0;
}