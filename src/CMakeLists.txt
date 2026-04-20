set(CMAKE_BUILD_RPATH ${CMAKE_BUILD_RPATH} /usr/local/lib64)
set(CMAKE_INSTALL_RPATH ${CMAKE_BUILD_RPATH} /usr/local/lib64)

add_compile_options(-O0 -g)

add_library(dataparser dataParser/xmlParser.cpp)
target_link_libraries(dataparser PRIVATE pugixml::pugixml)

add_executable(wttsapp main.cpp)
target_link_libraries(wttsapp PRIVATE dataparser)
