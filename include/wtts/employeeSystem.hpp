#pragma once

#include <memory>
#include <string>
#include <vector>
#include <wtts/employee.hpp>

namespace es {
enum class Result { Success, EmployeeIdNotUniqueError, EmployeeNotFoundError };

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
  std::vector<std::unique_ptr<Employee>> employees;
};

class EmployeeSystemFactory {
public:
  static std::unique_ptr<EmployeeSystem>
  createFromXML(std::string const &filePath);
};
} // namespace es
