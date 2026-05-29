#include <cassert>
#include <pugixml.hpp>
#include <wtts/employeeData.hpp>
#include <wtts/employeeSystem.hpp>
#include <wtts/employeeSystemFactory.hpp>
#include <wtts/interactiveCLI.hpp>
#include <wtts/shell.hpp>
#include <wtts/xmlParser.hpp>
#include <wtts/xmlWriter.hpp>

namespace sh {
Result run(Shell *shell) {
  if (auto res = initialize(shell); res != Result::Success)
    return res;

  while (shell->control.currentStateType != StateType::ExitInProgress)
    if (auto res = update(shell); res != Result::Success)
      return res;

  return Result::Success;
}

Result initialize(Shell *shell) {
  // Reads the XML interface file and populates internal structures
  if (auto res = initializeStateDesc(shell); res != Result::Success)
    return res;

  if (auto res = initializeInternalStates(shell); res != Result::Success)
    return res;

  // Creates windows and other NCURSES entities
  if (auto res = initializeInterface(shell); res != Result::Success)
    return res;

  if (auto res = initializeActionQueue(shell); res != Result::Success)
    return res;

  shell->esys = std::make_unique<es::EmployeeSystem>();
  return Result::Success;
}

Result updateAutoLogout(Shell *shell) {
  assert(shell->esys);
  auto empl = shell->esys->autoCheckOut();

  if (empl.size()) {
    for (auto const e : empl) {
      std::string title = "Logged out: " + e->getEmployeeName() +
                          e->getEmployeeSurname() + " (" + e->getEmployeeId() +
                          ")";
      shell->control.notifications.push_back(title);
    }
  }

  return Result::Success;
}

Result update(Shell *shell) {
  updateInputBuffer(shell);
  updateAutoLogout(shell);

  if (auto res = updateBanner(shell); res != Result::Success)
    return res;

  if (auto res = updateInputWindow(shell); res != Result::Success)
    return res;

  if (auto res = executeProcedure(shell); res != Result::Success)
    return res;

  shell->control.currentStateType =
      shell->control.actionQueue[shell->control.actionIndex]->type;

  if (auto ESC = 27; shell->control.signalReady && shell->control.signal == ESC)
    shell->control.currentStateType = StateType::ExitInProgress;

  return Result::Success;
}

Result addEmployee(Shell *shell) {
  auto const &args = shell->control.argStorage;
  auto const idx = shell->control.actionIndex;
  auto &queue = shell->control.actionQueue;

  auto const name = args.at(queue.at(idx)->argLookup.at("name"));
  auto const surname = args.at(queue.at(idx)->argLookup.at("surname"));
  auto const telephone = args.at(queue.at(idx)->argLookup.at("telephone"));
  auto const email = args.at(queue.at(idx)->argLookup.at("email"));
  auto const cardId = args.at(queue.at(idx)->argLookup.at("cardId"));
  auto const employeeId = args.at(queue.at(idx)->argLookup.at("employeeId"));

  bool retry = false, active = true;
  if (!name.size()) {
    markArgAsNotValid("name", "The argument is empty", shell);
    retry = true;
  }

  if (!surname.size()) {
    markArgAsNotValid("surname", "The argument is empty", shell);
    retry = true;
  }

  if (!telephone.size()) {
    markArgAsNotValid("telephone", "The argument is empty", shell);
    retry = true;
  }

  if (!email.size()) {
    markArgAsNotValid("email", "The argument is empty", shell);
    retry = true;
  }

  if (!cardId.size()) {
    markArgAsNotValid("cardId", "The argument is empty", shell);
    retry = true;
  }

  if (!employeeId.size()) {
    markArgAsNotValid("employeeId", "The argument is empty", shell);
    retry = true;
  }

  std::size_t stdWorkTime, maxWorkTime, hourlyWage;
  try {
    stdWorkTime =
        std::stoul(args.at(queue.at(idx)->argLookup.at("stdWorkTime")));
  } catch (...) {
    markArgAsNotValid("stdWorkTime", "Converting to number failed", shell);
    retry = true;
  }

  try {
    maxWorkTime =
        std::stoul(args.at(queue.at(idx)->argLookup.at("maxWorkTime")));
  } catch (...) {
    markArgAsNotValid("maxWorkTime", "Converting to number failed", shell);
    retry = true;
  }

  try {
    hourlyWage = std::stoul(args.at(queue.at(idx)->argLookup.at("hourlyWage")));
  } catch (...) {
    markArgAsNotValid("hourlyWage", "Converting to number failed", shell);
    retry = true;
  }

  try {
    active = bool(std::stoi(args.at(queue.at(idx)->argLookup.at("active"))));
  } catch (...) {
    markArgAsNotValid("active", "Converting to number failed", shell);
    retry = true;
  }

  auto const roleStr = args.at(queue.at(idx)->argLookup.at("role"));
  es::EmployeeRole role{};
  if (roleStr == "employee") {
    role = es::EmployeeRole::Employee;
  } else if (roleStr == "driver") {
    role = es::EmployeeRole::Driver;
  } else if (roleStr == "manager") {
    role = es::EmployeeRole::Manager;
  } else if (roleStr == "admin") {
    role = es::EmployeeRole::Admin;
  } else {
    markArgAsNotValid(
        "role", "Must be one of: employee, driver, manager, admin", shell);
    retry = true;
  }

  if (retry)
    return Result::InputNotValidError;

  es::Employee *e;
  es::Driver *d;
  es::Manager *m;
  es::Admin *a;
  es::PersonnelData *pd;
  es::AttendanceData *ad;

  switch (role) {
  case es::EmployeeRole::Employee:
    if (auto res = shell->esys->addEmployee(&e, &pd, &ad);
        res == es::Result::EmployeeIdNotUniqueError)
      markArgAsNotValid("employeeId", "The supplied id is not unique", shell);
    else if (res != es::Result::Success)
      return Result::InputNotValidError;
    break;
  case es::EmployeeRole::Driver:
    if (auto res = shell->esys->addEmployee(&d, &pd, &ad);
        res == es::Result::EmployeeIdNotUniqueError)
      markArgAsNotValid("employeeId", "The supplied id is not unique", shell);
    else if (res != es::Result::Success)
      return Result::InputNotValidError;
    break;
  case es::EmployeeRole::Manager:
    if (auto res = shell->esys->addEmployee(&m, &pd, &ad);
        res == es::Result::EmployeeIdNotUniqueError)
      markArgAsNotValid("employeeId", "The supplied id is not unique", shell);
    else if (res != es::Result::Success)
      return Result::InputNotValidError;
    break;
  case es::EmployeeRole::Admin:
    if (auto res = shell->esys->addEmployee(&a, &pd, &ad);
        res == es::Result::EmployeeIdNotUniqueError)
      markArgAsNotValid("employeeId", "The supplied id is not unique", shell);
    else if (res != es::Result::Success)
      return Result::InputNotValidError;
    break;
  default:
    assert(false && "Unknown employee role is not supported");
  }

  pd->setEmployeeId(employeeId);
  pd->setEmployeeName(name);
  pd->setEmployeeRole(role);
  pd->setEmployeeEmail(email);
  pd->setEmployeeActive(active);
  pd->setEmployeeCardId(cardId);
  pd->setEmployeeSurname(surname);
  pd->setEmployeeTelephone(telephone);
  pd->setEmployeeHourlyWage(hourlyWage);
  pd->setEmployeeMaxWorkTime(maxWorkTime);
  pd->setEmployeeStandardWorkTime(stdWorkTime);

  return Result::Success;
}

Result changeState(Shell *shell) {
  auto desc = shell->control.actionQueue[shell->control.actionIndex];
  destroyInputMenu(shell);

  assert(desc->argLookup.contains("target"));
  auto nsKey = desc->argLookup.at("target");

  assert(shell->control.argStorage.contains(nsKey));
  auto newState = shell->control.argStorage.at(nsKey);

  bool isValidState = false;
  for (auto const &[id, data] : shell->states)
    if (id == newState)
      isValidState = true;

  assert(isValidState);

  shell->control.actionQueue.clear();
  shell->control.actionIndex = 0;

  if (shell->states.at(newState).empty())  // If the state contains no actions,
    newState = shell->control.mainStateId; // return to the mainState

  auto &ns = shell->states.at(newState);
  for (auto &action : ns)
    shell->control.actionQueue.push_back(&action);

  shell->control.currentStateType =
      shell->control.actionQueue[shell->control.actionIndex]->type;
  return Result::Success;
}

Result markArgAsNotValid(std::string const &id, std::string const &reason,
                         Shell *shell) {
  assert(shell->control.argStorage.contains(id));
  auto value = shell->control.argStorage.at(id);
  shell->control.retryReason.push_back(reason);
  shell->control.retryVals.push_back(value);
  shell->control.retryArgs.push_back(id);
  return Result::Success;
}

Result injectRetrySequence(Shell *shell) {
  // Find query action prior to the most recent procedure.
  std::vector<StateActionDesc *> retryQueries{};

  std::size_t idx = shell->control.actionIndex;
  for (std::size_t i = 0; i < shell->control.actionIndex; ++i) {
    --idx;

    if (shell->control.actionQueue[idx]->type == StateType::ReadInProgress ||
        shell->control.actionQueue[idx]->type == StateType::SelectionInProgress)
      for (auto const &retryTarget : shell->control.retryArgs)
        if (shell->control.actionQueue[idx]->boundArgs.contains("out") &&
            shell->control.actionQueue[idx]->boundArgs.at("out") == retryTarget)
          retryQueries.push_back(shell->control.actionQueue[idx]);
  }

  assert(retryQueries.size() &&
         "There must have been a query operation to "
         "repeat; Otherwise, the operation is not valid");

  std::vector<StateActionDesc *> tmp{};
  for (std::size_t i = retryQueries.size(); i; --i)
    tmp.push_back(retryQueries.at(i - 1));
  retryQueries = std::move(tmp);

  // Inject retry actions into front of action queue.
  std::vector<StateActionDesc *> newQueue{};
  auto &retrySt = shell->states.at("__retry");
  for (auto &action : retrySt)
    newQueue.push_back(&action);

  // Append query actions that need to be repeated.
  for (auto actionPtr : retryQueries)
    newQueue.push_back(actionPtr);

  // Append the remaining actions from the original queue.
  for (std::size_t i = shell->control.actionIndex;
       i < shell->control.actionQueue.size(); ++i)
    newQueue.push_back(shell->control.actionQueue[i]);

  shell->control.actionQueue = std::move(newQueue);
  shell->control.actionIndex = 0;
  return Result::Success;
}

Result handleRetryAnswer(Shell *shell) {
  assert(shell->control.argStorage.contains("__retry"));
  bool answer = std::stoi(shell->control.argStorage.at("__retry"));
  assert(answer == 0 || answer == 1);

  if (!answer)
    initializeActionQueue(shell);
  else
    ++shell->control.actionIndex;

  return Result::Success;
}

Result populateEntriesWithEmployees(Shell *shell) {
  auto desc = shell->control.actionQueue[shell->control.actionIndex];
  std::string filter;

  if (desc->boundArgs.contains("filter"))
    filter = desc->boundArgs.at("filter");

  auto checkedIn = [](auto const, auto const, auto const a) {
    return a->getCurrentTimePeriod()->begin.year;
  };

  auto all = [](auto, auto, auto) { return true; };

  if (filter == "checked-in")
    shell->control.employees = shell->esys->getEmployeeBy(checkedIn);
  else
    shell->control.employees = shell->esys->getEmployeeBy(all);

  if (!shell->control.employees.size()) {
    shell->control.selectionTitles.push_back("No employees matching criteria");
    shell->control.selectionValues.push_back("No employees matching criteria");
  } else {
    for (auto e : shell->control.employees) {
      std::string title = e->getEmployeeName() + " ";
      title += e->getEmployeeSurname() + " (" +
               es::to_string(e->getEmployeeRole()) + ")";
      shell->control.selectionTitles.push_back(std::move(title));
      shell->control.selectionValues.push_back(e->getEmployeeId());
    }
    shell->control.selectionTitles.push_back("Back");
    shell->control.selectionValues.push_back("");
  }
  return Result::Success;
}

Result populateNotifications(Shell *shell) {
  shell->control.argStorage["prompt"] = "Notifications: ";
  for (auto const &n : shell->control.autoLoggedOut) {
    std::string label = n->getEmployeeName() + " " + n->getEmployeeSurname();
    label += " (" + es::to_string(n->getEmployeeRole()) + ")";
    shell->control.selectionTitles.push_back(std::move(label));
    shell->control.selectionValues.push_back("");
  }

  if (!shell->control.autoLoggedOut.size()) {
    shell->control.selectionTitles.push_back("No notifications");
    shell->control.selectionValues.push_back("");
  }

  return Result::Success;
}

Result injectNotification(std::string const &prompt,
                          std::vector<std::string> const &messages,
                          bool isErrorNotif, Shell *shell) {
  assert(prompt.size());
  assert(messages.size());
  std::vector<StateActionDesc *> queue;
  auto &actQ = shell->control.actionQueue;
  auto idx = shell->control.actionIndex;

  assert(shell->states.contains("__notify"));
  assert(shell->states.at("__notify").size());

  for (auto &act : shell->states.at("__notify"))
    queue.push_back(&act);

  if (!isErrorNotif)
    for (++idx; idx < actQ.size(); ++idx)
      queue.push_back(actQ[idx]);

  shell->control.argStorage["prompt"] = prompt;

  for (auto const &m : messages) {
    shell->control.selectionTitles.push_back(m);
    shell->control.selectionValues.push_back("");
  }

  shell->control.actionIndex = 0;
  actQ = std::move(queue);
  return Result::NotificationPending;
}

Result injectPromptError(std::string const &prompt, Shell *shell) {
  std::vector<std::string> opts{"OK"};
  return injectNotification(prompt, opts, true, shell);
}

Result injectPromptNotif(std::string const &prompt, Shell *shell) {
  std::vector<std::string> opts{"OK"};
  return injectNotification(prompt, opts, false, shell);
}

Result setActiveEmployee(Shell *shell) {
  if (!shell->control.employees.size()) {
    // If the input was not available we must return to the main state
    // and abort the current one. Removing subsequent actions will trigger this.
    while (shell->control.actionQueue.size() > 1)
      shell->control.actionQueue.pop_back();
    return Result::Success;
  }

  auto const desc = shell->control.actionQueue[shell->control.actionIndex];
  assert(desc->argLookup.contains("target"));
  auto const targetId = desc->argLookup.at("target");

  assert(shell->control.argStorage.contains(targetId));
  auto const id = shell->control.argStorage.at(targetId);

  if (id.empty()) // The back option was chosen
    return injectPromptNotif("Aborting selection", shell);

  auto empl = shell->esys->getEmployeeById(id);
  assert(empl);

  shell->control.activeEmployeeId = empl->getEmployeeId();
  return Result::Success;
}

Result removeEmployee(Shell *shell) {
  auto desc = shell->control.actionQueue[shell->control.actionIndex];
  assert(desc->argLookup.contains("target"));
  auto nsKey = desc->argLookup.at("target");
  assert(shell->control.argStorage.contains(nsKey));
  auto target = shell->control.argStorage.at(nsKey);

  if (shell->control.activeEmployeeId == target)
    shell->control.activeEmployeeId.clear();

  auto e = shell->esys->getEmployeeById(target);
  assert(e);

  if (shell->esys->removeEmployee(target) != es::Result::Success)
    return injectPromptError("Removing employee failed", shell);

  return injectPromptNotif("Successfully removed: " + e->getEmployeeName() +
                               " " + e->getEmployeeSurname(),
                           shell);
}

Result readSystem(Shell *shell) {
  auto desc = shell->control.actionQueue[shell->control.actionIndex];
  assert(desc->argLookup.contains("path"));
  assert(shell->control.argStorage.contains(desc->argLookup.at("path")));
  auto path = shell->control.argStorage.at(desc->argLookup.at("path"));

  std::unique_ptr<es::EmployeeSystem> sys;
  try {
    dp::XMLDataParser p{path};
    sys = es::EmployeeSystemFactory::create(&p);
    if (!sys)
      throw std::runtime_error{""};
  } catch (...) {
    injectPromptError("Failed to load system from file", shell);
    return Result::InternalError;
  }

  shell->control.activeEmployeeId.clear();
  shell->esys = std::move(sys);
  return injectPromptNotif("Successfully loaded system from file", shell);
}

Result writeSystem(Shell *shell) {
  auto desc = shell->control.actionQueue[shell->control.actionIndex];
  assert(desc->argLookup.contains("path"));
  assert(shell->control.argStorage.contains(desc->argLookup.at("path")));
  auto path = shell->control.argStorage.at(desc->argLookup.at("path"));
  assert(shell->esys);

  dp::XMLWriter writer{path};
  auto empl = shell->esys->getEmployeeBy([](auto, auto, auto) { return 1; });

  for (auto e : empl) {
    auto id = writer.addEmployee();
    writer.setEmployeeId(id, e->getEmployeeId());
    writer.setEmployeeName(id, e->getEmployeeName());
    writer.setEmployeeSurname(id, e->getEmployeeSurname());
    writer.setEmployeeRole(id, e->getEmployeeRole());
    writer.setEmployeeEmail(id, e->getEmployeeEmail());
    writer.setEmployeeCardId(id, e->getEmployeeCardId());
    writer.setEmployeeStatus(id, e->getEmployeeActive()
                                     ? dp::EmployeeStatus::Active
                                     : dp::EmployeeStatus::Inactive);
    writer.setEmployeeTelephone(id, e->getEmployeeTelephone());
    writer.setEmployeeHourlyWage(id, e->getEmployeeHourlyWage());
    writer.setEmployeeMaxWorkTime(id, e->getEmployeeMaxWorkTime());
    writer.setEmployeeStandardWorkTime(id, e->getEmployeeStandardWorkTime());

    auto at = shell->esys->getEmployeeAttendance(e);
    for (const auto &a : at->getRecords())
      writer.addEmployeeAttendance(id, a);
  }

  if (auto res = writer.writeData(); res != dp::Result::Success) {
    injectPromptError("Failed to load system from file", shell);
    return Result::InternalError;
  }

  return injectPromptNotif("The system has been successfully written to disk",
                           shell);
}

Result setAutoCheckoutTime(Shell *shell) {
  auto desc = shell->control.actionQueue[shell->control.actionIndex];
  assert(desc->argLookup.contains("hour"));
  assert(shell->control.argStorage.contains(desc->argLookup.at("hour")));
  assert(desc->argLookup.contains("minute"));
  assert(shell->control.argStorage.contains(desc->argLookup.at("minute")));
  auto hour = shell->control.argStorage.at(desc->argLookup.at("hour"));
  auto minute = shell->control.argStorage.at(desc->argLookup.at("minute"));
  assert(shell->esys);

  unsigned hourN{}, minuteN{};
  bool ok = true;

  try {
    hourN = std::stoi(hour);
  } catch (...) {
    markArgAsNotValid("hour", "Converting to number failed", shell);
    ok = false;
  }

  try {
    minuteN = std::stoi(minute);
  } catch (...) {
    markArgAsNotValid("minute", "Converting to number failed", shell);
    ok = false;
  }

  if (minuteN < 1 || minuteN > 60) {
    markArgAsNotValid("minute", "A valid minute must be in the range [1,60]",
                      shell);
    ok = false;
  }

  if (hourN < 1 || hourN > 24) {
    markArgAsNotValid("minute", "A valid hour must be in the range [1,24]",
                      shell);
    ok = false;
  }

  if (!ok)
    return Result::InputNotValidError;

  shell->esys->setAutoCheckoutTime(hourN, minuteN);

  return injectPromptNotif(
      "Successfully set auto checkout time to " + hour + ":" + minute, shell);
}

Result executeProcedure(Shell *shell) {
  if (shell->control.currentStateType != StateType::ProcedureInProgress)
    return Result::Success;

  auto desc = shell->control.actionQueue[shell->control.actionIndex];
  Result code{Result::Success};

  switch (desc->id) {
  case ActionId::ChangeState:
    changeState(shell);
    return Result::Success;
  case ActionId::HandleRetryAnswer:
    handleRetryAnswer(shell);
    return Result::Success;
  case ActionId::AddEmployee:
    code = addEmployee(shell);
    break;
  case ActionId::PopulateEntriesWithEmployees:
    populateEntriesWithEmployees(shell);
    break;
  case ActionId::SetActiveEmployee:
    setActiveEmployee(shell);
    break;
  case ActionId::PopulateNotifications:
    populateNotifications(shell);
    break;
  case ActionId::ClearNotifications:
    shell->control.notifications.clear();
    break;
  case ActionId::ReadSystem:
    code = readSystem(shell);
    break;
  case ActionId::WriteSystem:
    code = writeSystem(shell);
    break;
  case ActionId::SetAutoCheckoutTime:
    code = setAutoCheckoutTime(shell);
    break;
  case ActionId::RemoveEmployee:
    code = removeEmployee(shell);
    break;
  default:
    assert(false && "desc->id is not registered as a procedure id");
  }

  if (code == Result::Success) {
    ++shell->control.actionIndex;
    if (shell->control.actionIndex >= shell->control.actionQueue.size())
      initializeActionQueue(shell);
  } else if (code == Result::InputNotValidError)
    injectRetrySequence(shell);

  return Result::Success;
}

Result executeInputRead(Shell *shell) {
  if (shell->control.currentStateType != StateType::ReadInProgress)
    return Result::Success;

  auto desc = shell->control.actionQueue[shell->control.actionIndex];
  if (desc->boundArgs.contains("prompt")) {
    wmove(shell->ui.inputWin, 1, 1);
    wclrtoeol(shell->ui.inputWin);
    mvwprintw(shell->ui.inputWin, 1, 1, "%s %s",
              desc->boundArgs.at("prompt").c_str(),
              shell->control.inputBuffer.c_str());
  }

  if (shell->control.inputReady) {
    if (desc->boundArgs.contains("out")) {
      shell->control.argStorage[desc->boundArgs.at("out")] =
          shell->control.inputBuffer;
    }

    shell->control.inputBuffer.clear();
    shell->control.inputReady = false;

    ++shell->control.actionIndex;
    if (shell->control.actionIndex >= shell->control.actionQueue.size())
      initializeActionQueue(shell);
  }

  return Result::Success;
}

Result updateInputWindow(Shell *shell) {
  if (auto res = updateInputMenu(shell); res != Result::Success)
    return res;

  if (auto res = executeInputRead(shell); res != Result::Success)
    return res;

  if (auto res = stampInputWindow(shell); res != Result::Success)
    return res;

  using namespace std::chrono;
  static auto begin = steady_clock::now();

  if (steady_clock::now() - begin > shell->control.spf) {
    wrefresh(shell->ui.inputWin);
    begin = steady_clock::now();
  }

  return Result::Success;
}

#define MCR_CHK_DEP(inVarId)                                                   \
  do {                                                                         \
    auto inVarKeyAttr = action.attribute(inVarId);                             \
    if (!inVarKeyAttr)                                                         \
      return Result::MissingInVariableIdError;                                 \
    std::string inVarKey = inVarKeyAttr.as_string();                           \
    bool depSatisfied{false};                                                  \
    for (auto const &a : stateDesc)                                            \
      if (a.boundArgs.contains("out") && a.boundArgs.at("out") == inVarKey) {  \
        depSatisfied = true;                                                   \
        break;                                                                 \
      }                                                                        \
    if (!depSatisfied)                                                         \
      return Result::ProcedureDependencyNotSatisfiedError;                     \
    desc.argLookup.emplace(inVarId, inVarKey);                                 \
  } while (0)

Result initializeStateDesc(Shell *shell) {
  pugi::xml_document doc;
  if (!doc.load_file("./interface.xml"))
    return Result::StateDescFileOpenError;

  std::unordered_map<std::string, std::size_t> stateIdOcc;
  auto root = doc.first_child();

  auto mainState = root.child("state");
  if (!mainState)
    return Result::MissingStateError;

  for (auto state = root.child("state"); state;
       state = state.next_sibling("state")) {

    auto stateIdAttr = state.attribute("id");
    if (!stateIdAttr)
      return Result::MissingStateIdError;
    std::string const stateId = stateIdAttr.as_string();

    if (!stateIdOcc.contains(stateId))
      stateIdOcc.emplace(stateId, 0);
    ++stateIdOcc.at(stateId);
  }

  for (auto const &[id, occ] : stateIdOcc)
    if (occ > 1)
      return Result::StateIdNotUniqueError;

  for (auto state = root.child("state"); state;
       state = state.next_sibling("state")) {

    // Used to make sure there is only one query per var until a procedure
    std::unordered_map<std::string, std::size_t> argWriteCount;
    std::vector<StateActionDesc> stateDesc{};

    for (auto action = state.first_child(); action;
         action = action.next_sibling()) {

      StateActionDesc desc{};
      std::string const actionId = action.name();

      if (actionId == "select") {
        desc.type = StateType::SelectionInProgress;
        desc.id = ActionId::Select;

        auto promptAttr = action.attribute("prompt");
        if (promptAttr)
          desc.boundArgs.emplace("prompt", promptAttr.as_string());

        auto outAttr = action.attribute("out");
        if (outAttr) {
          std::string out = outAttr.as_string();
          desc.boundArgs.emplace("out", out);
          if (!argWriteCount.contains(out))
            argWriteCount.emplace(out, 0);
          ++argWriteCount.at(out);
        }

        for (auto entry = action.child("entry"); entry;
             entry = entry.next_sibling("entry")) {
          auto titleAttr = entry.attribute("title");
          auto valueAttr = entry.attribute("value");
          std::string title, value;
          if (titleAttr)
            title = titleAttr.as_string();
          if (valueAttr)
            value = valueAttr.as_string();
          desc.selectionTitles.push_back(std::move(title));
          desc.selectionValues.push_back(std::move(value));
        }
      } else if (actionId == "read") {
        desc.type = StateType::ReadInProgress;
        desc.id = ActionId::Read;

        auto promptAttr = action.attribute("prompt");
        if (promptAttr)
          desc.boundArgs.emplace("prompt", promptAttr.as_string());

        auto outAttr = action.attribute("out");
        if (outAttr) {
          std::string out = outAttr.as_string();
          desc.boundArgs.emplace("out", out);
          if (!argWriteCount.contains(out))
            argWriteCount.emplace(out, 0);
          ++argWriteCount.at(out);
        }
      } else if (actionId == "changeState") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::ChangeState;
        MCR_CHK_DEP("target");
      } else if (actionId == "addEmployee") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::AddEmployee;
        MCR_CHK_DEP("name");
        MCR_CHK_DEP("surname");
        MCR_CHK_DEP("telephone");
        MCR_CHK_DEP("email");
        MCR_CHK_DEP("cardId");
        MCR_CHK_DEP("employeeId");
        MCR_CHK_DEP("stdWorkTime");
        MCR_CHK_DEP("maxWorkTime");
        MCR_CHK_DEP("hourlyWage");
        MCR_CHK_DEP("role");
        MCR_CHK_DEP("active");
      } else if (actionId == "populateEntriesWithEmployees") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::PopulateEntriesWithEmployees;

        auto filterAttr = action.attribute("filter");
        if (filterAttr) {
          std::string filter = filterAttr.as_string();
          if (filter != "checked-in") /* || other_supported_filter */
            return Result::EmployeeFilterNotValidError;
          desc.boundArgs["filter"] = filter;
        }
      } else if (actionId == "setActiveEmployee") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::SetActiveEmployee;
        MCR_CHK_DEP("target");
      } else if (actionId == "populateNotifications") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::PopulateNotifications;
      } else if (actionId == "readSystem") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::ReadSystem;
        MCR_CHK_DEP("path");
      } else if (actionId == "writeSystem") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::WriteSystem;
        MCR_CHK_DEP("path");
      } else if (actionId == "clearNotifications") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::ClearNotifications;
      } else if (actionId == "setAutoCheckoutTime") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::SetAutoCheckoutTime;
        MCR_CHK_DEP("hour");
        MCR_CHK_DEP("minute");
      } else if (actionId == "removeEmployee") {
        desc.type = StateType::ProcedureInProgress;
        desc.id = ActionId::RemoveEmployee;
        MCR_CHK_DEP("target");
      } else
        return Result::ActionIdNotValidError;

      if (desc.type == StateType::ProcedureInProgress) {
        for (auto const &[id, occ] : argWriteCount)
          if (occ > 1)
            return Result::MultipleWritesToVariablePriorToProcedureError;
        argWriteCount.clear();
      }

      stateDesc.push_back(std::move(desc));
    }

    shell->states.emplace(std::string{state.attribute("id").as_string()},
                          std::move(stateDesc));
  }

  shell->control.mainStateId = mainState.attribute("id").as_string();
  std::vector<StateActionDesc> exitState{
      StateActionDesc{.type = StateType::ExitInProgress, .id = ActionId::Exit}};
  assert(!shell->states.contains("exit") && "The exit state id is reserved!");
  shell->states.emplace("exit", std::move(exitState));
  return Result::Success;
}

Result initializeInternalStates(Shell *handle) {
  assert(!handle->states.contains("__retry") &&
         "__retry is a reserved state name!");

  {
    std::unordered_map<std::string, std::string> boundArgs1{};
    boundArgs1.emplace("prompt", "The following arguments were not valid:");
    std::unordered_map<std::string, std::string> boundArgs2{};
    boundArgs2.emplace("prompt",
                       "Would you like to enter the arguments again?");
    boundArgs2.emplace("out", "__retry");
    std::unordered_map<std::string, std::string> boundArgs3{};
    boundArgs3.emplace("answer", "__retry");

    std::vector<StateActionDesc> retry{
        StateActionDesc{.type = StateType::SelectionInProgress,
                        .id = ActionId::ViewNotValidInput,
                        .boundArgs = std::move(boundArgs1)},
        StateActionDesc{.type = StateType::SelectionInProgress,
                        .id = ActionId::AskToRetryInput,
                        .boundArgs = std::move(boundArgs2),
                        .selectionTitles = {"Yes", "No"},
                        .selectionValues{"1", "0"}},
        StateActionDesc{.type = StateType::ProcedureInProgress,
                        .id = ActionId::HandleRetryAnswer,
                        .boundArgs = std::move(boundArgs3)}};

    handle->states.emplace("__retry", std::move(retry));
  }

  assert(!handle->states.contains("__notify") &&
         "__notify is a reserved state name!");

  {
    std::unordered_map<std::string, std::string> boundArgs1{};
    boundArgs1.emplace("prompt", "Notifications:");

    std::vector<StateActionDesc> notify{
        StateActionDesc{.type = StateType::SelectionInProgress,
                        .id = ActionId::Notify,
                        .boundArgs = std::move(boundArgs1)}};

    handle->states.emplace("__notify", std::move(notify));
  }

  return Result::Success;
}

Result initializeActionQueue(Shell *shell) {
  auto &state = shell->states.at(shell->control.mainStateId);
  shell->control.actionQueue.clear();
  shell->control.actionIndex = 0;

  for (auto &action : state)
    shell->control.actionQueue.push_back(&action);

  assert(state.size());
  shell->control.currentStateType = state[0].type;

  if (shell->control.selectionTitles.empty())
    shell->control.selectionTitles = state[0].selectionTitles;
  if (shell->control.selectionValues.empty())
    shell->control.selectionValues = state[0].selectionValues;
  return Result::Success;
}

Result initializeInterface(Shell *shell) {
  keypad(stdscr, 1);
  start_color();
  halfdelay(1);
  curs_set(0);

  auto const ui = &shell->ui;
  int const cHeight = 5;
  int height, width;

  getmaxyx(stdscr, height, width);
  ui->clockWin = newwin(cHeight, width, 0, 0);
  ui->inputWin = newwin(height - cHeight, width, cHeight, 0);
  keypad(ui->inputWin, 1);
  shell->ui.selectSub = derwin(shell->ui.inputWin, 0, 0, 2, 1);

  init_pair(1, COLOR_YELLOW, COLOR_BLACK);
  init_pair(2, COLOR_BLACK, COLOR_YELLOW);

  wattron(ui->clockWin, COLOR_PAIR(1));
  wattron(ui->inputWin, COLOR_PAIR(1));
  return Result::Success;
}

Result updateMenuPrompt(Shell *shell) {
  auto const &prompt = shell->ui.selection->prompt;
  wmove(shell->ui.inputWin, 1, 1);
  wclrtoeol(shell->ui.inputWin);
  mvwprintw(shell->ui.inputWin, 1, 1, "%s", prompt.c_str());
  return Result::Success;
}

Result createInputMenu(std::string const &prompt,
                       std::vector<std::string> const &entries, Shell *shell) {
  assert(entries.size());
  auto &select = shell->ui.selection;
  assert(!select);
  select = new InputMenu{};
  select->itemCount = entries.size();

  select->items = (ITEM **)calloc(select->itemCount + 1, sizeof(ITEM *));
  select->items[select->itemCount] = 0;

  for (std::size_t i = 0; i < select->itemCount; ++i) {
    char const *title = entries.at(i).c_str();
    assert(title);
    select->items[i] = new_item(title, "");
    assert(select->items[i]);
  }

  select->handle = new_menu(select->items);
  select->prompt = prompt;
  updateMenuPrompt(shell);

  set_menu_win(select->handle, shell->ui.inputWin);
  set_menu_sub(select->handle, shell->ui.selectSub);

  set_menu_fore(select->handle, COLOR_PAIR(2) | A_BOLD);
  set_menu_back(select->handle, COLOR_PAIR(1));
  set_menu_mark(select->handle, " * ");

  post_menu(select->handle);
  return Result::Success;
}

Result updateInputMenu(Shell *shell) {
  // Build the menu if necessary
  if (!shell->ui.selection &&
      shell->control.currentStateType == StateType::SelectionInProgress) {
    auto desc = shell->control.actionQueue[shell->control.actionIndex];

    if (shell->control.selectionTitles.empty())
      shell->control.selectionTitles = desc->selectionTitles;
    if (shell->control.selectionValues.empty())
      shell->control.selectionValues = desc->selectionValues;

    if (desc->id == ActionId::ViewNotValidInput) {
      shell->control.selectionTitles.clear();
      shell->control.selectionValues.clear();

      assert(shell->control.retryArgs.size() ==
             shell->control.retryReason.size());
      assert(shell->control.retryArgs.size() ==
             shell->control.retryVals.size());
      assert(shell->control.retryArgs.size());

      std::string title;
      for (std::size_t i = 0; i < shell->control.retryArgs.size(); ++i) {
        title = "ID: " + shell->control.retryArgs[i] + ", Value: " +
                (shell->control.retryVals[i].size()
                     ? "'" + shell->control.retryVals[i] + "'"
                     : "<EMPTY>") +
                ", Reason: " + shell->control.retryReason[i];
        shell->control.selectionTitles.push_back(std::move(title));
      }

      shell->control.retryReason.clear();
      shell->control.retryArgs.clear();
      shell->control.retryVals.clear();
    }

    std::string prompt{};
    if (shell->control.argStorage.contains("prompt") &&
        shell->control.argStorage.at("prompt").size())
      prompt = shell->control.argStorage.at("prompt");
    else if (desc->boundArgs.contains("prompt"))
      prompt = desc->boundArgs.at("prompt");
    createInputMenu(prompt, shell->control.selectionTitles, shell);
  }

  if (shell->ui.selection &&
      shell->control.currentStateType == StateType::SelectionInProgress) {

    if (shell->control.signalReady) {
      switch (shell->control.signal) {
      case KEY_UP:
      case 'k':
        menu_driver(shell->ui.selection->handle, REQ_UP_ITEM);
        shell->control.signalReady = false;
        break;
      case KEY_DOWN:
      case 'j':
        menu_driver(shell->ui.selection->handle, REQ_DOWN_ITEM);
        shell->control.signalReady = false;
        break;
      default:
        break;
      }
    }

    if (shell->control.inputReady) {
      auto desc = shell->control.actionQueue[shell->control.actionIndex];
      if (desc->boundArgs.contains("out")) {
        auto idx = getInputMenuSelection(shell);
        shell->control.argStorage[desc->boundArgs.at("out")] =
            shell->control.selectionValues.at(idx).size()
                ? shell->control.selectionValues.at(idx)
                : std::to_string(idx);
      }

      shell->control.inputReady = false;
      ++shell->control.actionIndex;
      if (shell->control.actionIndex >= shell->control.actionQueue.size())
        initializeActionQueue(shell);
      else
        shell->control.currentStateType =
            shell->control.actionQueue[shell->control.actionIndex]->type;

      shell->control.selectionTitles.clear();
      shell->control.selectionValues.clear();
      destroyInputMenu(shell);

      if (shell->control.argStorage.contains("prompt"))
        shell->control.argStorage.at("prompt").clear();
    }
  }

  return Result::Success;
}

std::size_t getInputMenuSelection(Shell *shell) {
  auto &select = shell->ui.selection;
  assert(select->handle);
  return item_index(current_item(select->handle));
}

void destroyInputMenu(Shell *shell) {
  if (!shell->ui.selection)
    return;

  auto &select = shell->ui.selection;
  unpost_menu(select->handle);
  free_menu(select->handle);

  for (std::size_t i = 0; i < select->itemCount; ++i)
    free_item(select->items[i]);

  free(select->items);

  delete select;
  select = nullptr;
}

Result stampInputWindow(Shell *shell) {
  constexpr static char const *head = "Work Time Tracking System (WTTTS)";

  auto const handle = shell->ui.inputWin;
  int width;
  width = getmaxx(stdscr);

  box(handle, 0, 0);
  mvwprintw(handle, 0, width / 2 - strlen(head) / 2, "%s", head);
  return Result::Success;
}

Result updateBanner(Shell *shell) {
  auto const handle = shell->ui.clockWin;
  int width;
  width = getmaxx(stdscr);

  using namespace std::chrono;
  auto now = system_clock::to_time_t(system_clock::now());
  auto text = std::ctime(&now);
  wmove(handle, 1, 0);
  wclrtoeol(handle);
  mvwprintw(handle, 1, width / 2 - strlen(text) / 2, "%s", text);

  wmove(handle, 2, 0);
  wclrtoeol(handle);
  auto notifs = std::to_string(shell->control.notifications.size());
  notifs += " notifications";
  mvwprintw(handle, 2, width / 2 - notifs.size() / 2, "%s", notifs.c_str());

  wmove(handle, 3, 0);
  wclrtoeol(handle);
  std::string activeEmpl = "User: ";
  if (shell->control.activeEmployeeId.size()) {
    auto const id = shell->control.activeEmployeeId;
    assert(shell->esys);
    auto empl = shell->esys->getEmployeeById(id);
    assert(empl);
    activeEmpl += empl->getEmployeeName() + " ";
    activeEmpl += empl->getEmployeeSurname() + " (";
    activeEmpl += es::to_string(empl->getEmployeeRole()) + ")";
  } else
    activeEmpl += "<DEFAULT>";
  mvwprintw(handle, 3, width / 2 - activeEmpl.size() / 2, "%s",
            activeEmpl.c_str());

  box(handle, 0, 0);
  wrefresh(handle);

  return Result::Success;
}

void updateInputBuffer(Shell *shell) {
  if (!shell->ui.inputWin)
    return;
  int ch = wgetch(shell->ui.inputWin);

  switch (ch) {
  case '\n':
    shell->control.inputReady = true;
    break;
  case 127:
    if (shell->control.inputBuffer.size())
      shell->control.inputBuffer.pop_back();
    break;
  default:
    if (shell->control.currentStateType == StateType::ReadInProgress &&
        std::isprint(ch))
      shell->control.inputBuffer.push_back(ch);
    else {
      shell->control.signalReady = true;
      shell->control.signal = ch;
    }
  }
}

void destroyShell(Shell *handle) {
  delwin(handle->ui.clockWin);
  delwin(handle->ui.inputWin);
  destroyInputMenu(handle);
  delete handle;
  endwin();
}

Result createShell(Shell **handle) {
  if (!initscr())
    return Result::NcursesInitError;
  if (!handle)
    return Result::NullptrHandleError;
  auto ptr = new Shell{};
  if (!ptr)
    return Result::ShellMemAllocError;
  *handle = ptr;
  return Result::Success;
}

std::string to_string(Result value) {
  std::string out;

  switch (value) {
  case Result::Success:
    out = "Success";
    break;
  case Result::NotificationPending:
    out = "NotificationPending";
    break;
  case Result::InternalError:
    out = "InternalError";
    break;
  case Result::InputNotValidError:
    out = "InputNotValidError";
    break;
  case Result::MissingStateError:
    out = "MissingStateError";
    break;
  case Result::EmployeeFilterNotValidError:
    out = "EmployeeFilterNotValidError";
    break;
  case Result::MissingInVariableIdError:
    out = "MissingInVariableIdError";
    break;
  case Result::ProcedureDependencyNotSatisfiedError:
    out = "ProcedureDependencyNotSatisfiedError";
    break;
  case Result::MissingStateIdError:
    out = "MissingStateIdError";
    break;
  case Result::StateIdNotUniqueError:
    out = "StateIdNotUniqueError";
    break;
  case Result::ActionIdNotValidError:
    out = "ActionIdNotValidError";
    break;
  case Result::MultipleWritesToVariablePriorToProcedureError:
    out = "MultipleWritesToVariablePriorToProcedureError";
    break;
  case Result::StateDescFileOpenError:
    out = "StateDescFileOpenError";
    break;
  case Result::NullptrHandleError:
    out = "NullptrHandleError";
    break;
  case Result::ShellMemAllocError:
    out = "ShellMemAllocError";
    break;
  case Result::NcursesInitError:
    out = "NcursesInitError";
    break;
  }

  return out;
}
} // namespace sh
