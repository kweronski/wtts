#include <iostream>
#include <cmath>
#include <wtts/employeeSystem.hpp>
#include <wtts/employeeData.hpp>
#include <wtts/logInfo.hpp>

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
  // 1. Create employee system
  es::EmployeeSystem system;

  // 2. Add employee to system
  es::Employee* emp = nullptr;
  es::PersonnelData* pData = nullptr;
  es::AttendanceData* aData = nullptr;

  // The wtts factory method correctly initializes and binds the pointers
  if (system.addEmployee(&emp, &pData, &aData) != es::Result::Success) {
      std::cerr << "Failed to add employee to system\n";
      return 1;
  }

  // 3. Configure employee info (PersonnelData)
  pData->setEmployeeHourlyWage(60); // 1 unit per minute

  // 4. Configure employee attendance (AttendanceData)
  auto addRecord = [&](unsigned day, unsigned h1, unsigned m1, unsigned h2,
                       unsigned m2, tu::AttendanceType type) {
    tu::TimePeriod tp{{2026, 4, day, h1, m1}, {2026, 4, day, h2, m2}, type};
    aData->addTimePeriod(tp);
  };

  // -> add work records corresponding to specified number of hours.
  addRecord(1, 8, 0, 10, 0, tu::AttendanceType::Work);   // 120 minutes = 120
  addRecord(2, 9, 0, 10, 15, tu::AttendanceType::Work);  // 75 minutes = 75

  // -> add records of different type to check correctness of calculations.
  addRecord(3, 8, 0, 16, 0, tu::AttendanceType::Sick);   // Sick leave: ignored = 0

  // 5. Calculate pay using employee API. (*)
  tu::TimePoint startOfMonth{2026, 4, 1, 0, 0};
  double totalPay = emp->calculatePay(startOfMonth);

  // 6. Verify equality of expected pay and calculated pay;
  ASSERT_APPROX_EQ(195.0, totalPay, "Calculating pay (Work + Other record type)");

  // 7. If the values are equal, return 0; Otherwise return 1;
  return hasErrors ? 1 : 0;
}