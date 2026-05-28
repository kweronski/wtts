#include "internals.hpp"
#include <pugixml.hpp>
#include <wtts/xmlWriter.hpp>

namespace dp {
XMLWriter::XMLWriter(std::string const &url)
    : DataWriter(url),
      storage_{new DataStorage, [](DataStorage *p) { delete p; }} {}

Result XMLWriter::writeData() const {
  pugi::xml_document doc;

  pugi::xml_node root = doc.append_child();
  root.set_name("root");

  for (auto const &empl : storage_->employees) {
    pugi::xml_node e = root.append_child();

    pugi::xml_attribute active = e.append_attribute("active");
    active.set_value(empl.status == EmployeeStatus::Active ? "true" : "false");

    {
      pugi::xml_node name = e.append_child();
      name.set_name("name");
      pugi::xml_attribute v = name.append_attribute("value");
      v.set_value(empl.name.c_str());
    }

    {
      pugi::xml_node surname = e.append_child();
      surname.set_name("surname");
      pugi::xml_attribute v = surname.append_attribute("value");
      v.set_value(empl.surname.c_str());
    }

    {
      pugi::xml_node tel = e.append_child();
      tel.set_name("telephone");
      pugi::xml_attribute v = tel.append_attribute("value");
      v.set_value(empl.telephone.c_str());
    }

    {
      pugi::xml_node email = e.append_child();
      email.set_name("email");
      pugi::xml_attribute v = email.append_attribute("value");
      v.set_value(empl.email.c_str());
    }

    {
      pugi::xml_node id = e.append_child();
      id.set_name("id");
      pugi::xml_attribute v = id.append_attribute("value");
      v.set_value(empl.id.c_str());
    }

    {
      pugi::xml_node swt = e.append_child();
      swt.set_name("stdWorkTime");
      pugi::xml_attribute v = swt.append_attribute("value");
      v.set_value(empl.standardWorkTime);
    }

    {
      pugi::xml_node mwt = e.append_child();
      mwt.set_name("maxWorkTime");
      pugi::xml_attribute v = mwt.append_attribute("value");
      v.set_value(empl.maxWorkTime);
    }

    {
      pugi::xml_node hw = e.append_child();
      hw.set_name("hourlyWage");
      pugi::xml_attribute v = hw.append_attribute("value");
      v.set_value(empl.hourlyWage);
    }

    {
      pugi::xml_node n = e.append_child();
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

    {
      pugi::xml_node n = e.append_child();
      n.set_name("role");
      pugi::xml_attribute v = n.append_attribute("value");
      v.set_value(empl.cardId.c_str());
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

      {
        auto n = inst.append_child();
        n.set_name("begin");
        pugi::xml_attribute y = n.append_attribute("year");
        y.set_value(p.begin.year);
        pugi::xml_attribute m = n.append_attribute("month");
        m.set_value(p.begin.month);
        pugi::xml_attribute d = n.append_attribute("day");
        d.set_value(p.begin.day);
        pugi::xml_attribute h = n.append_attribute("hour");
        h.set_value(p.begin.hour);
        pugi::xml_attribute M = n.append_attribute("minute");
        M.set_value(p.begin.minute);
      }

      {
        auto n = inst.append_child();
        n.set_name("end");
        pugi::xml_attribute y = n.append_attribute("year");
        y.set_value(p.end.year);
        pugi::xml_attribute m = n.append_attribute("month");
        m.set_value(p.end.month);
        pugi::xml_attribute d = n.append_attribute("day");
        d.set_value(p.end.day);
        pugi::xml_attribute h = n.append_attribute("hour");
        h.set_value(p.end.hour);
        pugi::xml_attribute M = n.append_attribute("minute");
        M.set_value(p.end.minute);
      }
    }
  }

  if (!doc.save_file(url_.c_str()))
    return Result::CouldNotOpenFileError;
  return Result::Success;
}

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
