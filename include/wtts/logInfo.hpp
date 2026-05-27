#pragma once

#include <ctime>
#include <string>

namespace tu {
enum class AttendanceType { Vacation, Work, Delivery, Sick };
std::string to_string(AttendanceType const);

struct TimePoint {
  unsigned year, month, day, hour, minute;

  void populate(); // Fills data members with current time
};

struct TimePeriod {
  TimePoint begin, end;
  AttendanceType type;
};

// Formats a human-readable attendance entry description.
std::string makeAttendanceInstStr(TimePeriod const *);

bool operator>=(TimePoint const &lhs, TimePoint const &rhs);

bool operator<(TimePoint const &lhs, TimePoint const &rhs);

// Returns elapsed time between two points in minutes.
double operator-(TimePoint const &lhs, TimePoint const &rhs);

std::time_t to_time_t(TimePoint const &tp);
} // namespace tu
