#include <iostream>
#include <wtts/employeeSystem.hpp>
#include <wtts/logInfo.hpp>

namespace es {
Result EmployeeSystem::removeEmployee(std::string const &id) {
  // First, find the employee and check they are not checked in
  Employee *found = getEmployeeById(id);
  if (!found)
    return Result::EmployeeNotFoundError;

  auto it = attendance_.find(found);
  if (it != attendance_.end() && it->second) {
    if (it->second->getCurrentTimePeriod()->begin.year) // checked in
      return Result::EmployeeIsCheckedInError;
  }

  if (tryRemoveEmployee(employees_, id))
    return Result::Success;
  if (tryRemoveEmployee(managers_, id))
    return Result::Success;
  if (tryRemoveEmployee(drivers_, id))
    return Result::Success;
  if (tryRemoveEmployee(admins_, id))
    return Result::Success;

  return Result::EmployeeNotFoundError;
}

Result EmployeeSystem::setEmployeeAbsence(Employee *e, tu::TimePoint const &t) {
  AttendanceData *a;

  try {
    a = attendance_.at(e).get();
  } catch (...) {
    return Result::AttendanceExtractionError;
  }

  return a->setAbsence(t);
}

std::vector<Employee *> EmployeeSystem::getEmployeeBy(Predicate func) {
  std::vector<Employee *> empl;

  for (auto &e : employees_)
    if (func(e.get(), personnel_.at(e.get()).get(),
             attendance_.at(e.get()).get()))
      empl.push_back(e.get());
  for (auto &e : drivers_)
    if (func(e.get(), personnel_.at(e.get()).get(),
             attendance_.at(e.get()).get()))
      empl.push_back(e.get());
  for (auto &e : managers_)
    if (func(e.get(), personnel_.at(e.get()).get(),
             attendance_.at(e.get()).get()))
      empl.push_back(e.get());
  for (auto &e : admins_)
    if (func(e.get(), personnel_.at(e.get()).get(),
             attendance_.at(e.get()).get()))
      empl.push_back(e.get());

  return empl;
}

Employee *EmployeeSystem::getEmployeeById(std::string const &id) const {
  for (auto &e : employees_)
    if (e->getEmployeeId() == id)
      return e.get();
  for (auto &e : drivers_)
    if (e->getEmployeeId() == id)
      return e.get();
  for (auto &e : managers_)
    if (e->getEmployeeId() == id)
      return e.get();
  for (auto &e : admins_)
    if (e->getEmployeeId() == id)
      return e.get();
  return nullptr;
}

std::vector<Employee *>
EmployeeSystem::getEmployeeByName(std::string const &name,
                                  std::string const &surname) const {
  std::vector<Employee *> empl;
  for (auto &e : employees_)
    if (e->getEmployeeName() == name && e->getEmployeeSurname() == surname)
      empl.push_back(e.get());
  for (auto &e : drivers_)
    if (e->getEmployeeName() == name && e->getEmployeeSurname() == surname)
      empl.push_back(e.get());
  for (auto &e : managers_)
    if (e->getEmployeeName() == name && e->getEmployeeSurname() == surname)
      empl.push_back(e.get());
  for (auto &e : admins_)
    if (e->getEmployeeName() == name && e->getEmployeeSurname() == surname)
      empl.push_back(e.get());
  return empl;
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
  case es::EmployeeRole::Admin:
    value = "Admin";
    break;
  case es::EmployeeRole::Employee:
    value = "Employee";
    break;
  }

  return value;
}

void printEmployeeData(PersonnelData *p, AttendanceData *attendance) {
  if (!p)
    return;
  std::cout << "Employee: Id: " << p->getEmployeeId() << " (";
  std::cout << (p->getEmployeeActive() ? "active" : "inactive") << ")"
            << std::endl;
  std::cout << "\tName: " << p->getEmployeeName() << std::endl;
  std::cout << "\tSurname: " << p->getEmployeeSurname() << std::endl;
  std::cout << "\tTelephone: " << p->getEmployeeTelephone() << std::endl;
  std::cout << "\tEmail: " << p->getEmployeeEmail() << std::endl;
  std::cout << "\tStandard WT: " << p->getEmployeeStandardWorkTime()
            << std::endl;
  std::cout << "\tMax WT: " << p->getEmployeeMaxWorkTime() << std::endl;
  std::cout << "\tHourly Wage: " << p->getEmployeeHourlyWage() << std::endl;
  std::cout << "\tRole: " << to_string(p->getEmployeeRole()) << std::endl;
  std::cout << "\tCardId: " << p->getEmployeeCardId() << std::endl;

  if (!attendance)
    return;
  std::cout << "\tAttendance:" << std::endl;
  for (auto a : attendance->getRecords())
    std::cout << "\t\t" << tu::makeAttendanceInstStr(&a) << "\n";
}

void EmployeeSystem::printData(Employee *const empl,
                               bool skipAttendance) const {
  if (empl) {
    auto p = personnel_.at(empl).get();
    if (skipAttendance)
      printEmployeeData(p, 0);
    else {
      auto attendance = attendance_.at(empl).get();
      printEmployeeData(p, attendance);
    }
  } else {
    for (auto const &e : employees_) {
      auto p = personnel_.at(e.get()).get();
      if (skipAttendance)
        printEmployeeData(p, 0);
      else {
        auto attendance = attendance_.at(e.get()).get();
        printEmployeeData(p, attendance);
      }
    }
  }
}

Result EmployeeSystem::addEmployee(Employee **const e, PersonnelData **const p,
                                   AttendanceData **const a) {
  if (!e)
    return Result::EmployeeIsNullptrError;

  auto pd = std::make_unique<PersonnelData>();
  auto ad = std::make_unique<AttendanceData>();

  auto ptr = std::make_unique<Employee>(pd.get(), ad.get());
  *e = ptr.get();
  employees_.emplace(std::move(ptr));

  personnel_.emplace(*e, std::move(pd));
  attendance_.emplace(*e, std::move(ad));

  if (p)
    *p = personnel_.at(*e).get();
  if (a)
    *a = attendance_.at(*e).get();
  return Result::Success;
}

Result EmployeeSystem::addEmployee(Driver **const e, PersonnelData **const p,
                                   AttendanceData **const a) {
  if (!e)
    return Result::EmployeeIsNullptrError;

  auto pd = std::make_unique<PersonnelData>();
  auto ad = std::make_unique<AttendanceData>();

  auto ptr = std::make_unique<Driver>(pd.get(), ad.get());
  *e = ptr.get();
  drivers_.emplace(std::move(ptr));

  personnel_.emplace(*e, std::move(pd));
  attendance_.emplace(*e, std::move(ad));

  if (p)
    *p = personnel_.at(*e).get();
  if (a)
    *a = attendance_.at(*e).get();
  return Result::Success;
}

Result EmployeeSystem::addEmployee(Manager **const e, PersonnelData **const p,
                                   AttendanceData **const a) {
  if (!e)
    return Result::EmployeeIsNullptrError;

  auto pd = std::make_unique<PersonnelData>();
  auto ad = std::make_unique<AttendanceData>();

  auto ptr = std::make_unique<Manager>(pd.get(), ad.get(), this);
  *e = ptr.get();
  managers_.emplace(std::move(ptr));

  personnel_.emplace(*e, std::move(pd));
  attendance_.emplace(*e, std::move(ad));

  if (p)
    *p = personnel_.at(*e).get();
  if (a)
    *a = attendance_.at(*e).get();
  return Result::Success;
}

Result EmployeeSystem::addEmployee(Admin **const e, PersonnelData **const p,
                                   AttendanceData **const a) {
  if (!e)
    return Result::EmployeeIsNullptrError;

  auto pd = std::make_unique<PersonnelData>();
  auto ad = std::make_unique<AttendanceData>();

  auto ptr = std::make_unique<Admin>(pd.get(), ad.get(), this);
  *e = ptr.get();
  admins_.emplace(std::move(ptr));

  personnel_.emplace(*e, std::move(pd));
  attendance_.emplace(*e, std::move(ad));

  if (p)
    *p = personnel_.at(*e).get();
  if (a)
    *a = attendance_.at(*e).get();
  return Result::Success;
}

std::vector<Employee *> EmployeeSystem::autoCheckOut() {
  std::vector<Employee *> checkedOut;
  tu::TimePoint now;
  now.populate();

  auto const targetMinutes = autoCheckoutHour_ * 60 + autoCheckoutMinute_;
  auto const nowMinutes = now.hour * 60 + now.minute;

  if (nowMinutes < targetMinutes)
    return {};

  for (auto &e : employees_)
    if (e->checkOut() == Result::Success)
      checkedOut.push_back(e.get());
  for (auto &e : drivers_)
    if (e->checkOut() == Result::Success)
      checkedOut.push_back(e.get());
  for (auto &e : managers_)
    if (e->checkOut() == Result::Success)
      checkedOut.push_back(e.get());
  for (auto &e : admins_)
    if (e->checkOut() == Result::Success)
      checkedOut.push_back(e.get());

  return checkedOut;
}
} // namespace es
