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
std::string formatEmployeeRecord(Employee const *e) {
  return e->getEmployeeName() + " " + e->getEmployeeSurname() +
         " ID: " + e->getEmployeeId() + " (" + to_string(e->getEmployeeRole()) +
         ")";
}

std::string formatEmployeeRecord(std::size_t index, Employee const *e) {
  return std::to_string(index) + ". " + formatEmployeeRecord(e);
}

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
      message += "\t" + formatEmployeeRecord(employee) + "\n";

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
      .description = "Calculate employee pay", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();

        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *) { return true; });

        if (emp.empty()) {
          MCR_CNF_LOG("No employees found in the system\n", s);
          return;
        }

        s->pushMenuState();

        s->getInterface()->menu.clear();

        for (auto e : emp) {
          s->getInterface()->menu.push_back(ShellMenuEntry{
              .description = e->getEmployeeName() + " " +
                             e->getEmployeeSurname() +
                             " ID: " + e->getEmployeeId(),
              .callback = [s, e]() {
                std::size_t year, month;
                tu::TimePoint tp;
                tp.populate();

                if (!readBounded("Enter start year: \n", &year, 1970,
                                 tp.year + 1, s)) {
                  s->popMenuState();
                  return;
                }
                if (!readBounded("Enter start month: \n", &month, 1, 13, s)) {
                  s->popMenuState();
                  return;
                }

                tu::TimePoint start{.year = unsigned(year),
                                    .month = unsigned(month),
                                    .day = 1,
                                    .hour = 0,
                                    .minute = 0};

                auto pay = e->calculatePay(start);

                std::ostringstream oss;
                oss << std::fixed << std::setprecision(2);
                oss << "Calculated pay for " << e->getEmployeeName() << " "
                    << e->getEmployeeSurname() << ": " << pay << "\n";

                MCR_CNF_LOG(oss.str(), s);
                s->popMenuState();
              }});
        }

        s->getInterface()->menu.push_back(ShellMenuEntry{
            .description = "Back", .callback = [s]() { s->popMenuState(); }});

        s->getInterface()->menu.push_back(ShellMenuEntry{
            .description = "Exit", .callback = [s]() { s->requestExit(); }});
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Edit employee info", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();

        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *) { return true; });

        if (emp.empty()) {
          MCR_CNF_LOG("No employees found in the system\n", s);
          return;
        }

        s->pushMenuState();
        s->getInterface()->menu.clear();

        for (auto e : emp) {
          s->getInterface()->menu.push_back(ShellMenuEntry{
              .description = e->getEmployeeName() + " " +
                             e->getEmployeeSurname() +
                             " ID: " + e->getEmployeeId(),
              .callback = [s, e]() {
                std::lock_guard<std::mutex> lock{s->getSystemGuard()};
                auto sys = s->getSystem();

                auto activeEmp =
                    sys->getEmployeeById(s->getCurrentEmployeeId());

                if (!activeEmp) {
                  MCR_CNF_LOG("Error: Could not resolve active user\n", s);
                  return;
                }

                GeneralAdmin *admin = nullptr;
                switch (activeEmp->getEmployeeRole()) {
                case EmployeeRole::Manager:
                  admin = dynamic_cast<Manager *>(activeEmp);
                  break;
                case EmployeeRole::Admin:
                  admin = dynamic_cast<Admin *>(activeEmp);
                  break;
                default:
                  MCR_CNF_LOG(
                      "Error: Active user does not have edit privileges\n", s);
                  return;
                }

                if (!admin) {
                  MCR_CNF_LOG("Error: Failed to resolve admin interface\n", s);
                  return;
                }

                s->pushMenuState();
                buildEditEmployeeMenu(s, e, admin);
              }});
        }

        s->getInterface()->menu.push_back(ShellMenuEntry{
            .description = "Back", .callback = [s]() { s->popMenuState(); }});

        s->getInterface()->menu.push_back(ShellMenuEntry{
            .description = "Exit", .callback = [s]() { s->requestExit(); }});
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Add employee", .callback = [s]() {
        // --- Collect string fields ---
        auto readString = [s](std::string const &prompt) -> std::string {
          s->setInputInstruction(prompt);
          auto val = s->readLine();
          s->setInputInstruction("");
          return val;
        };

        std::string name = readString("Enter employee name: \n");
        std::string surname = readString("Enter employee surname: \n");
        std::string phone = readString("Enter employee telephone: \n");
        std::string email = readString("Enter employee email: \n");
        std::string cardId = readString("Enter employee card ID: \n");
        std::string empId = readString("Enter employee ID: \n");

        // --- Collect numeric fields ---
        std::size_t stdWork{}, maxWork{}, wage{};

        if (!readBounded("Enter standard work time (hours/day): \n", &stdWork,
                         1, 169, s))
          return;
        if (!readBounded("Enter max work time (hours/day): \n", &maxWork, 1,
                         169, s))
          return;
        if (!readBounded("Enter hourly wage: \n", &wage, 1, 100001, s))
          return;

        // --- Role selection ---
        std::size_t roleIdx{};
        if (!readBounded("Select role: \n"
                         "\t1. Employee\n"
                         "\t2. Driver\n"
                         "\t3. Manager\n"
                         "\t4. Admin\n",
                         &roleIdx, 1, 5, s))
          return;

        EmployeeRole role{};
        switch (roleIdx) {
        case 1:
          role = EmployeeRole::Employee;
          break;
        case 2:
          role = EmployeeRole::Driver;
          break;
        case 3:
          role = EmployeeRole::Manager;
          break;
        case 4:
          role = EmployeeRole::Admin;
          break;
        }

        // --- Active status ---
        std::size_t activeIdx{};
        if (!readBounded("Set employee as active? \n\t1. Yes\n\t2. No\n",
                         &activeIdx, 1, 3, s))
          return;
        bool active = (activeIdx == 1);

        // --- Add to system ---
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();

        PersonnelData *pd{};
        AttendanceData *ad{};
        Result result{};

        switch (roleIdx) {
        case 1: {
          Employee *e{};
          result = sys->addEmployee(&e, &pd, &ad);
          break;
        }
        case 2: {
          Driver *e{};
          result = sys->addEmployee(&e, &pd, &ad);
          break;
        }
        case 3: {
          Manager *e{};
          result = sys->addEmployee(&e, &pd, &ad);
          break;
        }
        case 4: {
          Admin *e{};
          result = sys->addEmployee(&e, &pd, &ad);
          break;
        }
        }

        if (result != Result::Success) {
          MCR_CNF_LOG(
              "Error: Failed to add employee: " + to_string(result) + "\n", s);
          return;
        }

        if (!pd) {
          MCR_CNF_LOG("Error: Personnel data pointer was not assigned\n", s);
          return;
        }

        // --- Populate PersonnelData via returned pointer ---
        pd->setEmployeeName(name);
        pd->setEmployeeSurname(surname);
        pd->setEmployeeTelephone(phone);
        pd->setEmployeeEmail(email);
        pd->setEmployeeCardId(cardId);
        pd->setEmployeeId(empId);
        pd->setEmployeeStandardWorkTime(static_cast<unsigned>(stdWork));
        pd->setEmployeeMaxWorkTime(static_cast<unsigned>(maxWork));
        pd->setEmployeeHourlyWage(static_cast<unsigned>(wage));
        pd->setEmployeeRole(role);
        pd->setEmployeeActive(active);

        std::ostringstream oss;
        oss << "Successfully added employee:\n"
            << "\tName:                " << name << " " << surname << "\n"
            << "\tID:                  " << empId << "\n"
            << "\tCard ID:             " << cardId << "\n"
            << "\tTelephone:           " << phone << "\n"
            << "\tEmail:               " << email << "\n"
            << "\tRole:                " << to_string(role) << "\n"
            << "\tStandard work time:  " << stdWork << "h/week\n"
            << "\tMax work time:       " << maxWork << "h/week\n"
            << "\tHourly wage:         " << wage << "\n"
            << "\tActive:              " << (active ? "Yes" : "No") << "\n";

        MCR_CNF_LOG(oss.str(), s);
      }});

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

        std::string message = "Checked in employees: \n";
        for (std::size_t i = 0; i < emp.size(); ++i)
          message += "\t" + formatEmployeeRecord(i + 1, emp[i]) + "\n";

        MCR_CNF_LOG(message, s);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "List all employees", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();
        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *) { return true; });

        std::string message = "All employees: \n";
        for (std::size_t i = 0; i < emp.size(); ++i)
          message += "\t" + formatEmployeeRecord(i + 1, emp[i]) + "\n";

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
        for (std::size_t i = 0; i < emp.size(); ++i)
          allEmployees.push_back({formatEmployeeRecord(emp[i]), emp[i]});

        s->pushMenuState();
        buildEmployeeSelectionMenu(s, allEmployees);
      }});
}

void buildEditEmployeeMenu(Shell *s, Employee *e, GeneralAdmin *admin,
                           std::shared_ptr<PersonnelData>) {
  // Working copy — all edits staged here until Save is selected
  auto sys = s->getSystem();
  auto pd = sys->getEmployeeInfo(e);
  if (!pd) {
    MCR_CNF_LOG("Error: Could not retrieve employee data\n", s);
    return;
  }

  auto staged = std::make_shared<PersonnelData>(*pd); // copy current state

  auto rebuild = [s, e, admin, staged]() {
    buildEditEmployeeMenu(s, e, admin, staged);
  };

  s->getInterface()->menu.clear();

  auto makeStringEntry =
      [s, staged, rebuild](std::string const &description,
                           std::string (PersonnelData::*getter)() const,
                           void (PersonnelData::*setter)(std::string const &)) {
        return ShellMenuEntry{
            .description = description + " [" + (staged.get()->*getter)() + "]",
            .callback = [s, staged, description, setter, rebuild]() {
              s->setInputInstruction("Enter new " + description + ": \n");
              auto val = s->readLine();
              s->setInputInstruction("");
              if (val.empty()) {
                MCR_CNF_LOG("No changes made to " + description + "\n", s);
                return;
              }
              (staged.get()->*setter)(val);
              MCR_CNF_LOG("Staged update to " + description + "\n", s);
            }};
      };

  auto makeNumericEntry = [s, staged](std::string const &description,
                                      unsigned (PersonnelData::*getter)() const,
                                      void (PersonnelData::*setter)(unsigned),
                                      std::size_t begin, std::size_t end) {
    return ShellMenuEntry{
        .description = description + " [" +
                       std::to_string((staged.get()->*getter)()) + "]",
        .callback = [s, staged, description, setter, begin, end]() {
          std::size_t val{};
          if (!readBounded("Enter new " + description + ": \n", &val, begin,
                           end, s))
            return;
          (staged.get()->*setter)(static_cast<unsigned>(val));
          MCR_CNF_LOG("Staged update to " + description + "\n", s);
        }};
  };

  s->getInterface()->menu.push_back(
      makeStringEntry("Name", &PersonnelData::getEmployeeName,
                      &PersonnelData::setEmployeeName));
  s->getInterface()->menu.push_back(
      makeStringEntry("Surname", &PersonnelData::getEmployeeSurname,
                      &PersonnelData::setEmployeeSurname));
  s->getInterface()->menu.push_back(
      makeStringEntry("Telephone", &PersonnelData::getEmployeeTelephone,
                      &PersonnelData::setEmployeeTelephone));
  s->getInterface()->menu.push_back(
      makeStringEntry("Email", &PersonnelData::getEmployeeEmail,
                      &PersonnelData::setEmployeeEmail));
  s->getInterface()->menu.push_back(
      makeStringEntry("Card ID", &PersonnelData::getEmployeeCardId,
                      &PersonnelData::setEmployeeCardId));
  s->getInterface()->menu.push_back(
      makeStringEntry("Employee ID", &PersonnelData::getEmployeeId,
                      &PersonnelData::setEmployeeId));
  s->getInterface()->menu.push_back(
      makeNumericEntry("Standard work time (h/week)",
                       &PersonnelData::getEmployeeStandardWorkTime,
                       &PersonnelData::setEmployeeStandardWorkTime, 1, 169));
  s->getInterface()->menu.push_back(makeNumericEntry(
      "Max work time (h/week)", &PersonnelData::getEmployeeMaxWorkTime,
      &PersonnelData::setEmployeeMaxWorkTime, 1, 169));
  s->getInterface()->menu.push_back(
      makeNumericEntry("Hourly wage", &PersonnelData::getEmployeeHourlyWage,
                       &PersonnelData::setEmployeeHourlyWage, 1, 100001));

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Role [" + to_string(staged->getEmployeeRole()) + "]",
      .callback = [s, staged]() {
        std::size_t roleIdx{};
        if (!readBounded("Select new role: \n"
                         "\t1. Employee\n"
                         "\t2. Driver\n"
                         "\t3. Manager\n"
                         "\t4. Admin\n",
                         &roleIdx, 1, 5, s))
          return;
        EmployeeRole role{};
        switch (roleIdx) {
        case 1:
          role = EmployeeRole::Employee;
          break;
        case 2:
          role = EmployeeRole::Driver;
          break;
        case 3:
          role = EmployeeRole::Manager;
          break;
        case 4:
          role = EmployeeRole::Admin;
          break;
        }
        staged->setEmployeeRole(role);
        MCR_CNF_LOG("Staged update to Role\n", s);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = std::string{"Active ["} +
                     (staged->getEmployeeActive() ? "Yes" : "No") + "]",
      .callback = [s, staged]() {
        std::size_t activeIdx{};
        if (!readBounded("Set active status: \n"
                         "\t1. Yes\n"
                         "\t2. No\n",
                         &activeIdx, 1, 3, s))
          return;
        staged->setEmployeeActive(activeIdx == 1);
        MCR_CNF_LOG("Staged update to Active status\n", s);
      }});

  // Save — this is where GeneralAdmin::editEmployee does its work
  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Save changes", .callback = [s, e, admin, staged]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto result = admin->editEmployee(e, *staged);
        if (result != Result::Success) {
          MCR_CNF_LOG(
              "Error: Failed to save changes: " + to_string(result) + "\n", s);
          return;
        }
        MCR_CNF_LOG("Changes saved successfully\n", s);
        s->popMenuState();
      }});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Discard & back", .callback = [s]() {
                       MCR_CNF_LOG("Changes discarded\n", s);
                       s->popMenuState();
                     }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
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

  for (auto e : employees) {
    s->getInterface()->menu.push_back(ShellMenuEntry{
        .description = e.first, .callback = [s, e]() {
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
      .description = "Remove employee from system", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto sys = s->getSystem();

        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *) { return true; });

        if (emp.empty()) {
          MCR_CNF_LOG("No employees found in the system\n", s);
          return;
        }

        s->pushMenuState();
        s->getInterface()->menu.clear();

        for (auto e : emp) {
          s->getInterface()->menu.push_back(ShellMenuEntry{
              .description = e->getEmployeeName() + " " +
                             e->getEmployeeSurname() +
                             " ID: " + e->getEmployeeId(),
              .callback = [s, e]() {
                std::lock_guard<std::mutex> lock{s->getSystemGuard()};
                auto sys = s->getSystem();

                // Confirm before removing
                std::size_t confirmIdx{};
                if (!readBounded("Are you sure you want to remove " +
                                     e->getEmployeeName() + " " +
                                     e->getEmployeeSurname() +
                                     "?\n"
                                     "\t1. Yes\n"
                                     "\t2. No\n",
                                 &confirmIdx, 1, 3, s)) {
                  s->popMenuState();
                  return;
                }

                if (confirmIdx == 2) {
                  s->popMenuState();
                  return;
                }

                auto result = sys->removeEmployee(e->getEmployeeId());

                if (result != Result::Success) {
                  MCR_CNF_LOG("Error: Failed to remove employee: " +
                                  to_string(result) + "\n",
                              s);
                  s->popMenuState();
                  return;
                }

                MCR_CNF_LOG(
                    "Successfully removed employee: " + e->getEmployeeName() +
                        " " + e->getEmployeeSurname() +
                        " ID: " + e->getEmployeeId() + "\n",
                    s);
                s->popMenuState();
              }});
        }

        s->getInterface()->menu.push_back(ShellMenuEntry{
            .description = "Back", .callback = [s]() { s->popMenuState(); }});

        s->getInterface()->menu.push_back(ShellMenuEntry{
            .description = "Exit", .callback = [s]() { s->requestExit(); }});
      }});

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

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Check-out", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto emp = s->getSystem()->getEmployeeById(s->getCurrentEmployeeId());
        if (auto result = emp->checkOut(); result != Result::Success) {
          MCR_CNF_LOG(
              "Could not check out; Reason: " + to_string(result) + "\n", s);
          return;
        }
        MCR_CNF_LOG("Successfully checked out\n", s);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Print info", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto emp = s->getSystem()->getEmployeeById(s->getCurrentEmployeeId());

        if (!emp) {
          MCR_CNF_LOG("Error: Employee not found\n", s);
          return;
        }

        std::ostringstream oss;
        oss << "Employee info:\n"
            << "\tName:               " << emp->getEmployeeName() << " "
            << emp->getEmployeeSurname() << "\n"
            << "\tID:                 " << emp->getEmployeeId() << "\n"
            << "\tCard ID:            " << emp->getEmployeeCardId() << "\n"
            << "\tTelephone:          " << emp->getEmployeeTelephone() << "\n"
            << "\tEmail:              " << emp->getEmployeeEmail() << "\n"
            << "\tRole:               " << to_string(emp->getEmployeeRole())
            << "\n"
            << "\tStandard work time: " << emp->getEmployeeStandardWorkTime()
            << "h/week\n"
            << "\tMax work time:      " << emp->getEmployeeMaxWorkTime()
            << "h/week\n"
            << "\tHourly wage:        " << emp->getEmployeeHourlyWage() << "\n"
            << "\tActive:             "
            << (emp->getEmployeeActive() ? "Yes" : "No") << "\n";

        MCR_CNF_LOG(oss.str(), s);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Calculate pay", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto emp = s->getSystem()->getEmployeeById(s->getCurrentEmployeeId());
        if (!emp) {
          MCR_CNF_LOG("Error: Employee not found\n", s);
          return;
        }

        std::size_t year, month;
        tu::TimePoint tp;
        tp.populate();

        if (!readBounded("Enter start year: \n", &year, 1970, tp.year + 1, s))
          return;
        if (!readBounded("Enter start month: \n", &month, 1, 13, s))
          return;

        tu::TimePoint start{.year = unsigned(year),
                            .month = unsigned(month),
                            .day = 1,
                            .hour = 0,
                            .minute = 0};

        auto pay = emp->calculatePay(start);

        std::ostringstream oss;
        oss << std::fixed << std::setprecision(2);
        oss << "Calculated pay for " << emp->getEmployeeName() << " "
            << emp->getEmployeeSurname() << ": " << pay << "\n";

        MCR_CNF_LOG(oss.str(), s);
      }});
}

void appendDriverMenuEntries(Shell *s) {
  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Log beginning of delivery", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto emp = s->getSystem()->getEmployeeById(s->getCurrentEmployeeId());
        if (!emp) {
          MCR_CNF_LOG("Error: Employee not found\n", s);
          return;
        }
        auto driver = dynamic_cast<Driver *>(emp);
        if (!driver) {
          MCR_CNF_LOG("Error: Employee is not a driver\n", s);
          return;
        }
        if (auto result = driver->logDeliveryBegin();
            result != Result::Success) {
          MCR_CNF_LOG("Could not log delivery begin; Reason: " +
                          to_string(result) + "\n",
                      s);
          return;
        }
        MCR_CNF_LOG("Successfully logged beginning of delivery\n", s);
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Log end of delivery", .callback = [s]() {
        std::lock_guard<std::mutex> lock{s->getSystemGuard()};
        auto emp = s->getSystem()->getEmployeeById(s->getCurrentEmployeeId());
        if (!emp) {
          MCR_CNF_LOG("Error: Employee not found\n", s);
          return;
        }
        auto driver = dynamic_cast<Driver *>(emp);
        if (!driver) {
          MCR_CNF_LOG("Error: Employee is not a driver\n", s);
          return;
        }
        if (auto result = driver->logDeliveryEnd(); result != Result::Success) {
          MCR_CNF_LOG(
              "Could not log delivery end; Reason: " + to_string(result) + "\n",
              s);
          return;
        }
        MCR_CNF_LOG("Successfully logged end of delivery\n", s);
      }});
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
