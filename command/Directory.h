#ifndef DIRECTORY_H
#define DIRECTORY_H

#include <command/DirectoryGrammar.h>

#include <experimental/filesystem>

struct CdTrait{
  using Rule = parser::directory_type;
  using Data = ast::CdCommand;

  static const Rule& rule();
};

class Cd: public parser::RuleCommand<CdTrait> {
  using path = std::experimental::filesystem::path;

  path _current;
  path _home;
  std::vector<path> _history; // TODO: continue from here fix this
  size_t _idx;

  bool matches(const Line & line) const;

public:
  Cd();
  ~Cd() override;

  void execute(const typename CdTrait::Data & data, Output & output) override;

  Status cd(const Line& parameters, Output& curses);

  /// get the current directory
  const path& cwd() const;

  /// get the current user's home directory
  const path& home() const;

  friend class CdVisitor;
};

#endif
