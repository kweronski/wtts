#pragma once

#include <memory>
#include <wtts/dataParser.hpp>

namespace dp {
struct DataStorage;

class XMLWriter : public DataWriter {
public:
  XMLWriter(std::string const &url);
  Result writeData() const override;
  using ID = Employee *;
  ID addEmployee() override;
  void addEmployeeAttendance(ID const, tu::TimePeriod const) override;

  void setEmployeeStatus(ID const id, EmployeeStatus const) override;
  void setEmployeeName(ID const id, std::string const &) override;
  void setEmployeeSurname(ID const id, std::string const &) override;
  void setEmployeeTelephone(ID const id, std::string const &) override;
  void setEmployeeEmail(ID const id, std::string const &) override;
  void setEmployeeId(ID const id, std::string const &) override;

  void setEmployeeStandardWorkTime(ID const id, unsigned const) override;
  void setEmployeeMaxWorkTime(ID const id, unsigned const) override;
  void setEmployeeHourlyWage(ID const id, unsigned const) override;
  void setEmployeeRole(ID const id, es::EmployeeRole const) override;
  void setEmployeeCardId(ID const id, std::string const &) override;

protected:
  std::unique_ptr<DataStorage, void (*)(DataStorage *)> storage_;
};
} // namespace dp
