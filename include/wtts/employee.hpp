#pragma once

#include <wtts/employeeData.hpp>
#include <wtts/logInfo.hpp>

namespace es {
class Employee {
public:
  Result checkIn();
  Result checkOut();
  double calculatePay(tu::TimePoint const &start) const;

protected:
  AttendanceData *attendance_;
  PersonnelData *personnel_;
};

class Driver : public Employee {
public:
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
};

class Admin : public Employee, public GeneralAdmin {
public:
  void editSettings();
  void removeEmployee(Employee *const);
};
} // namespace es
