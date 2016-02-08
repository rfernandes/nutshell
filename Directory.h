#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <experimental/filesystem>

class Command;

class Directory {
  using path = std::experimental::filesystem::path;

  path _current;
public:
  Directory(Command& command);

  std::string cd(std::string parameters);

  /// get the current directory
  const path& cd() const;
};

#endif
