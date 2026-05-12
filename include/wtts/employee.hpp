#pragma once

#include <wtts/employeeData.hpp>
#include <wtts/logInfo.hpp>

namespace es {
class Employee {
public:
  Employee(PersonnelData *p, AttendanceData *a)
      : personnel_{p}, attendance_{a} {}

  virtual ~Employee() = default;

  Result checkIn();
  Result checkOut();
  double calculatePay(tu::TimePoint const &start) const;

  double calculatePay(tu::TimePeriod const &period) const;

  bool getEmployeeActive() const { return personnel_->getEmployeeActive(); }
  std::string getEmployeeName() const { return personnel_->getEmployeeName(); }
  std::string getEmployeeSurname() const {
    return personnel_->getEmployeeSurname();
  }
  std::string getEmployeeTelephone() const {
    return personnel_->getEmployeeTelephone();
  }
  std::string getEmployeeEmail() const {
    return personnel_->getEmployeeEmail();
  }
  unsigned getEmployeeStandardWorkTime() const {
    return personnel_->getEmployeeStandardWorkTime();
  }
  unsigned getEmployeeMaxWorkTime() const {
    return personnel_->getEmployeeMaxWorkTime();
  }
  unsigned getEmployeeHourlyWage() const {
    return personnel_->getEmployeeHourlyWage();
  }
  EmployeeRole getEmployeeRole() const { return personnel_->getEmployeeRole(); }
  std::string getEmployeeCardId() const {
    return personnel_->getEmployeeCardId();
  }
  std::string getEmployeeId() const { return personnel_->getEmployeeId(); }

protected:
  PersonnelData *personnel_;
  AttendanceData *attendance_;
};

class Driver : public Employee {
public:
  Driver(PersonnelData *p, AttendanceData *a) : Employee(p, a) {}
  Result logDeliveryBegin();
  Result logDeliveryEnd();
};

class EmployeeSystem;

enum class SystemSettings { AutoCheckoutHour, AutoCheckoutMinute };

class GeneralAdmin {
public:
  GeneralAdmin(EmployeeSystem *sys) : system_{sys} {}
  Result setAbsence(std::string const &id, tu::TimePoint const &);
  Result addEmployee(PersonnelData const &);
  Result editEmployee(std::string const &id, PersonnelData const &newData);

protected:
  EmployeeSystem *system_;
};

class Manager : public Employee, public GeneralAdmin {
public:
  Manager(PersonnelData *p, AttendanceData *a, EmployeeSystem *s)
      : Employee(p, a), GeneralAdmin(s) {}
};

class Admin : public Employee, public GeneralAdmin {
public:
  Admin(PersonnelData *p, AttendanceData *a, EmployeeSystem *s)
      : Employee(p, a), GeneralAdmin(s) {}
  Result editSettings(SystemSettings setting, unsigned value);
  void removeEmployee(Employee *const);
};
} // namespace es
