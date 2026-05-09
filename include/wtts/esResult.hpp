#pragma once

namespace es {
enum class Result {
  Success,
  EmployeeIdNotUniqueError,
  EmployeeNotFoundError,
  EmployeeIsNullptrError,
  EmployeeAlreadyCheckedIn,
  EmployeeNotCheckedIn,
  DeliveryAlreadyCheckedIn,
  DeliveryNotCheckedIn
};
}
