#include "metadata.hpp"
#include <algorithm>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include "../files/files.hpp"
#include "../string/string.hpp"


namespace FirnLibs{ namespace Mp3{


bool GetMetadata(Json::Value &metaData, const std::string &fileName)
{
  TagLib::MPEG::File file(fileName.c_str());
  // If it has an ID3v2Tag, we use that.
  if(file.hasID3v2Tag())
  {
    TagLib::ID3v2::Tag *tag = file.ID3v2Tag(true);
    for(TagLib::List<TagLib::ID3v2::Frame *>::ConstIterator theitr = tag->frameList().begin(); theitr != tag->frameList().end(); theitr++)
    {
      std::string framevalue = (*theitr)->toString().to8Bit();
      TagLib::ByteVector tmpie = (*theitr)->frameID();
      std::string trigger = std::string(tmpie.data(), tmpie.data() + 4);
      metaData[trigger] = framevalue;
    }
  }
  else
    return false;
  // Now save the file path, the bitrate and the track length.
  metaData["FILE"] = fileName;
  TagLib::MPEG::Properties *properties = file.audioProperties();
  metaData["LENG"] = properties->length();
  metaData["BITR"] = properties->bitrate();
  metaData["MTIM"] = FirnLibs::String::StringPrintf("%ld", FirnLibs::Files::FileModifiedTime(fileName));
  return true;
}


}}
