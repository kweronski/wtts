add_executable(testPayCalculator payCalculator.cpp)

target_link_libraries(testPayCalculator PRIVATE employeeSystem)

add_test(NAME payCalculatorTest001 COMMAND testPayCalculator
	1 # number of hours worked
	250 # hourly wage
)

add_test(NAME payCalculatorTest002 COMMAND testPayCalculator
	8 # number of hours worked
	250 # hourly wage
)

add_test(NAME payCalculatorTest003 COMMAND testPayCalculator
	40 # number of hours worked
	250 # hourly wage
)

add_test(NAME payCalculatorTest003 COMMAND testPayCalculator
	160 # number of hours worked
	250 # hourly wage
)
