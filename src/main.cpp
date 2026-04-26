#include <iostream>
#include <wtts/employeeSystemFactory.hpp>
#include <wtts/xmlParser.hpp>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Too few arguments; Provide path to data storage" << std::endl;
    return 1;
  }

  auto parser = std::make_unique<dp::XMLDataParser>(argv[1]);
  auto system = es::EmployeeSystemFactory::create(parser.get());

  // system->printData();

  /*
auto ids = p->getEmployeeIdentifiers();

for (auto const &id : ids) {
std::cout << "Employee: Id: " << id << " (";
std::cout << to_string(p->getEmployeeStatus(id)) << ")" << std::endl;
std::cout << "\tName: " << p->getEmployeeName(id) << std::endl;
std::cout << "\tSurname: " << p->getEmployeeSurname(id) << std::endl;
std::cout << "\tTelephone: " << p->getEmployeeTelephone(id) << std::endl;
std::cout << "\tEmail: " << p->getEmployeeEmail(id) << std::endl;
std::cout << "\tStandard WT: " << p->getEmployeeStandardWorkTime(id)
        << std::endl;
std::cout << "\tMax WT: " << p->getEmployeeMaxWorkTime(id) << std::endl;
std::cout << "\tHourly Wage: " << p->getEmployeeHourlyWage(id) << std::endl;
std::cout << "\tRole: " << dp::to_string(p->getEmployeeRole(id))
        << std::endl;
std::cout << "\tCardId: " << p->getEmployeeCardId(id) << std::endl;

std::cout << "\tAttendance:" << std::endl;
auto attendance = p->getEmployeeAttendance(id);
for (auto a : attendance)
std::cout << "\t\t" << dp::makeAttendanceInstStr(a) << "\n";
}

  */
}
