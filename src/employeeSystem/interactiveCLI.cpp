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

#ifdef MCR_BLK_ERR
#error "MCR_BLK_ERR is already defined"
#endif

#define MCR_BLK_ERR(str, shell)                                                \
  do {                                                                         \
    shell->setInputInstruction(str);                                           \
    auto oldPrompt = shell->getPromptText();                                   \
    shell->setPromptText("(Press enter to acknowledge)> ");                    \
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
  termios oldt, newt;
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
  int flags = fcntl(STDIN_FILENO, F_GETFL, 0);
  fcntl(STDIN_FILENO, F_SETFL, flags | O_NONBLOCK);

  char c{};
  while ((c = getchar()) != '\n') {
    if (c == -1) // no input
      continue;

    if ((c == '\b' || c == 127) && ui_.buf.size())
      ui_.buf.pop_back();
    else
      ui_.buf.push_back(c);
  }

  fcntl(STDIN_FILENO, F_SETFL, flags & ~O_NONBLOCK);
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

  if (ui_.buf.size()) {
    auto line = ui_.buf.merge<std::string>();
    ui_.buf.clear();
    return line;
  }

  return "";
}

void Shell::run() {
  greet();

  buildMainMenu(this);

  ScopedThread ui{std::thread{[this]() {
    while (!this->exit_)
      this->renderUserInterface();
    write("\n");
  }}};

  std::string input;
  while (!this->exit_) {
    input = this->readLine();
    if (input == "exit") {
      this->requestExit(); // required to halt async thread
      break;
    }

    std::size_t index{};
    ui_.greeting.clear();

    try {
      index = std::stoul(input);
    } catch (...) {
      MCR_BLK_ERR("'" + input + "' is not a valid number\n", this);
      continue;
    }

    if (!index) {
      MCR_BLK_ERR("'" + input + "' is not a valid index\n", this);
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
      .description = "Set employee absence", .callback = []() {}});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Calculate employee pay", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Edit employee info", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Add employee", .callback = []() {}});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Print payment list", .callback = []() {}});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "List checked-in employees", .callback = []() {}});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "List all employees", .callback = [s]() {
        auto sys = s->getSystem();
        auto emp =
            sys->getEmployeeBy([](Employee const *, PersonnelData const *,
                                  AttendanceData const *) { return true; });
      }});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Select employee", .callback = []() {}});
}

void appendAdminMenuEntries(Shell *s) {
  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Remove employee from system", .callback = []() {}});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Edit system settings", .callback = []() {}});
}

void appendEmployeeMenuEntries(Shell *s) {
  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Check-in", .callback = []() {}});

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

  auto emp = s->getSystem()->getEmployeeById(s->getCurrentEmployeeId());
  s->setPromptText(emp->getEmployeeName() + " " + emp->getEmployeeSurname() +
                   +" (" + to_string(emp->getEmployeeRole()) + ")> ");
}

void appendDriverMenuEntries(Shell *s) {
  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Log beginning of delivery",
                     .callback = [s]() { s->requestExit(); }});

  s->getInterface()->menu.push_back(
      ShellMenuEntry{.description = "Log end of delivery",
                     .callback = [s]() { s->requestExit(); }});
}

void buildEmployeeMenu(Shell *s) {
  s->getInterface()->menu.clear();

  appendEmployeeMenuEntries(s);

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void buildDriverMenu(Shell *s) {
  s->getInterface()->menu.clear();

  appendEmployeeMenuEntries(s);
  appendDriverMenuEntries(s);

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});
}

void buildManagerMenu(Shell *s) {
  s->getInterface()->menu.clear();

  appendGeneralAdminMenuEntries(s);

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});

  s->setPromptText("(Manager)> ");
}

void buildAdminMenu(Shell *s) {
  s->getInterface()->menu.clear();

  appendGeneralAdminMenuEntries(s);
  appendAdminMenuEntries(s);

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});

  s->setPromptText("(Admin)> ");
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

        dp::XMLDataParser parser{path};
        if (auto result = parser.loadData(); result != dp::Result::Success) {
          MCR_BLK_ERR("Error: " + dp::to_string(result) + "\n", s);
          return;
        }

        try {
          auto system = EmployeeSystemFactory::create(&parser);
          s->setSystem(std::move(system));
          buildAdminMenu(s);
          if (!system) {
            MCR_BLK_ERR("Error: Failed to create employee system\n", s);
            return;
          }
        } catch (...) {
          MCR_BLK_ERR("Error: Failed to create employee system\n", s);
          return;
        }
      }});

  s->getInterface()->menu.push_back(ShellMenuEntry{
      .description = "Exit", .callback = [s]() { s->requestExit(); }});

  s->setPromptText("(Admin)> ");
}
} // namespace es
