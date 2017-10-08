#include "aes.hpp" 
#include "zip.hpp"
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <algorithm>

namespace FirnLibs { namespace Crypto {

AES::AES(const ByteVector &cryptParamsIn)
{
  Init(cryptParamsIn);
}

void AES::Init(const ByteVector &cryptParamsIn)
{
  if(cryptParamsIn.size() == 48)
  {
    this->cryptParams = cryptParamsIn;
  }
  else
  {
    this->cryptParams.resize(48);
    CryptoPP::AutoSeededRandomPool Jesus;
    Jesus.GenerateBlock(&(this->cryptParams[0]), 48);
  }
}

AES::ErrorCode AES::EncryptData(ByteVector &outData, const ByteVector &inData) const
{
  if(inData.size() == 0)
    return InvalidInput;

  // Zip the data to increase enthropy
  ByteVector zippedData;
  FirnLibs::Crypto::Zip(zippedData, inData);

  // Sanity check
  if(zippedData.size() > MaxMsgSize)
    return ErrorCode::InputTooLarge;

  // Calculate the final size
  uint32_t finalSize = zippedData.size() + blockSize + 4;
  // Now round that up to make sure that we have a full block, but only if we are not on a multiple of 16
  // This is because AES is a block cipher of 16 block size.  This library supports smaller sizes, presumably by padding with something.
  // Other libraries may not agree.
  if(finalSize % blockSize)
    finalSize = ((finalSize / blockSize) + 1) * blockSize;

  // Allocate space
  outData.resize(finalSize);
  // Generate a random block to obscure stuff
  CryptoPP::AutoSeededRandomPool Jesus;
  Jesus.GenerateBlock(&outData[0], blockSize);
  // Since we encrypt to block size, we will need the size of the actual data at the other end.
  *((uint32_t *)&outData[blockSize]) = zippedData.size();
  // And now for the actual data.
  memcpy(&outData[blockSize + 4], &zippedData[0], zippedData.size());

  CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(&cryptParams[0], 32, &cryptParams[32], 16);
  cfbEncryption.ProcessData(&outData[0], &outData[0], outData.size());
  return NoError;
}

AES::ErrorCode AES::DecryptData(ByteVector &outData, ByteVector inData) const
{
  // Sanity check.  If data is less than the bare minimum or not a multiple of 16.
  if((inData.size() < 20)
     || (inData.size() % blockSize))
  {
    return InvalidInput;
  }

  CryptoPP::CFB_Mode<CryptoPP::AES>::Decryption cfbDecryption(&cryptParams[0], 32, &cryptParams[32], 16);
  cfbDecryption.ProcessData(&inData[0], &inData[0], inData.size());

  uint32_t finalSize = *((uint32_t *)&inData[16]);

  // Sanity check
  if((finalSize < inData.size() - 20 - 15)
     || (finalSize > inData.size() - 20))
  {
    return InvalidInput;
  }

  if(!FirnLibs::Crypto::Unzip(outData, &inData[blockSize + 4], finalSize))
  {
    return InvalidInput;
  }

  return NoError;
}


}}
    

