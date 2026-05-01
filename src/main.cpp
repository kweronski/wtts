#include <iostream>
#include <wtts/employeeSystemFactory.hpp>
#include <wtts/xmlParser.hpp>

int main(int argc, char **argv) {
  if (argc < 2) {
    std::cerr << "Too few arguments; Provide path to data storage" << std::endl;
    return 1;
  }

  try {
    auto parser = std::make_unique<dp::XMLDataParser>(argv[1]);
    auto system = es::EmployeeSystemFactory::create(parser.get());

    system->printData();
  }

  catch (std::exception const &e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return 1;
  }
}
