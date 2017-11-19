#pragma once
#include <SFML/Audio/SoundStream.hpp>
#include <mpg123.h>
#include <functional>
#include <mutex>
#include <vector>

namespace FirnLibs
{
  namespace Mp3
  {
    class Mp3Stream : public sf::SoundStream
    {
    public:
      Mp3Stream();
      ~Mp3Stream();

      //bool SetCurrent(const std::string &track);
      void SetNextGetter(const std::function<std::string (const std::string &lastTrack)> callback);
      void PlayTrack(const std::string &track);
      std::string GetCurrent() { return curTrack; }
    protected:
      std::vector<unsigned char> dataChunk;
      bool onGetData(SoundStream::Chunk& data);
      void initialize();
      void onSeek(sf::Time offset);
      mpg123_handle *mh;
      void Cleanup();
      std::string curTrack;
      int err;
      bool mpg123_ok;
      std::recursive_mutex theMutex;
      std::function<std::string (const std::string &lastTrack)> nextGetter;
      long rate, pendingRate;
      int channels, encoding;
    };
  }
}
