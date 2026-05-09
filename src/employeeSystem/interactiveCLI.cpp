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

  menu->push_back(
      {.description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void buildAdminMenu(Shell *s) {
  auto menu = s->getMenu();
  menu->clear();

  menu->push_back(
      {.description = "List employees", .callback = [s]() {
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

                     buildEmployeeMenu(s);
                     s->setPromptText(emp->getEmployeeName() + " " +
                                      emp->getEmployeeSurname() + "> ");
                   }});

  menu->push_back(
      {.description = "Exit", .callback = [s]() { s->requestExit(); }});

  s->setPromptText("(admin)> ");
}

void buildMainMenu(Shell *s) {
  auto menu = s->getMenu();
  menu->clear();

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

  s->setPromptText("(wtts)> ");
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
