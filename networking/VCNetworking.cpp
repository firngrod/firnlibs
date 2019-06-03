#include "VCNetworking.hpp"
#include <io.h>
#include <fcntl.h>
#include <vector>
#include <algorithm>
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
    int rc = GetFD(fdTok);

    FdInfo tmpInfo;
    tmpInfo.type = FdType::ReadPipe;
    tmpInfo.data = (__int64) new std::vector<unsigned char>();
    tmpInfo.event = std::shared_ptr<FdInfo::EventHolder>(new FdInfo::EventHolder(WSACreateEvent()));
    mref.insert({ rc, tmpInfo });
    pipefds[0] = rc;
    rc = GetFD(fdTok);
    tmpInfo.type = FdType::WritePipe;
    mref.insert({ rc, tmpInfo });
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
    {
      auto fdTok = fdVec.Get("Firnlibs::VCNetworking::poll");
      auto &mref = *fdTok.operator->();
      for (size_t i = 0; i < nfds; ++i)
      {
        handles[i] = *mref[fds[i].fd].event;
      }
    }

    DWORD rc = WaitForMultipleObjects((DWORD)handles.size(), &handles[0], false, -1);

    // Okay, we have returned.  Find out which if any of the handles did something.
    if (rc < WAIT_OBJECT_0 || rc > WAIT_OBJECT_0 + nfds)
    {
      rc = GetLastError();
      return -1;
    }

    for (DWORD i = rc - WAIT_OBJECT_0; i < nfds; i++)
    {
      DWORD rc2 = WaitForSingleObject(handles[i], 0);
      if (rc2 != WAIT_TIMEOUT)
      {
        // This event is signalled.
        fds[i].revents = fds[i].events;
      }
    }

    return 0;
  }


  int VCNetworking::close(int sockfd)
  {
    auto fdTok = fdVec.Get("FirnLibs::VCNetworking::close");
    auto &mref = *fdTok.operator->();

    std::map<int, FdInfo>::iterator fItr = mref.find(sockfd);
    int rc = -10;
    switch (fItr->second.type)
    {
    case FdType::WritePipe:
    {
      rc =SetEvent((HANDLE)*fItr->second.event);
      break;
    }
    case FdType::ReadPipe:
    {
      rc =SetEvent((HANDLE)*fItr->second.event);
      break;
    }
    }

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
      *readydata = (int)((std::vector<unsigned char>*)info.data)->size();
      return 0;
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
      std::vector<unsigned char> &datVec = *((std::vector<unsigned char>*)info.data);
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
      std::vector<unsigned char> &datVec = *((std::vector<unsigned char>*)info.data);
      ssize_t minSize = min(bufsize, datVec.size());
      memcpy_s(buf, bufsize, &datVec[0], minSize);
      datVec.erase(datVec.begin(), datVec.begin() + minSize);
      return (int)bufsize;
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
      int rc = WSAEventSelect(info.data, &info.event, FD_ACCEPT | FD_CLOSE);
    }
    return ::listen(info.data, backlog);
  }
}