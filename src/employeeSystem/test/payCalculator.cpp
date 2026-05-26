#include <iostream>
#include <string>
#include <wtts/employeeSystem.hpp>

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Error: expected 2 arguments; Usage: " << argv[0] << " ";
    std::cerr << "<number of hours worked> <hourly wage>" << std::endl;
    return 1;
  }

  std::size_t hoursWorked{}, hourlyWage{};

  try {
    hoursWorked = std::stoul(argv[1]);
  } catch (...) {
    std::cerr << "Failed to convert: " << argv[1] << " to hours worked\n";
    return 1;
  }

  try {
    hourlyWage = std::stoul(argv[2]);
  } catch (...) {
    std::cerr << "Failed to convert: " << argv[2] << " to hourly wage\n";
    return 1;
  }

  std::size_t const expectedPay = hourlyWage * hoursWorked;
  (void)expectedPay;

  /* PART 1:
   * Create an instance of the EmployeeSystem class
   *
   * Add an employee to the system
   *
   * Configure employee info (ID, name, surname, hourly wage, attendance data)
   * Note: when adding time periods to attendance data, make sure to add
   * time periods that are not payable.
   *
   * (suggestion): convert hours to days, for instance: attendance data records
   * = hours / 8 + (hours % 8 ? 1 : 0); use AttendanceData::addTimePeriod(...)
   *
   * Calculate employee pay using Employee::calculatePay()
   *
   * Compare calculated pay to expectedPay;
   * If not equal return 1;
   */
}
