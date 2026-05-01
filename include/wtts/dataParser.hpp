#pragma once
#include <string>
#include <vector>
#include <wtts/employeeData.hpp>
#include <wtts/logInfo.hpp>

namespace dp {
enum class Result {
  Success,
  CouldNotOpenFileError,
  StringToNumberConversionError,
  MissingPersonalInfoError,
  MissingEmployeeInfoError,
  MissingEmployeeActiveStatusError,
  MissingEmployeeNameError,
  MissingEmployeeSurnameError,
  MissingEmployeeEmailError,
  MissingEmployeeTelephoneError,
  MissingEmployeeStandardWorkTimeError,
  MissingEmployeeMaxWorkTimeError,
  MissingEmployeeHourlyWageError,
  MissingEmployeeRoleError,
  MissingEmployeeCardIdError,
  MissingEmployeeIdError,
  UnknownEmployeeRoleError,
  MissingAttendanceBeginYearError,
  MissingAttendanceBeginMonthError,
  MissingAttendanceBeginDayError,
  MissingAttendanceBeginHourError,
  MissingAttendanceBeginMinuteError,
  MissingAttendanceEndYearError,
  MissingAttendanceEndMonthError,
  MissingAttendanceEndDayError,
  MissingAttendanceEndHourError,
  MissingAttendanceEndMinuteError,
  MissingAttendanceTypeError,
  UnknownAttendanceTypeError,
  EmployeeIdNotUniqueError
};

std::string to_string(Result const);

enum class EmployeeStatus { Active, Inactive };

std::string to_string(EmployeeStatus const);

class DataParser {
public:
  DataParser(std::string const &url) : url_{url} {}
  virtual Result loadData() = 0;
  using ID = std::string;
  virtual std::vector<ID> getEmployeeIdentifiers() const = 0;
  virtual std::vector<tu::TimePeriod *>
  getEmployeeAttendance(ID const &id) const = 0;

  // Personal info
  virtual EmployeeStatus getEmployeeStatus(ID const &id) const = 0;
  virtual std::string getEmployeeName(ID const &id) const = 0;
  virtual std::string getEmployeeSurname(ID const &id) const = 0;
  virtual std::string getEmployeeTelephone(ID const &id) const = 0;
  virtual std::string getEmployeeEmail(ID const &id) const = 0;

  // Employee info
  virtual unsigned getEmployeeStandardWorkTime(ID const &id) const = 0;
  virtual unsigned getEmployeeMaxWorkTime(ID const &id) const = 0;
  virtual unsigned getEmployeeHourlyWage(ID const &id) const = 0;
  virtual es::EmployeeRole getEmployeeRole(ID const &id) const = 0;
  virtual std::string getEmployeeCardId(ID const &id) const = 0;

  // Attendance info
  virtual ~DataParser() = default;

protected:
  std::string url_;
};
} // namespace dp
