cmake_minimum_required(VERSION 3.25)
project(wtts LANGUAGES CXX)
include(CTest)

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
set(CMAKE_CXX_STANDARD_REQUIRED 1)
set(CMAKE_CXX_STANDARD 20)

find_package(PugiXML REQUIRED)

find_path(HARU_INCLUDE_DIR NAMES hpdf.h)
find_library(HARU_LIBRARY NAMES hpdf)

if(HARU_INCLUDE_DIR AND HARU_LIBRARY)
    message(STATUS "Znaleziono libHaru w: ${HARU_LIBRARY}")
    include_directories(${HARU_INCLUDE_DIR})
else()
    message(FATAL_ERROR "Nie znaleziono biblioteki libHaru! Upewnij sie, ze jest zainstalowana.")
endif()

include_directories(include)
add_subdirectory(src)
