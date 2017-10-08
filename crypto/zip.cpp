#include "zip.hpp"
#include <cryptopp/gzip.h>
#include <cryptopp/base64.h>
#include <cryptopp/filters.h>


namespace FirnLibs
{
  namespace Crypto
  {
    void ZipEncode(std::string &zipEncoded, const unsigned char *datPtr, const size_t &datSize)
    {
      CryptoPP::ArraySource s(datPtr, datSize, true, new CryptoPP::Gzip(new CryptoPP::Base64Encoder(new CryptoPP::StringSink(zipEncoded)), CryptoPP::Gzip::MAX_DEFLATE_LEVEL));
    }

    void UnzipDecode(unsigned char *bufPtr, const size_t &bufSize, const std::string &zipEncoded)
    {
      CryptoPP::StringSource s(zipEncoded, true, new CryptoPP::Base64Decoder(new CryptoPP::Gunzip(new CryptoPP::ArraySink(bufPtr, bufSize))));
    }

    bool Zip(ByteVector &zipped, const ByteVector &unzipped)
    {
      if(unzipped.size() == 0)
        return false;

      return Zip(zipped, &unzipped[0], unzipped.size());
    }

    bool Zip(ByteVector &zipped, const unsigned char *unzipped, const size_t &unzippedSize)
    {
      if(unzippedSize == 0)
        return false;
      
      CryptoPP::Gzip zipper;
      zipper.Put(unzipped, unzippedSize);
      zipper.MessageEnd();

      uint64_t available = zipper.MaxRetrievable();
      if(!available)
        return false;

      zipped.resize(available);
      zipper.Get(&zipped[0], zipped.size());
      return true;
    }

    bool Unzip(ByteVector &unzipped, const ByteVector &zipped)
    {
      return Unzip(unzipped, &zipped[0], zipped.size());
    }

    bool Unzip(ByteVector &unzipped, const unsigned char *zipped, const size_t &zippedSize)
    {
      if(zippedSize == 0)
        return false;

      CryptoPP::Gunzip unzipper;
      unzipper.Put(zipped, zippedSize);
      unzipper.MessageEnd();

      uint64_t available = unzipper.MaxRetrievable();
      if(!available)
        return false;

      unzipped.resize(available);
      unzipper.Get(&unzipped[0], unzipped.size());
      return true;
    }
  }
}


