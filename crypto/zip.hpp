#pragma once
#include "types.hpp"
#include <string>

namespace FirnLibs
{
  namespace Crypto
  {
    // Zip and encode in base 64
    void ZipEncode(std::string &zipEncoded, const unsigned char *datPtr, const size_t &datSize);
    void UnzipDecode(unsigned char *bufPtr, const size_t &bufSize, const std::string &zipEncoded);

    // Zip to and from vectors
    bool Zip(ByteVector &zipped, const ByteVector &unzipped);
    bool Zip(ByteVector &zipped, const unsigned char *unzipped, const size_t &unzippedSize);
    bool Unzip(ByteVector &unzipped, const ByteVector &zipped);
    bool Unzip(ByteVector &unzipped, const unsigned char *zipped, const size_t &zippedSize);
  }
}
