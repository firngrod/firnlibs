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
      ListenerAdd, // Uses fd, *pAddr, pCallback, identifier.  pCallback refers to an std::function<void (const std::shared_ptr<Client> &)>
      ConnectionAdd, // Uses fd, *pAddr, pCallback, identifier.  pCallback refers to an std::function<void (const std::vector<unsigned char> &)>
    };
    struct PipeMessagePack
    {
      PipeMessage msg;
      int fd = -1;
      void *pAddr;
      void *pCallback; // This is a pointer to an std::function type which is different depending on the message.
      uint64_t identifier = 0;
    };

    // Client stuff
  public:
    #include "client.hpp"
  protected:
    friend Client;

    // Listening stuff
  public:
    #include "listener.hpp"
  protected:
    uint64_t Listen(const int &port, const std::function<void (const std::shared_ptr<Client> &)> &callback);
    void AddSocket(const PipeMessagePack &pack);
    friend Listener;


    // Things to do with the internal workings, mainly the PollDancer.
  protected:
    uint64_t GetIdentifier();
    void SignalCloseSocket(const uint64_t &identifier);
    void Cleanup();
    std::thread selectThread;

    // Select dancer function
    void PollDancer();


    int pipe_fds[2];  // Signal pipe for the select dancer
    bool HandlePipe(const pollfd &pfd, std::vector<PipeMessagePack> &pipeData);

    // Listener handling.
    struct ListenerState
    {
      std::function<void (const std::shared_ptr<Client> &)> callback;
      sockaddr_in addr;
      uint64_t identifier;
    };
    std::map<int, ListenerState> listeners;  // Listening sockets.
    bool HandleListener(const pollfd &pfd, const ListenerState &lState);

    // Client handling.
    struct ClientState
    {
      sockaddr addr;
      std::function<void (const std::vector<unsigned char> &)> callback;
      uint64_t identifier;
    };
    std::map<int, ClientState> clients;
    bool HandleClient(const pollfd &pfd, const ClientState &cState);

    // Client message handling
    struct MessagePack
    {
      int fd;
      std::vector<unsigned char> messageData;
    };

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
