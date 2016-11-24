cmake_minimum_required(VERSION 3.1)
project(nutshell)

add_definitions(
-std=c++14
-pedantic
-Wall
-Wextra
)

add_library(lnutshell STATIC
command/BuiltIn.cpp
command/Command.cpp
command/Directory.cpp
command/Executable.cpp
command/Function.cpp
command/Pid.cpp
command/String.cpp
command/Variable.cpp
io/Cursor.cpp
io/Input.cpp
io/Output.cpp
shell/Assistive.cpp
shell/History.cpp
shell/LineBuffer.cpp
shell/Module.cpp
shell/Path.cpp
shell/Predictive.cpp
shell/Shell.cpp
shell/Utils.cpp
)

add_executable(nutshell
main.cpp
)

find_package(Boost COMPONENTS system REQUIRED QUIET)

target_include_directories(lnutshell
PUBLIC
${CMAKE_SOURCE_DIR}
${Boost_INCLUDE_DIRS}
)

target_link_libraries(lnutshell
stdc++fs

${Boost_LIBRARIES}
)

target_link_libraries(nutshell
-Wl,-whole-archive lnutshell
-Wl,-no-whole-archive
)

install(TARGETS nutshell RUNTIME DESTINATION bin)
