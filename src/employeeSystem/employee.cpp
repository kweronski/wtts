#include <wtts/employee.hpp>

namespace es {
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
