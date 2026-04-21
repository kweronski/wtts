#include <wtts/dataParser.hpp>

namespace dp {
std::string to_string(Result const code) {
  std::string value;

  switch (code) {
  case Result::Success:
    value = "Success";
    break;
  case Result::CouldNotOpenFileError:
    value = "CouldNotOpenFileError";
    break;
  case Result::StringToNumberConversionError:
    value = "StringToNumberConversionError";
    break;
  case Result::MissingPersonalInfoError:
    value = "MissingPersonalInfoError";
    break;
  case Result::MissingEmployeeInfoError:
    value = "MissingEmployeeInfoError";
    break;
  case Result::MissingEmployeeActiveStatusError:
    value = "MissingEmployeeActiveStatusError";
    break;
  case Result::MissingEmployeeNameError:
    value = "MissingEmployeeNameError";
    break;
  case Result::MissingEmployeeSurnameError:
    value = "MissingEmployeeSurnameError";
    break;
  case Result::MissingEmployeeEmailError:
    value = "MissingEmployeeEmailError";
    break;
  case Result::MissingEmployeeTelephoneError:
    value = "MissingEmployeeTelephoneError";
    break;
  case Result::MissingEmployeeStandardWorkTimeError:
    value = "MissingEmployeeStandardWorkTimeError";
    break;
  case Result::MissingEmployeeMaxWorkTimeError:
    value = "MissingEmployeeMaxWorkTimeError";
    break;
  case Result::MissingEmployeeHourlyWageError:
    value = "MissingEmployeeHourlyWageError";
    break;
  case Result::MissingEmployeeRoleError:
    value = "MissingEmployeeRoleError";
    break;
  case Result::MissingEmployeeCardIdError:
    value = "MissingEmployeeCardIdError";
    break;
  case Result::MissingEmployeeIdError:
    value = "MissingEmployeeIdError";
    break;
  case Result::UnknownEmployeeRoleError:
    value = "UnknownEmployeeRoleError";
    break;
  case Result::MissingAttendanceBeginYearError:
    value = "MissingAttendanceBeginYearError";
    break;
  case Result::MissingAttendanceBeginMonthError:
    value = "MissingAttendanceBeginMonthError";
    break;
  case Result::MissingAttendanceBeginDayError:
    value = "MissingAttendanceBeginDayError";
    break;
  case Result::MissingAttendanceBeginHourError:
    value = "MissingAttendanceBeginHourError";
    break;
  case Result::MissingAttendanceBeginMinuteError:
    value = "MissingAttendanceBeginMinuteError";
    break;
  case Result::MissingAttendanceEndYearError:
    value = "MissingAttendanceEndYearError";
    break;
  case Result::MissingAttendanceEndMonthError:
    value = "MissingAttendanceEndMonthError";
    break;
  case Result::MissingAttendanceEndDayError:
    value = "MissingAttendanceEndDayError";
    break;
  case Result::MissingAttendanceEndHourError:
    value = "MissingAttendanceEndHourError";
    break;
  case Result::MissingAttendanceEndMinuteError:
    value = "MissingAttendanceEndMinuteError";
    break;
  case Result::MissingAttendanceTypeError:
    value = "MissingAttendanceTypeError";
    break;
  case Result::UnknownAttendanceTypeError:
    value = "UnknownAttendanceTypeError";
    break;
  case Result::EmployeeIdNotUniqueError:
    value = "EmployeeIdNotUniqueError";
    break;
  }

  return value;
}
} // namespace dp
