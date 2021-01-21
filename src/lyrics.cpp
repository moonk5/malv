#include <iostream>
#include <signal.h>

#include <lyrics.h>

volatile bool signal_window_changed = false; 
void _win_resize_handler(int sig) {
  if (sig == SIGWINCH)
    signal_window_changed = true;
}

moonk5::malv::lyrics_synchronizer::lyrics_synchronizer
  (malv::mpd::player& mpd, malv::ui& ui)
  : m_player(mpd)
  , m_ui(ui)
  , m_len_song_collection(0)
  , m_idx_song_collection(0) {

  signal(SIGWINCH, _win_resize_handler);
}

void moonk5::malv::lyrics_synchronizer::run() {
  mpd::song m_song;
  int delay = 0; // unit in ms

  int mpd_elapsed_time = 0;   // base timestamp
  int total_elapsed_time = 0; // working timestamp

  int state = m_player.get_state();
  int idx_lyrics = 0;
  bool lyrics_exist = false;
  std::string prompt_msg = "";

  struct moonk5::malv::ui::screen_size m_scr; 

  while (state != moonk5::malv::mpd::player::state::QUIT) {
    state = m_player.get_state();
    if (state == mpd::player::state::PLAY) {
      mpd::song& tmp = m_player.get_curr_song();
      if (tmp.id() != m_song.id() && tmp.title() != m_song.title()) { 
        m_song = tmp;
        delay = 0;
        idx_lyrics = 0;
        m_idx_song_collection = 0;
        m_len_song_collection = m_lyrics.get_song_collection().size();
        lyrics_exist = false;

        if (_load_lyrics(m_song.title(), m_song.artist())) {
          lyrics_exist = true;
          mpd_elapsed_time = m_player.get_elapsed();
          total_elapsed_time = mpd_elapsed_time;
          // advance  if total_time != 0
          // which means mpd was already playing some songs
          _advance_idx_lyrics(total_elapsed_time, idx_lyrics,
              m_lyrics.get_song_collection());
          m_ui.write_text("");
        } else {
          m_ui.write_text("No Lyrics Exists for this tune!");
        }

        m_ui.write_title(m_song.title(), m_song.artist());
      }
      
      if (signal_window_changed) {
        m_ui.resize_screen(true);
        m_ui.write_title(m_song.title(), m_song.artist());
      }

      if (lyrics_exist) {
        int temp_elapsed_time = m_player.get_elapsed();
        if (mpd_elapsed_time != temp_elapsed_time) {
          // The base timestamp moved by RWD / FFD
          mpd_elapsed_time = temp_elapsed_time;
          total_elapsed_time = mpd_elapsed_time;

          _advance_idx_lyrics(total_elapsed_time, idx_lyrics,
              m_lyrics.get_song_collection());
        }

        const std::vector<moonk5::alsong::song_info>& collection = 
          m_lyrics.get_song_collection();

        if (idx_lyrics < collection[m_idx_song_collection].lyrics_collection.size()) {
          delay = m_player.get_delay(); 
          alsong::time_lyrics tl = collection[m_idx_song_collection].lyrics_collection[idx_lyrics];
          if (total_elapsed_time >= tl.time + delay) {
            m_ui.write_lyrics(m_idx_song_collection, idx_lyrics, collection);
            ++idx_lyrics;
          } else if (signal_window_changed && idx_lyrics > 0) {
            m_ui.write_lyrics(m_idx_song_collection, idx_lyrics - 1, collection);
          }
        }       
        total_elapsed_time += 200;
      }


      signal_window_changed = false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
  }

  m_player.stop_thread();
}

bool moonk5::malv::lyrics_synchronizer::_load_lyrics(const std::string& title,
    const std::string& artist) {
  if (!m_lyrics.read(title, artist))  {
    alsong::lyrics_fetcher lyrics_fetcher;
    std::string resp = "";
    lyrics_fetcher.fetch_lyric_list(title, artist, resp);
    m_lyrics.parse_lyric_list(resp);

    if (m_lyrics.get_song_list_collection().empty())
      return false;

    resp = "";
    lyrics_fetcher.fetch_lyric(m_lyrics.get_song_list_collection()[0].lyric_id, resp);
    m_lyrics.parse_lyric(resp, title, artist);
    if (!m_lyrics.write(title, artist))
      return false;
  }
  return true;
}

void moonk5::malv::lyrics_synchronizer::_advance_idx_lyrics(int total_elapsed_time,
    int& idx_lyrics, const std::vector<alsong::song_info>& collection) {
  if (collection.size() == 0) return;
  if (total_elapsed_time == 0 && idx_lyrics == 0) return;
  if (idx_lyrics == collection[m_idx_song_collection].lyrics_collection.size())
    idx_lyrics = 0;

  int idx_tmp = idx_lyrics;
  int len_lyrics_collection =
    collection[m_idx_song_collection].lyrics_collection.size();
  bool move_backward = false;
  alsong::time_lyrics tl =
    collection[m_idx_song_collection].lyrics_collection[idx_tmp];

  if (tl.time == total_elapsed_time)
    return;

  if (tl.time > total_elapsed_time)
    move_backward = true;

  while(1) {
    tl = collection[m_idx_song_collection].lyrics_collection[idx_tmp];

    if (move_backward) {
      if (tl.time <= total_elapsed_time)
        break;
      --idx_tmp;
    } else {
      if (tl.time > total_elapsed_time) {
        --idx_lyrics;
        break;
      }
      ++idx_tmp;
    }

    // boundary check
    if (idx_tmp < 0 || idx_tmp == len_lyrics_collection)
      break;

    idx_lyrics = idx_tmp;
  }
}
