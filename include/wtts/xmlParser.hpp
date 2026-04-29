#pragma once
#include <memory>
#include <wtts/dataParser.hpp>
#include <wtts/logInfo.hpp>

namespace pugi {
class xml_document;
}

namespace dp {
struct DataStorage;

class XMLDataParser : public DataParser {
public:
  XMLDataParser(std::string const &url);
  Result loadData() override;
  std::vector<ID> getEmployeeIdentifiers() const override;
  std::vector<tu::TimePeriod *>
  getEmployeeAttendance(ID const &id) const override;

  // Personal info
  EmployeeStatus getEmployeeStatus(ID const &id) const override;
  std::string getEmployeeName(ID const &id) const override;
  std::string getEmployeeSurname(ID const &id) const override;
  std::string getEmployeeTelephone(ID const &id) const override;
  std::string getEmployeeEmail(ID const &id) const override;

  // Employee info
  unsigned getEmployeeStandardWorkTime(ID const &id) const override;
  unsigned getEmployeeMaxWorkTime(ID const &id) const override;
  unsigned getEmployeeHourlyWage(ID const &id) const override;
  es::EmployeeRole getEmployeeRole(ID const &id) const override;
  std::string getEmployeeCardId(ID const &id) const override;

private:
  std::unique_ptr<pugi::xml_document, void (*)(pugi::xml_document *)> document_;
  std::unique_ptr<DataStorage, void (*)(DataStorage *)> storage_;
};
} // namespace dp
