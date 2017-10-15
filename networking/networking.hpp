#pragma once
#define _Networking_h_
#include <vector>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../threading/threadpool.hpp"
#include <map>

namespace FirnLibs
{
  class Networking
  {
  public:
    class Client
    {
    };


    // Listening stuff
    #include "listener.hpp"
  protected:
    int Listen(const int &port, void (*callback)(Listener::AcceptState *), void * callbackState);
    friend Listener;


    // Things to do with the internal workings, mainly the PollDancer.
  protected:
    void SignalCloseSocket(const int &fd);
    void Cleanup();
    std::thread selectThread;

    // Select dancer function
    void PollDancer();
    static void PollDancerStat();

    // Pipe handling.
    enum PipeMessage
    {
      SocketRemove, // Uses fd
      ListenerAdd, // Uses fd, *pAddr, callback, callbackState
      ConnectionAdd, // Uses fd and *pAddr.
    };

    struct PipeMessagePack
    {
      PipeMessage msg;
      int fd;
      void *pAddr;
      void *callback;
      void *state;
    };
    int pipe_fds[2];  // Signal pipe for the select dancer
    bool HandlePipe(const pollfd &pfd, std::vector<PipeMessagePack> &pipeData);

    // Listener handling.
    struct ListenerState
    {
      void (*callback)(Listener::AcceptState *);
      void *state;
      sockaddr_in addr;
    };
    std::map<int, ListenerState> listeners;  // Listening sockets.
    bool HandleListener(const pollfd &pfd, std::map<int, sockaddr> &incClients, const ListenerState &lState);

    // Client handling.
    struct ClientState
    {
      sockaddr addr;
      std::vector<char> dataBuf;
    };
    std::map<int, ClientState> clients;
    bool HandleClient(const pollfd &pfd);

    // Client message handling
    struct MessagePack
    {
      int fd;
      std::vector<unsigned char> messageData;
    };
    static void MessageHandler(void *pack);
    FirnLibs::Threading::Threadpool msgThreadpool;





    // Singleton class definitions
  public:
    static void Init();
    Networking(const Networking &) = delete;
    void operator=(const Networking &) = delete;
    ~Networking();
  protected:
    static Networking &GetInstance();
    Networking();

  };
}
