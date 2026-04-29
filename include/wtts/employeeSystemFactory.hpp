#pragma once

#include <wtts/dataParser.hpp>
#include <wtts/employeeSystem.hpp>

namespace es {
class EmployeeSystemFactory {
public:
  static std::unique_ptr<EmployeeSystem> create(dp::DataParser *parser);
};
} // namespace es
