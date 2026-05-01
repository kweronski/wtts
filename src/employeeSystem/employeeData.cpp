#include <wtts/employeeData.hpp>

namespace es {
Result AttendanceData::addTimePeriod() { return Result::Success; }

Result AttendanceData::addTimePeriod(tu::TimePeriod p) {
  attendance_.push_back(std::move(p));
  return Result::Success;
}

Result AttendanceData::getHoursWorkedSince(tu::TimePoint const &) const {
  return Result::Success;
}

bool PersonnelData::getEmployeeActive() const { return active_; }

std::string PersonnelData::getEmployeeName() const { return name_; }

std::string PersonnelData::getEmployeeSurname() const { return surname_; }

std::string PersonnelData::getEmployeeTelephone() const { return telephone_; }

std::string PersonnelData::getEmployeeEmail() const { return email_; }

unsigned PersonnelData::getEmployeeStandardWorkTime() const {
  return workTime_;
}

unsigned PersonnelData::getEmployeeMaxWorkTime() const { return maxWorkTime_; }

unsigned PersonnelData::getEmployeeHourlyWage() const { return hourlyWage_; }

EmployeeRole PersonnelData::getEmployeeRole() const { return role_; }

std::string PersonnelData::getEmployeeCardId() const { return cardId_; }

std::string PersonnelData::getEmployeeId() const { return id_; }

void PersonnelData::setEmployeeActive(bool v) { active_ = v; }

void PersonnelData::setEmployeeName(std::string const &v) { name_ = v; }

void PersonnelData::setEmployeeSurname(std::string const &v) { surname_ = v; }

void PersonnelData::setEmployeeTelephone(std::string const &v) {
  telephone_ = v;
}

void PersonnelData::setEmployeeEmail(std::string const &v) { email_ = v; }

void PersonnelData::setEmployeeStandardWorkTime(unsigned v) { workTime_ = v; }

void PersonnelData::setEmployeeMaxWorkTime(unsigned v) { maxWorkTime_ = v; }

void PersonnelData::setEmployeeHourlyWage(unsigned v) { hourlyWage_ = v; }

void PersonnelData::setEmployeeRole(EmployeeRole v) { role_ = v; }

void PersonnelData::setEmployeeCardId(std::string const &v) { cardId_ = v; }

void PersonnelData::setEmployeeId(std::string const &v) { id_ = v; }
} // namespace es
