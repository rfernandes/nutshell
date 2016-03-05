cmake_minimum_required(VERSION 3.1)
project(nutshell)

add_definitions(
-std=c++14
-pedantic
-Wall
-Wextra
)

add_executable(nutshell
Command.cpp
Cursor.cpp
Input.cpp
Output.cpp
Directory.cpp
Executable.cpp
History.cpp
Line.cpp
ParseLine.cpp
Path.cpp
Pid.cpp
Prompt.cpp
Shell.cpp
main.cpp
)

target_include_directories(nutshell
PRIVATE
)

target_link_libraries(nutshell
stdc++fs
)

install(TARGETS nutshell RUNTIME DESTINATION bin)
