#include "wtts/employeeData.hpp"
#include "wtts/logInfo.hpp"
#include <fstream>
#include <iostream>
#include <string>
#include <wtts/xmlParser.hpp>
#include <wtts/xmlWriter.hpp>

#ifdef MCR_ERR_CHECK
#error "MCR_ERR_CHECK is already defined"
#endif

#define MCR_ERR_CHECK(a, c)                                                    \
  do {                                                                         \
    std::string txtLine;                                                       \
    std::getline(c, txtLine);                                                  \
    ++lineNumber;                                                              \
    if (a != txtLine) {                                                        \
      std::cerr << a << " [xml]"                                               \
                << " != " << txtLine << " [txt@" << lineNumber << "]";         \
      std::cerr << " ----------------------> Data missmatch! \n";              \
      return 1;                                                                \
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
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path/to/source/text>" << std::endl;
    return 1;
  }

  try {
    std::string const spath{argv[1]}, opath{"/tmp/xmlWriterTest.xml"};
    es::XMLWriter w{opath};
    es::XMLParser r{opath};
    std::ifstream s{spath};

    if (!s.is_open()) {
      std::cerr << "Failed to open text source file: " << spath << std::endl;
      return 1;
    }

    /* Load up the text data into the xml writer */
    std::string record;
    std::getline(s, record);

    while (!s.eof()) {
      auto storage = w.addEmployee();

      es::EmployeeStatus status{es::EmployeeStatus::Active};
      if (record == "inactive")
        status = es::EmployeeStatus::Inactive;
      else if (record != "active") {
        std::cerr << "Unsupported status: " << record << std::endl;
        return 1;
      }
      w.setEmployeeStatus(storage, status);

      std::getline(s, record);
      w.setEmployeeName(storage, record);

      std::getline(s, record);
      w.setEmployeeSurname(storage, record);

      std::getline(s, record);
      w.setEmployeeTelephone(storage, record);

      std::getline(s, record);
      w.setEmployeeEmail(storage, record);

      std::getline(s, record);
      w.setEmployeeId(storage, record);

      unsigned value{};

      std::getline(s, record);
      value = std::stoi(record);
      w.setEmployeeStandardWorkTime(storage, record);

      std::getline(s, record);
      value = std::stoi(record);
      w.setEmployeeMaxWorkTime(storage, record);

      std::getline(s, record);
      value = std::stoi(record);
      w.setEmployeeHourlyWage(storage, record);

      es::EmployeeRole role{};
      std::getline(s, record);
      if (record == "manager")
        role = es::EmployeeRole::Manager;
      else if (record == "employee")
        role = es::EmployeeRole::Employee;
      else if (record == "driver")
        role = es::EmployeeRole::Driver;
      else if (record == "admin")
        role = es::EmployeeRole::Admin;
      else
        role = es::EmployeeRole::Unknown;
      w.setEmployeeRole(storage, record);

      std::getline(s, record);
      w.setEmployeeCardId(storage, record);

      while (record != "active" && record != "inactive") {
        tu::TimePeriod t{};

        std::getline(s, record);
        if (record == "Sick")
          t.type = tu::AttendanceType::Sick;
        else if (record == "Vacation")
          t.type = tu::AttendanceType::Vacation;
        else if (record == "Delivery")
          t.type = tu::AttendanceType::Delivery;
        else if (record == "Work")
          t.type = tu::AttendanceType::Work;
        else {
          std::cerr << "Unknown attendance type: " << record << std::endl;
          return 1;
        }

        std::getline(s, record);
        t.begin.year = std::stoi(record);
        std::getline(s, record);
        t.begin.month = std::stoi(record);
        std::getline(s, record);
        t.begin.day = std::stoi(record);
        std::getline(s, record);
        t.begin.hour = std::stoi(record);
        std::getline(s, record);
        t.begin.minute = std::stoi(record);
        std::getline(s, record);
        t.end.year = std::stoi(record);
        std::getline(s, record);
        t.end.month = std::stoi(record);
        std::getline(s, record);
        t.end.day = std::stoi(record);
        std::getline(s, record);
        t.end.hour = std::stoi(record);
        std::getline(s, record);
        t.end.minute = std::stoi(record);

        w.addEmployeeAttendance(storage, t);
      }
    }

    w.writeData();

    r.loadData();
    if (auto result = r.loadData(); result != dp::Result::Success) {
      std::cerr << "Usage: " << argv[0]
                << " <path to xml data> <path to txt cross reference>\n"
                << " Incorrect path to xml data?\n";
      return 1;
    }

    std::ifstream file{spath};
    std::size_t lineNumber{};

    for (auto &&id : r.getEmployeeIdentifiers()) {
      MCR_ERR_CHECK(statusToString(r.getEmployeeStatus(id)), file);
      MCR_ERR_CHECK(r.getEmployeeName(id), file);
      MCR_ERR_CHECK(r.getEmployeeSurname(id), file);
      MCR_ERR_CHECK(r.getEmployeeEmail(id), file);
      MCR_ERR_CHECK(r.getEmployeeTelephone(id), file);
      MCR_ERR_CHECK(std::to_string(r.getEmployeeStandardWorkTime(id)), file);
      MCR_ERR_CHECK(std::to_string(r.getEmployeeMaxWorkTime(id)), file);
      MCR_ERR_CHECK(std::to_string(r.getEmployeeHourlyWage(id)), file);
      MCR_ERR_CHECK(roleToString(r.getEmployeeRole(id)), file);
      MCR_ERR_CHECK(r.getEmployeeCardId(id), file);

      for (auto instance : r.getEmployeeAttendance(id))
        for (auto &&element : serialize(*instance))
          MCR_ERR_CHECK(element, file);
    }
  } catch (std::exception const &e) {
    std::cerr << "Error: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Error: unknown exception" << std::endl;
    return 1;
  }
}
