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

std::string to_string(es::EmployeeRole const code) {
  std::string value;

  switch (code) {
  case es::EmployeeRole::Unknown:
    value = "Unknown";
    break;
  case es::EmployeeRole::Manager:
    value = "Manager";
    break;
  case es::EmployeeRole::Driver:
    value = "Driver";
    break;
  case es::EmployeeRole::Cleaner:
    value = "Cleaner";
    break;
  case es::EmployeeRole::Chef:
    value = "Chef";
    break;
  case es::EmployeeRole::Waiter:
    value = "Waiter";
    break;
  case es::EmployeeRole::Admin:
    value = "Admin";
    break;
  }

  return value;
}
} // namespace dp
