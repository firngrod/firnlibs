#include "networking.hpp"
#include <unistd.h>
#include <algorithm>
#include <sys/ioctl.h>
#include <cstring>
//#include <iostream>

namespace FirnLibs {

// Init
void Networking::Init()
{
  GetInstance();
}

Networking &Networking::GetInstance()
{
  static Networking instance;
  return instance;
}

Networking::Networking() : msgThreadpool(2)
{
  if(pipe(pipe_fds))
    return;

  auto lambda = [](Networking *instance) -> void
  {
    instance->PollDancer();
  };

  selectThread = std::thread(lambda, this);
}

Networking::~Networking()
{
  Cleanup();
}

void Networking::PollDancer()
{
  std::vector<pollfd> pfds;
  std::vector<PipeMessagePack> pipeData;

  bool cleanup = false;
  while(!cleanup)
  {
    // Reset the poll fds.
    pfds.resize(0);
    pipeData.resize(0);

    // Add the pipe.
    pfds.push_back(pollfd());
    pfds[0].fd = pipe_fds[0];
    pfds[0].events = POLLIN;
    
    // Add the listening sockets.
    for(auto lItr: listeners)
    {
      pfds.push_back(pollfd());
      pfds.back().fd=lItr.first;
      pfds.back().events = POLLIN;
    }

    // Add the client sockets.
    for(auto cItr: clients)
    {
      pfds.push_back(pollfd());
      pfds.back().fd=cItr.first;
      // Poll for incoming data.
      pfds.back().events = POLLIN;
      // If we have data to send, poll for ready to send.
      // DO NOT poll for this if you don't have data.  It will always be ready to send.
      if(cItr.second.sendBuf.size() > 0)
      {
        pfds.back().events |= POLLOUT;
      }
    }

    // Do the poll
    poll(&pfds[0], pfds.size(), -1);

    // Grab a snapshot of known descriptors so we know if any given descriptor is a listener, a data socket, whatever.
    // Notice that we don't lock the variables apart from just getting the copies.
    // It is valid to do this because we don't care if we get known descriptors which were not polled, and we know that
    // no descriptors were polled which are not currently known because we ONLY delete descriptors at the end of this thread.
    for(auto pItr: pfds)
    {
      // If nothing has happened, skip ahead.
      if(!pItr.revents)
        continue;
      
      // If it was the signal pipe, handle that.
      if(pItr.fd == pipe_fds[0])
      {
        cleanup = !HandlePipe(pItr, pipeData);
        continue;
      }

      // If it was a listener, handle that
      auto lItr = listeners.find(pItr.fd);
      if(lItr != listeners.end())
      {
        if(!HandleListener(pItr, lItr->second))
        {
          // If handler returned false, the listener is dead to us.
          // Add it to the list of sockets to remove.  We piggyback on the pipe.
          // This should generally never happen since we would normally kill listeners by piping it in the first place.
          pipeData.push_back(PipeMessagePack());
          pipeData.back().msg = SocketRemove;
          pipeData.back().identifier = lItr->second.identifier;
          pipeData.back().errorNo = errno;
        }
        continue;
      }

      // Finally, handle client connections.
      auto cItr = clients.find(pItr.fd);
      if(cItr != clients.end())
      {
        if(!HandleClient(pItr, cItr->second))
        {
          // If the client is closing, pretend that the kill signal was piped.
          pipeData.push_back(PipeMessagePack());
          pipeData.back().msg = SocketRemove;
          pipeData.back().identifier = cItr->second.identifier;
          pipeData.back().errorNo = errno;
        }
      }
    }

    // Now that we have all the stuff handled, let's see what we need to add/remove.
    // Add new listeners.
    for(auto lItr: pipeData)
    {
      if(lItr.msg == ListenerAdd)
      {
        // Add the listener to the list of listeners.
        ListenerState &lState = listeners[lItr.fd];
        lState.addr = (*(sockaddr_in*)lItr.pAddr);
        lState.callback = *((std::function<void (const std::shared_ptr<Client> &)> *)lItr.pCallback);
        lState.errorCallback = *lItr.pErrorCallback;
        lState.identifier = lItr.identifier;
        delete (sockaddr_in *)lItr.pAddr;
        delete (std::function<void (const std::shared_ptr<Client> &)> *)lItr.pCallback;
        delete lItr.pErrorCallback;

        // Actually listen
        listen(lItr.fd, 10);

        continue;
      }

      // Add new clients;
      if(lItr.msg == ConnectionAdd)
      {
        clients[lItr.fd] = ClientState();
        clients[lItr.fd].addr = *((sockaddr *)lItr.pAddr);
        clients[lItr.fd].callback = *((std::function<void (const std::vector<unsigned char> &)> *)lItr.pCallback);
        clients[lItr.fd].errorCallback = *lItr.pErrorCallback;
        clients[lItr.fd].identifier = lItr.identifier;
        delete (std::function<void (const std::vector<unsigned char> &)> *)lItr.pCallback;
        delete (sockaddr *)lItr.pAddr;
        delete lItr.pErrorCallback;
        continue;
      }

      if(lItr.msg == SocketRemove)
      {
        // Remove action depends on the type of socket.
        close(lItr.fd);

        // Find the fd with the specified identifier.
        {
          auto itr = listeners.begin();
          for(auto itrEnd = listeners.end(); itr != itrEnd; itr++)
            if(itr->second.identifier == lItr.identifier) break;
            
          if(itr != listeners.end())
          {
            if(itr->second.errorCallback != nullptr)
            {
              msgThreadpool.Push([itr, lItr]() -> void
              {
                itr->second.errorCallback(lItr.errorNo);
              });
            }
            listeners.erase(itr);
            continue;
          }
        }
        
        {
          auto itr = clients.begin();
          for(auto itrEnd = clients.end(); itr != itrEnd; itr++)
            if(itr->second.identifier == lItr.identifier) break;

          if(itr != clients.end())
          {
            if(itr->second.errorCallback != nullptr)
            {
              msgThreadpool.Push([itr, lItr]() -> void
              {
                itr->second.errorCallback(lItr.errorNo);
              });
            }
            clients.erase(itr);
            continue;
          }
        }
      }// if(lItr.msg == SocketRemove)

      if(lItr.msg == ConnectionQueueData)
      {
        auto cItr = clients.begin();
        for(auto cEnd = clients.end(); cItr != cEnd; cItr++)
          if(cItr->second.identifier == lItr.identifier) break;

        if(cItr != clients.end())
        {
          cItr->second.sendBuf.insert(cItr->second.sendBuf.end(), lItr.pDataBuf->begin(), lItr.pDataBuf->end());
          delete lItr.pDataBuf;
        }
      }
    }
  } // End master select loop while(!cleanup)

  // At this point, we need to clean up in general.  Loop everything we know and close it.
  close(pipe_fds[0]);
  for(auto itr: listeners)
    close(itr.first);

  for(auto itr: clients)
    close(itr.first);

  listeners.clear();
  clients.clear();
}

bool Networking::HandlePipe(const pollfd &pfd, std::vector<PipeMessagePack> &pipeData)
{
  // Let's see how much data is in the pipe;
  int readyData = 0;
  int rc = ioctl(pfd.fd, FIONREAD, &readyData);

  // First a sanity check.  Data amount MUST be a multiplier of PipeMsgSize, which is the current message size.
  // If it's not, wait for the rest.
  if(readyData % sizeof(PipeMessagePack) != 0)
    return true;

  // If no data is in the pipe, then the poll must have run because the pipe broke.
  if( readyData == 0)
  {
    return false;
  }

  // Otherwise, someone poked us in order to tell us that something needs to happen regarding sockets.
  // Either sockets need to be added or removed.
  std::vector<char> dataVec(readyData);
  rc = read(pfd.fd, &dataVec[0], dataVec.size());
  
  // Push the piped data to the map for later handling.
  for(size_t i = 0; i * sizeof(PipeMessagePack) < dataVec.size(); i++)
  {
    pipeData.push_back(*((PipeMessagePack *)&dataVec[i * sizeof(PipeMessagePack)]));
  }

  return true;
}

bool Networking::HandleListener(const pollfd &pfd, const ListenerState &lState)
{
  if(!(pfd.revents & POLLIN))
    return true;

  sockaddr clientAddr;
  socklen_t clilen = sizeof(clientAddr);
  // Get the incoming socket.
  int newfd = 0;

  newfd = accept(pfd.fd, &clientAddr, &clilen);
  if(newfd >= 0)
  {
    // Handle the new client.  This is done by first putting it on hold until the user can specify callback and such.
    // Create a message to send to the user.
    std::shared_ptr<Client> client = std::shared_ptr<Client>(new Client());
    client->limboState = new PipeMessagePack;
    client->limboState->identifier = GetIdentifier();
    client->limboState->fd = newfd;
    client->limboState->pAddr = (void *) new sockaddr(clientAddr); // TODO Make sure this is deleted.


    auto lambda = [lState, client]() -> void 
    {
      lState.callback(client);
    };

    msgThreadpool.Push(lambda);
  }
  // At this point, if we have no sockets, we have an error on the listener and it needs to die.
  if(newfd < 0)
    return false;

  return true;
}


bool Networking::HandleClient(const pollfd &pfd, ClientState &cState)
{
  // First, see if we have incoming data on the socket.
  if(pfd.revents & POLLIN)
  {
    int readyData = 0;
    int rc = ioctl(pfd.fd, FIONREAD, &readyData);

    // If no ready data, the trigger was the socket closing on the other end.
    if(readyData == 0)
    {
      return false;
    }

    // Receive the data.  First make room in the state data vector
    std::vector<unsigned char> message(readyData);

    // Now receive while there is data on the socket.
    size_t received = 0;
    while(received < readyData)
    {
      received += read(pfd.fd, &message[received], readyData - received);
    }

    msgThreadpool.Push([message, cState](){ cState.callback(message); });
  }
  
  // Now see if we are ready to send more data.
  // There should be no need to check for pending data since we should only have polled for this if there is.
  if(pfd.revents & POLLOUT)
  {
    // Do send.  Ask it not to block.
    ssize_t sentData = send(pfd.fd, &cState.sendBuf[0], cState.sendBuf.size(), MSG_DONTWAIT);
    
    // Did we have error?
    if(sentData == -1)
    {
      if(errno != EAGAIN || errno != EWOULDBLOCK)
      {
        return false;
      }
      return true;
    }

    // We sent some data.  Clear it from the send buffer.
    cState.sendBuf.erase(cState.sendBuf.begin(), cState.sendBuf.begin() + sentData);
  }

  return true;
}

uint64_t Networking::Listen(const int &port, const std::function<void (const std::shared_ptr<Client> &)> &callback,
                            const std::function<void (const int &)> &errorCallback)
{
  if(port == 0)
    return -1;

  // Get the fd
  int sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockFd < 0)
    return -1;

  // Bind the socket.
  sockaddr_in *lAddr = new sockaddr_in();
  memset(lAddr, 0, sizeof(sockaddr_in));
  lAddr->sin_family = AF_INET;
  lAddr->sin_addr.s_addr = INADDR_ANY;
  lAddr->sin_port = htons(port);
  if(bind(sockFd, (sockaddr *)lAddr, sizeof(sockaddr)) < 0)
  {
    delete lAddr;
    return -1;
  }

  // Message the listener to the select thread.
  PipeMessagePack data;
  data.msg = ListenerAdd;
  data.fd = sockFd;
  data.pAddr = (void *)lAddr;
  data.pCallback = (void *) new std::function<void (const std::shared_ptr<Client> &)>(callback);
  data.pErrorCallback = new std::function<void (const int &)>(errorCallback);
  data.identifier = GetIdentifier();

  write(pipe_fds[1], (char *)&data, sizeof(data));
  return data.identifier;
}

void Networking::SignalSocket(const PipeMessagePack &pack)
{
  write(pipe_fds[1], (char *)&pack, sizeof(pack));
}

void Networking::SignalCloseSocket(const uint64_t &identifier)
{
  if(identifier == 0)
    return;

  PipeMessagePack data;
  data.msg = SocketRemove;
  data.identifier = identifier;
  write(pipe_fds[1], (char *)&data, sizeof(data));
}

void Networking::Cleanup()
{
  // Close the pipe.  This will be interpreted as a close signal.
  close(pipe_fds[1]);

  // Finish up the queue.
  msgThreadpool.FinishUp();
  // Wait for the select thread to close.
  selectThread.join();
}

uint64_t Networking::GetIdentifier()
{
  // Overflow and identifier collision is possible, but not probable.  The world will probably end first.
  static int identifier = 0;
  // identifier == 0 signals invalid identifier.
  do identifier++; while (identifier == 0);
  return identifier;
}


}