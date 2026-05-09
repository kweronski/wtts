#include <wtts/employeeData.hpp>
#include <wtts/employeeSystemFactory.hpp>
#include <wtts/interactiveCLI.hpp>
#include <wtts/xmlParser.hpp>

namespace es {
void Shell::run() {
  greet();

  buildMainMenu(this);

  while (prompt())
    ;
}

bool Shell::prompt() {
  do {
    for (std::size_t i = 1; i <= menu_.size(); ++i)
      output_ << i << ". " << menu_.at(i - 1).description << std::endl;

    write(prompt_);

    std::size_t const index = readIndex(readLine());

    try {
      if (index)
        menu_.at(index - 1).callback();
    } catch (...) {
      continue;
    }

  } while (!exit_);

  return false;
}

void buildEmployeeMenu(Shell *s) {
  auto menu = s->getMenu();
  menu->clear();

  menu->push_back({.description = "Check-in", .callback = [s]() {}});

  menu->push_back({.description = "Check-out", .callback = [s]() {}});

  menu->push_back({.description = "Print info", .callback = [s]() {}});

  menu->push_back({.description = "Calculate pay", .callback = [s]() {
                     auto sys = s->getSystem();
                     auto id = s->getCurrentEmployeeId();
                     auto emp = sys->getEmployeeById(id);
                     auto tp = tu::TimePoint{};
                     tp.populate(); // now
                     s->write("Current pay: ", emp->calculatePay(tp), "\n");
                   }});

  menu->push_back({.description = "Exit to admin",
                   .callback = [s]() { buildAdminMenu(s); }});

  menu->push_back(
      {.description = "Exit", .callback = [s]() { s->requestExit(); }});

  auto emp = s->getSystem()->getEmployeeById(s->getCurrentEmployeeId());
  s->setPromptText(emp->getEmployeeName() + " " + emp->getEmployeeSurname() +
                   +" (" + to_string(emp->getEmployeeRole()) + ")> ");
}

void buildGeneralAdminMenu(Shell *s) {
  auto menu = s->getMenu();
  menu->clear();

  menu->push_back(
      {.description = "Set employee absence", .callback = [s]() {}});

  menu->push_back(
      {.description = "Calculate employee pay", .callback = [s]() {}});

  menu->push_back({.description = "Edit employee info", .callback = [s]() {}});

  menu->push_back({.description = "Add employee", .callback = [s]() {}});

  menu->push_back({.description = "Print payment list", .callback = [s]() {}});

  menu->push_back(
      {.description = "List checked-in employees", .callback = [s]() {}});

  menu->push_back(
      {.description = "List all employees", .callback = [s]() {
         auto sys = s->getSystem();
         auto emp =
             sys->getEmployeeBy([](Employee const *e, PersonnelData const *,
                                   AttendanceData const *) { return true; });

         for (auto e : emp)
           s->write(e->getEmployeeName(), " ", e->getEmployeeSurname(), " ",
                    e->getEmployeeId(), "\n");
       }});

  menu->push_back({.description = "Select employee", .callback = [s]() {
                     auto const sys = s->getSystem();
                     s->write("Enter employee ID: ");
                     auto const id = s->readLine();
                     auto const emp = sys->getEmployeeById(id);

                     if (!emp) {
                       s->write("Did not find employee with id: ", id, "\n");
                       return;
                     }

                     s->setCurrentEmployeeId(id);
                     buildEmployeeMenu(s);
                   }});
}

void buildManagerMenu(Shell *s) {
  buildGeneralAdminMenu(s);
  auto menu = s->getMenu();

  menu->push_back(
      {.description = "Exit", .callback = [s]() { s->requestExit(); }});

  s->setPromptText("(Manager)> ");
}

void buildAdminMenu(Shell *s) {
  buildGeneralAdminMenu(s);
  auto menu = s->getMenu();

  menu->push_back(
      {.description = "Remove employee from system", .callback = [s]() {}});

  menu->push_back(
      {.description = "Edit system settings", .callback = [s]() {}});

  menu->push_back(
      {.description = "Exit", .callback = [s]() { s->requestExit(); }});

  s->setPromptText("(Admin)> ");
}

void buildMainMenu(Shell *s) {
  buildAdminMenu(s);
  auto menu = s->getMenu();

  menu->push_back(
      {.description = "Initialize system from disk", .callback = [s]() {
         s->write("Enter path to file: ");
         auto path = s->readLine();
         auto parser = std::make_unique<dp::XMLDataParser>(path);
         try {
           auto system = EmployeeSystemFactory::create(parser.get());
           s->setSystem(std::move(system));
           buildAdminMenu(s);
         } catch (std::exception const &e) {
           s->write(e.what(), "\n");
         }
       }});

  menu->push_back(
      {.description = "Exit", .callback = [s]() { s->requestExit(); }});
}

std::size_t Shell::readIndex(std::string const &str) {
  try {
    return std::stoul(str);
  } catch (...) {
    return 0;
  }
}

std::string Shell::readLine() {
  std::string line;
  std::getline(input_, line);
  return line;
}

void Shell::greet() {
  write("Welcome to WTTS (Work Time Tracking System)", "\n");
}
} // namespace es
