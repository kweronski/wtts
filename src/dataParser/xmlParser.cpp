#include <pugixml.hpp>
#include <wtts/xmlParser.hpp>

#include "internals.hpp"

namespace dp {
XMLDataParser::XMLDataParser()
    : document_{new pugi::xml_document{}, [](auto p) { delete p; }},
      storage_{new DataStorage{}, [](auto p) { delete p; }} {}

Result XMLDataParser::loadData(std::string const &url) {
  document_->load_file(url.c_str());
  auto root = document_->first_child();

  if (!root) // The data file is empty. Return early.
    return Result::Success;

  for (auto node = root.child("employee"); node; node = node.next_sibling()) {
    Employee<DataParser::ID> data{};

    std::string active;
    if (auto activeVal = node.attribute("active").value(); !activeVal)
      return Result::MissingEmployeeActiveStatusError;
    else
      active = activeVal;

    if (active == "true")
      data.status = EmployeeStatus::Active;
    else
      data.status = EmployeeStatus::Inactive;

    auto personal = node.child("personalInfo");
    if (!personal)
      return Result::MissingPersonalInfoError;

    auto name = personal.child("name");
    if (!name)
      return Result::MissingEmployeeNameError;
    data.name = name.attribute("value").value();

    auto surname = personal.child("surname");
    if (!surname)
      return Result::MissingEmployeeSurnameError;
    data.surname = surname.attribute("value").value();

    auto email = personal.child("email");
    if (!email)
      return Result::MissingEmployeeEmailError;
    data.email = email.attribute("value").value();

    auto telephone = personal.child("telephone");
    if (!telephone)
      return Result::MissingEmployeeTelephoneError;
    data.telephone = telephone.attribute("value").value();

    auto workRelated = node.child("employeeInfo");
    if (!workRelated)
      return Result::MissingEmployeeInfoError;

    auto standardWorkTime = workRelated.child("stdWorkTime");
    if (!standardWorkTime)
      return Result::MissingEmployeeStandardWorkTimeError;
    data.standardWorkTime = std::chrono::minutes(
        std::stoul(standardWorkTime.attribute("value").value()));

    auto maxWorkTime = workRelated.child("maxWorkTime");
    if (!maxWorkTime)
      return Result::MissingEmployeeMaxWorkTimeError;
    data.maxWorkTime = std::chrono::minutes(
        std::stoul(maxWorkTime.attribute("value").value()));

    auto hourlyWage = workRelated.child("hourlyWage");
    if (!hourlyWage)
      return Result::MissingEmployeeHourlyWageError;
    data.hourlyWage = std::stoul(hourlyWage.attribute("value").value());

    auto role = workRelated.child("role");
    if (!role)
      return Result::MissingEmployeeRoleError;
    if (auto roleVal = std::string{role.attribute("value").value()};
        roleVal == "manager")
      data.role = EmployeeRole::Manager;
    else if (roleVal == "driver")
      data.role = EmployeeRole::Driver;
    else if (roleVal == "cleaner")
      data.role = EmployeeRole::Cleaner;
    else if (roleVal == "chef")
      data.role = EmployeeRole::Chef;
    else if (roleVal == "waiter")
      data.role = EmployeeRole::Waiter;
    else if (roleVal == "admin")
      data.role = EmployeeRole::Admin;
    else
      return Result::UnknownEmployeeRoleError;

    auto cardId = workRelated.child("cardId");
    if (!cardId)
      return Result::MissingEmployeeCardIdError;
    data.cardId = cardId.attribute("value").value();

    auto id = workRelated.child("id");
    if (!id)
      return Result::MissingEmployeeIdError;
    data.id = id.attribute("value").value();

    if (auto attendance = node.child("attendance"); attendance) {
    }

    storage_->employees.push_back(std::move(data));
    auto back = &storage_->employees.back();
    storage_->employeeMap.emplace(back->id, back);
  }

  return Result::Success;
}

std::vector<XMLDataParser::ID> XMLDataParser::getEmployeeIdentifiers() {
  std::vector<XMLDataParser::ID> ids;
  for (auto const &e : storage_->employees)
    ids.push_back(e.id);
  return ids;
}

// Personal info
bool XMLDataParser::getEmployeeActiveStatus(ID const &id) {
  return storage_->employeeMap.at(id)->status == EmployeeStatus::Active;
}
std::string XMLDataParser::getEmployeeName(ID const &id) {
  return storage_->employeeMap.at(id)->name;
}
std::string XMLDataParser::getEmployeeSurname(ID const &id) {
  return storage_->employeeMap.at(id)->surname;
}
std::string XMLDataParser::getEmployeeTelephone(ID const &id) {
  return storage_->employeeMap.at(id)->telephone;
}
std::string XMLDataParser::getEmployeeEmail(ID const &id) {
  return storage_->employeeMap.at(id)->email;
}

// Employee info
unsigned XMLDataParser::getEmployeeStandardWorkTime(ID const &id) { return 0; }
unsigned XMLDataParser::getEmployeeMaxWorkTime(ID const &id) { return 0; }
unsigned XMLDataParser::getEmployeeHourlyWage(ID const &id) { return 0; }
unsigned XMLDataParser::getEmployeeRole(ID const &id) { return 0; }
std::string XMLDataParser::getEmployeeCardId(ID const &id) { return ""; }
}
