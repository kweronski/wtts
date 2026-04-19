#pragma once

#include <string>
#include <chrono>
#include <list>

// PHASE 0 - LOAD DATA FROM DISK
enum class UserType {
    unknown, manager, driver, cleaner, chef, waiter, admin
};

struct TimePeriod {
    std::chrono::time_point<std::chrono::system_clock> start, end;
};

struct User {
    std::string name, surname, telephone;
    UserType role;
    std::chrono::seconds standardWorkTime, maxWorkTime;

		std::list<TimePeriod> attendance;
};

struct AbsenceLog {

};

struct DataStorage {
    std::list<User> users;

};
