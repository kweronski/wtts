#pragma once

#include <list>
#include <unordered_map>
#include <string>
#include <wtts/dataParser.hpp>

namespace dp {
enum class AttendanceType { Vacation, Work, Delivery, Sick };
std::string to_string(AttendanceType const);

struct TimePoint {
  unsigned year, month, day, hour, minute;
};

struct TimePeriod {
  TimePoint begin, end;
  AttendanceType type;
};

template <typename T> struct Employee {
  EmployeeStatus status;
  std::string name, surname, telephone, email;
  EmployeeRole role;
  std::string cardId;
  T id;
  unsigned standardWorkTime, maxWorkTime;
  unsigned hourlyWage{};
  std::list<TimePeriod> attendance;
};

struct DataStorage {
  using ID = std::string;
  std::unordered_map<ID, Employee<ID> *> employeeMap;
  std::list<Employee<ID>> employees;
};
} // namespace dp
