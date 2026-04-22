#include <stdexcept>
#include <wtts/employeeSystem.hpp>
#include <wtts/xmlParser.hpp>

namespace es {
std::unique_ptr<EmployeeSystem>
EmployeeSystemFactory::createFromXML(std::string const &filePath) {

  dp::XMLDataParser parser;
  if (auto result = parser.loadData(filePath); result != dp::Result::Success)
    throw std::runtime_error{"Failed to parse employee data: " +
                             dp::to_string(result)};

  auto system = std::make_unique<EmployeeSystem>();

  /* TODO: Use DataParser interface to construct employee objects
   * and add them to the employee system */

  return system;
}
} // namespace es
