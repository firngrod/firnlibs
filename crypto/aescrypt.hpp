#include <vector>
#include <climits>

namespace FirnLibs
{
  namespace Crypto
  {
    class AesCrypt
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
      AesCrypt(const std::vector<unsigned char> &cryptParams = std::vector<unsigned char>());
      void Init(const std::vector<unsigned char> &cryptParams = std::vector<unsigned char>());

      // Encrypt data with the current encryption parameters.
      ErrorCode EncryptData(std::vector<unsigned char> &outData, const std::vector<unsigned char> &inData) const;

      // Decrypt data with the current encryption parameters.
      ErrorCode DecryptData(std::vector<unsigned char> &outData, std::vector<unsigned char> inData) const;
    protected:
      
      std::vector<unsigned char> cryptParams;
      const static int blockSize = 16;
    };
  }
}
