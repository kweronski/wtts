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
} // namespace dp
