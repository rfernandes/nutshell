#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "Command.h"
#include "Line.h"

#include <experimental/filesystem>

class Curses;

class Directory {
  using path = std::experimental::filesystem::path;

  path _current;
public:
  Directory();

  Command::Status cd(const Line& parameters, Curses& curses);

  /// get the current directory
  const path& cd() const;
};

#endif
