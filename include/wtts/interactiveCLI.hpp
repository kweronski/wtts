#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <iostream>
#include <mutex>
#include <wtts/employeeSystem.hpp>

namespace es {
template <typename F> struct MenuEntry {
  std::string description;
  F callback;
};

using ShellMenuEntry = MenuEntry<std::function<void()>>;

class Shell;
// All build functions clear the existing menu
void buildMainMenu(Shell *s);
void buildAdminMenu(Shell *s);
void buildManagerMenu(Shell *s);
void buildDriverMenu(Shell *s);
void buildEmployeeMenu(Shell *s);
void buildEmployeeSelectionMenu(
    Shell *, std::vector<std::pair<std::string, Employee *>> const &);
void buildSettingsMenu(Shell *s);

// All menu entry appending functions do not clear the existing menu
void appendGeneralAdminMenuEntries(Shell *s);
void appendAdminMenuEntries(Shell *s);
void appendEmployeeMenuEntries(Shell *s);
void appendDriverMenuEntries(Shell *s);

template <typename T> struct GuardedResource {
private:
  mutable std::mutex guard;
  T resource;

public:
  using value_type = T;

  template <typename... Pack>
  GuardedResource(Pack &&...args) : resource{std::forward<Pack>(args)...} {}

  void set(T &&value) {
    std::lock_guard<std::mutex> lock{guard};
    resource = std::move(value);
  }
  void set(T const &value) {
    std::lock_guard<std::mutex> lock{guard};
    resource = value;
  }

  template <typename V> void push_back(V &&value) {
    std::lock_guard<std::mutex> lock{guard};
    resource.push_back(std::move(value));
  }

  template <typename V> void push_back(V const &value) {
    std::lock_guard<std::mutex> lock{guard};
    resource.push_back(value);
  }

  void pop_back() {
    std::lock_guard<std::mutex> lock{guard};
    resource.pop_back();
  }

  void clear() {
    std::lock_guard<std::mutex> lock{guard};
    resource.clear();
  }

  std::size_t size() const {
    std::lock_guard<std::mutex> lock{guard};
    return resource.size();
  }

  T get() const {
    std::lock_guard<std::mutex> lock{guard};
    return resource;
  }

  template <typename V> V merge() {
    std::lock_guard<std::mutex> lock{guard};
    V v;
    for (auto const &e : resource)
      v += e;
    return v;
  }

  template <typename K> auto at(K const &key) {
    std::lock_guard<std::mutex> lock{guard};
    return resource.at(key);
  }

  operator T() const {
    std::lock_guard<std::mutex> lock{guard};
    return static_cast<T>(resource);
  }
};

struct UserInterface {
  GuardedResource<std::string> greeting;
  GuardedResource<std::vector<ShellMenuEntry>> menu;
  GuardedResource<std::string> inputInstruction;
  GuardedResource<std::string> prompt;
  GuardedResource<std::string> date;
  GuardedResource<std::chrono::milliseconds> tick{
      std::size_t((1.0 / 60.0) * 1000.0)};
  GuardedResource<std::chrono::time_point<std::chrono::system_clock>>
      lastRefresh;
  GuardedResource<std::vector<char>> buf; // Stores input
};

class Shell {
public:
  Shell(std::istream &in, std::ostream &out) : input_{in}, output_{out} {
    buildMainMenu(this);
  }

  template <typename... P> void write(P &&...args) {
    std::lock_guard<std::mutex> lock{consoleGuardOut_};
    (output_ << ... << args) << std::flush;
  }
  void run();
  void greet();
  void renderUserInterface();
  std::size_t readIndex();
  std::string readLine();
  void requestExit() { exit_ = true; }
  void setInputInstruction(std::string s) {
    ui_.inputInstruction.set(std::move(s));
  }
  void setPromptText(std::string s) { ui_.prompt.set(std::move(s)); }
  std::string getPromptText() const { return ui_.prompt.get(); }

  void setCurrentEmployeeId(std::string id) {
    currentEmployeeId_ = std::move(id);
  }
  std::string getCurrentEmployeeId() const { return currentEmployeeId_; }

  void setSystem(std::unique_ptr<EmployeeSystem> s) { system_ = std::move(s); }
  EmployeeSystem *getSystem() { return system_.get(); }
  std::mutex &getSystemGuard() { return systemGuard_; }
  std::mutex &getConsoleGuardIn() { return consoleGuardIn_; }

  UserInterface *getInterface() { return &ui_; }

  void handleInput();
  void autoCheckout();

private:
  std::unique_ptr<EmployeeSystem> system_{std::make_unique<EmployeeSystem>()};
  std::mutex systemGuard_;

  std::string currentEmployeeId_;

  std::istream &input_;
  std::ostream &output_;

  UserInterface ui_;

  std::mutex consoleGuardOut_;
  std::mutex consoleGuardIn_;
  std::atomic<bool> exit_{0};
};
} // namespace es
