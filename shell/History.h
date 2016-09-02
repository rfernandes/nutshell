#ifndef HISTORY_H
#define HISTORY_H

#include <command/Command.h>
#include <shell/Line.h>

#include <chrono>
#include <experimental/string_view>

class History: public Command {
public:
  struct Entry
  {
    Line line;
    int status;
    std::chrono::system_clock::time_point startTime, endTime;
  };
    
  std::vector<Entry> _history;
  std::vector<Entry>::const_iterator _idx;

  friend class Visitor;
public:
  History();

  History(const History&) = delete;
  ~History() override;

  void add(const Entry& entry);
  const Line& forward(const Line& current);
  const Line& backward(const Line& current);

  void clear();
  const std::vector<Entry>& list() const;

  std::experimental::string_view suggest(const Line& line) const;

  Description parse(const Line& line, Output& output, bool execute) override;
};

#endif
