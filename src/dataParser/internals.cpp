#include "internals.hpp"

namespace dp {
std::string to_string(EmployeeStatus const code) {
  std::string value;

  switch (code) {
  case EmployeeStatus::Active:
    value = "Active";
    break;
  case EmployeeStatus::Inactive:
    value = "Inactive";
    break;
  }

  return value;
}

std::string to_string(EmployeeRole const code) {
  std::string value;

  switch (code) {
  case EmployeeRole::Unknown:
    value = "Unknown";
    break;
  case EmployeeRole::Manager:
    value = "Manager";
    break;
  case EmployeeRole::Driver:
    value = "Driver";
    break;
  case EmployeeRole::Cleaner:
    value = "Cleaner";
    break;
  case EmployeeRole::Chef:
    value = "Chef";
    break;
  case EmployeeRole::Waiter:
    value = "Waiter";
    break;
  case EmployeeRole::Admin:
    value = "Admin";
    break;
  }

  return value;
}

std::string to_string(AttendanceType const code) {
  std::string value;

  switch (code) {
  case AttendanceType::Vacation:
    value = "Vacation";
    break;
  case AttendanceType::Work:
    value = "Work";
    break;
  case AttendanceType::Delivery:
    value = "Delivery";
    break;
  case AttendanceType::Sick:
    value = "Sick";
    break;
  }

  return value;
}

std::string makeAttendanceInstStr(TimePeriod const *p) {
  return to_string(p->type) + " (" + std::to_string(p->begin.year) + "/" +
         std::to_string(p->begin.month) + "/" + std::to_string(p->begin.day) +
         ", " + std::to_string(p->begin.hour) + ":" +
         std::to_string(p->begin.minute) + ") - " + "(" +
         std::to_string(p->end.year) + "/" + std::to_string(p->end.month) +
         "/" + std::to_string(p->end.day) + ", " + std::to_string(p->end.hour) +
         ":" + std::to_string(p->end.minute) + ")";
}
} // namespace dp
