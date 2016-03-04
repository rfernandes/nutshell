#ifndef CURSES_H
#define CURSES_H

#include <type_traits>
#include <string>

class Curses {
public:
  Curses();

  unsigned get();
  void refresh();
};

namespace curses_manip {

class color{
  unsigned _pairId;
public:
  explicit color (unsigned pairId);
  unsigned pairId() const;
};

Curses& reset(Curses& curses);

} // namespace curses_manip

Curses& operator << (Curses& curses, const char * cstr);
Curses& operator << (Curses& curses, const std::string& str);
Curses& operator << (Curses& curses, char ch);

template <typename Integer,
          typename = std::enable_if_t<std::is_integral<Integer>::value>>
Curses& operator << (Curses& curses, Integer integral){
  return curses << std::to_string(integral);
}

Curses& operator << (Curses& curses, Curses& (*pf)(Curses &));

Curses& operator << (Curses& curses, const curses_manip::color& obj);

class Cursor{
public:
  struct Position{
    unsigned x, y;
  };

  void up(unsigned rows = 1);
  void down(unsigned rows = 1);
  void left(unsigned columns = 1);
  void right(unsigned columns = 1);
  void position(Position position);
  Position position() const;
};

#endif
