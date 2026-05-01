#pragma once

#include <wtts/employeeData.hpp>
#include <wtts/logInfo.hpp>

namespace es {
class Employee {
public:
  Employee(PersonnelData *p, AttendanceData *a)
      : personnel_{p}, attendance_{a} {}
  Result checkIn();
  Result checkOut();
  double calculatePay(tu::TimePoint const &start) const;

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

class GeneralAdmin {
public:
  void updateAbsence(Employee *const);
  void updatePayment(Employee *const);
  void editEmployee(Employee *const);
  void registerEmployee(Employee **);
  void activateemployee(Employee *const);
  void deactivateemployee(Employee *const);
};

class Manager : public Employee, public GeneralAdmin {
public:
  Manager(PersonnelData *p, AttendanceData *a) : Employee(p, a) {}
};

class Admin : public Employee, public GeneralAdmin {
public:
  Admin(PersonnelData *p, AttendanceData *a) : Employee(p, a) {}
  void editSettings();
  void removeEmployee(Employee *const);
};
} // namespace es
