#include <vector>
#include <string>

namespace dp {
enum class Result {
  Success,
  CouldNotOpenFileError,
  MissingPersonalInfoError,
  MissingEmployeeInfoError,
  MissingEmployeeActiveStatusError,
  MissingEmployeeNameError,
  MissingEmployeeSurnameError,
  MissingEmployeeEmailError,
  MissingEmployeeTelephoneError,
  MissingEmployeeStandardWorkTimeError,
  MissingEmployeeMaxWorkTimeError,
  MissingEmployeeHourlyWageError,
  MissingEmployeeRoleError,
  MissingEmployeeCardIdError,
  MissingEmployeeIdError,
  UnknownEmployeeRoleError
};

class DataParser {
	public:
		virtual Result loadData(std::string const& url) = 0;
		using ID = std::string;
                virtual std::vector<ID> getEmployeeIdentifiers() = 0;

                // Personal info
                virtual bool getEmployeeActiveStatus(ID const &id) = 0;
                virtual std::string getEmployeeName(ID const &id) = 0;
                virtual std::string getEmployeeSurname(ID const &id) = 0;
                virtual std::string getEmployeeTelephone(ID const &id) = 0;
                virtual std::string getEmployeeEmail(ID const &id) = 0;

                // Employee info
                virtual unsigned getEmployeeStandardWorkTime(ID const &id) = 0;
                virtual unsigned getEmployeeMaxWorkTime(ID const &id) = 0;
                virtual unsigned getEmployeeHourlyWage(ID const &id) = 0;
                virtual unsigned getEmployeeRole(ID const &id) = 0;
                virtual std::string getEmployeeCardId(ID const &id) = 0;

                // Attendance info
		virtual ~DataParser() = default;
};
}
