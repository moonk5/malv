#ifndef MALV_STATUS_H
#define MALV_STATUS_H
#include <mpd/client.h>

namespace moonk5
{
  namespace malv
  {
    namespace mpd
    {
      class status
      {
        public:
          status() { }
          status(mpd_status *status)
            : m_status(status, mpd_status_free) { }

          bool repeat() { return mpd_status_get_repeat(m_status.get()); }
          bool random() { return mpd_status_get_random(m_status.get()); }

          unsigned queue_length() { return mpd_status_get_queue_length(m_status.get()); }
          unsigned queue_version() { return mpd_status_get_queue_version(m_status.get()); }

          mpd_state state() { return mpd_status_get_state(m_status.get()); }
          
          int song_pos() { return mpd_status_get_song_pos(m_status.get()); }
          int song_id() { return mpd_status_get_song_id(m_status.get()); }

          int next_song_pos() { return mpd_status_get_next_song_pos(m_status.get()); }
          int next_song_id() { return mpd_status_get_next_song_id(m_status.get()); }
          
          unsigned elapsed() { return mpd_status_get_elapsed_time(m_status.get()); }
          unsigned elapsed_ms() { return mpd_status_get_elapsed_ms(m_status.get()); }
          unsigned total_time() { return mpd_status_get_total_time(m_status.get()); }

          const char * error() { return mpd_status_get_error(m_status.get()); }

        private:
          std::shared_ptr<mpd_status> m_status;
      };
    }
  }
}
#endif // MALV_STATUS_H
