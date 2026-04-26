#pragma once

#include <wtts/logInfo.hpp>

namespace es {
enum class Result { Success, EmployeeIdNotUniqueError, EmployeeNotFoundError };

class Employee {
public:
  Result checkIn();
};

class Driver : public Employee {};

class Manager : public Employee {};

class Admin : public Employee {};
} // namespace es
