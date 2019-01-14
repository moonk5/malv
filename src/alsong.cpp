// alsong.cpp
#include <iostream>
#include <alsong.h>

std::string moonk5::time_conversion::to_simple_string(unsigned int time_in_ms) {
  const char time_fmt[] = "%02d:%02d.%02d";
  std::vector<char> buff(sizeof(time_fmt));

  if (time_in_ms <= 0)
    return "00:00.00";

  unsigned int minutes = 0;
  unsigned int seconds = 0;
  unsigned int milliseconds = time_in_ms % 1000;
  time_in_ms -= milliseconds;
  minutes = time_in_ms / (60 * 1000);
  time_in_ms -= minutes * (60 * 1000); 
  seconds = time_in_ms / 1000; 

  std::snprintf(&buff[0], buff.size(), time_fmt,
      minutes, seconds, milliseconds);

  return &buff[0];
}

unsigned int moonk5::time_conversion::to_milliseconds(const std::string& time_in_str) {
  unsigned int milliseconds = 0;
  milliseconds += stoi(time_in_str.substr(0, 2)) * 60 * 1000;  // mins
  milliseconds += stoi(time_in_str.substr(3, 2)) * 1000;       // secs
  milliseconds += stoi(time_in_str.substr(6, 2));              // mils

  return milliseconds;
}

std::string moonk5::alsong::time_lyrics::to_json_string() {
  std::string str_json = "{\"time\":\""
    + time_conversion::to_simple_string(time)
    + "\",\"lyrics\":[";
  for (std::string l : lyrics)
    str_json += "\"" + l + "\",";
  str_json.replace(str_json.size()-1, 1, 1, ']');
  str_json += "}";
  return str_json;
}


void moonk5::alsong::song_info::add_lyrics(const std::string& time, const std::string& lyrics) {

  // convert string time (mm:ss.SS) to milliseconds
  unsigned int ms = time_conversion::to_milliseconds(time);
  // compare time with previous timestamp
  // NOTE : if curr time is equal to prev time then assume it as
  // the user wants to support multi-languages or multi-lines
  if (lyrics_collection.size() > 0 && ms == lyrics_collection.back().time) {
    // case #1 : multi-languages or multi-lines
    lyrics_collection.back().lyrics.push_back(lyrics);
  } else {
    // case #2 : either first or only one line of lyrics for specific time
    alsong::time_lyrics obj;
    obj.time = ms;
    obj.lyrics.push_back(lyrics);
    lyrics_collection.push_back(obj);
  }
}

std::string moonk5::alsong::song_info::to_json_string() {
  std::string str_json = "{";
  str_json += "\"title\":\"" + title + "\",";
  str_json += "\"artist\":\"" + artist + "\",";
  str_json += "\"album\":\"" + album + "\",";
  str_json += "\"written_by\":\"" + written_by + "\",";
  str_json += "\"lyrics\":[";
  for (alsong::time_lyrics tl : lyrics_collection)
    str_json += tl.to_json_string() + ",";
  str_json.replace(str_json.size()-1, 1, 1, ']');
  str_json += "}";
  return str_json;
}


CURLcode moonk5::alsong::lyrics_fetcher::fetch(const std::string& title,
    const std::string& artist, std::string &output, unsigned timeout=10) {
  CURLcode result;
  CURL *curl = nullptr;

  if (title.empty() || artist.empty())
    result;

  curl = curl_easy_init();

  std::string soap(SOAP_TEMPLATE);
  soap = std::regex_replace(soap, std::regex("\\$title"), title);
  soap = std::regex_replace(soap, std::regex("\\$artist"), artist);

  struct curl_slist *headers = NULL;
  headers = curl_slist_append(headers,
      "Content-Type: application/soap+xml; charset=utf-8");
  headers = curl_slist_append(headers, "Accept: text/plain");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, soap.length());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, soap.c_str());
  curl_easy_setopt(curl, CURLOPT_URL, URL.c_str());
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &output);
  curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, timeout);
  result = curl_easy_perform(curl);

  curl_easy_cleanup(curl);

  return result;
}

size_t moonk5::alsong::lyrics_fetcher::write_data(char *buffer,
    size_t size, size_t nmemb, void *data) {
  size_t result = size * nmemb;
  static_cast<std::string *>(data)->append(buffer, result);
  return result;
} 

moonk5::alsong::lyrics_serializer::lyrics_serializer(const std::string& lyrics_path,
    unsigned int lyrics_count) {
  set_lyrics_folder_path(lyrics_path);
  max_lyrics_count = lyrics_count;
}

moonk5::alsong::lyrics_serializer::lyrics_serializer(unsigned int lyrics_count)
  : lyrics_serializer(std::string(getenv("HOME")) + "/.alsong",
      lyrics_count) {

  }

bool moonk5::alsong::lyrics_serializer::parse(const std::string& alsong_raw,
    const std::string& title, const std::string& artist) {
  tinyxml2::XMLDocument doc;
  doc.Parse(alsong_raw.c_str(), alsong_raw.size());

  tinyxml2::XMLElement* result = 
    doc.FirstChildElement("soap:Envelope")
    ->FirstChildElement("soap:Body")
    ->FirstChildElement("GetResembleLyric2Response")
    ->FirstChildElement("GetResembleLyric2Result");

  if (result == nullptr) {
    std::cerr << "Invalid format, couldn't recognized target tag names\n";
    return false;
  }

  int count = 0;
  for (tinyxml2::XMLNode* child =
      result->FirstChildElement("ST_GET_RESEMBLELYRIC2_RETURN");
      child;
      child = child->NextSiblingElement("ST_GET_RESEMBLELYRIC2_RETURN")) {
    alsong::song_info song;
    //song.title = find_child(&child, "strTitle");
    //song.artist = find_child(&child, "strArtistName");
    song.title = title;
    song.artist = artist;

    song.album = find_child(&child, "strAlbumName");
    song.written_by = find_child(&child, "strRegisterName");
    std::string lyrics_raw = find_child(&child, "strLyric");
    parse_lyrics(lyrics_raw, song);

    song_collection.push_back(song);

    ++count;
    if (count >= max_lyrics_count)
      break;
  }

  return true;
}

const std::vector<moonk5::alsong::song_info>&
moonk5::alsong::lyrics_serializer::get_song_collection() {
  return song_collection;
}

bool moonk5::alsong::lyrics_serializer::write(bool overwrite) {
  if (song_collection.size() <= 0)
    return false;

  // file nameing convention
  // 'artist - title.lyrics' 
  std::string filename = create_filename(song_collection[0].artist,
      song_collection[0].title);

  std::filesystem::path lyrics_path = lyrics_folder_path / filename;
  if (std::filesystem::exists(lyrics_path) && overwrite == false) {
    std::cerr << "moonk5::alsong::lyrics_serializer::write() - "
      << "file already exists\n";
    return false;
  } else {
    std::ofstream ofs(lyrics_path);
    ofs << to_json_string() << std::endl;
    ofs.close();
  }
  return true;
}

bool moonk5::alsong::lyrics_serializer::read(const std::string& title,
    const std::string& artist) {
  std::string filename = create_filename(artist, title);
  std::filesystem::path lyrics_path = lyrics_folder_path / filename;

  song_collection.clear();

  if (std::filesystem::exists(lyrics_path)) {
    // file exists ... de-serialize
    std::ifstream ifs(lyrics_path);
    nlohmann::json j;
    ifs >> j;

    auto& collection = j.at("song_collection");
    for (auto&& s : collection) {
      alsong::song_info song;
      song.title = s.at("title");
      song.artist = s.at("artist");
      song.album = s.at("album");
      song.written_by = s.at("written_by");
      auto& tls = s.at("lyrics");
      for (auto&& tl : tls) {
        auto& ls = tl.at("lyrics");
        for (auto&& l : ls) {
          song.add_lyrics(tl.at("time"), l);
        }
      }
      song_collection.push_back(song);
    }
    ifs.close(); 

    return true;
  }

  return false;
}

void moonk5::alsong::lyrics_serializer::set_lyrics_folder_path(
    const std::string& path) {
  lyrics_folder_path = path;
  if (!std::filesystem::exists(lyrics_folder_path)) {
    std::filesystem::create_directory(lyrics_folder_path);
  }
}

std::string moonk5::alsong::lyrics_serializer::to_json_string() {
  std::string str_json = "{\"song_collection\":[";
  for (int i = 0; i < song_collection.size(); ++i) {
    song_info &song = song_collection[i];
    str_json += song.to_json_string();
    if (i < song_collection.size() - 1)
      str_json += ",";
  }
  str_json += "]}";
  return str_json;
}


std::string moonk5::alsong::lyrics_serializer::create_filename(
    std::string artist, std::string title) {
  boost::to_upper(artist);
  boost::to_upper(title);
  return artist + " - " + title + ".lyrics";
}

std::string moonk5::alsong::lyrics_serializer::find_child(
    tinyxml2::XMLNode** root,
    const std::string& value) {
  std::string text = "";
  tinyxml2::XMLElement* element =
    (*root)->FirstChildElement(value.c_str());
  if (element != nullptr && element->GetText() != nullptr)
    text = element->GetText();
  return text;
}

void moonk5::alsong::lyrics_serializer::parse_lyrics(std::string& input,
    alsong::song_info& output) {
  const std::string REGEX_TIME =
    "\\[([0-5][0-9]:[0-5][0-9].[0-9][0-9])\\]";
  const std::string REGEX_TIME_LYRICS =
    "(" + REGEX_TIME + ")(.*)(\n)";
  std::regex reg_ex(REGEX_TIME_LYRICS);
  std::smatch match;

  // replace unnecessary XML tags;
  boost::replace_all(input, "<br>", "\n");
  boost::replace_all(input, "\"", "\\\"");
  boost::replace_all(input, "[00:00.00]\n", "");

  // search for time and lyrics
  while (std::regex_search(input, match, reg_ex)) {
    output.add_lyrics(match[2], match[3]);
    input = match.suffix().str();
  }
}
