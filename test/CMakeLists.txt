add_executable(nutshellTest
CursorCase.cpp
InputCase.cpp
LineBufferCase.cpp
HistoryCase.cpp
UtilsCase.cpp
)

find_package(Boost 1.62 REQUIRED COMPONENTS unit_test_framework)

target_link_libraries(nutshellTest
-Wl,-whole-archive lnutshell
-Wl,-no-whole-archive
${Boost_LIBRARIES}
)

target_include_directories(nutshellTest PRIVATE
${Boost_INCLUDE_DIRS}
)


if(${CMAKE_CXX_COMPILER_ID} MATCHES GNU AND ${cov})
  message(STATUS "Enabling profile build for tests")
  set_target_properties(lnutshell
    PROPERTIES
      COMPILE_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 --coverage"
      LINK_FLAGS "-fprofile-arcs")
endif()


add_test(NAME nutshellTest COMMAND nutshellTest)
