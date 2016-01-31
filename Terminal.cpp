#include "Terminal.h"

#include <stdexcept>

#include <ncurses.h>

Terminal::Terminal() {
  setlocale(LC_ALL, "");
  initscr();
  cbreak();
  raw();
  noecho();
  scrollok(stdscr, TRUE);
  keypad(stdscr, TRUE);
  if (!has_colors()) {
    endwin();
    throw std::runtime_error("No color support");
  }
  start_color();
  use_default_colors();
}

Terminal::~Terminal(){
  endwin();
}