#include <winsock2.h>
#include <Windows.h>
#include <map>
#include "../threading/guardedvar.hpp"
typedef INT_PTR ssize_t;
typedef int socklen_t;

namespace FirnLibs
{
  class VCNetworking
  {
  protected:
    struct pollfd {

      int  fd;
      SHORT   events;
      SHORT   revents;

    };

    int pipe(int *pipefds);

    int poll(pollfd *fds, size_t nfds, int timeout);

    int listen(int sockfd, int backlog);

    int close(int sockfd);

    int ioctl(int fd, unsigned long request, int *readydata);

    int read(int fd, char * buf, ssize_t bufsize);

    int write(int fd, char *buf, size_t bufsize);

    int accept(int fd, sockaddr * addr, socklen_t* addrlen);

    int send(int fd, char * buf, size_t size, int flags);

    int socket(int af, int type, int protocol);

    hostent * gethostbyname(const char *name);

    inline void bzero(void *buf, const size_t &n)
    {
      memset(buf, 0, n);
    }

    inline void bcopy(void *target, void *src, const size_t &n)
    {
      memcpy_s(target, n, src, n);
    }

    VCNetworking()
    {
      lastFD = 0;
    }
  private:
    enum FdType
    {
      Undefined,
      Pipe,
      Socket,
    };
    struct FdInfo
    {
      FdType type;
      __int64 data;
      FdInfo(const FdType &type, const __int64 &data)
      {
        this->type = type;
        this->data = data;
      }
      FdInfo()
      {
        type = FdType::Undefined;
        data = -1;
      }
    };
    FirnLibs::Threading::GuardedVar<std::map<int, FdInfo> > fdVec;

    int lastFD;
    inline int GetFD(const FirnLibs::Threading::GuardedVar<std::map<int, FdInfo> >::Token &fdTok)
    {
      std::map<int, int> test;
      while ((fdTok->find(lastFD) != fdTok->end()) || (lastFD == -1))
        ++lastFD;
      return lastFD++;
    }
  };
}

