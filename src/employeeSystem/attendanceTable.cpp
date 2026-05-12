#include <wtts/attendanceTable.hpp>

namespace es {

std::vector<AttendanceTable::Record>
AttendanceTable::getRecords(tu::TimePeriod const &range) const {
  std::vector<Record> result;

  for (auto const &p : data_->getRecords()) {
    if (p.begin >= range.begin && range.end >= p.begin)
      result.push_back({p.begin, p.end, p.type});
  }

  return result;
}

std::string to_string(AttendanceTable::Record const &r) {
  tu::TimePeriod p{r.begin, r.end, r.type};
  return tu::makeAttendanceInstStr(&p);
}

} // namespace es
