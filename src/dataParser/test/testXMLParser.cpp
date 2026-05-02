#include "wtts/dataParser.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <wtts/xmlParser.hpp>

enum { success, error };

// dopisac makro
#ifdef MCR_ERR_CHECK
#error "MCR_ERR_CHECK is already defined"
#endif

#define MCR_ERR_CHECK(a, c)                                                    \
  do {                                                                         \
    std::string txtLine;                                                       \
    std::getline(c, txtLine);                                                  \
    if (a != txtLine) {                                                        \
      std::cerr << a << " [xml]"                                               \
                << " != " << txtLine << " [txt]";                              \
      std::cerr << " ----------------------> Data missmatch! \n";              \
      return error;                                                            \
    }                                                                          \
  } while (0)

std::string roleToString(es::EmployeeRole r) {
  std::string out;
  switch (r) {
  case es::EmployeeRole::Admin:
    out = "admin";
    break;
  case es::EmployeeRole::Manager:
    out = "manager";
    break;
  case es::EmployeeRole::Employee:
    out = "employee";
    break;
  case es::EmployeeRole::Driver:
    out = "driver";
    break;
  case es::EmployeeRole::Unknown:
    out = "unknown";
    break;
  }
  return out;
}

std::vector<std::string> serialize(tu::TimePeriod const &p) {
  std::vector<std::string> out;

  out.push_back(tu::to_string(p.type));

  out.push_back(std::to_string(p.begin.year));
  out.push_back(std::to_string(p.begin.month));
  out.push_back(std::to_string(p.begin.day));
  out.push_back(std::to_string(p.begin.hour));
  out.push_back(std::to_string(p.begin.minute));

  out.push_back(std::to_string(p.end.year));
  out.push_back(std::to_string(p.end.month));
  out.push_back(std::to_string(p.end.day));
  out.push_back(std::to_string(p.end.hour));
  out.push_back(std::to_string(p.end.minute));

  return out;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0]
              << " <path to xml data> <path to txt cross reference>\n";
    return error;
  }

  dp::XMLDataParser parser{argv[1]};
  if (auto result = parser.loadData(); result != dp::Result::Success) {
    // something went wrong
    std::cerr << "Usage: " << argv[0]
              << " <path to xml data> <path to txt cross reference>\n"
              << " Incorrect path to xml data?\n";
    return error;
  }

  /* LOGIC:
   * Use DataParser interface to extract information from XML file.
   * Use txt file to verify that the extracted data from the XML file
   * is correct.
   */

  std::ifstream file(argv[2]);
  if (!file.is_open()) {
    std::cerr << "Cannot open the file\n";
    std::cout << "Path: " << argv[2] << std::endl;
    return error;
  }

  auto const statusToString = [](dp::EmployeeStatus s) {
    if (s == dp::EmployeeStatus::Active)
      return "active";
    return "inactive";
  };

  for (auto &&id : parser.getEmployeeIdentifiers()) {
    MCR_ERR_CHECK(statusToString(parser.getEmployeeStatus(id)), file);
    MCR_ERR_CHECK(parser.getEmployeeName(id), file);
    MCR_ERR_CHECK(parser.getEmployeeSurname(id), file);
    MCR_ERR_CHECK(parser.getEmployeeEmail(id), file);
    MCR_ERR_CHECK(parser.getEmployeeTelephone(id), file);
    MCR_ERR_CHECK(std::to_string(parser.getEmployeeStandardWorkTime(id)), file);
    MCR_ERR_CHECK(std::to_string(parser.getEmployeeMaxWorkTime(id)), file);
    MCR_ERR_CHECK(std::to_string(parser.getEmployeeHourlyWage(id)), file);
    MCR_ERR_CHECK(roleToString(parser.getEmployeeRole(id)), file);
    MCR_ERR_CHECK(parser.getEmployeeCardId(id), file);

    for (auto instance : parser.getEmployeeAttendance(id))
      for (auto &&element : serialize(*instance))
        MCR_ERR_CHECK(element, file);
    return success;
  }
}
