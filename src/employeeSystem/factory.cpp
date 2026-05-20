#include <wtts/employeeSystemFactory.hpp>

namespace es {
std::unique_ptr<EmployeeSystem>
EmployeeSystemFactory::create(dp::DataParser *parser) {
  auto system = std::make_unique<EmployeeSystem>();

  if (auto r = parser->loadData(); r != dp::Result::Success) {
    throw std::runtime_error{"Factory failed to load data from storage: " +
                             dp::to_string(r)};
  }

  auto identifiers = parser->getEmployeeIdentifiers();
  for (auto const &id : identifiers) {
    AttendanceData *a{};
    PersonnelData *p{};
    Employee *e{};
    Driver *d{};
    Manager *m{};
    Admin *adm{};

    auto type = parser->getEmployeeRole(id);
    switch (type) {
    case EmployeeRole::Employee:
      system->addEmployee(&e, &p, &a);
      break;
    case EmployeeRole::Manager:
      system->addEmployee(&m, &p, &a);
      break;
    case EmployeeRole::Driver:
      system->addEmployee(&d, &p, &a);
      break;
    case EmployeeRole::Admin:
      system->addEmployee(&adm, &p, &a);
      break;
    default:
      throw std::runtime_error{"Employee factory: unknown employee type"};
    }

    p->setEmployeeId(id);
    p->setEmployeeActive(parser->getEmployeeStatus(id) ==
                         dp::EmployeeStatus::Active);
    p->setEmployeeName(parser->getEmployeeName(id));
    p->setEmployeeSurname(parser->getEmployeeSurname(id));
    p->setEmployeeTelephone(parser->getEmployeeTelephone(id));
    p->setEmployeeEmail(parser->getEmployeeEmail(id));
    p->setEmployeeStandardWorkTime(parser->getEmployeeStandardWorkTime(id));
    p->setEmployeeMaxWorkTime(parser->getEmployeeMaxWorkTime(id));
    p->setEmployeeHourlyWage(parser->getEmployeeHourlyWage(id));
    p->setEmployeeRole(parser->getEmployeeRole(id));
    p->setEmployeeCardId(parser->getEmployeeCardId(id));

    for (auto att : parser->getEmployeeAttendance(id)) {
      a->addTimePeriod(*att);
    }
  }

  return system;
}
} // namespace es
