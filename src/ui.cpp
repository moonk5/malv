#include <ui.h>

#include <iostream>

moonk5::malv::ui::ui(moonk5::malv::mpd::player &player)
  : m_player(player)
  , m_bold(true)
  , m_fg_color(COLOR_BLACK)
  , m_bg_color(COLOR_WHITE)
  , m_delay(0)
  , m_prev_idx_lyrics(0) {

}

void moonk5::malv::ui::init_screen() {
  setlocale(LC_ALL, "");
  initscr();
  clear();
  noecho();
  cbreak();
  curs_set(0);
  keypad(stdscr, TRUE);
  refresh();
  resize_screen();

  start_color();
  _set_color();
}

void moonk5::malv::ui::close_screen() {
  delwin(m_win_title);
  delwin(m_win_lyrics);
  endwin();
}

void moonk5::malv::ui::clear_screen() { clear(); }

void moonk5::malv::ui::refresh_screen() { refresh(); }

void moonk5::malv::ui::get_screen_size(WINDOW * win, screen_size& scr) {
  getmaxyx(win, scr.h, scr.w);
}

void moonk5::malv::ui::resize_screen(bool recreate) {
  if (recreate) {
    delwin(m_win_title);
    delwin(m_win_lyrics);
    delwin(m_win_status);
    endwin();
    refresh();
  }

  getmaxyx(stdscr, m_scr.h, m_scr.w);

  m_win_title = newwin(1, m_scr.w, 0, 0);
  m_win_lyrics = newwin(m_scr.h - 2, m_scr.w, 1, 0);
  m_win_status = newwin(1, m_scr.w, m_scr.h - 1, 0);

  _set_color();

  write_status_delay(m_player.get_delay());
}

void moonk5::malv::ui::is_bold(bool on_off) {
  m_bold = on_off;
}

short moonk5::malv::ui::get_fg_color() {
  return m_fg_color;
}

void moonk5::malv::ui::set_fg_color(short fg) {
  if (fg != m_fg_color) {
    m_fg_color = 7 & fg;
    _set_color(); 
  }
}

short moonk5::malv::ui::get_bg_color() {
  return m_bg_color;
}

void moonk5::malv::ui::set_bg_color(short bg) {
  if (bg != m_bg_color) {
    m_bg_color = 7 & bg;
    _set_color();
  }
}

void moonk5::malv::ui::toggle_fg_color() {
  m_fg_color = (m_fg_color + 1) & 7;
  _set_color();
}

void moonk5::malv::ui::toggle_bg_color() {
  m_bg_color = (m_bg_color + 1) & 7;
  _set_color();
}

void moonk5::malv::ui::toggle_fg_bold() {
  if (m_bold)
    m_bold = false;
  else
    m_bold = true;
}

void moonk5::malv::ui::write_title(const std::string& title,
    const std::string& artist) {
    screen_size scr_size;
    werase(m_win_title);
    get_screen_size(m_win_title, scr_size);
    
    std::string tmp = artist + " - " + title; 

    wmove(m_win_title, 0, (scr_size.w - tmp.size()) / 2);
    wprintw(m_win_title, "%s", tmp.c_str());
    
    wrefresh(m_win_title);
}

void moonk5::malv::ui::write_text(const std::string& text) {
  // display string at the center of screen
  screen_size scr_size;
  get_screen_size(m_win_lyrics, scr_size);
  wclear(m_win_lyrics);

  write_text((scr_size.w - text.size()) / 2,
      scr_size.h / 2, text);

  wrefresh(m_win_lyrics);
}

void moonk5::malv::ui::write_text(int x, int y, const std::string& text) {
  wmove(m_win_lyrics, y, x);
  wprintw(m_win_lyrics, "%s", text.c_str());
}

void moonk5::malv::ui::write_lyrics(int idx_song_collection, int idx_lyrics,    // add langugage?
    const std::vector<moonk5::alsong::song_info>& collection) {
  screen_size scr_size;
  moonk5::alsong::time_lyrics tl = 
    collection[idx_song_collection].lyrics_collection[idx_lyrics];

  werase(m_win_lyrics);
  get_screen_size(m_win_lyrics, scr_size);

  int lines = tl.lyrics.size();
  int x, y;
  y = (scr_size.h - lines) / 2;
  for (std::string l : tl.lyrics) {
    x = (scr_size.w - l.size()) / 2;
    write_text(x, y++, l);
  }

  wrefresh(m_win_lyrics);
}

void moonk5::malv::ui::write_status_delay(int delay) {
  screen_size scr_size;
  werase(m_win_status);
  get_screen_size(m_win_status, scr_size);

  std::string tmp = "(Delay:" + std::to_string(delay) + ")";

  wmove(m_win_status, 0, scr_size.w - tmp.size());
  wprintw(m_win_status, "%s", tmp.c_str());
  wrefresh(m_win_status);
}


void moonk5::malv::ui::run() {
  int ch = 0;
  bool exit = false;

  init_screen();

  while (!exit) {
    if ((ch = wgetch(stdscr)) == ERR) {
    } else {
      switch (ch) {
        case 10:  m_player.play();            break;
        case 's': m_player.stop();            break;
        case 'p': m_player.pause();           break;
        case '<': m_player.prev_track();      break;
        case '>': m_player.next_track();      break;
        case 'h': m_player.adjust_delay(-200);
                  write_status_delay(m_player.get_delay());       break; 
        case 'g': m_player.adjust_delay(200);
                  write_status_delay(m_player.get_delay());       break;
        case 'f': toggle_fg_color();          break;
        case 'c': toggle_bg_color();          break;
        case 'b': toggle_fg_bold();           break;
        case 'q': exit = true;
                  m_player.quit();            break;
      }
    }
  }

  close_screen();
}

void moonk5::malv::ui::_set_color() {
  const int pair = 1;
  init_pair(pair, m_fg_color, m_bg_color);
  wbkgd(m_win_lyrics, COLOR_PAIR(pair));
  if (m_bold) attron(A_BOLD);
  wrefresh(m_win_lyrics);
}
