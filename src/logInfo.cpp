#include <chrono>
#include <wtts/logInfo.hpp>

namespace tu {
std::string to_string(AttendanceType const code) {
  std::string value;

  switch (code) {
  case AttendanceType::Vacation:
    value = "Vacation";
    break;
  case AttendanceType::Work:
    value = "Work";
    break;
  case AttendanceType::Delivery:
    value = "Delivery";
    break;
  case AttendanceType::Sick:
    value = "Sick";
    break;
  }

  return value;
}

std::string makeAttendanceInstStr(TimePeriod const *p) {
  return to_string(p->type) + " (" + std::to_string(p->begin.year) + "/" +
         std::to_string(p->begin.month) + "/" + std::to_string(p->begin.day) +
         ", " + std::to_string(p->begin.hour) + ":" +
         std::to_string(p->begin.minute) + ") - " + "(" +
         std::to_string(p->end.year) + "/" + std::to_string(p->end.month) +
         "/" + std::to_string(p->end.day) + ", " + std::to_string(p->end.hour) +
         ":" + std::to_string(p->end.minute) + ")";
}

void TimePoint::populate() {
  auto now = std::chrono::system_clock::now();
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);
  std::tm *tm_ptr = std::localtime(&now_c);

  year = tm_ptr->tm_year + 1900;
  month = tm_ptr->tm_mon + 1;
  day = tm_ptr->tm_mday;
  hour = tm_ptr->tm_hour;
  minute = tm_ptr->tm_min;
}

bool operator>=(const TimePoint &lhs, const TimePoint &rhs) {
  if (lhs.year != rhs.year)
    return lhs.year > rhs.year;
  if (lhs.month != rhs.month)
    return lhs.month > rhs.month;
  if (lhs.day != rhs.day)
    return lhs.day > rhs.day;
  if (lhs.hour != rhs.hour)
    return lhs.hour > rhs.hour;
  return lhs.minute >= rhs.minute;
}

bool operator<(const TimePoint &lhs, const TimePoint &rhs) {
  return !(lhs >= rhs);
}

std::time_t to_time_t(const TimePoint &tp) {
  std::tm tm{};
  tm.tm_year = tp.year - 1900;
  tm.tm_mon = tp.month - 1;
  tm.tm_mday = tp.day;
  tm.tm_hour = tp.hour;
  tm.tm_min = tp.minute;
  tm.tm_sec = 0;

  return std::mktime(&tm);
}

double operator-(TimePoint const &lhs, TimePoint const &rhs) {
  std::time_t t1 = to_time_t(lhs);
  std::time_t t2 = to_time_t(rhs);

  double diff_minutes = std::difftime(t1, t2) / 60.0;
  return diff_minutes;
}
} // namespace tu
