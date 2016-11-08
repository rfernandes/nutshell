#ifndef INPUT_H
#define INPUT_H

#include <iostream>

class Input{
  std::istream& _in;
public:

  enum Key{
    Backspace = 127,

    Unknown = 257,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    CtrlUp,
    CtrlDown,
    CtrlLeft,
    CtrlRight,
    CtrlHome,
    CtrlEnd,
    CtrlM,
    Delete,
  };

  Input(std::istream &in);
  ~Input();

  unsigned get();
};

#endif
