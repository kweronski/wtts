#pragma once

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <wtts/employee.hpp>
#include <wtts/paymentList.hpp>

namespace es {
class EmployeeSystem {
public:
  EmployeeSystem() = default;

  Result addEmployee(Employee **const, PersonnelData **const,
                     AttendanceData **const);
  Result addEmployee(Driver **const, PersonnelData **const,
                     AttendanceData **const);
  Result addEmployee(Manager **const, PersonnelData **const,
                     AttendanceData **const);
  Result addEmployee(Admin **const, PersonnelData **const,
                     AttendanceData **const);

  void setAutoCheckoutTime(unsigned hour, unsigned minute) {
    autoCheckoutHour_ = hour;
    autoCheckoutMinute_ = minute;
  }

  std::pair<unsigned, unsigned> getAutoCheckoutTime() const {
    return {autoCheckoutHour_, autoCheckoutMinute_};
  }

  // Checks out all employees that are checked in past the auto checkout time
  std::vector<Employee *> autoCheckOut();

  Employee *getEmployeeById(std::string const &id) const;

  std::vector<Employee *> getEmployeeByName(std::string const &name,
                                            std::string const &surname) const;

  void printData(Employee *const = 0, bool skipAttendance = false) const;

  using Predicate = std::function<bool(Employee const *, PersonnelData const *,
                                       AttendanceData const *)>;

  std::vector<Employee *> getEmployeeBy(Predicate func);

  std::vector<PaymentRecord> createPaymentList(tu::TimePeriod const &tp,
                                               Predicate func);

  Result setEmployeeAbsence(Employee *, tu::TimePoint const &);

private:
  std::set<std::unique_ptr<Employee>> employees_;
  std::set<std::unique_ptr<Driver>> drivers_;
  std::set<std::unique_ptr<Manager>> managers_;
  std::set<std::unique_ptr<Admin>> admins_;

  std::unordered_map<Employee *, std::unique_ptr<AttendanceData>> attendance_;
  std::unordered_map<Employee *, std::unique_ptr<PersonnelData>> personnel_;

  unsigned autoCheckoutHour_, autoCheckoutMinute_;
};
} // namespace es
