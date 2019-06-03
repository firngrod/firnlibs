#include <winsock2.h>
#include <Windows.h>
#include <map>
#include <vector>
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

    VCNetworking();

    int pipe(int *pipefds);

    int socket(int af, int type, int protocol);

    int poll(pollfd *fds, size_t nfds, int timeout);

    int close(int sockfd);

    int ioctl(int fd, unsigned long request, int *readydata);

    int write(int fd, char *buf, size_t bufsize);

    int setsockopt(_In_ SOCKET s, _In_ int level, _In_ int optname, _In_reads_bytes_opt_(optlen) const char FAR * optval, _In_ int optlen);

    int bind(_In_ SOCKET s, _In_reads_bytes_(namelen) const struct sockaddr FAR * name, _In_ int namelen);

    int read(int fd, char * buf, ssize_t bufsize);

    int listen(int sockfd, int backlog);

    int accept(int fd, sockaddr * addr, socklen_t* addrlen) { return 0; }

    int send(int fd, char * buf, size_t size, int flags) { return 0; }

    int connect(_In_ SOCKET s, _In_reads_bytes_(namelen) const struct sockaddr FAR * name, _In_ int namelen) { return 0; }

    hostent * gethostbyname(const char *name) { return nullptr; }

    inline void bzero(void *buf, const size_t &n)
    {
      memset(buf, 0, n);
    }

    inline void bcopy(void *target, void *src, const size_t &n)
    {
      memcpy_s(target, n, src, n);
    }

  private:

    HANDLE pipeEvent;
    WSADATA wsaData;


    enum FdType
    {
      Undefined,
      ReadPipe,
      WritePipe,
      Socket,
    };
    struct FdInfo
    {
      FdType type;
      __int64 data;
      class EventHolder
      {
      public:
        EventHolder()
        {
          event = INVALID_HANDLE_VALUE;
        }
        EventHolder(HANDLE event)
        {
          this->event = event;
        }
        ~EventHolder()
        {
          if (event != INVALID_HANDLE_VALUE)
            CloseHandle(event);
        }
        operator HANDLE () const
        {
          return event;
        }
        HANDLE &operator=(HANDLE event)
        {
          return this->event = event;
        }
      private:
        HANDLE event;
      };
      std::shared_ptr<EventHolder> event;
      FdInfo(const FdType &type, const __int64 &data)
      {
        this->type = type;
        this->data = data;
        this->event = nullptr;
      }
      FdInfo()
      {
        type = FdType::Undefined;
        data = -1;
        this->event = nullptr;
      }
      ~FdInfo()
      {
        switch (type)
        {
        case FdType::ReadPipe:
        case FdType::WritePipe:
        {
          delete (std::vector<unsigned char> *) data;
          break;
        }
        }
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

