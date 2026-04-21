#pragma once

#include <string>
#include <chrono>
#include <list>

namespace dp {
enum class EmployeeStatus { Active, Inactive };

enum class EmployeeRole {
  Unknown,
  Manager,
  Driver,
  Cleaner,
  Chef,
  Waiter,
  Admin
};

enum class TimePeriodType { Vacation, Work, Delivery, UnpaidLeave };

struct TimePoint {
  unsigned year, month, day, hour, minute;
};

struct TimePeriod {
  TimePoint start, end;
  TimePeriodType type;
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
