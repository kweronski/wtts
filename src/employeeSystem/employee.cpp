#include <wtts/employee.hpp>

namespace es {
Result Employee::checkIn() {
  tu::TimePoint tp;
  tp.populate();
  attendance_->getCurrentTimePeriod()->begin = std::move(tp);
  return Result::Success;
}

Result Employee::checkOut() {
  tu::TimePoint tp;
  tp.populate();
  attendance_->getCurrentTimePeriod()->end = std::move(tp);
  attendance_->getCurrentTimePeriod()->type = tu::AttendanceType::Work;
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
