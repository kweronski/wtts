#pragma once

#include <list>
#include <wtts/esResult.hpp>
#include <wtts/logInfo.hpp>

namespace es {
enum class EmployeeRole { Unknown, Employee, Manager, Driver, Admin };

std::string to_string(EmployeeRole const);

class AttendanceData {
public:
  tu::TimePeriod const *getCurrentTimePeriod() const { return &current_; }
  tu::TimePeriod *getCurrentTimePeriod() { return &current_; }

  // Verify the validity of current_ and add it to attendance_
  Result addTimePeriod();

  // Low level variant; Add time period with no checks to attendance_
  Result addTimePeriod(tu::TimePeriod);

  // Look through attendance_ and calculate hours worked since a specific time
  Result getHoursWorkedSince(tu::TimePoint const &) const;

  const std::list<tu::TimePeriod> &getRecords() const { return attendance_; }

private:
  tu::TimePeriod current_{};
  std::list<tu::TimePeriod> attendance_;
};

class PersonnelData {
public:
  bool getEmployeeActive() const;
  std::string getEmployeeName() const;
  std::string getEmployeeSurname() const;
  std::string getEmployeeTelephone() const;
  std::string getEmployeeEmail() const;

  unsigned getEmployeeStandardWorkTime() const;
  unsigned getEmployeeMaxWorkTime() const;
  unsigned getEmployeeHourlyWage() const;
  EmployeeRole getEmployeeRole() const;
  std::string getEmployeeCardId() const;
  std::string getEmployeeId() const;

  void setEmployeeActive(bool);
  void setEmployeeName(std::string const &);
  void setEmployeeSurname(std::string const &);
  void setEmployeeTelephone(std::string const &);
  void setEmployeeEmail(std::string const &);

  void setEmployeeStandardWorkTime(unsigned);
  void setEmployeeMaxWorkTime(unsigned);
  void setEmployeeHourlyWage(unsigned);
  void setEmployeeRole(EmployeeRole);
  void setEmployeeCardId(std::string const &);
  void setEmployeeId(std::string const &);

private:
  bool active_{};
  std::string name_, surname_, telephone_, email_;
  unsigned workTime_{}, maxWorkTime_{}, hourlyWage_{};
  EmployeeRole role_;
  std::string cardId_, id_;
};
} // namespace es
