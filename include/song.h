#ifndef MALV_SONG_H
#define MALV_SONG_H
#include <memory>
#include <mpd/client.h>

#include <iostream>
namespace moonk5
{
  namespace malv
  {
    namespace mpd
    {
      class song
      {
        public:
          song() {}
          song(int id, mpd_song *song)
            : m_id(id)
            , m_song(song, mpd_song_free) { 
            if (mpd_song_get_tag(m_song.get(), MPD_TAG_TITLE, 0) != nullptr ||
                mpd_song_get_tag(m_song.get(), MPD_TAG_ARTIST, 0) != nullptr) {
              m_title = mpd_song_get_tag(m_song.get(), MPD_TAG_TITLE, 0);
              m_artist = mpd_song_get_tag(m_song.get(), MPD_TAG_ARTIST, 0);
              if (mpd_song_get_tag(m_song.get(), MPD_TAG_ALBUM, 0) != nullptr) {
                m_album = mpd_song_get_tag(m_song.get(), MPD_TAG_ALBUM, 0);
              }
            } else {
              std::cerr << "New song detected with tags missing!!!\n";
            }
          }

          int id() { return m_id; }
          std::string title() { return m_title; }
          std::string artist() { return m_artist; }
          std::string album() { return m_album; }

        private:
          int m_id;
          std::string m_title = "";
          std::string m_artist = "";
          std::string m_album = "";
          std::shared_ptr<mpd_song> m_song;
      };
    }
  }
}
#endif // MALV_SONG_H
