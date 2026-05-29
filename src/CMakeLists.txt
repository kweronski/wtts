find_package(Curses REQUIRED)

add_compile_options(-O0 -g -pg -Werror -Wall -Wextra -Wpedantic)
#add_compile_options(-O3 -Wall -Wextra -Wpedantic)

add_library(dataparser
	dataParser/xmlParser.cpp
	dataParser/xmlWriter.cpp
	dataParser/dataParser.cpp
	dataParser/internals.cpp
	logInfo.cpp
)
target_link_libraries(dataparser PRIVATE pugixml::pugixml)

add_library(employeeSystem
	employeeSystem/employeeSystem.cpp
	employeeSystem/factory.cpp
	employeeSystem/employeeData.cpp
	employeeSystem/attendanceTable.cpp
	employeeSystem/employee.cpp
	employeeSystem/interactiveCLI.cpp
	employeeSystem/esResult.cpp
	logInfo.cpp)
target_link_libraries(employeeSystem PRIVATE dataparser pugixml::pugixml)

add_executable(wttsapp main.cpp)
target_link_libraries(wttsapp PRIVATE employeeSystem ncurses menu panel form)

add_subdirectory(dataParser/test)
