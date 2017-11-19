#include "mp3stream.hpp"
#include <iostream>
#include "../files/files.hpp"
#include <vector>

namespace FirnLibs{ namespace Mp3{


Mp3Stream::Mp3Stream() : sf::SoundStream()
{
  pendingRate = 0;
  err = mpg123_init();
  if(err != MPG123_OK || (mh = mpg123_new(NULL, &err)) == NULL)
  {
    fprintf(stderr, "Basic setup went wrong: %s", mpg123_plain_strerror(err));
    mpg123_ok = false;
    return;
  }
  mpg123_ok = true;
  initialize();
}


void Mp3Stream::initialize()
{
  SoundStream::initialize(2, 44100);
}


Mp3Stream::~Mp3Stream()
{
  Cleanup();
  mpg123_exit();
}


void Mp3Stream::PlayTrack(const std::string &track)
{
  std::lock_guard<std::recursive_mutex> theLock(theMutex);
  if(mpg123_open(mh, track.c_str()) != MPG123_OK)
    return;

  mpg123_getformat(mh, &pendingRate, &channels, &encoding);
  curTrack = track;
  play();
}


void Mp3Stream::SetNextGetter(const std::function<std::string (const std::string &lastTrack)> callback)
{
  nextGetter = callback;
}


void Mp3Stream::Cleanup()
{
  if(mh != nullptr)
  {
    mpg123_close(mh);
    mpg123_delete(mh);
    curTrack = "";
  }
}


bool Mp3Stream::onGetData(SoundStream::Chunk &data)
{
  std::lock_guard<std::recursive_mutex> theLock(theMutex);

  if(pendingRate != 0)
  {
    dataChunk.resize(pendingRate);
    rate = pendingRate;
    pendingRate = 0;
    SoundStream::initialize(channels, rate);
  }

  data.samples = (sf::Int16 *)&dataChunk[0];
  
  size_t bytesRead = 0;
  err = mpg123_read(mh, &dataChunk[0], dataChunk.size(), &bytesRead);
  data.sampleCount = bytesRead/2;

  // Let's see if we finished the file.
  if(bytesRead == 0 || bytesRead != dataChunk.size())
  {
    bool openedNew = false;
    do
    {
      if(nextGetter != nullptr)
        curTrack = nextGetter(curTrack);
      else
        curTrack = "";

      if(curTrack != "")
      {
        if(mpg123_open(mh, curTrack.c_str()) != MPG123_OK)
          continue;

        mpg123_getformat(mh, &pendingRate, &channels, &encoding);
        openedNew = true;
      }
    } while(curTrack != "" && openedNew == false);
  }

  if(curTrack != "" && bytesRead == 0)
  {
    return onGetData(data);
  }
  
  return data.sampleCount > 0;
}


void Mp3Stream::onSeek(sf::Time offset)
{
}


}}
