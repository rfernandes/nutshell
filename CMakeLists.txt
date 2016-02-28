cmake_minimum_required(VERSION 3.1)
project(nutshell)

add_definitions(
-std=c++14
-pedantic
-Wall
-Wextra
)

set(CURSES_NEED_NCURSES TRUE)
find_package(Curses REQUIRED QUIET)

add_executable(nutshell
Command.cpp
Curses.cpp
Directory.cpp
Executable.cpp
History.cpp
Line.cpp
Path.cpp
Pid.cpp
Prompt.cpp
Shell.cpp
Terminal.cpp
main.cpp
)

target_include_directories(nutshell
PRIVATE
${CURSES_INCLUDE_DIR}
)

target_link_libraries(nutshell
${CURSES_LIBRARIES}
stdc++fs
)

install(TARGETS nutshell RUNTIME DESTINATION bin)
