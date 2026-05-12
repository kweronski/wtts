#pragma once

#include <string>
#include <vector>
#include <wtts/employeeData.hpp>
#include <wtts/logInfo.hpp>

namespace es {

class AttendanceTable {
public:
  struct Record {
    tu::TimePoint begin;
    tu::TimePoint end;
    tu::AttendanceType type;
  };

  AttendanceTable(AttendanceData const *data) : data_{data} {}

  std::vector<Record> getRecords(tu::TimePeriod const &range) const;

private:
  AttendanceData const *data_;
};

std::string to_string(AttendanceTable::Record const &r);

} // namespace es
