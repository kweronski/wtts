#pragma once

#include <menu.h>
#include <ncurses.h>
#include <wtts/employeeSystem.hpp>
#include <wtts/interactiveCLI.hpp>

namespace sh {
struct InputMenu {
  MENU *handle{};
  ITEM **items{};
  std::size_t itemCount{};
  std::string prompt{};
};

struct Interface {
  WINDOW *clockWin{};
  WINDOW *inputWin{};
  InputMenu *selection{};
  WINDOW *selectSub{};
};

enum class StateType {
  SelectionInProgress,
  ReadInProgress,
  ProcedureInProgress,
  ExitInProgress
};

enum class ActionId {
  Select,
  ChangeState,
  Read,
  AddEmployee,
  PopulateEntriesWithEmployees,
  SetActiveEmployee,
  ViewNotValidInput,
  AskToRetryInput,
  HandleRetryAnswer,
  Exit,
  PopulateNotifications,
  ClearNotifications,
  ReadSystem,
  WriteSystem,
  Notify,
  SetAutoCheckoutTime,
  RemoveEmployee
};

struct StateActionDesc {
  StateType type{};
  ActionId id{};
  std::unordered_map<std::string, std::string> argLookup{};
  std::unordered_map<std::string, std::string> boundArgs{};
  std::vector<std::string> selectionTitles{};
  std::vector<std::string> selectionValues{};
};

struct StateTracker {
  StateType currentStateType{StateType::SelectionInProgress};
  std::string mainStateId{};

  std::unordered_map<std::string, std::string> argStorage;

  bool signalReady{}, inputReady{};
  std::string inputBuffer;
  int signal{};

  std::chrono::milliseconds spf{16}; // seconds per frame

  std::vector<std::string> selectionTitles{};
  std::vector<std::string> selectionValues{};

  std::string activeEmployeeId{};

  std::vector<std::string> retryReason{};
  std::vector<std::string> retryArgs{};
  std::vector<std::string> retryVals{};

  std::vector<StateActionDesc *> actionQueue{};
  std::size_t actionIndex{};

  std::vector<std::string> notifications{};
  std::vector<es::Employee *> employees{};
  std::vector<es::Employee *> autoLoggedOut{};
};

struct Shell {
  std::unordered_map<std::string, std::vector<StateActionDesc>> states;
  std::unique_ptr<es::EmployeeSystem> esys{};
  StateTracker control{};
  Interface ui{};
};

Result update(Shell *handle);
Result updateBanner(Shell *handle);
void updateInputBuffer(Shell *handle);
Result updateInputWindow(Shell *handle);
Result stampInputWindow(Shell *handle);
Result executeProcedure(Shell *handle);
Result executeInputRead(Shell *handle);
Result markArgAsNotValid(std::string const &id, std::string const &reason,
                         Shell *handle);
Result injectRetrySequence(Shell *handle);

Result initialize(Shell *handle);
Result initializeInterface(Shell *handle);
Result initializeStateDesc(Shell *handle);
Result initializeActionQueue(Shell *handle);
Result initializeInternalStates(Shell *handle);

Result createInputMenu(std::string const &prompt,
                       std::vector<std::string> const &entries, Shell *shell);
std::size_t getInputMenuSelection(Shell *shell);
Result updateInputMenu(Shell *shell);
void destroyInputMenu(Shell *shell);
} // namespace sh
