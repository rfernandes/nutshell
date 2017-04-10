#ifndef LINEBUFFER_H
#define LINEBUFFER_H

#include <core/Module.h>
#include <io/Cursor.h>
#include <shell/Line.h>

class LineBuffer: public Module{
  std::string _line;
  std::size_t _idx {0};
public:

  LineBuffer();
  explicit LineBuffer(std::string line);

  void backwardKillWord();

  void clear();

  void insert(unsigned ch);
  std::size_t idx() const;
  std::size_t pos() const;

  void line(const std::string& line);
  const std::string& line() const;

  std::experimental::string_view firstWord() const;

  bool keyPress(unsigned int keystroke, Shell & shell) override;
  void commandExecuted(const ParseResult & parseResult, const Line & line) override;
  void lineUpdated(const ParseResult & parseResult, const LineBuffer & line, Shell & shell) override;
};

#endif
