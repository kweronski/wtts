#include <wtts/employee.hpp>
#include <wtts/employeeSystem.hpp>

namespace es {
Result GeneralAdmin::setAbsence(std::string const &id,
                                tu::TimePoint const &tp) {
  auto e = system_->getEmployeeById(id);
  if (!e)
    return Result::EmployeeNotFoundError;
  return system_->setEmployeeAbsence(e, tp);
}

Result GeneralAdmin::addEmployee(PersonnelData const &pd) {
  PersonnelData *new_pd{};
  AttendanceData *new_ad{};
  Result result{};

  switch (pd.getEmployeeRole()) {
  case EmployeeRole::Employee: {
    Employee *e{};
    result = system_->addEmployee(&e, &new_pd, &new_ad);
    break;
  }
  case EmployeeRole::Driver: {
    Driver *e{};
    result = system_->addEmployee(&e, &new_pd, &new_ad);
    break;
  }
  case EmployeeRole::Manager: {
    Manager *e{};
    result = system_->addEmployee(&e, &new_pd, &new_ad);
    break;
  }
  case EmployeeRole::Admin: {
    Admin *e{};
    result = system_->addEmployee(&e, &new_pd, &new_ad);
    break;
  }
  default:
    return Result::UnknownEmployeeRoleError;
  }

  if (result != Result::Success)
    return result;

  if (!new_pd)
    return Result::EmployeeNotFoundError;

  *new_pd = pd;
  return Result::Success;
}

Result GeneralAdmin::editEmployee(std::string const &id,
                                  PersonnelData const &newData) {
  auto e = system_->getEmployeeById(id);
  if (!e)
    return Result::EmployeeNotFoundError;

  auto pd = system_->getEmployeeInfo(e);
  if (!pd)
    return Result::EmployeeNotFoundError;

  if (pd->getEmployeeId() != newData.getEmployeeId())
    return Result::EmployeeIdNotUniqueError;

  *pd = newData;
  return Result::Success;
}

Result GeneralAdmin::activateEmployee(std::string const &id) {
  auto e = system_->getEmployeeById(id);
  if (!e)
    return Result::EmployeeNotFoundError;
  auto pd = system_->getEmployeeInfo(e);
  if (!pd)
    return Result::EmployeeNotFoundError;
  if (pd->getEmployeeActive())
    return Result::EmployeeAlreadyActive;
  pd->setEmployeeActive(true);
  return Result::Success;
}

Result GeneralAdmin::deactivateEmployee(std::string const &id) {
  auto e = system_->getEmployeeById(id);
  if (!e)
    return Result::EmployeeNotFoundError;
  auto pd = system_->getEmployeeInfo(e);
  if (!pd)
    return Result::EmployeeNotFoundError;
  if (!pd->getEmployeeActive())
    return Result::EmployeeAlreadyInactive;
  pd->setEmployeeActive(false);
  return Result::Success;
}

void Admin::removeEmployee(Employee *const e) {
  if (!e)
    return;
  system_->removeEmployee(e->getEmployeeId());
}

Result Driver::logDeliveryBegin() {
  if (attendance_->getCurrentDeliveryTimePeriod()->begin.year)
    return Result::DeliveryAlreadyCheckedIn;

  tu::TimePoint tp;
  tp.populate();
  attendance_->getCurrentDeliveryTimePeriod()->begin = std::move(tp);

  return Result::Success;
}

Result Driver::logDeliveryEnd() {
  if (!attendance_->getCurrentDeliveryTimePeriod()->begin.year)
    return Result::DeliveryNotCheckedIn;

  tu::TimePoint tp;
  tp.populate();
  attendance_->getCurrentDeliveryTimePeriod()->end = std::move(tp);
  attendance_->getCurrentDeliveryTimePeriod()->type =
      tu::AttendanceType::Delivery;

  attendance_->addTimePeriod(
      std::move(*attendance_->getCurrentDeliveryTimePeriod()));
  *attendance_->getCurrentDeliveryTimePeriod() = {};
  return Result::Success;
}

Result Employee::checkIn() {
  if (attendance_->getCurrentTimePeriod()->begin.year)
    return Result::EmployeeAlreadyCheckedIn;

  tu::TimePoint tp;
  tp.populate();
  attendance_->getCurrentTimePeriod()->begin = std::move(tp);

  return Result::Success;
}

Result Employee::checkOut() {
  if (!attendance_->getCurrentTimePeriod()->begin.year)
    return Result::EmployeeNotCheckedIn;

  tu::TimePoint tp;
  tp.populate();
  attendance_->getCurrentTimePeriod()->end = std::move(tp);
  attendance_->getCurrentTimePeriod()->type = tu::AttendanceType::Work;

  attendance_->addTimePeriod(std::move(*attendance_->getCurrentTimePeriod()));
  *attendance_->getCurrentTimePeriod() = {};
  return Result::Success;
}

double Employee::calculatePay(tu::TimePoint const &start) const {
  auto const hourlyWage = this->getEmployeeHourlyWage();
  auto const minuteWage = double(hourlyWage) / 60.0;

  double total{};

  for (auto const &p : attendance_->getRecords()) {
    if (!(p.begin >= start) || (p.type != tu::AttendanceType::Work &&
                                p.type != tu::AttendanceType::Delivery))
      continue;

    auto minutes = p.end - p.begin;
    total += minutes * minuteWage;
  }

  return total;
}
} // namespace es
