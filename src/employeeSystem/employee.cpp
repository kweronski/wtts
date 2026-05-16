#include <wtts/employee.hpp>
#include <wtts/employeeSystem.hpp>

namespace es {
Result GeneralAdmin::generatePayrollReport(std::string const &id, std::string const &filepath) {
  auto e = system_->getEmployeeById(id);
  if (!e)
    return Result::EmployeeNotFoundError;

  PersonnelData *pd{};
  AttendanceData *ad{};

  Result res = system_->getEmployeeData(&e, &pd, &ad);
  if (res != Result::Success)
    return res;

  return pdfReportGenerator::generatePayrollReport(filepath, pd, ad);
}
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

std::vector<PaymentRecord>
GeneralAdmin::generatePaymentList(tu::TimePeriod const &period) {
  auto employees = system_->getEmployeeBy(
      [](Employee const *, PersonnelData const *pd, AttendanceData const *ad) {
        if (!pd->getEmployeeActive())
          return false;
        for (auto const &r : ad->getRecords()) {
          if (r.type == tu::AttendanceType::Work)
            return true;
        }
        return false;
      });

  std::vector<PaymentRecord> result;
  result.reserve(employees.size());
  for (auto *e : employees)
    result.push_back({.recipient = e, .value = e->calculatePay(period)});
  return result;
}

Result Admin::editSettings(SystemSettings setting, unsigned value) {
  switch (setting) {
  case SystemSettings::AutoCheckoutHour:
    system_->setAutoCheckoutTime(value, system_->getAutoCheckoutTime().second);
    break;
  case SystemSettings::AutoCheckoutMinute:
    system_->setAutoCheckoutTime(system_->getAutoCheckoutTime().first, value);
    break;
  }
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

double Employee::calculatePay(tu::TimePeriod const &period) const {
  auto const hourlyWage = this->getEmployeeHourlyWage();
  auto const minuteWage = double(hourlyWage) / 60.0;
  double total{};

  for (auto const &p : attendance_->getRecords()) {
    bool rangeBeginNOK = !(p.begin >= period.begin);
    bool rangeEndNOK = period.end < p.end;
    if (rangeBeginNOK || rangeEndNOK || p.type != period.type)
      continue;

    auto minutes = p.end - p.begin;
    total += minutes * minuteWage;
  }

  return total;
}
} // namespace es
