#pragma once

#include <string>

namespace tu {
enum class AttendanceType { Vacation, Work, Delivery, Sick };
std::string to_string(AttendanceType const);

struct TimePoint {
  unsigned year, month, day, hour, minute;
};

struct TimePeriod {
  TimePoint begin, end;
  AttendanceType type;
};
} // namespace tu
