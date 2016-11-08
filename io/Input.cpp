#include "Input.h"

#include <iostream>

#include <termios.h>
#include <unistd.h>

using namespace std;

namespace {
  struct termios oldt,newt;

  unsigned decode(istream& in, char ch) {
    in.get(); // Skip
    switch (ch = in.get()) {
      case 'A': return Input::Up;
      case 'B': return Input::Down;
      case 'C': return Input::Right;
      case 'D': return Input::Left;
      case 'H': return Input::Home;
      case 'F': return Input::End;
      case 'M': return Input::CtrlM;
      case '1': {
        in.get(); // Skip ;
        switch (ch = in.get()) {
          case '5': {
            switch (ch = in.get()) {
              case 'A': return Input::CtrlUp;
              case 'B': return Input::CtrlDown;
              case 'C': return Input::CtrlRight;
              case 'D': return Input::CtrlLeft;
              case 'H': return Input::CtrlHome;
              case 'F': return Input::CtrlEnd;
              default : cout << " >>" << +ch << "<<"; return Input::Unknown;
            }
          }
          default:
            return Input::Unknown;
        }
      }
      case '3': in.get(); return Input::Delete; // Skip tilde
      default : cout << " >>" << +ch << "<<"; return Input::Unknown;
    }
  }
}

Input::Input(istream& in)
: _in(in)
{
  std::ios_base::sync_with_stdio(false);

  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

Input::~Input() {
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

unsigned int Input::get() {
  int ch {_in.get()};
  return ch == '\x1b' ? decode(_in, ch) : ch;
}
