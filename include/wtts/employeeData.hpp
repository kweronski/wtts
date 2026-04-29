#pragma once

#include <list>
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
  tu::TimePeriod *getCurrentTimePeriod() { return &current_; }

  // Verify the validity of current_ and add it to attendance_
  Result addTimePeriod();

  // Look through attendance_ and calculate hours worked since a specific time
  Result getHoursWorkedSince(tu::TimePoint const &) const;

private:
  tu::TimePeriod current_;
  std::list<tu::TimePeriod> attendance_;
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

private:
  bool active_{};
  std::string name_, surname_, telephone_, email_;
  unsigned workTime_{}, maxWorkTime_{}, hourlyWage_{};
  EmployeeRole role_;
  std::string cardId_;
};
} // namespace es
