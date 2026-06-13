#include <wtts/esResult.hpp>

namespace es {
std::string to_string(Result const r) {
  std::string out;
  switch (r) {
  case Result::Success:
    out = "Success";
    break;
  case Result::SystemWriteError:
    out = "SystemWriteError";
    break;
  case Result::EmployeeIdNotUniqueError:
    out = "EmployeeIdNotUniqueError";
    break;
  case Result::EmployeeNotFoundError:
    out = "EmployeeNotFoundError";
    break;
  case Result::EmployeeIsNullptrError:
    out = "EmployeeIsNullptrError";
    break;
  case Result::EmployeeAlreadyCheckedIn:
    out = "EmployeeAlreadyCheckedIn";
    break;
  case Result::EmployeeNotCheckedIn:
    out = "EmployeeNotCheckedIn";
    break;
  case Result::DeliveryAlreadyCheckedIn:
    out = "DeliveryAlreadyCheckedIn";
    break;
  case Result::DeliveryNotCheckedIn:
    out = "DeliveryNotCheckedIn";
    break;
  case Result::AttendanceExtractionError:
    out = "AttendanceExtractionError";
    break;
  case Result::EmployeeIsCheckedInError:
    out = "EmployeeIsCheckedInError";
    break;
  case Result::UnknownEmployeeRoleError:
    out = "UnknownEmployeeRoleError";
    break;
  case Result::InvalidDate:
    out = "InvalidDate";
    break;
  }
  return out;
}
} // namespace es
