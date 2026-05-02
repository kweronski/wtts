#pragma once

#include <wtts/employee.hpp>

namespace es {
class AbsenceTable {
public:
  void printData(tu::TimePeriod const &,
                 std::vector<Employee *> const *const) const;

private:
  EmployeeSystem *system_;
};
} // namespace es
