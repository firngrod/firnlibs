#pragma once
#define _Networking_h_
#include <vector>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "../threading/threadpool.hpp"
#include "../threading/guardedvar.hpp"
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
      SocketRemove, // Uses identifier, errorNo

      ListenerAdd, // Uses fd, *pAddr, pCallback, pErrorCallback, pCleanupCallback, identifier. 
                   // pCallback refers to an std::function<void (const std::shared_ptr<Client> &)>

      ConnectionAdd, // Uses fd, pAddr, pCallback, pErrorCallback, pCleanupCallback, identifier.
                     // pCallback refers to an std::function<void (const std::vector<unsigned char> &)>

      ConnectionQueueData, // Uses identifier, pDataBuf
    };
    struct PipeMessagePack
    {
      PipeMessage msg;
      int fd = -1;
      void *pAddr;
      void *pCallback; // This is a pointer to an std::function type which is different depending on the message.
      std::function<void (const int &)> *pErrorCallback;
      std::function<void ()> *pCleanupCallback;
      uint64_t identifier = 0;
      std::vector<unsigned char> * pDataBuf;
      int errorNo = 0;
    };

    // Client stuff
  public:
    #include "client.hpp"
    static std::shared_ptr<Client> NewClient();
  protected:
    friend Client;
    uint64_t ConnectTCP(const int &port, const std::string &address, const std::string &localAddress, 
                        const std::function<void (const std::vector<unsigned char> &)> &callback,
                        const std::function<void (const int &)> &errorCallback, const std::function<void ()> &cleanupCallback);

    // Listening stuff
  public:
    #include "listener.hpp"
  protected:
    uint64_t Listen(const int &port, const std::function<void (const std::shared_ptr<Client> &)> &callback,
                    const std::function<void (const int &)> &errorCallback, const std::function<void ()> &cleanupCallback);
    void SignalSocket(const PipeMessagePack &pack);
    friend Listener;


    // Things to do with the internal workings, mainly the PollDancer.
  protected:
    uint64_t GetIdentifier();
    void SignalCloseSocket(const uint64_t &identifier);
    void Cleanup();
    std::thread selectThread;

    // Select dancer function
    void PollDancer();

    // Pipe handling
    int pipe_fds[2];  // Signal pipe for the select dancer
    bool HandlePipe(const pollfd &pfd, std::vector<PipeMessagePack> &pipeData);

    // Listener handling.
    struct ListenerState
    {
      std::function<void (const std::shared_ptr<Client> &)> callback;
      std::function<void (const int &)> errorCallback;
      std::function<void ()> cleanupCallback;
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
      std::function<void (const int &)> errorCallback;
      std::function<void ()> cleanupCallback;
      uint64_t identifier;
      std::vector<unsigned char> sendBuf;
    };
    std::map<int, ClientState> clients;
    bool HandleClient(const pollfd &pfd, ClientState &cState);

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
