#include <chrono>
#include <iostream>
#include <thread>

#include <sys/eventfd.h>
#include <unistd.h>
#include <poll.h>

#include <player.h>

moonk5::malv::mpd::player::player(const std::string& host, unsigned int port)
  : m_host(host)
  , m_port(port)
  , m_conn(mpd_connection_new(host.c_str(), port, 0))
  , m_running(true)
  , m_delay(0)
{
  if (mpd_connection_get_error(m_conn) != MPD_ERROR_SUCCESS)
    throw std::runtime_error(
        std::string("Connecting to MPD failed ...")
        + mpd_connection_get_error_message(m_conn)
    );

  m_event_fd = eventfd(0, EFD_NONBLOCK);
  
  m_status = status(mpd_run_status(m_conn));
  if (m_status.state() == MPD_STATE_PLAY) {
      m_state = mpd::player::state::PLAY;
      m_curr_song = song(m_status.song_id(), mpd_run_current_song(m_conn));
  } else {
    m_state = static_cast<mpd::player::state>(m_status.state());
  }
}

moonk5::malv::mpd::player::~player() 
{
  close(m_event_fd);
}

void moonk5::malv::mpd::player::play()
{
  add_external_task([](mpd_connection *c) {
    mpd_run_play(c);
  });
}

void moonk5::malv::mpd::player::stop()
{
  add_external_task([](mpd_connection *c) {
    mpd_run_stop(c);
  });
}

void moonk5::malv::mpd::player::pause()
{
  add_external_task([](mpd_connection *c) {
    mpd_run_toggle_pause(c);
  });
}

void moonk5::malv::mpd::player::prev_track()
{
  add_external_task([](mpd_connection *c) {
    mpd_run_previous(c);
  });
}

void moonk5::malv::mpd::player::next_track()
{
  add_external_task([](mpd_connection *c) {
      mpd_run_next(c);
  });
}

void moonk5::malv::mpd::player::random()
{
  add_external_task([](mpd_connection *c) {
    //mpd_run_random(c);
  });
}

void moonk5::malv::mpd::player::adjust_delay(int delay)
{
  m_delay += delay;
}

void moonk5::malv::mpd::player::stop_thread()
{
  m_running = false;
  add_external_task([](mpd_connection *c) {
    mpd_connection_free(c);
  });
}

moonk5::malv::mpd::player::state moonk5::malv::mpd::player::get_state()
{
  return m_state;
}

int moonk5::malv::mpd::player::get_delay()
{
  return m_delay;
}

moonk5::malv::mpd::song& moonk5::malv::mpd::player::get_curr_song()
{
  typedef std::promise<mpd::song&> promise_type;
  auto promise_ptr = std::make_shared<promise_type>();

  promise_ptr->set_value(m_curr_song);
  eventfd_write(m_event_fd, 1);

  return promise_ptr->get_future().get();
}

unsigned moonk5::malv::mpd::player::get_elapsed()
{
  typedef std::promise<unsigned> promise_type;
  auto promise_ptr = std::make_shared<promise_type>();

  promise_ptr->set_value(m_status.elapsed_ms());
  eventfd_write(m_event_fd, 1);

  return promise_ptr->get_future().get();
}

void moonk5::malv::mpd::player::run()
{ 
  m_conn = mpd_connection_new(m_host.c_str(), m_port, 0);
  if (!m_conn) {
    std::cerr << "[ERR]mpd_manager::_start - failed to connect with mpd\n";
    return;
  }

  while (m_running) {
    
    mpd_send_idle_mask(m_conn, 
        static_cast<mpd_idle>(
          MPD_IDLE_PLAYER | 
          MPD_IDLE_OPTIONS | 
          MPD_IDLE_PLAYLIST));

    struct pollfd pollfds[] =
    {{m_event_fd, POLLIN, 0}
      ,{mpd_connection_get_fd(m_conn), POLLIN, 0}};

    if (poll(pollfds, 2, -1) <= 0) {
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else {
      uint64_t dummy;
      eventfd_read(m_event_fd, &dummy);
    }

    enum mpd_idle idle_event = mpd_run_noidle(m_conn);
    if (idle_event & MPD_IDLE_PLAYER) {
      //std::cout << "MPD_IDLE_PLAYER\n";
            
      m_status = status(mpd_run_status(m_conn));
      switch (m_status.state()) {
        case MPD_STATE_STOP:
          //std::cout << "STATE: STOP\n";
          m_state = mpd::player::state::STOP;
          break;
        case MPD_STATE_PLAY:
          //std::cout << "STATE: PLAY\n";
          m_state = mpd::player::state::PLAY;
          m_curr_song = song(m_status.song_id(), mpd_run_current_song(m_conn));
          break;
        case MPD_STATE_PAUSE:
          //std::cout << "STATE : PAUSE\n";
          m_state = mpd::player::state::PAUSE;
          break;
        default:
          //std::cout << "STATE : UNKNOWN\n";
          m_state = mpd::player::state::UNKNOWN;
          break;
      };
    }

    if (idle_event & MPD_IDLE_OPTIONS) {
      //std::cout << "MPD_IDLE_OPTIONS\n";

    }

    if (idle_event & MPD_IDLE_PLAYLIST) {
      //std::cout << "MPD_IDLE_PLAYLIST\n";
    }

    std::lock_guard<std::mutex> lock(m_external_tasks_mutex);
    while (!m_external_tasks.empty()) {
      m_external_tasks.front()(m_conn);
      m_external_tasks.pop_front();
    }
  }
}

void moonk5::malv::mpd::player::quit()
{
  m_state = mpd::player::state::QUIT;
}

void moonk5::malv::mpd::player::add_external_task(std::function<void(mpd_connection *)> t)
{
  std::lock_guard<std::mutex> lock(m_external_tasks_mutex);
  m_external_tasks.push_back(t);

  eventfd_write(m_event_fd, 1);
}
