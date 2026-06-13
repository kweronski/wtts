#include <iostream>
#include <wtts/employeeSystem.hpp>
#include <wtts/employeeData.hpp>
#include <wtts/esResult.hpp>

bool hasErrors = false;
#define ASSERT_EQUAL(expected, actual, testName)                               \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      std::cerr << "[ERROR] " << testName << " | Expected: "                   \
                << es::to_string(expected) << ", but got: "                    \
                << es::to_string(actual) << "\n";                              \
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

  if (system.addEmployee(&emp, &pData, &aData) != es::Result::Success) {
      std::cerr << "Failed to add employee\n";
      return 1;
  }

  // Test 1: Check in successfully
  ASSERT_EQUAL(es::Result::Success, emp->checkIn(), "Initial check-in");

  // Test 2: Try to check in again (should fail with AlreadyCheckedIn)
  ASSERT_EQUAL(es::Result::EmployeeAlreadyCheckedIn, emp->checkIn(), "Double check-in prevention");

  // Test 3: Check out successfully
  ASSERT_EQUAL(es::Result::Success, emp->checkOut(), "Check-out after work");

  // Test 4: Try to check out again when already checked out
  ASSERT_EQUAL(es::Result::EmployeeNotCheckedIn, emp->checkOut(), "Double check-out prevention");

  return hasErrors ? 1 : 0;
}