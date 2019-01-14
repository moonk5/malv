#include <curses.h>
#include <chrono>
#include <exception>
#include <iostream>
#include <string>
#include <thread>
#include <vector>

#include <player.h>
#include <ui.h>
#include <lyrics.h>

int main(int argc, char *argv[])
{
  std::string mpd_host = "127.0.0.1";
  int mpd_port = 6611;
  
  if (argc > 2) {
    mpd_host = argv[1];
    mpd_port = std::stoi(argv[2]);
  }

  moonk5::malv::mpd::player m_player(mpd_host, mpd_port);
  moonk5::malv::ui m_ui(m_player);
  moonk5::malv::lyrics_synchronizer m_lyrics(m_player, m_ui);

  std::thread player_thread(&moonk5::malv::mpd::player::run,
      std::ref(m_player));
  std::thread ui_thread(&moonk5::malv::ui::run, std::ref(m_ui));
  std::thread lyrics_thread(&moonk5::malv::lyrics_synchronizer::run,
      std::ref(m_lyrics));

  player_thread.join();
  ui_thread.join();
  lyrics_thread.join();

  return 0;
}
