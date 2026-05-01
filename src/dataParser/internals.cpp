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
} // namespace dp
