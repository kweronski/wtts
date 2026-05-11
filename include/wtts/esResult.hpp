#pragma once

#include <string>

namespace es {
enum class Result {
  Success,
  EmployeeIdNotUniqueError,
  EmployeeNotFoundError,
  EmployeeIsNullptrError,
  EmployeeAlreadyCheckedIn,
  EmployeeNotCheckedIn,
  DeliveryAlreadyCheckedIn,
  DeliveryNotCheckedIn,
  AttendanceExtractionError
};

std::string to_string(Result const);
} // namespace es
