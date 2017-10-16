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
  protected:
    // Struct definitions

    // Pipe handling.
    enum PipeMessage
    {
      SocketRemove, // Uses identifier
      ListenerAdd, // Uses fd, *pAddr, callback, callbackState, identifier
      ConnectionAdd, // Uses fd and *pAddr.
    };
    struct PipeMessagePack
    {
      PipeMessage msg;
      int fd = -1;
      void *pAddr;
      void *callback;
      void *state;
      uint64_t identifier = 0;
    };

    // Client stuff
  public:
    #include "client.hpp"
  protected:
    friend Client;

    // Listening stuff
    #include "listener.hpp"
  protected:
    uint64_t Listen(const int &port, void (*callback)(Listener::AcceptState *), void * callbackState);
    friend Listener;


    // Things to do with the internal workings, mainly the PollDancer.
  protected:
    uint64_t GetIdentifier();
    void SignalCloseSocket(const uint64_t &identifier);
    void Cleanup();
    std::thread selectThread;

    // Select dancer function
    void PollDancer();
    static void PollDancerStat();


    int pipe_fds[2];  // Signal pipe for the select dancer
    bool HandlePipe(const pollfd &pfd, std::vector<PipeMessagePack> &pipeData);

    // Listener handling.
    struct ListenerState
    {
      void (*callback)(Listener::AcceptState *);
      void *state;
      sockaddr_in addr;
      uint64_t identifier;
    };
    std::map<int, ListenerState> listeners;  // Listening sockets.
    bool HandleListener(const pollfd &pfd, const ListenerState &lState);
    struct ListenerStagingState
    {
      void (*callback)(Listener::AcceptState *);
      Listener::AcceptState * state;
    };
    static void StatListenerStaging(void *listenerStagingState);

    // Client handling.
    struct ClientState
    {
      sockaddr addr;
      std::vector<char> dataBuf;
      uint64_t identifier;
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
