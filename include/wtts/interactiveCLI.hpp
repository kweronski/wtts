#pragma once

#include <string>

namespace sh {
enum class Result {
  Success,
  NullptrHandleError,
  ShellMemAllocError,
  NcursesInitError,
  StateDescFileOpenError,
  MissingStateIdError,
  StateIdNotUniqueError,
  ActionIdNotValidError,
  MultipleWritesToVariablePriorToProcedureError,
  MissingInVariableIdError,
  ProcedureDependencyNotSatisfiedError,
  EmployeeFilterNotValidError,
  MissingStateError,
  InputNotValidError,
  InternalError,
  NotificationPending
};

std::string to_string(Result);

struct Shell;

Result createShell(Shell **handle);
void destroyShell(Shell *handle);
Result run(Shell *handle);
} // namespace sh
