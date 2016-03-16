cmake_minimum_required(VERSION 3.1)
project(nutshell)

add_definitions(
-std=c++14
-pedantic
-Wall
-Wextra
)

add_executable(nutshell
command/Command.cpp
command/Directory.cpp
command/Executable.cpp
command/Pid.cpp
command/Variable.cpp
io/Cursor.cpp
io/Input.cpp
io/Output.cpp
main.cpp
shell/History.cpp
shell/Line.cpp
shell/Path.cpp
shell/Prompt.cpp
shell/Shell.cpp
)

target_include_directories(nutshell
PRIVATE
${CMAKE_SOURCE_DIR}
)

target_link_libraries(nutshell
stdc++fs
)

install(TARGETS nutshell RUNTIME DESTINATION bin)
