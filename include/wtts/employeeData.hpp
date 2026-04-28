#pragma once

#include <wtts/esResult.hpp>
#include <wtts/logInfo.hpp>

namespace es {
enum class EmployeeRole {
  Unknown,
  Manager,
  Driver,
  Cleaner,
  Waiter,
  Chef,
  Admin
};

std::string to_string(EmployeeRole const);

class AttendanceData {
public:
  Result addTimePeriod(tu::TimePeriod);
};

class PersonnelData {
public:
  using ID = std::string;
  bool getEmployeeActive(ID const &id);
  std::string getEmployeeName(ID const &id);
  std::string getEmployeeSurname(ID const &id);
  std::string getEmployeeTelephone(ID const &id);
  std::string getEmployeeEmail(ID const &id);

  unsigned getEmployeeStandardWorkTime(ID const &id);
  unsigned getEmployeeMaxWorkTime(ID const &id);
  unsigned getEmployeeHourlyWage(ID const &id);
  EmployeeRole getEmployeeRole(ID const &id);
  std::string getEmployeeCardId(ID const &id);
};
} // namespace es
