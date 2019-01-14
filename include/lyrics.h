#ifndef MALV_LYRICS_H
#define MALV_LYRICS_H

#include <string>
#include <vector>

#include <mpd/client.h>

#include <alsong.h>
#include <player.h>
#include <ui.h>

// TODO
// - Rewind support - X
// - bug fixed : repeating same song - X
// - bug fixed : unmated title and artist between local and server tag - X
// - collection select
// - language on off (JAP, KOR, ENG)
// - delay +/- - X

namespace moonk5
{
  namespace malv
  {
    class lyrics_synchronizer
    {
      public:
        lyrics_synchronizer(malv::mpd::player& mpd, malv::ui& ui);
          
        void run();

        int get_len_song_collection();
        void set_song_collection(int index);

        int get_len_song_language();
        void set_song_language(int index);

      private:
        bool _load_lyrics(const std::string& title, const std::string& artist);
        void _advance_idx_lyrics(int total_elapsed_time, int& idx_lyrics,
            const std::vector<alsong::song_info>& collection);

        malv::mpd::player& m_player;
        malv::ui& m_ui;

        moonk5::alsong::lyrics_serializer m_lyrics;

        int m_len_song_collection;
        int m_idx_song_collection;

        int m_len_song_language;
        int m_idx_song_language;
    };
  }
}
#endif // MALV_LYRICS_H
