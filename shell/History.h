#ifndef HISTORY_H
#define HISTORY_H

#include <command/Command.h>
#include <shell/Line.h>
#include <shell/Module.h>

#include <chrono>
#include <experimental/string_view>

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
  const Line& forward(const Line& current);
  const Line& backward(const Line& current);
public:
  History();

  History(const History&) = delete;
  ~History() override;

  void clear();
  const std::vector<Entry>& list() const;

  std::experimental::string_view suggest(const Line& line) const;

  void commandExecute(const Line &line, Shell &shell) override;
  void commandExecuted(const ParseResult &parseResult, Shell &shell) override;
  bool keyPress(unsigned int keystroke, Shell &shell) override;
};

#endif
