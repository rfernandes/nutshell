cmake_minimum_required(VERSION 3.1)
project(nutshell)

add_definitions(
-std=c++14
-pedantic
-Wall
-Wextra
)

add_library(lnutshell STATIC
command/Directory.cpp
command/Executable.cpp
command/Function.cpp
command/Pid.cpp
command/String.cpp
command/Variable.cpp
core/BuiltIn.cpp
core/Command.cpp
core/Module.cpp
core/Utils.cpp
io/Cursor.cpp
io/Input.cpp
io/Output.cpp
shell/Assistive.cpp
shell/History.cpp
shell/LineBuffer.cpp
shell/Path.cpp
shell/Predictive.cpp
shell/Shell.cpp
)

add_executable(nutshell
main.cpp
)

find_package(Boost REQUIRED QUIET)

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


option(cov "Enable profiling and coverage report analysis" OFF)

if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU AND ${cov})
  message(STATUS "Enabling profile build for tests")
  set_target_properties(lnutshell
    PROPERTIES
      COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -fprofile-arcs -ftest-coverage --coverage"
      LINK_FLAGS "--coverage")
  target_link_libraries(lnutshell
    gcov)
endif()


enable_testing()
add_subdirectory(test)
