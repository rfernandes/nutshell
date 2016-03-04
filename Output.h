#ifndef OUTPUT_H
#define OUTPUT_H

#include <type_traits>
#include <string>

class Output {
public:
  Output();

  unsigned get();
  void refresh();
};

namespace manip {

class color{
  unsigned _pairId;
public:
  explicit color (unsigned pairId);
  unsigned pairId() const;
};

Output& reset(Output& curses);

}

Output& operator << (Output& curses, const std::string& str);
Output& operator << (Output& curses, char ch);

template <typename Integer,
          typename = std::enable_if_t<std::is_integral<Integer>::value>>
Output& operator << (Output& curses, Integer integral){
  return curses << std::to_string(integral);
}

Output& operator << (Output& curses, Output& (*pf)(Output &));

Output& operator << (Output& curses, const manip::color& obj);

#endif
