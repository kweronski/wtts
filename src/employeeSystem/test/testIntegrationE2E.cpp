#include <iostream>
#include <cmath>
#include <wtts/employeeSystem.hpp>
#include <wtts/employeeData.hpp>
#include <wtts/esResult.hpp>

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

#define ASSERT_SUCCESS(result, testName)                                       \
  do {                                                                         \
    if ((result) != es::Result::Success) {                                     \
      std::cerr << "[ERROR] " << testName << " | Operation failed!\n";         \
      hasErrors = true;                                                        \
    }                                                                          \
  } while (0)

int main() {
  std::cout << "--- Starting End-to-End (E2E) Integration Test ---\n";
  es::EmployeeSystem system;

  // ==========================================
  // Employee 1: Standard hours
  // ==========================================
  es::Employee* emp1 = nullptr;
  es::PersonnelData* pData1 = nullptr;
  es::AttendanceData* aData1 = nullptr;

  system.addEmployee(&emp1, &pData1, &aData1);
  pData1->setEmployeeHourlyWage(100u); // 100 units per hour

  // Day 1: 8 hours (8:00 - 16:00) -> 800
  ASSERT_SUCCESS(aData1->addTimePeriod({{2026u, 6u, 1u, 8u, 0u}, {2026u, 6u, 1u, 16u, 0u}, tu::AttendanceType::Work}), "Emp1: Record Day 1");
  // Day 2: 8 hours (8:00 - 16:00) -> 800
  ASSERT_SUCCESS(aData1->addTimePeriod({{2026u, 6u, 2u, 8u, 0u}, {2026u, 6u, 2u, 16u, 0u}, tu::AttendanceType::Work}), "Emp1: Record Day 2");
  // Day 3: 4 hours (9:00 - 13:00) -> 400
  ASSERT_SUCCESS(aData1->addTimePeriod({{2026u, 6u, 3u, 9u, 0u}, {2026u, 6u, 3u, 13u, 0u}, tu::AttendanceType::Work}), "Emp1: Record Day 3");

  // ==========================================
  // Employee 2: Overtime / different wage
  // ==========================================
  es::Employee* emp2 = nullptr;
  es::PersonnelData* pData2 = nullptr;
  es::AttendanceData* aData2 = nullptr;

  system.addEmployee(&emp2, &pData2, &aData2);
  pData2->setEmployeeHourlyWage(200u); // 200 units per hour

  // Day 1: 10 hours (10:00 - 20:00) -> 2000
  ASSERT_SUCCESS(aData2->addTimePeriod({{2026u, 6u, 1u, 10u, 0u}, {2026u, 6u, 1u, 20u, 0u}, tu::AttendanceType::Work}), "Emp2: Record Day 1");
  // Day 2: 2 hours (18:00 - 20:00) -> 400
  ASSERT_SUCCESS(aData2->addTimePeriod({{2026u, 6u, 2u, 18u, 0u}, {2026u, 6u, 2u, 20u, 0u}, tu::AttendanceType::Work}), "Emp2: Record Day 2");

  // ==========================================
  // Verification of Payroll System (End of Month)
  // ==========================================
  std::cout << "\n--- Verifying Pay Calculations ---\n";
  tu::TimePoint startOfJune{2026u, 6u, 1u, 0u, 0u};

  // Emp1 expects: 800 + 800 + 400 = 2000
  double totalPay1 = emp1->calculatePay(startOfJune);
  ASSERT_APPROX_EQ(2000.0, totalPay1, "Emp1 Final Pay Calculation");

  // Emp2 expects: 2000 + 400 = 2400
  double totalPay2 = emp2->calculatePay(startOfJune);
  ASSERT_APPROX_EQ(2400.0, totalPay2, "Emp2 Final Pay Calculation");

  return hasErrors ? 1 : 0;
}