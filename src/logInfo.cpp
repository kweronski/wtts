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
} // namespace tu
