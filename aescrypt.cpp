#include "aescrypt.hpp" 
#include <cryptopp/osrng.h>
#include <cryptopp/modes.h>
#include <cryptopp/aes.h>
#include <algorithm>

namespace FirnLibs { namespace Crypto {

CryptoPP::AutoSeededRandomPool Jesus;
AesCrypt::AesCrypt(const std::vector<unsigned char> &cryptParams)
{
  Init(cryptParams);
}

void AesCrypt::Init(const std::vector<unsigned char> &cryptParams)
{
  if(cryptParams.size() == 48)
  {
    this->cryptParams = cryptParams;
  }
  else
  {
    this->cryptParams.resize(48);
    Jesus.GenerateBlock(&this->cryptParams[0], 48);
  }
}

AesCrypt::ErrorCode AesCrypt::EncryptData(std::vector<unsigned char> &outData, const std::vector<unsigned char> &inData) const
{
  // Sanity check
  if(inData.size() > MaxMsgSize)
    return ErrorCode::InputTooLarge;

  // Calculate the final size
  uint32_t finalSize = inData.size() + blockSize + 4;
  // Now round that up to make sure that we have a full block, but only if we are not on a multiple of 16
  // This is because AES is a block cipher of 16 block size.  This library supports smaller sizes, presumably by padding with something.
  // Other libraries may not agree.
  if(finalSize % blockSize)
    finalSize = ((finalSize / blockSize) + 1) * blockSize;

  // Allocate space
  outData.resize(finalSize);
  // Generate a random block to obscure stuff
  Jesus.GenerateBlock(&outData[0], blockSize);
  // Since we encrypt to block size, we will need the size of the actual data at the other end.
  *((uint32_t *)&outData[blockSize]) = inData.size();
  // And now for the actual data.
  memcpy(&outData[20], &inData[0], inData.size());

  CryptoPP::CFB_Mode<CryptoPP::AES>::Encryption cfbEncryption(&cryptParams[0], 32, &cryptParams[32], 16);
  cfbEncryption.ProcessData(&outData[0], &outData[0], inData.size());
  return NoError;
}

AesCrypt::ErrorCode AesCrypt::DecryptData(std::vector<unsigned char> &outData, std::vector<unsigned char> inData) const
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
  outData.resize(finalSize);
  memcpy(&outData[0], &inData[20], finalSize);
  return NoError;
}


}}
    

