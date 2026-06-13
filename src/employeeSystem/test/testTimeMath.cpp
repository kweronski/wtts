#include <iostream>
#include <cmath>
#include <wtts/logInfo.hpp>

bool hasErrors = false;
#define ASSERT_EQUAL(expected, actual, testName)                               \
  do {                                                                         \
    if (std::abs((expected) - (actual)) > 0.001) {                             \
      std::cerr << "[ERROR] " << testName << " | Expected: " << (expected)     \
                << ", but got: " << (actual) << "\n";                          \
      hasErrors = true;                                                        \
    } else {                                                                   \
      std::cout << "[SUCCESS] " << testName << "\n";                           \
    }                                                                          \
  } while (0)

#define ASSERT_BOOL(expected, actual, testName)                                \
  do {                                                                         \
    if ((expected) != (actual)) {                                              \
      std::cerr << "[ERROR] " << testName << " | Expected bool: "              \
                << (expected ? "true" : "false")                               \
                << ", but got: " << (actual ? "true" : "false") << "\n";       \
      hasErrors = true;                                                        \
    } else {                                                                   \
      std::cout << "[SUCCESS] " << testName << "\n";                           \
    }                                                                          \
  } while (0)

int main() {
  // Test case 1: Time difference crossing midnight
  // 2026-12-31 23:50 to 2027-01-01 00:10
  tu::TimePoint tp1{2026, 12, 31, 23, 50};
  tu::TimePoint tp2{2027, 1, 1, 0, 10};

  double diffMinutes = tp2 - tp1;
  ASSERT_EQUAL(20.0, diffMinutes, "Time difference crossing midnight");

  // Test case 2: Edge cases on year/month transition for comparison operators
  tu::TimePoint endOfYear{2026, 12, 31, 23, 59};
  tu::TimePoint startOfNextYear{2027, 1, 1, 0, 0};

  ASSERT_BOOL(true, endOfYear < startOfNextYear, "Dec 31 23:59 < Jan 1 00:00");
  ASSERT_BOOL(false, endOfYear >= startOfNextYear, "Dec 31 23:59 >= Jan 1 00:00");

  return hasErrors ? 1 : 0;
}