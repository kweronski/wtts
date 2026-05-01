#pragma once

#include <wtts/employee.hpp>

namespace es {
class PaymentList {
  struct Record {
    Employee *recipient;
    double value;
  };

public:
  void addPayment(Record);

  void printData() const;

  void updatePayment(Employee const *const, double const newValue);

private:
  std::list<Record> payments_;
};
} // namespace es
