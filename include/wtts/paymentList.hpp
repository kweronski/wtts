#pragma once

#include <wtts/employee.hpp>

namespace es {
struct PaymentRecord {
  Employee *recipient;
  double value;
};
} // namespace es
