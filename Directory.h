#ifndef DIRECTORY_H
#define DIRECTORY_H

#include "Command.h"
#include "Line.h"
#include "Curses.h"

#include <experimental/filesystem>


class Cd: public Command {
  using path = std::experimental::filesystem::path;

  path _current;
  std::vector<path> _history; // TODO: continue from here fix this
  size_t _idx;
public:
  Cd();

  Command::Status execute(const Line & line, Curses & curses) override;
  bool matches(const Line & line) const override;
  Suggestions suggestions(const Line & line) const override;

  Command::Status cd(const Line& parameters, Curses& curses);

  /// get the current directory
  const path& cwd() const;

  friend class CdVisitor;
};

#endif