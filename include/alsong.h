/* *
 *
 * Alsong Lyrics Fetcher
 * version 1.0.0
 * https://github.com/moonk5/alsong-lyrics-fetcher.git
 *
 * Copyright (C) 2018-2021 moonk5
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef ALSONG_LYRICS_FETCHER_H
#define ALSONG_LYRICS_FETCHER_H

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <locale>
#include <regex>
#include <vector>

#include <boost/algorithm/string/case_conv.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <tinyxml2.h>

#define ALSONG_LYRICS_FETCHER_MAJOR 1
#define ALSONG_LYRICS_FETCHER_MINOR 0
#define ALSONG_LYRICS_FETCHER_PATCH 0

namespace moonk5
{
  namespace time_conversion
  {
    std::string to_simple_string(unsigned int time_in_ms);
    unsigned int to_milliseconds(const std::string& time_in_str);
  }

  namespace alsong
  {
    const std::string ALSONG_LYRICS_FETCHER_VERSION =
      std::to_string(ALSONG_LYRICS_FETCHER_MAJOR) + "." +
      std::to_string(ALSONG_LYRICS_FETCHER_MINOR) + "." +
      std::to_string(ALSONG_LYRICS_FETCHER_PATCH);

    struct time_lyrics
    {
      unsigned int time = 0; // unit in milliseconds
      std::vector<std::string> lyrics;

      std::string to_json_string();
    };

    struct song_list
    {
      std::string lyric_id = "";
      std::string title = "";
      std::string artist = "";
      std::string album = "";

      std::string to_json_string();
    };

    struct song_info
    {
      std::string lyric_id = "";
      std::string title = "";
      std::string artist = "";
      std::string album = "";
      std::string written_by = "";
      unsigned int language_count = 0;
      int delay = 0;

      std::vector<time_lyrics> lyrics_collection;
      void add_lyrics(const std::string& time, const std::string& lyrics);
      std::string to_json_string();
    };

    struct lyrics_fetcher
    {
      const std::string URL =
        "http://lyrics.alsong.co.kr/alsongwebservice/service1.asmx";

      const std::string ENC_DATA =
        "7c2d15b8f51ac2f3b2a37d7a445c3158455defb8a58d621eb77a3ff8ae4921318e49cefe24e515f79892a4c29c9a3e204358698c1cfe79c151c04f9561e945096ccd1d1c0a8d8f265a2f3fa7995939b21d8f663b246bbc433c7589da7e68047524b80e16f9671b6ea0faaf9d6cde1b7dbcf1b89aa8a1d67a8bbc566664342e12";

      const std::string SOAP_TEMPLATE_LYRIC_LIST =
        R"(<SOAP-ENV:Envelope
           xmlns:SOAP-ENV="http://www.w3.org/2003/05/soap-envelope"
           xmlns:SOAP-ENC="http://www.w3.org/2003/05/soap-encoding"
           xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
           xmlns:xsd="http://www.w3.org/2001/XMLSchema"
           xmlns:ns2="ALSongWebServer/Service1Soap"
           xmlns:ns1="ALSongWebServer"
           xmlns:ns3="ALSongWebServer/Service1Soap12">
           <SOAP-ENV:Body>
           <ns1:GetResembleLyricList2>
           <ns1:encData>$encdata</ns1:encData>
           <ns1:title>$title</ns1:title>
           <ns1:artist>$artist</ns1:artist>
           <ns1:pageNo>1</ns1:pageNo>
           </ns1:GetResembleLyricList2>
           </SOAP-ENV:Body>
           </SOAP-ENV:Envelope>
          )";

      const std::string SOAP_TEMPLATE_LYRIC_BY_ID =
        R"(<SOAP-ENV:Envelope
           xmlns:SOAP-ENV="http://www.w3.org/2003/05/soap-envelope"
           xmlns:SOAP-ENC="http://www.w3.org/2003/05/soap-encoding"
           xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
           xmlns:xsd="http://www.w3.org/2001/XMLSchema"
           xmlns:ns2="ALSongWebServer/Service1Soap"
           xmlns:ns1="ALSongWebServer"
           xmlns:ns3="ALSongWebServer/Service1Soap12">
           <SOAP-ENV:Body>
           <ns1:GetLyricByID2>
           <ns1:encData>$encdata</ns1:encData>
           <ns1:lyricID>$lyricId</ns1:lyricID>
           </ns1:GetLyricByID2>
           </SOAP-ENV:Body>
           </SOAP-ENV:Envelope>
          )";

      CURLcode fetch_lyric_list(const std::string& title, const std::string& artist,
            std::string &output);
      CURLcode fetch_lyric(const std::string& lyric_id, std::string &output);

      private:
      CURLcode _fetch(const std::string& soap, std::string &output,
          unsigned timeout=10);

      static size_t write_data(char *buffer, size_t size,
          size_t nmemb, void *data);
    }; // struct moonk5::alsong::lyrics_fetcher

    class lyrics_serializer
    {
      public:
        lyrics_serializer(const std::string& lyrics_path,
            unsigned int lyrics_count=3);

        lyrics_serializer(unsigned int lyrics_count=3);

        bool parse_lyric_list(const std::string& alsong_raw);
        bool parse_lyric(const std::string& alsong_raw,
            const std::string& title, const std::string& artist);

        const std::vector<song_list>& get_song_list_collection();
        const std::vector<song_info>& get_song_collection();

        // serialization
        // transformates a song_info object in memory to a file
        bool write(const std::string& title, const std::string& artist,
            bool overwrite=false);
        bool write(bool overwrite=false);

        //deserialize
        // transformates a lyrics file into a song_info object
        bool read(const std::string& title, const std::string& artist);

        void set_lyrics_folder_path(const std::string& path);

        std::string to_json_string();

      private:
        std::string create_filename(std::string artist, std::string title);

        std::string find_child(tinyxml2::XMLNode** root,
            const std::string& value);

        void parse_lyrics(std::string& input, song_info& output);

        std::filesystem::path lyrics_folder_path;
        std::vector<song_list> song_list_collection;
        std::vector<song_info> song_collection;
        unsigned int max_lyrics_count;
    }; // class moonk5::alsong::lyrics_serializer
  }
}
#endif // ALSONG_LYRICS_FETCHER_H
