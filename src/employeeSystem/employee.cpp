#include <wtts/employee.hpp>
#include <wtts/employeeSystem.hpp>

namespace es {
void Employee::updateCurrentTimePeriodBegin(tu::TimePoint tp) {
  attendance_->getCurrentTimePeriod()->begin = std::move(tp);
}

void Employee::updateCurrentTimePeriodEnd(tu::TimePoint tp,
                                          tu::AttendanceType at) {
  attendance_->getCurrentTimePeriod()->end = std::move(tp);
  attendance_->getCurrentTimePeriod()->type = at;
  attendance_->addTimePeriod(std::move(*attendance_->getCurrentTimePeriod()));
  *attendance_->getCurrentTimePeriod() = {};
}

void Driver::updateCurrentDeliveryTimePeriodBegin(tu::TimePoint tp) {
  attendance_->getCurrentDeliveryTimePeriod()->begin = std::move(tp);
}

void Driver::updateCurrentDeliveryTimePeriodEnd(tu::TimePoint tp,
                                                tu::AttendanceType at) {
  attendance_->getCurrentDeliveryTimePeriod()->end = std::move(tp);
  attendance_->getCurrentDeliveryTimePeriod()->type = at;
  attendance_->addTimePeriod(
      std::move(*attendance_->getCurrentDeliveryTimePeriod()));
  *attendance_->getCurrentDeliveryTimePeriod() = {};
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

  updatePersonnelData(pd, new_pd);

  return Result::Success;
}

void GeneralAdmin::updatePersonnelData(PersonnelData const &newData,
                                       PersonnelData *oldData) {

  *oldData = newData;
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

  updatePersonnelData(newData, pd);

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

  updateCurrentDeliveryTimePeriodBegin(tp);
  return Result::Success;
}

Result Driver::logDeliveryEnd() {
  if (!attendance_->getCurrentDeliveryTimePeriod()->begin.year)
    return Result::DeliveryNotCheckedIn;

  tu::TimePoint tp;
  tp.populate();

  updateCurrentDeliveryTimePeriodEnd(tp, tu::AttendanceType::Delivery);
  return Result::Success;
}

Result Employee::checkIn() {
  if (attendance_->getCurrentTimePeriod()->begin.year)
    return Result::EmployeeAlreadyCheckedIn;

  tu::TimePoint tp;
  tp.populate();

  updateCurrentTimePeriodBegin(std::move(tp));
  return Result::Success;
}

Result Employee::checkOut() {
  if (!attendance_->getCurrentTimePeriod()->begin.year)
    return Result::EmployeeNotCheckedIn;

  tu::TimePoint tp;
  tp.populate();

  updateCurrentTimePeriodEnd(std::move(tp), tu::AttendanceType::Work);
  return Result::Success;
}

bool Employee::recordQualifiesForPayment(tu::TimePoint const &offset,
                                         tu::TimePoint const &tp,
                                         tu::AttendanceType const type) const {
  return tp >= offset && (type == tu::AttendanceType::Work ||
                          type == tu::AttendanceType::Delivery);
}

bool Employee::recordQualifiesForPayment(tu::TimePeriod const &interval,
                                         tu::TimePeriod const &tp) const {
  return tp.begin >= interval.begin && tp.end < interval.end &&
         tp.type == interval.type;
}

void Employee::updateTotalPay(tu::TimePeriod const &period,
                              double &total) const {
  auto const hourlyWage = this->getEmployeeHourlyWage();
  auto const minuteWage = double(hourlyWage) / 60.0;
  total += (period.end - period.begin) * minuteWage;
}

double Employee::calculatePay(tu::TimePoint const &start) const {
  double total{};

  for (auto const &p : attendance_->getRecords())
    if (recordQualifiesForPayment(start, p.begin, p.type))
      updateTotalPay(p, total);

  return total;
}

double Employee::calculatePay(tu::TimePeriod const &period) const {
  double total{};

  for (auto const &p : attendance_->getRecords())
    if (recordQualifiesForPayment(period, p))
      updateTotalPay(p, total);

  return total;
}
} // namespace es
