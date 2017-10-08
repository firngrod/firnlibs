#include "types.hpp"
#include <climits>

namespace FirnLibs
{
  namespace Crypto
  {
    class AES
    {
    public:
      // Max number in a uint32 minus 16 bytes for randomblock and 4 bytes for size.
      // This number is chosen to guarantee compliance with 32-bit systems and should be enough for anything anyway.
      const static unsigned int MaxMsgSize = UINT_MAX - 16 - 4;

      enum ErrorCode
      {
        NoError,
        InputTooLarge,
        InvalidInput,
      };

      // Init with a specific set of parameters if cryptParams.size() == 48, otherwise a random key is generated.
      AES(const ByteVector &cryptParamsIn = ByteVector(0));
      void Init(const ByteVector &cryptParamsIn = ByteVector(0));

      // Encrypt data with the current encryption parameters.
      ErrorCode EncryptData(ByteVector &outData, const ByteVector &inData) const;

      // Decrypt data with the current encryption parameters.
      ErrorCode DecryptData(ByteVector &outData, ByteVector inData) const;
    protected:
      
      ByteVector cryptParams;
      const static int blockSize = 16;
    };
  }
}
