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

    es::Employee *e;
    es::AttendanceData *a;
    es::PersonnelData *p;

    system->addEmployee(&e, &p, &a);

    p->setEmployeeHourlyWage(60);

    tu::TimePeriod p1{
        .begin =
            tu::TimePoint{
                .year = 2026, .month = 1, .day = 4, .hour = 8, .minute = 30},
        .end =
            tu::TimePoint{
                .year = 2026, .month = 1, .day = 4, .hour = 16, .minute = 30},
        .type = tu::AttendanceType::Work};

    a->addTimePeriod(p1);

    std::cout << "Calculated pay: " << e->calculatePay(p1.begin) << std::endl;
  }

  catch (std::exception const &e) {
    std::cerr << "Caught exception: " << e.what() << std::endl;
    return 1;
  } catch (...) {
    std::cerr << "Caught unknown exception" << std::endl;
    return 1;
  }
}
