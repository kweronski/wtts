#pragma once

#include <functional>
#include <memory>
#include <set>
#include <string>
#include <unordered_map>
#include <vector>
#include <wtts/employee.hpp>

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
  Result autoCheckOut();

  Employee *getEmployeeById(std::string const &id) const;

  std::vector<Employee *> getEmployeeByName(std::string const &name,
                                            std::string const &surname) const;

  void printData(Employee *const = 0, bool skipAttendance = false) const;

	using Predicate =
		std::function<bool(Employee const*, PersonnelData const*, AttendanceData const*)>;

	std::vector<Employee*> getEmployeeBy(Predicate func);

private:
  std::set<std::unique_ptr<Employee>> employees_;
  std::set<std::unique_ptr<Driver>> drivers_;
  std::set<std::unique_ptr<Manager>> managers_;
  std::set<std::unique_ptr<Admin>> admins_;

  std::unordered_map<Employee *, std::unique_ptr<AttendanceData>> attendance_;
  std::unordered_map<Employee *, std::unique_ptr<PersonnelData>> personnel_;
};
} // namespace es
