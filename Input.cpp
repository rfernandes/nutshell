#include "Input.h"

#include <iostream>

#include <termios.h>
#include <unistd.h>

using namespace std;

namespace {
  struct termios oldt,newt;

  unsigned decode(char ch) {
    cin.get(); // Skip
    switch (ch = cin.get()) {
      case 'A': return Input::Up;
      case 'B': return Input::Down;
      case 'C': return Input::Right;
      case 'D': return Input::Left;
      case 'H': return Input::Home;
      case 'F': return Input::End;
      case '3': cin.get(); return Input::Delete; // Skip tilde
      default : cout << " | " << ch << "|"; return Input::Unknown;
    }
  }
}

Input::Input() {
  tcgetattr(STDIN_FILENO, &oldt);
  newt = oldt;
  newt.c_lflag &= ~(ICANON | ECHO);
  tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

Input::~Input() {
  tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

unsigned int Input::get() {
  int ch {cin.get()};
  return ch == '\x1b' ? decode(ch) : ch;
}
