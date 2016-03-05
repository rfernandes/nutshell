#ifndef INPUT_H
#define INPUT_H

class Input{
public:

  enum Key{
    Backspace = 127,

    Unknown = 257,
    Left,
    Right,
    Home,
    End,
    Up,
    Down,
    Delete
  };

  Input();
  ~Input();

  unsigned get();
};

#endif
