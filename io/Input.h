#ifndef INPUT_H
#define INPUT_H

class Input{
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
    Delete
  };

  Input();
  ~Input();

  unsigned get();
};

#endif
