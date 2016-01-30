cmake_minimum_required(VERSION 3.1)
project(nutshell)

add_definitions(
-std=c++14
-DBOOST_SYSTEM_NO_DEPRECATED
-pedantic
-Wall
-Wextra
)

find_package(Boost COMPONENTS filesystem REQUIRED QUIET)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED QUIET)

add_executable(nutshell
Command.cpp
Prompt.cpp
Shell.cpp
Terminal.cpp
main.cpp
)

target_include_directories(nutshell
PRIVATE
${Boost_INCLUDE_DIRS}
${CURSES_INCLUDE_DIR}
)

message(STATUS "Hi ${CURSES_NCURSES_LIBRARY}")


target_link_libraries(nutshell
${Boost_LIBRARIES}
${CURSES_LIBRARIES}
)

install(TARGETS nutshell RUNTIME DESTINATION bin)
