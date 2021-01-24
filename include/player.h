#ifndef MALV_PLAYER_H
#define MALV_PLAYER_H

#include <chrono>
#include <future>
#include <thread>
#include <memory>
#include <mutex>
#include <queue>

#include <mpd/client.h>
#include <status.h>
#include <song.h>

namespace moonk5
{
  namespace malv
  {
    namespace mpd
    {
      class player
      {
        public:
          enum state {
            UNKNOWN = 0,
            STOP,
            PLAY,
            PAUSE,
            QUIT
          };

          player(const std::string& host, unsigned int port);
          ~player();

          // inherited functions from class state
          void play(); 
          void stop();
          void pause();
          void prev_track();
          void next_track(); 
          void random();

          void run();
          void quit();

          void start_thread();
          void stop_thread();

          /*
             enum mpd_state { MPD_STATE_UNKNOWN = 0, MPD_STATE_STOP = 1,
             MPD_STATE_PLAY = 2, MPD_STATE_PAUSE = 3 }
             */
          state get_state();
          song& get_curr_song();
          song& get_next_song();
          
          int get_delay();
          void set_delay(int delay);
          void adjust_delay(int delay);
          
          unsigned get_elapsed();

        private:
          int m_event_fd;
          bool m_running;

          std::string m_host;
          unsigned int m_port;

          struct mpd_connection *m_conn;
          class status m_status;
          class song m_curr_song;
          class song m_next_song;
          state m_state;

          void add_external_task(std::function<void(mpd_connection *)> t);

          std::mutex m_external_tasks_mutex;
          std::deque<std::function<void(mpd_connection *)>> m_external_tasks;

          int m_delay;
      };
    }
  }
}
#endif // MALV_PLAYER_H
