#ifndef CURSES_H
#define CURSES_H

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

Curses& operator << (Curses& curses, const std::string& str);
Curses& operator << (Curses& curses, char ch);
Curses& operator << (Curses& curses, Curses& (*pf)(Curses &));

Curses& operator << (Curses& curses, const curses_manip::color& obj);


#endif