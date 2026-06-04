#include "internals.hpp"
#include <pugixml.hpp>
#include <wtts/xmlWriter.hpp>

namespace dp {
XMLWriter::XMLWriter(std::string const &url)
    : DataWriter(url),
      storage_{new DataStorage, [](DataStorage *p) { delete p; }} {}

#ifdef MCR_ADD_ATTR
#error "MCR_ADD_ATTR is already defined"
#endif

#define MCR_ADD_CWV(parent, name, val)                                         \
  do {                                                                         \
    pugi::xml_node node = parent.append_child();                               \
    node.set_name(name);                                                       \
    pugi::xml_attribute v = node.append_attribute("value");                    \
    v.set_value(val);                                                          \
  } while (0)

#ifdef MCR_INS_ATT
#error "MCR_INS_ATT is laready defined"
#endif

#define MCR_INS_ATT(name, tp)                                                  \
  {                                                                            \
    auto n = inst.append_child();                                              \
    n.set_name(name);                                                          \
    pugi::xml_attribute y = n.append_attribute("year");                        \
    y.set_value(tp.year);                                                      \
    pugi::xml_attribute m = n.append_attribute("month");                       \
    m.set_value(tp.month);                                                     \
    pugi::xml_attribute d = n.append_attribute("day");                         \
    d.set_value(tp.day);                                                       \
    pugi::xml_attribute h = n.append_attribute("hour");                        \
    h.set_value(tp.hour);                                                      \
    pugi::xml_attribute M = n.append_attribute("minute");                      \
    M.set_value(tp.minute);                                                    \
  }

Result XMLWriter::writeData() const {
  pugi::xml_document doc;

  pugi::xml_node root = doc.append_child();
  root.set_name("root");

  for (auto const &empl : storage_->employees) {
    pugi::xml_node e = root.append_child();
    e.set_name("employee");
    pugi::xml_node pi = e.append_child();
    pi.set_name("personalInfo");
    pugi::xml_node ai = e.append_child();
    ai.set_name("employeeInfo");

    pugi::xml_attribute active = e.append_attribute("active");
    active.set_value(empl.status == EmployeeStatus::Active ? "true" : "false");

    MCR_ADD_CWV(pi, "name", empl.name.c_str());
    MCR_ADD_CWV(pi, "surname", empl.surname.c_str());
    MCR_ADD_CWV(pi, "telephone", empl.telephone.c_str());
    MCR_ADD_CWV(pi, "email", empl.email.c_str());
    MCR_ADD_CWV(ai, "id", empl.id.c_str());
    MCR_ADD_CWV(ai, "stdWorkTime", empl.standardWorkTime);
    MCR_ADD_CWV(ai, "maxWorkTime", empl.maxWorkTime);
    MCR_ADD_CWV(ai, "hourlyWage", empl.hourlyWage);
    MCR_ADD_CWV(ai, "cardId", empl.cardId.c_str());

    {
      pugi::xml_node n = ai.append_child();
      n.set_name("role");
      pugi::xml_attribute v = n.append_attribute("value");
      switch (empl.role) {
      case es::EmployeeRole::Employee:
        v.set_value("employee");
        break;
      case es::EmployeeRole::Driver:
        v.set_value("driver");
        break;
      case es::EmployeeRole::Manager:
        v.set_value("manager");
        break;
      case es::EmployeeRole::Admin:
        v.set_value("admin");
        break;
      case es::EmployeeRole::Unknown:
        v.set_value("unknown");
        break;
      }
    }

    auto at = e.append_child();
    at.set_name("attendance");

    for (auto const &p : empl.attendance) {
      auto inst = at.append_child();
      inst.set_name("instance");

      {
        auto n = inst.append_child();
        n.set_name("type");
        pugi::xml_attribute v = n.append_attribute("value");
        switch (p.type) {
        case tu::AttendanceType::Delivery:
          v.set_value("delivery");
          break;
        case tu::AttendanceType::Sick:
          v.set_value("sick");
          break;
        case tu::AttendanceType::Vacation:
          v.set_value("vacation");
          break;
        case tu::AttendanceType::Work:
          v.set_value("work");
          break;
        }
      }

      MCR_INS_ATT("begin", p.begin);
      MCR_INS_ATT("end", p.end);
    }
  }

  if (!doc.save_file(url_.c_str()))
    return Result::CouldNotOpenFileError;
  return Result::Success;
}

#undef MCR_ADD_ATTR

Employee *XMLWriter::addEmployee() {
  storage_->employees.push_back({});
  return &storage_->employees.back();
}

void XMLWriter::addEmployeeAttendance(ID const id, tu::TimePeriod const p) {
  id->attendance.push_back(p);
}

void XMLWriter::setEmployeeStatus(ID const id, EmployeeStatus const s) {
  id->status = s;
}

void XMLWriter::setEmployeeName(ID const id, std::string const &v) {
  id->name = v;
}

void XMLWriter::setEmployeeSurname(ID const id, std::string const &v) {
  id->surname = v;
}

void XMLWriter::setEmployeeTelephone(ID const id, std::string const &v) {
  id->telephone = v;
}

void XMLWriter::setEmployeeEmail(ID const id, std::string const &v) {
  id->email = v;
}

void XMLWriter::setEmployeeId(ID const id, std::string const &v) {
  storage_->employeeMap.emplace(v, id);
  id->id = v;
}

void XMLWriter::setEmployeeStandardWorkTime(ID const id, unsigned const v) {
  id->standardWorkTime = v;
}

void XMLWriter::setEmployeeMaxWorkTime(ID const id, unsigned const v) {
  id->maxWorkTime = v;
}

void XMLWriter::setEmployeeHourlyWage(ID const id, unsigned const v) {
  id->hourlyWage = v;
}

void XMLWriter::setEmployeeRole(ID const id, es::EmployeeRole const v) {
  id->role = v;
}

void XMLWriter::setEmployeeCardId(ID const id, std::string const &v) {
  id->cardId = v;
}
} // namespace dp
