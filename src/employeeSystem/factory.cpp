#include <wtts/employeeSystemFactory.hpp>

namespace es {
std::unique_ptr<EmployeeSystem>
EmployeeSystemFactory::create(dp::DataParser *parser) {
  auto system = std::make_unique<EmployeeSystem>();

  if (auto r = parser->loadData(); r != dp::Result::Success) {
    throw std::runtime_error{"Factory failed to load data from storage: " +
                             dp::to_string(r)};
  }

  auto identifiers = parser->getEmployeeIdentifiers();
  for (auto const &id : identifiers) {
    // TODO: Use system interface to create and configure employees
  }

  return system;
}
} // namespace es
