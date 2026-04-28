#pragma once

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

  Result addEmployee(Employee **const);
  Result removeEmployee(Employee const *);
  Result autoCheckOut();

  template <typename T> T *getEmployeeById(std::string const &id);

  template <typename T>
  std::vector<T *> getEmployeeByName(std::string const &name,
                                     std::string const &surname);

private:
  std::set<std::unique_ptr<Employee>> employees_;
  std::set<std::unique_ptr<Driver>> drivers_;
  std::set<std::unique_ptr<Manager>> managers_;
  std::set<std::unique_ptr<Admin>> admins_;

  std::unordered_map<Employee *, AttendanceData> attendance_;
  std::unordered_map<Employee *, PersonnelData> personnel_;
};
} // namespace es
