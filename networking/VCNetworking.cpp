#include "VCNetworking.hpp"
#include <io.h>
#include <fcntl.h>

namespace FirnLibs
{

  int VCNetworking::pipe(int *pipefds)
  {
    int rc = _pipe(pipefds, 0x1000, _O_BINARY);
    if (rc)
      return rc;

    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::pipe");
    auto &mref = *fdTok.operator->();
    rc = GetFD(fdTok);
    mref[rc].type = FdType::Pipe;
    mref[rc].data = pipefds[0];
    pipefds[0] = rc;
    rc = GetFD(fdTok);
    mref[rc].type = FdType::Pipe;
    mref[rc].data = pipefds[1];
    pipefds[0] = rc;

    return 0;
  }

  int VCNetworking::socket(int af, int type, int protocol)
  {
    SOCKET rc = ::socket(af, type, protocol);
    if (rc == INVALID_SOCKET)
      return -1;
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::pipe");
    auto &mref = *fdTok.operator->();
    int tmp = GetFD(fdTok);
    mref[tmp].type = FdType::Socket;
    mref[tmp].data = rc;
    return tmp;
  }

}