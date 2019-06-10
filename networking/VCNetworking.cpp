#include "VCNetworking.hpp"
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>
#include "ws2tcpip.h"
#pragma comment(lib, "Ws2_32.lib")

namespace FirnLibs
{
  VCNetworking::VCNetworking()
  {
    WSAStartup(MAKEWORD(2, 2), &wsaData);
    lastFD = 0;
  }

  int VCNetworking::pipe(int *pipefds)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::pipe");
    auto &mref = *fdTok.operator->();
    FdInfo tmpInfo;
    tmpInfo.type = FdType::ReadPipe;
    tmpInfo.event = std::shared_ptr<FdInfo::EventHolder>(new FdInfo::EventHolder(WSACreateEvent()));

    int rc = GetFD(fdTok);
    mref.insert({ rc, tmpInfo });
    __int64 tmp = mref[rc].data = (__int64) new std::shared_ptr<std::vector<unsigned char> >(new std::vector<unsigned char>());
    std::shared_ptr<std::recursive_mutex> mut = mref[rc].mut = std::shared_ptr<std::recursive_mutex>(new std::recursive_mutex);
    pipefds[0] = rc;
    rc = GetFD(fdTok);
    tmpInfo.type = FdType::WritePipe;
    mref.insert({ rc, tmpInfo });
    mref[rc].data = (__int64) new std::shared_ptr<std::vector<unsigned char> >(*((std::shared_ptr<std::vector<unsigned char> >*)tmp));
    mref[rc].mut = mut;
    pipefds[1] = rc;

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

  int VCNetworking::poll(pollfd *fds, size_t nfds, int timeout)
  {
    std::vector<HANDLE> handles(nfds);
    bool sendIsReady = false;
    {
      auto fdTok = fdVec.Get("Firnlibs::VCNetworking::poll");
      auto &mref = *fdTok.operator->();
      for (size_t i = 0; i < nfds; ++i)
      {
        handles[i] = *mref[fds[i].fd].event;
        if (fds[i].events & POLLOUT)
        {
          if (mref[fds[i].fd].sendReady)
          {
            fds[i].revents |= POLLOUT;
            sendIsReady = true;
          }
        }
      }
    }


    DWORD rc = WaitForMultipleObjects((DWORD)handles.size(), &handles[0], false, sendIsReady ? 0 : timeout);

    // Okay, we have returned.  Find out which if any of the handles did something.
    if (rc < WAIT_OBJECT_0 || rc > WAIT_OBJECT_0 + nfds)
    {
      if (!sendIsReady)
      {
        rc = GetLastError();
        return -1;
      }
      else
      {
        return 0;
      }
    }

    auto fdTok = fdVec.Get("Firnlibs::VCNetworking::poll");
    auto &mref = *fdTok.operator->();

    DWORD i;
    for (i = rc - WAIT_OBJECT_0; i < nfds; i++)
    {
      DWORD rc2 = WaitForSingleObject(handles[i], 0);
      if (rc2 != WAIT_TIMEOUT)
      {
        auto &info = mref[fds[i].fd];
        if (info.type == FdType::Socket)
        {
          WSANETWORKEVENTS events;
          int enumrc = WSAEnumNetworkEvents(info.data, *info.event, &events);
          // This event is signalled.
          if (events.lNetworkEvents & (FD_ACCEPT | FD_READ | FD_CLOSE))
          {
            fds[i].revents |= fds[i].events & (POLLIN | POLLERR);
          }
          if (events.lNetworkEvents & FD_WRITE)
          {
            info.sendReady = true;
            fds[i].revents |= fds[i].events & (POLLOUT);
          }
        }
        else if (info.type == FdType::ReadPipe)
        {
          fds[i].revents |= fds[i].events & (POLLIN | POLLERR);
        }
      }
    }

    return 0;
  }


  int VCNetworking::close(int sockfd)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();

    std::map<int, FdInfo>::iterator fItr = mref.find(sockfd);
    switch (fItr->second.type)
    {
    case FdType::WritePipe:
    {
      delete (std::shared_ptr<std::vector<unsigned char> > *) fItr->second.data;
      try
      {
        WSACloseEvent((HANDLE)*fItr->second.event);
      }
      catch (...)
      {

      }
      *fItr->second.event = INVALID_HANDLE_VALUE;
      break;
    }
    case FdType::ReadPipe:
    {
      delete (std::shared_ptr<std::vector<unsigned char> > *) fItr->second.data;
      try
      {
        WSACloseEvent((HANDLE)*fItr->second.event);
      }
      catch (...)
      {

      }
      *fItr->second.event = INVALID_HANDLE_VALUE;
      break;
    }
    case FdType::Socket:
    {
      closesocket(fItr->second.data);
    }
    }
    mref.erase(fItr);

    return 0;
  }

  int VCNetworking::ioctl(int fd, unsigned long request, int *readydata)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[fd];

    switch (info.type)
    {
    case FdType::ReadPipe:
    {
      *readydata = (int)(*(std::shared_ptr<std::vector<unsigned char> > *)info.data)->size();
      return 0;
    }
    case FdType::Socket:
    {
      u_long ready = 0;
      int rcioctl = ioctlsocket(info.data, FIONREAD, &ready);
      *readydata = ready;
      return rcioctl;
    }
    }

    return 0;
  }

  int VCNetworking::write(int fd, char *buf, size_t bufsize)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[fd];

    switch (info.type)
    {
    case FdType::WritePipe:
    {
      std::lock_guard<std::recursive_mutex> lock(*info.mut);
      std::vector<unsigned char> &datVec = *(*(std::shared_ptr<std::vector<unsigned char> > *)info.data);
      datVec.insert(datVec.end(), (unsigned char*)buf, (unsigned char*)buf + bufsize);
      WSASetEvent(*info.event);
      return (int)bufsize;
    }
    }

    return 0;
  }


  int VCNetworking::read(int fd, char * buf, ssize_t bufsize)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[fd];
    switch (info.type)
    {
    case FdType::ReadPipe:
    {
      std::lock_guard<std::recursive_mutex> lock(*info.mut);
      std::vector<unsigned char> &datVec = *(*(std::shared_ptr<std::vector<unsigned char> > *)info.data);
      ssize_t minSize = min(bufsize, datVec.size());
      memcpy_s(buf, bufsize, &datVec[0], minSize);
      datVec.erase(datVec.begin(), datVec.begin() + minSize);
      if(datVec.size() == 0)
        WSAResetEvent(*info.event);
      return (int)bufsize;
    }
    case FdType::Socket:
    {
      int rc = recv(info.data, buf, bufsize, 0);
      return rc;
    }
    }

    return 0;
  }


  int VCNetworking::setsockopt(_In_ SOCKET s, _In_ int level, _In_ int optname, _In_reads_bytes_opt_(optlen) const char FAR * optval, _In_ int optlen)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[s];
    s = info.data;
    return ::setsockopt(s, level, optname, optval, optlen);
  }


  int VCNetworking::bind(_In_ SOCKET s, _In_reads_bytes_(namelen) const struct sockaddr FAR * name, _In_ int namelen)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[s];
    s = info.data;
    return ::bind(s, name, namelen);
  }


  int VCNetworking::listen(int sockfd, int backlog)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[sockfd];
    if (info.event == nullptr)
    {
      info.event = std::shared_ptr<FdInfo::EventHolder>(new FdInfo::EventHolder(WSACreateEvent()));
      WSAEventSelect(info.data, *info.event, FD_ACCEPT | FD_CLOSE);
    }
    return ::listen(info.data, backlog);
  }


  int VCNetworking::accept(int fd, sockaddr * addr, socklen_t* addrlen)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[fd];
    SOCKET rc = ::accept(info.data, addr, addrlen);
    if (rc == INVALID_SOCKET)
      return rc;
    int tmp = GetFD(fdTok);
    FdInfo tmpInfo;
    tmpInfo.type = FdType::Socket;
    tmpInfo.event = std::shared_ptr<FdInfo::EventHolder>(new FdInfo::EventHolder(WSACreateEvent()));
    tmpInfo.data = rc;
    WSAEventSelect(rc, *tmpInfo.event, FD_READ | FD_CLOSE | FD_WRITE);
    tmpInfo.sendReady = true;
    mref.insert({ tmp, tmpInfo });
    return tmp;
  }


  int VCNetworking::send(int fd, char * buf, size_t size, int flags)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[fd];
    if (info.sendReady == false)
      return -1;
    int rc = ::send(info.data, buf, size, flags);
    if (rc == -1)
    {
      rc = WSAGetLastError();
      if (rc == WSAEWOULDBLOCK)
      {
        info.sendReady = false;
        errno = EWOULDBLOCK;
        rc = -1;
      }
      else
      {
        return -1;
      }
    }
    if (rc == 0)
    {
      bool nosent = false;
    }
    return rc;
  }


  int VCNetworking::connect(_In_ SOCKET s, _In_reads_bytes_(namelen) const struct sockaddr FAR * name, _In_ int namelen)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();
    FdInfo &info = mref[s];
    int rc = ::connect(info.data, name, namelen);
    info.event = std::shared_ptr<FdInfo::EventHolder>(new FdInfo::EventHolder(WSACreateEvent()));
    WSAEventSelect(info.data, *info.event, FD_READ | FD_CLOSE | FD_WRITE);
    info.sendReady = true;
    return rc;
  }


  VCNetworking::hostent * VCNetworking::gethostbyname(const char *name)
  {
    static hostent tmp;

    addrinfo *result, hints;
    bzero(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    int rc = GetAddrInfoA(name, NULL, &hints, &result);

    if (rc != 0)
      return nullptr;

    IN_ADDR *testie = &((sockaddr_in *)result->ai_addr)->sin_addr;

    bcopy(testie, tmp.h_addr, 4);
    tmp.h_length = 4;

    return &tmp;
  }
}