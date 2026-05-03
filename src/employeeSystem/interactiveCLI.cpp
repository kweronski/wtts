#include <wtts/interactiveCLI.hpp>

namespace es {
void Shell::run() {
  greet();

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

void buildMainMenu(Shell *s) {
  auto menu = s->getMenu();
  menu->clear();

  menu->push_back({.description = "List active employees",
                   .callback = [s]() { s->write("Coming soon...\n"); }});

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

void Shell::initSystemFromXML() {}

void Shell::greet() {
  write("Welcome to WTTS (Work Time Tracking System)", "\n");
}
} // namespace es
