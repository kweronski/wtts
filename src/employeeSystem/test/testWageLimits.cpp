#include <iostream>
#include <wtts/employeeData.hpp>

int main() {
  es::PersonnelData pData;

  // Happy Flow
  double standardWage = 50.0;
  pData.setEmployeeHourlyWage(standardWage);
  if (pData.getEmployeeHourlyWage() != standardWage) {
    std::cerr << "[ERROR] Failed to set standard hourly wage\n";
    return 1;
  }

  pData.setEmployeeHourlyWage(0.0);
  if (pData.getEmployeeHourlyWage() != 0.0) {
    std::cerr << "[ERROR] Failed to set zero hourly wage\n";
    return 1;
  }

  double highWage = 999999.99;
  pData.setEmployeeHourlyWage(highWage);
  if (pData.getEmployeeHourlyWage() != highWage) {
    std::cerr << "[ERROR] Failed to set very high hourly wage\n";
    return 1;
  }

  // 4. Test Negative/Overflow Wage
  // Assuming the system should clamp to 0 or a positive limit if negative is provided
  pData.setEmployeeHourlyWage(-100.0);
  if (pData.getEmployeeHourlyWage() < 0.0) {
    std::cerr << "[ERROR] System accepted negative wage: "
              << pData.getEmployeeHourlyWage() << "\n";
    return 1;
  }

  std::cout << "[SUCCESS] Wage limits and boundary tests passed\n";
  return 0;
}