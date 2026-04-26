#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <wtts/employee.hpp>

namespace dp {
class DataParser;
}

namespace es {
class EmployeeSystem {
public:
  EmployeeSystem() = default;

  Result addEmployee(std::unique_ptr<Employee>);
  Result removeEmployee(Employee const *);
  Result activateEmployee(Employee *);
  Result deactivateEmployee(Employee *);

  template <typename T> T *getEmployeeById(std::string const &id);

  template <typename T>
  std::vector<T *> getEmployeeByName(std::string const &name,
                                     std::string const &surname);

private:
  std::vector<std::unique_ptr<Employee>> employees_;
  std::vector<std::unique_ptr<Employee>> drivers_;
  std::vector<std::unique_ptr<Employee>> managers_;
  std::vector<std::unique_ptr<Employee>> admins_;

  using Attendance = int;
  std::unordered_map<Employee *, Attendance> attendance_;
};
} // namespace es
