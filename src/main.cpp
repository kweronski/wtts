#include <iostream>
#include <wtts/xmlParser.hpp>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Too few arguments; Provide path to data storage" << std::endl;
    return 1;
  }

  dp::DataParser *p = new dp::XMLDataParser{};
  if (auto result = p->loadData(argv[1]); result != dp::Result::Success) {
    std::cerr << "Failed to parse data with error code: " << unsigned(result)
              << std::endl;
    return 2;
  }

  auto ids = p->getEmployeeIdentifiers();

  for (auto const &id : ids) {
    std::cout << "Employee: " << id << std::endl;
    std::cout << "\tName: " << p->getEmployeeName(id) << std::endl;
    std::cout << "\tSurname: " << p->getEmployeeSurname(id) << std::endl;
    std::cout << "\tTelephone: " << p->getEmployeeTelephone(id) << std::endl;
    std::cout << "\tEmail: " << p->getEmployeeEmail(id) << std::endl;
    std::cout << "\tStandard WT: " << p->getEmployeeStandardWorkTime(id)
              << std::endl;
    std::cout << "\tMax WT: " << p->getEmployeeMaxWorkTime(id) << std::endl;
    std::cout << "\tHourly Wage: " << p->getEmployeeHourlyWage(id) << std::endl;
    std::cout << "\tRole: " << p->getEmployeeRole(id) << std::endl;
    std::cout << "\tCardId: " << p->getEmployeeCardId(id) << std::endl;

    auto attendance = p->getEmployeeAttendance(id);
    for (auto a : attendance)
      std::cout << "\t" << dp::makeAttendanceInstStr(a) << std::endl;
  }

  delete p;
}
