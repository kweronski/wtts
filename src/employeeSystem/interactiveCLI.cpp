#include <iomanip>
#include <thread>
#include <wtts/employeeData.hpp>
#include <wtts/employeeSystemFactory.hpp>
#include <wtts/interactiveCLI.hpp>
#include <wtts/xmlParser.hpp>

// For input handling
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>

#ifdef MCR_CNF_LOG
#error "MCR_CNF_LOG is already defined"
#endif

#define MCR_CNF_LOG(str, shell)                                                \
  do {                                                                         \
    std::lock_guard<std::mutex> lock{shell->getConsoleGuardIn()};              \
    shell->setInputInstruction(str);                                           \
    auto oldPrompt = shell->getPromptText();                                   \
    shell->setPromptText("(Press enter)> ");                                   \
    shell->readLine();                                                         \
    shell->setPromptText(oldPrompt);                                           \
    shell->setInputInstruction("");                                            \
  } while (0);

class ScopedThread {
  std::thread t;

public:
  explicit ScopedThread(std::thread t_) : t(std::move(t_)) {}

  ~ScopedThread() {
    if (t.joinable()) {
      t.join(); // automatically join on destruction
    }
  }

  // non-copyable
  ScopedThread(const ScopedThread &) = delete;
  ScopedThread &operator=(const ScopedThread &) = delete;

  // movable
  ScopedThread(ScopedThread &&other) noexcept : t(std::move(other.t)) {}
  ScopedThread &operator=(ScopedThread &&other) noexcept {
    if (t.joinable())
      t.join();
    t = std::move(other.t);
    return *this;
  }
};

std::string getCurrentTime() {
  // 1. Get current time as time_point
  auto now = std::chrono::system_clock::now();

  // 2. Convert to time_t (calendar time)
  std::time_t now_c = std::chrono::system_clock::to_time_t(now);

  // 3. Convert to local time
  std::tm local_tm;
#if defined(_WIN32) || defined(_WIN64)
  localtime_s(&local_tm, &now_c); // thread-safe on Windows
#else
  localtime_r(&now_c, &local_tm); // thread-safe on Linux/macOS
#endif

  // 4. Format into string
  std::ostringstream oss;
  oss << std::put_time(&local_tm, "%Y/%m/%d %H:%M:%S");
  return oss.str();
}

namespace es {
void Shell::greet() {
  ui_.greeting.set("Welcome to WTTS (Work Time Tracking System)\n");
}

std::string Shell::readLine() {
  char c{};
  while ((c = getchar()) != '\n') {
    if (c == -1) // no input
      continue;

    if ((c == '\b' || c == 127) && ui_.buf.size())
      ui_.buf.pop_back();
    else
      ui_.buf.push_back(c);
  }

  if (ui_.buf.size()) {
    auto line = ui_.buf.merge<std::string>();
    ui_.buf.clear();
    return line;
  }

  return "";
}

void Shell::run() {
  termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

  greet();

  buildMainMenu(this);

  ScopedThread ui{std::thread{[this]() {
    while (!this->exit_)
      this->renderUserInterface();
    write("\n");
  }}};

  ScopedThread autoCheckout{std::thread{[this]() {
    while (!this->exit_) {
      std::this_thread::sleep_for(std::chrono::seconds{1});
      this->autoCheckout();
    }
  }}};

  this->handleInput();

  fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

void Shell::autoCheckout() {
  std::lock_guard<std::mutex> lock{systemGuard_};

  if (!system_)
    return;

  std::string message;

  if (auto checkedOut = system_->autoCheckOut(); checkedOut.size()) {
    message += "Checked out the following emploees: \n";

    for (auto employee : checkedOut)
      message += "\t" + employee->getEmployeeName() + " " +
                 employee->getEmployeeSurname() + " " +
                 employee->getEmployeeId() + "\n";

    MCR_CNF_LOG(message, this);
  }
}

void Shell::handleInput() {
  while (!this->exit_) {
    std::size_t index{};
    std::string input;

    {
      std::lock_guard<std::mutex> lock{consoleGuardIn_};
      char c = getchar();
      if (c == -1)
        continue;
      else if (c != '\n' && c != 127 && c != '\b')
        ui_.buf.push_back(c);
      else if ((c == '\b' || c == 127) && ui_.buf.size())
        ui_.buf.pop_back();
      if (c != '\n')
        continue;
    }

    input = ui_.buf.merge<std::string>();
    ui_.buf.clear();

    if (input == "exit") {
      this->requestExit(); // required to halt async thread
      break;
    }

    ui_.greeting.clear();

    try {
      index = std::stoul(input);
    } catch (...) {
      MCR_CNF_LOG("'" + input + "' is not a valid number\n", this);
      continue;
    }

    if (!index) {
      MCR_CNF_LOG("'" + input + "' is not a valid index\n", this);
      continue;
    }

    auto entry = ui_.menu.at<std::size_t>(index - 1);
    entry.callback();
  }
}

void Shell::renderUserInterface() {
  auto const now = std::chrono::system_clock::now();
  if (now - ui_.lastRefresh.get() < ui_.tick.get())
    return;
  ui_.lastRefresh.set(now);

  write("\033[2J\033[H"); // Clear screen: works on ANSI terminals
  write(ui_.greeting.get());

  ui_.date.set(getCurrentTime());
  write("(WTTS) ", ui_.date.get(), "\n");

  for (std::size_t i = 1; i <= ui_.menu.size(); ++i)
    write(i, ". ", ui_.menu.at(i - 1).description, "\n");

  write(ui_.inputInstruction.get());
  write(ui_.prompt.get());
  write(ui_.buf.merge<std::string>());
}

void appendGeneralAdminMenuEntries(Shell *s) {
  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Set employee absence", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();

        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *) { return true; });

        s->pushMenuState();
        buildAbsenceEmployeeSelectionMenu(s, emp);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Calculate employee pay", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Edit employee info", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Add employee", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Print payment list", .callback = []() {}});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "List checked-in employees", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();

        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *a) {
              return a->getCurrentTimePeriod()->begin.year;
            });

        std::string message = "\tChecked in employees: \n";
        for (std::size_t i = 0; i < emp.size(); ++i) {
          auto const employee = emp[i];
          message += "\t" + std::to_string(i + 1) + ". " +
                     employee->getEmployeeName() + " " +
                     employee->getEmployeeSurname() +
                     " ID: " + employee->getEmployeeId() + "\n";
        }

        MCR_CNF_LOG(message, s);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "List all employees", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();
        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *) { return true; });

        std::string message = "\tAll employees: \n";
        for (std::size_t i = 0; i < emp.size(); ++i) {
          auto const employee = emp[i];
          message += "\t" + std::to_string(i + 1) + ". " +
                     employee->getEmployeeName() + " " +
                     employee->getEmployeeSurname() +
                     " ID: " + employee->getEmployeeId() + "\n";
        }

        MCR_CNF_LOG(message, s);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Select employee", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();
        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *) { return true; });

        std::vector<std::pair<std::string, Employee *>> allEmployees;
        for (std::size_t i = 0; i < emp.size(); ++i) {
          auto employee = emp[i];
          allEmployees.push_back({employee->getEmployeeName() + " " +
                                      employee->getEmployeeSurname() + " " +
                                      employee->getEmployeeId(),
                                  employee});
        }

        s->pushMenuState();
        buildEmployeeSelectionMenu(s, allEmployees);
      }});
}

void buildAbsenceEmployeeSelectionMenu(
    Shell *s, std::vector<Employee *> const &employees) {
  s->getInterface()->menu.clear();

  for (auto e : employees) {
    s->getInterface()->menu.push_back(ShellMenuEntry{
        .description = e->getEmployeeName() + " " + e->getEmployeeSurname() +
                       " ID: " + e->getEmployeeId(),
        .callback = [s, e]() {
          std::lock_guard<std::mutex> lock{s->getSystemGuard()};
          auto sys = s->getSystem();

          std::size_t year, month, day;
          tu::TimePoint tp;
          tp.populate();

          if (!readBounded("Enter absence year: \n", &year, tp.year,
                           tp.year + 1, s)) {
            s->popMenuState();
            return;
          }
          if (!readBounded("Enter absence month: \n", &month, 1, 13, s)) {
            s->popMenuState();
            return;
          }
          if (!readBounded("Enter absence day: \n", &day, 1, 32, s)) {
            s->popMenuState();
            return;
          }

          if (auto result = sys->setEmployeeAbsence(
                  e, tu::TimePoint{.year = unsigned(year),
                                   .month = unsigned(month),
                                   .day = unsigned(day),
                                   .hour = 0,
                                   .minute = 0});
              result != Result::Success) {
            MCR_CNF_LOG(
                "Error: Failed to set absence: " + to_string(result) + "\n", s);
          } else
            MCR_CNF_LOG("Successfully set absence data\n", s);

          s->popMenuState();
        }});
  }

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Back", .callback = [s]() { s->popMenuState(); }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void buildEmployeeSelectionMenu(
    Shell *s,
    std::vector<std::pair<std::string, Employee *>> const &employees) {
  s->getInterface()->menu.clear();

  for (auto const &e : employees) {
    s->getInterface()->menu.push_back(ShellMenuEntry{
        .description = e.first, .callback = [s, &e]() {
          s->pushMenuState();
          s->setCurrentEmployeeId(e.second->getEmployeeId());

          switch (e.second->getEmployeeRole()) {
          case EmployeeRole::Employee:
            buildEmployeeMenu(s, true);
            break;
          case EmployeeRole::Driver:
            buildDriverMenu(s, true);
            break;
          case EmployeeRole::Admin:
            buildAdminMenu(s, true);
            break;
          default:
            buildManagerMenu(s, true);
            break;
          }

          s->setPromptText(e.second->getEmployeeName() + " " +
                           e.second->getEmployeeSurname() + +" (" +
                           to_string(e.second->getEmployeeRole()) + ")> ");
        }});
  }

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Back", .callback = [s]() { s->popMenuState(); }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void appendAdminMenuEntries(Shell *s) {
  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Remove employee from system", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Edit system settings", .callback = [s]() {
                       s->pushMenuState();
                       buildSettingsMenu(s);
                     }});
}

bool readBounded(std::string const &prompt, std::size_t *idx, std::size_t begin,
                 std::size_t end, Shell *s) {
  s->setInputInstruction(prompt);
  auto value = s->readLine();
  s->setInputInstruction("");

  std::size_t index{};
  try {
    index = std::stoul(value);
  } catch (...) {
    MCR_CNF_LOG("'" + value + "' is not a valid value\n", s);
    return false;
  }

  if (index < begin || index >= end) {
    MCR_CNF_LOG("'" + value + "' is not within the required range: (" +
                    std::to_string(begin) + ", " + std::to_string(end) + ")\n",
                s);
    return false;
  }

  *idx = index;
  return true;
}

void buildSettingsMenu(Shell *s) {
  s->getInterface()->menu.clear();

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Set auto checkout time", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};

        std::size_t hour, minute;
        if (!readBounded("Enter auto checkout hour: \n", &hour, 0, 25, s))
          return;
        if (!readBounded("Enter auto checkout minute: \n", &minute, 0, 61, s))
          return;

        s->getSystem()->setAutoCheckoutTime(hour, minute);
        s->popMenuState();
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Back", .callback = [s]() { s->popMenuState(); }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void appendEmployeeMenuEntries(Shell *s) {
  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Check-in", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto emp = s->getSystem()->getEmployeeById(s->getCurrentEmployeeId());
        if (auto result = emp->checkIn(); result != Result::Success) {
          MCR_CNF_LOG("Could not check in; Reason: " + to_string(result) + "\n",
                      s);
          return;
        }

        MCR_CNF_LOG("Successfully checked in\n", s);
      }});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Check-out", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Print info", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Calculate pay", .callback = [s]() {
                       // auto sys = s->getSystem();
                       auto id = s->getCurrentEmployeeId();
                       // auto emp = sys->getEmployeeById(id);
                       auto tp = tu::TimePoint{};
                       tp.populate(); // now
                     }});
}

void appendDriverMenuEntries(Shell *s) {
  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Log beginning of delivery",
                     .callback = [s]() { s->requestExit(); }});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Log end of delivery",
                     .callback = [s]() { s->requestExit(); }});
}

void buildEmployeeMenu(Shell *s, bool addJmpToPrev) {
  s->getInterface()->menu.clear();

  appendEmployeeMenuEntries(s);

  if (addJmpToPrev)
    s->getInterface()->menu.push_back(ShellMenuEntry{
        .description = "Back", .callback = [s]() { s->popMenuState(); }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void buildDriverMenu(Shell *s, bool addJmpToPrev) {
  s->getInterface()->menu.clear();

  appendEmployeeMenuEntries(s);
  appendDriverMenuEntries(s);

  if (addJmpToPrev)
    s->getInterface()->menu.push_back(ShellMenuEntry{
        .description = "Back", .callback = [s]() { s->popMenuState(); }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void buildManagerMenu(Shell *s, bool addJmpToPrev) {
  s->getInterface()->menu.clear();

  if (s->getCurrentEmployeeId().size())
    appendEmployeeMenuEntries(s);
  appendGeneralAdminMenuEntries(s);

  if (addJmpToPrev)
    s->getInterface()->menu.push_back(ShellMenuEntry{
        .description = "Back", .callback = [s]() { s->popMenuState(); }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void buildAdminMenu(Shell *s, bool addJmpToPrev) {
  s->getInterface()->menu.clear();

  if (s->getCurrentEmployeeId().size())
    appendEmployeeMenuEntries(s);
  appendGeneralAdminMenuEntries(s);
  appendAdminMenuEntries(s);

  if (addJmpToPrev)
    s->getInterface()->menu.push_back(ShellMenuEntry{
        .description = "Back", .callback = [s]() { s->popMenuState(); }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void buildMainMenu(Shell *s) {
  s->getInterface()->menu.clear();

  appendGeneralAdminMenuEntries(s);
  appendAdminMenuEntries(s);

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Initialize system from disk", .callback = [s]() {
        s->setInputInstruction("Enter path to data storage: \n");
        auto path = s->readLine();
        s->setInputInstruction("");

        try {
          dp::XMLDataParser parser{path};
          auto system = EmployeeSystemFactory::create(&parser);
          s->setSystem(std::move(system));
          buildAdminMenu(s, false);
        } catch (std::exception const &e) {
          MCR_CNF_LOG("Error: employee system: " + std::string{e.what()} + "\n",
                      s);
          return;
        } catch (...) {
          MCR_CNF_LOG("Error: Failed to create employee system: \n", s);
          return;
        }

        MCR_CNF_LOG("Success: Loaded employee system from disk.\n", s);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});

  s->setPromptText("(Admin)> ");
}

void Shell::pushMenuState() {
  MenuState state{};
  state.menu = ui_.menu.get();
  state.currentEmployeeId = getCurrentEmployeeId();
  state.inputInstruction = ui_.inputInstruction.get();
  state.prompt = ui_.prompt.get();

  previousMenus_.push_back(std::move(state));
}

void Shell::popMenuState() {
  if (!previousMenus_.size())
    throw std::runtime_error{
        "Reverting to a previous menu state was requested, "
        "but no previous menu exists"};

  auto state = std::move(previousMenus_.back());
  previousMenus_.pop_back();

  ui_.menu.set(std::move(state.menu));
  setCurrentEmployeeId(std::move(state.currentEmployeeId));
  ui_.inputInstruction.set(std::move(state.inputInstruction));
  ui_.prompt.set(std::move(state.prompt));
}
} // namespace es
