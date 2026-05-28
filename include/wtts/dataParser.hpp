#pragma once
#include <string>
#include <vector>
#include <wtts/employeeData.hpp>

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

  virtual ~DataParser() = default;

protected:
  std::string url_;
};

struct Employee;

class DataWriter {
public:
  DataWriter(std::string const &url) : url_{url} {}
  virtual Result writeData() const = 0;
  using ID = Employee *;
  virtual ID addEmployee() = 0;
  virtual void addEmployeeAttendance(ID const, tu::TimePeriod const) = 0;

  virtual void setEmployeeStatus(ID const id, EmployeeStatus const) = 0;
  virtual void setEmployeeName(ID const id, std::string const &) = 0;
  virtual void setEmployeeSurname(ID const id, std::string const &) = 0;
  virtual void setEmployeeTelephone(ID const id, std::string const &) = 0;
  virtual void setEmployeeEmail(ID const id, std::string const &) = 0;
  virtual void setEmployeeId(ID const id, std::string const &) = 0;

  virtual void setEmployeeStandardWorkTime(ID const id, unsigned const) = 0;
  virtual void setEmployeeMaxWorkTime(ID const id, unsigned const) = 0;
  virtual void setEmployeeHourlyWage(ID const id, unsigned const) = 0;
  virtual void setEmployeeRole(ID const id, es::EmployeeRole const) = 0;
  virtual void setEmployeeCardId(ID const id, std::string const &) = 0;

  virtual ~DataWriter() = default;

protected:
  std::string url_;
};
} // namespace dp
