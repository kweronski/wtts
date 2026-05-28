#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include <wtts/dataParser.hpp>

namespace dp {
struct Employee {
  EmployeeStatus status;
  std::string name, surname, telephone, email;
  es::EmployeeRole role;
  std::string cardId;
  std::string id;
  unsigned standardWorkTime, maxWorkTime;
  unsigned hourlyWage{};
  std::list<tu::TimePeriod> attendance;
};

struct DataStorage {
  using ID = std::string;
  std::unordered_map<ID, Employee *> employeeMap;
  std::list<Employee> employees;
};
} // namespace dp
