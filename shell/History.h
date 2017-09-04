#ifndef HISTORY_H
#define HISTORY_H

#include <core/Command.h>
#include <core/Module.h>
#include <shell/Line.h>

#include <chrono>
#include <string_view>

class History: public Module {
public:
  struct Entry {
    Line line;
    int status;
    std::chrono::system_clock::time_point startTime, endTime;
  };

private:
  std::chrono::system_clock::time_point _startTime;
  std::vector<Entry> _history;
  std::vector<Entry>::const_iterator _idx;

  friend class Visitor;

  void add(const Entry& entry);
  const Line& forward();
  const Line& backward();
public:
  History();

  History(const History&) = delete;
  ~History() override;

  void clear();
  const std::vector<Entry>& list() const;

  std::string_view suggest(const Line& line) const;

  void commandExecute(const Line &line) override;
  void commandExecuted(const ParseResult &parseResult, const Line &line) override;
  bool keyPress(unsigned int keystroke, Shell &shell) override;
};

#endif
