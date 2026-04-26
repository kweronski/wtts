#pragma once

#include <list>
#include <string>
#include <unordered_map>
#include <wtts/dataParser.hpp>
#include <wtts/logInfo.hpp>

namespace dp {
template <typename T> struct Employee {
  EmployeeStatus status;
  std::string name, surname, telephone, email;
  EmployeeRole role;
  std::string cardId;
  T id;
  unsigned standardWorkTime, maxWorkTime;
  unsigned hourlyWage{};
  std::list<tu::TimePeriod> attendance;
};

struct DataStorage {
  using ID = std::string;
  std::unordered_map<ID, Employee<ID> *> employeeMap;
  std::list<Employee<ID>> employees;
};
} // namespace dp
