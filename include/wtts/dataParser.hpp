#pragma once
#include <string>
#include <vector>
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

enum class EmployeeRole {
  Unknown,
  Manager,
  Driver,
  Cleaner,
  Waiter,
  Chef,
  Admin
};

std::string to_string(EmployeeStatus const);

std::string to_string(EmployeeRole const);

struct TimePeriod;
std::string makeAttendanceInstStr(TimePeriod const *);

class DataParser {
public:
  DataParser(std::string const &url) : url_{url} {}
  virtual Result loadData() = 0;
  using ID = std::string;
  virtual std::vector<ID> getEmployeeIdentifiers() = 0;
  virtual std::vector<tu::TimePeriod *> getEmployeeAttendance(ID const &id) = 0;

  // Personal info
  virtual EmployeeStatus getEmployeeStatus(ID const &id) = 0;
  virtual std::string getEmployeeName(ID const &id) = 0;
  virtual std::string getEmployeeSurname(ID const &id) = 0;
  virtual std::string getEmployeeTelephone(ID const &id) = 0;
  virtual std::string getEmployeeEmail(ID const &id) = 0;

  // Employee info
  virtual unsigned getEmployeeStandardWorkTime(ID const &id) = 0;
  virtual unsigned getEmployeeMaxWorkTime(ID const &id) = 0;
  virtual unsigned getEmployeeHourlyWage(ID const &id) = 0;
  virtual EmployeeRole getEmployeeRole(ID const &id) = 0;
  virtual std::string getEmployeeCardId(ID const &id) = 0;

  // Attendance info
  virtual ~DataParser() = default;

protected:
  std::string url_;
};
} // namespace dp
