#ifndef MALV_UI_H
#define MALV_UI_H

#include <curses.h>
#include <signal.h>

#include <alsong.h>
#include <player.h>

namespace moonk5
{
  namespace malv
  {
    class ui
    {
      public:
        struct screen_size {
          size_t h;
          size_t w;
        };

        ui(moonk5::malv::mpd::player &player);

        void init_screen();
        void close_screen();
        void clear_screen();
        void refresh_screen();

        void get_screen_size(WINDOW * win, screen_size& scr);
        void resize_screen(bool recreate=false);
        
        void is_bold(bool on_off);
        short get_fg_color();
        void set_fg_color(short fg);
        short get_bg_color();
        void set_bg_color(short bg);
        
        void toggle_fg_color();
        void toggle_bg_color();
        void toggle_fg_bold();

        void write_title(const std::string& title, const std::string& artist);
        
        void write_text(const std::string& text);
        void write_text(int x, int y, const std::string& text);
        void write_lyrics(int idx_song_collection, int idx_lyrics,    // add langugage?
            const std::vector<moonk5::alsong::song_info>& collection);
        
        void write_status_delay(int delay);

        // main loop
        void run();
        
      private:
        void _set_color();
        
        screen_size m_scr;
        
        WINDOW * m_win_title;
        WINDOW * m_win_lyrics;
        WINDOW * m_win_status;

        bool m_bold;
        short m_fg_color;
        short m_bg_color;

        int m_delay;
        int m_prev_idx_lyrics;

        moonk5::malv::mpd::player &m_player;
    };
  }
}
#endif // MALV_UI_H
