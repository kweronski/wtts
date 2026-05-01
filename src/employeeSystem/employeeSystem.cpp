#include <iostream>
#include <wtts/employeeSystem.hpp>
#include <wtts/logInfo.hpp>

namespace es {
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

void EmployeeSystem::printData() const {
  for (auto const &e : employees_) {
    auto p = personnel_.at(e.get()).get();
    std::cout << "Employee: Id: " << p->getEmployeeId() << " (";
    std::cout << std::boolalpha << p->getEmployeeActive() << ")" << std::endl;
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

    std::cout << "\tAttendance:" << std::endl;
    auto attendance = attendance_.at(e.get()).get();
    for (auto a : attendance->getRecords())
      std::cout << "\t\t" << tu::makeAttendanceInstStr(&a) << "\n";
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
  employees_.emplace(std::move(ptr));

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

  auto ptr = std::make_unique<Manager>(pd.get(), ad.get());
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

Result EmployeeSystem::addEmployee(Admin **const e, PersonnelData **const p,
                                   AttendanceData **const a) {
  if (!e)
    return Result::EmployeeIsNullptrError;

  auto pd = std::make_unique<PersonnelData>();
  auto ad = std::make_unique<AttendanceData>();

  auto ptr = std::make_unique<Admin>(pd.get(), ad.get());
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

Result EmployeeSystem::autoCheckOut() { return Result::Success; }
} // namespace es
