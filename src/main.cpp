#include <iostream>
#include <wtts/xmlParser.hpp>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Too few arguments; Provide path to data storage" << std::endl;
    return 1;
  }

  dp::DataParser *p = new dp::XMLDataParser{};
  if (auto result = p->loadData(argv[1]); result != dp::Result::Success) {
    std::cerr << "Failed to open data storage" << std::endl;
    return 2;
  }

  auto ids = p->getEmployeeIdentifiers();

  for (auto const &id : ids) {
    std::cout << p->getEmployeeName(id) << " " << p->getEmployeeSurname(id)
              << std::endl;
    std::cout << "\t" << p->getEmployeeEmail(id) << std::endl;
  }

  delete p;
}
