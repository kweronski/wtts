#pragma once

#include <functional>
#include <iostream>
#include <wtts/employeeSystem.hpp>

namespace es {
template <typename F> struct MenuEntry {
  std::string description;
  F callback;
};

using ShellMenuEntry = MenuEntry<std::function<void()>>;

class Shell;
void buildMainMenu(Shell *s);

class Shell {
public:
  Shell(std::istream &in, std::ostream &out) : input_{in}, output_{out} {
    buildMainMenu(this);
  }

  void run();

  void greet();

  bool prompt();

  std::size_t readIndex(std::string const &str);

  std::string readLine();

  void initSystemFromXML();

  template <typename... P> void write(P &&...args) {
    (output_ << ... << args) << std::flush;
  }

  std::vector<ShellMenuEntry> *getMenu() { return &menu_; }

  void requestExit() { exit_ = true; }

private:
  std::unique_ptr<EmployeeSystem> system_;

  std::istream &input_;
  std::ostream &output_;
  std::string prompt_{"wtts> "};

  std::vector<ShellMenuEntry> menu_;

  bool exit_{0};
};
} // namespace es
