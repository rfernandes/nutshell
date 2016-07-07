#ifndef HISTORY_H
#define HISTORY_H

#include <command/Command.h>
#include <shell/Line.h>

#include <experimental/string_view>

class History: public Command {
  std::vector<Line> _history;
  std::vector<Line>::const_iterator _idx;

  friend class Visitor;
public:
  History();

  History(const History&) = delete;
  ~History() override;

  void add(const Line& command);
  const Line& forward(const Line& current);
  const Line& backward(const Line& current);

  void clear();
  const std::vector<Line>& list() const;

  std::experimental::string_view suggest(const Line& line) const;

  Description parse(const Line& line, Output& output, bool execute) override;
};

#endif
