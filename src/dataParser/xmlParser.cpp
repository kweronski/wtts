#include <pugixml.hpp>
#include <wtts/xmlParser.hpp>

#include "internals.hpp"

namespace dp {
XMLDataParser::XMLDataParser()
    : document_{new pugi::xml_document{}, [](auto p) { delete p; }},
      storage_{new DataStorage{}, [](auto p) { delete p; }} {}

#ifdef MCR_XML_NODE
#error "MCR_XML_NODE is already defined"
#endif

#ifdef MCR_ATTR_VAL
#error "MCR_ATTR_VAL is already defined"
#endif

#ifdef MCR_ATTR
#error "MCR_ATTR is already defined"
#endif

#ifdef MCR_ATTR_NUM
#error "MCR_ATTR_NUM is already defined"
#endif

#ifdef MCR_NUM
#error "MCR_NUM is already defined"
#endif

#ifdef MCR_TIME
#error "MCR_TIME is already defined"
#endif

// These macros are only visible for the duration of the loadData function.
#define MCR_XML_NODE(node, childId, parent, err)                               \
  if (!parent)                                                                 \
    return err;                                                                \
  auto node = parent.child(#childId);                                          \
  if (!node)                                                                   \
    return err;

#define MCR_ATTR_VAL(dst, childId, parent, err)                                \
  if (parent) {                                                                \
    if (!parent.child(#childId))                                               \
      return err;                                                              \
    else if (!parent.child(#childId).attribute("value") ||                     \
             !parent.child(#childId).attribute("value").value())               \
      return err;                                                              \
    else                                                                       \
      dst = parent.child(#childId).attribute("value").value();                 \
  } else                                                                       \
    return err;

#define MCR_ATTR(dst, attrId, node, err)                                       \
  if (!node || !node.attribute(#attrId) || !node.attribute(#attrId).value())   \
    return err;                                                                \
  dst = node.attribute(#attrId).value();

#define MCR_ATTR_NUM(dst, childId, parent, err, conversionFunc)                \
  {                                                                            \
    std::string tmp;                                                           \
    MCR_ATTR_VAL(tmp, childId, parent, err);                                   \
    try {                                                                      \
      dst = conversionFunc(tmp);                                               \
    } catch (...) {                                                            \
      return Result::StringToNumberConversionError;                            \
    }                                                                          \
  }

#define MCR_NUM(dst, attrId, node, err, conversionFunc)                        \
  {                                                                            \
    std::string tmp;                                                           \
    MCR_ATTR(tmp, attrId, node, err);                                          \
    try {                                                                      \
      dst = conversionFunc(tmp);                                               \
    } catch (...) {                                                            \
      return Result::StringToNumberConversionError;                            \
    }                                                                          \
  }

#define MCR_TIME(time, node)                                                   \
  {                                                                            \
    std::string attType;                                                       \
    MCR_ATTR_VAL(attType, type, node, Result::MissingAttendanceTypeError);     \
    if (attType == "vacation")                                                 \
      time.type = AttendanceType::Vacation;                                    \
    else if (attType == "delivery")                                            \
      time.type = AttendanceType::Delivery;                                    \
    else if (attType == "work")                                                \
      time.type = AttendanceType::Work;                                        \
    else if (attType == "sick")                                                \
      time.type = AttendanceType::Sick;                                        \
    else                                                                       \
      return Result::UnknownAttendanceTypeError;                               \
                                                                               \
    MCR_NUM(time.begin.year, year, node.child("begin"),                        \
            Result::MissingAttendanceBeginYearError, std::stoul);              \
    MCR_NUM(time.begin.month, month, node.child("begin"),                      \
            Result::MissingAttendanceBeginMonthError, std::stoul);             \
    MCR_NUM(time.begin.day, day, node.child("begin"),                          \
            Result::MissingAttendanceBeginDayError, std::stoul);               \
    MCR_NUM(time.begin.hour, hour, node.child("begin"),                        \
            Result::MissingAttendanceBeginHourError, std::stoul);              \
    MCR_NUM(time.begin.minute, minute, node.child("begin"),                    \
            Result::MissingAttendanceBeginMinuteError, std::stoul);            \
                                                                               \
    MCR_NUM(time.end.year, year, node.child("end"),                            \
            Result::MissingAttendanceBeginYearError, std::stoul);              \
    MCR_NUM(time.end.month, month, node.child("end"),                          \
            Result::MissingAttendanceBeginMonthError, std::stoul);             \
    MCR_NUM(time.end.day, day, node.child("end"),                              \
            Result::MissingAttendanceBeginDayError, std::stoul);               \
    MCR_NUM(time.end.hour, hour, node.child("end"),                            \
            Result::MissingAttendanceBeginHourError, std::stoul);              \
    MCR_NUM(time.end.minute, minute, node.child("end"),                        \
            Result::MissingAttendanceBeginMinuteError, std::stoul);            \
  }

Result XMLDataParser::loadData(std::string const &url) {
  if (!document_->load_file(url.c_str()))
    return Result::CouldNotOpenFileError;

  auto root = document_->first_child();

  if (!root) // The data file is empty. Return early.
    return Result::Success;

  for (auto node = root.child("employee"); node; node = node.next_sibling()) {
    Employee<DataParser::ID> data{};

    std::string emplStatus;
    MCR_ATTR(emplStatus, active, node,
             Result::MissingEmployeeActiveStatusError);
    if (emplStatus == "true")
      data.status = EmployeeStatus::Active;
    else
      data.status = EmployeeStatus::Inactive;

    MCR_XML_NODE(personal, personalInfo, node,
                 Result::MissingPersonalInfoError);

    MCR_ATTR_VAL(data.name, name, personal, Result::MissingEmployeeNameError);
    MCR_ATTR_VAL(data.surname, surname, personal,
                 Result::MissingEmployeeSurnameError);
    MCR_ATTR_VAL(data.email, email, personal,
                 Result::MissingEmployeeEmailError);
    MCR_ATTR_VAL(data.telephone, telephone, personal,
                 Result::MissingEmployeeTelephoneError);

    MCR_XML_NODE(workRelated, employeeInfo, node,
                 Result::MissingEmployeeInfoError);

    MCR_ATTR_NUM(data.standardWorkTime, stdWorkTime, workRelated,
                 Result::MissingEmployeeStandardWorkTimeError, std::stoul);
    MCR_ATTR_NUM(data.maxWorkTime, maxWorkTime, workRelated,
                 Result::MissingEmployeeMaxWorkTimeError, std::stoul);
    MCR_ATTR_NUM(data.hourlyWage, hourlyWage, workRelated,
                 Result::MissingEmployeeHourlyWageError, std::stoul);

    std::string roleVal;
    MCR_ATTR_VAL(roleVal, role, workRelated, Result::MissingEmployeeRoleError);

    if (roleVal == "manager")
      data.role = EmployeeRole::Manager;
    else if (roleVal == "driver")
      data.role = EmployeeRole::Driver;
    else if (roleVal == "cleaner")
      data.role = EmployeeRole::Cleaner;
    else if (roleVal == "chef")
      data.role = EmployeeRole::Chef;
    else if (roleVal == "waiter")
      data.role = EmployeeRole::Waiter;
    else if (roleVal == "admin")
      data.role = EmployeeRole::Admin;
    else
      return Result::UnknownEmployeeRoleError;

    MCR_ATTR_VAL(data.cardId, cardId, workRelated,
                 Result::MissingEmployeeCardIdError);
    MCR_ATTR_VAL(data.id, id, workRelated, Result::MissingEmployeeIdError);

    if (auto attendance = node.child("attendance"); attendance) {
      for (auto instance = attendance.child("instance"); instance;
           instance = instance.next_sibling()) {
        TimePeriod time{};
        MCR_TIME(time, instance);
        data.attendance.push_back(std::move(time));
      }
    }

    storage_->employees.push_back(std::move(data));
    auto back = &storage_->employees.back();
    storage_->employeeMap.emplace(back->id, back);
  }

  return Result::Success;
}

#undef MCR_XML_NODE
#undef MCR_ATTR_VAL
#undef MCR_ATTR
#undef MCR_ATTR_NUM
#undef MCR_NUM
#undef MCR_TIME

std::vector<XMLDataParser::ID> XMLDataParser::getEmployeeIdentifiers() {
  std::vector<XMLDataParser::ID> ids;
  for (auto const &e : storage_->employees)
    ids.push_back(e.id);
  return ids;
}

// Personal info
bool XMLDataParser::getEmployeeActiveStatus(ID const &id) {
  return storage_->employeeMap.at(id)->status == EmployeeStatus::Active;
}
std::string XMLDataParser::getEmployeeName(ID const &id) {
  return storage_->employeeMap.at(id)->name;
}
std::string XMLDataParser::getEmployeeSurname(ID const &id) {
  return storage_->employeeMap.at(id)->surname;
}
std::string XMLDataParser::getEmployeeTelephone(ID const &id) {
  return storage_->employeeMap.at(id)->telephone;
}
std::string XMLDataParser::getEmployeeEmail(ID const &id) {
  return storage_->employeeMap.at(id)->email;
}

// Employee info
unsigned XMLDataParser::getEmployeeStandardWorkTime(ID const &id) {
  return storage_->employeeMap.at(id)->standardWorkTime;
}
unsigned XMLDataParser::getEmployeeMaxWorkTime(ID const &id) {
  return storage_->employeeMap.at(id)->maxWorkTime;
}
unsigned XMLDataParser::getEmployeeHourlyWage(ID const &id) {
  return storage_->employeeMap.at(id)->hourlyWage;
}
unsigned XMLDataParser::getEmployeeRole(ID const &id) {
  return unsigned(storage_->employeeMap.at(id)->role);
}
std::string XMLDataParser::getEmployeeCardId(ID const &id) {
  return storage_->employeeMap.at(id)->cardId;
}

// Attendance info
std::vector<TimePeriod *> XMLDataParser::getEmployeeAttendance(ID const &id) {
  std::vector<TimePeriod *> attendance;
  attendance.reserve(storage_->employeeMap.at(id)->attendance.size());
  for (auto &a : storage_->employeeMap.at(id)->attendance)
    attendance.push_back(&a);
  return attendance;
}

std::string makeAttendanceInstStr(TimePeriod const *p) {
  return std::to_string(unsigned(p->type)) + " (" +
         std::to_string(p->begin.year) + "/" + std::to_string(p->begin.month) +
         "/" + std::to_string(p->begin.day) + ", " +
         std::to_string(p->begin.hour) + ":" + std::to_string(p->begin.minute) +
         ") - " + "(" + std::to_string(p->end.year) + "/" +
         std::to_string(p->end.month) + "/" + std::to_string(p->end.day) +
         ", " + std::to_string(p->end.hour) + ":" +
         std::to_string(p->end.minute) + ")";
}
} // namespace dp
