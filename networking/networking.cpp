#include "networking.hpp"
#include <unistd.h>
#include <algorithm>
#include <iostream>
#include <sys/ioctl.h>
#include <cstring>

namespace FirnLibs {

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

  selectThread = std::thread(PollDancerStat);
}

Networking::~Networking()
{
  Cleanup();
}


void Networking::PollDancerStat()
{
  GetInstance().PollDancer();
}

void Networking::PollDancer()
{
  std::vector<pollfd> pfds;
  std::vector<PipeMessagePack> pipeData;
  std::map<int, sockaddr> incClients;

  bool cleanup = false;
  while(!cleanup)
  {
    // Reset the poll fds.
    pfds.resize(0);
    pipeData.resize(0);
    incClients.clear();

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
        if(!HandleListener(pItr, incClients, lItr->second))
        {
          // If handler returned false, the listener is dead to us.
          // Add it to the list of sockets to remove.  We piggyback on the pipe.
          // This should generally never happen since we would normally kill listeners by piping it in the first place.
          pipeData.push_back(PipeMessagePack());
          pipeData.back().msg = SocketRemove;
          pipeData.back().fd = pItr.fd;
        }
        continue;
      }

      // Finally, handle client connections.
      auto cItr = clients.find(pItr.fd);
      if(cItr != clients.end())
      {
        if(!HandleClient(pItr))
        {
          pipeData.push_back(PipeMessagePack());
          pipeData.back().msg = SocketRemove;
          pipeData.back().fd = pItr.fd;
        }
      }
    }

    // Now that we have all the stuff handled, let's see what we need to add/remove.
    for(auto incItr: incClients)
    {
      clients[incItr.first] = ClientState();
      clients[incItr.first].addr = incItr.second;
    }

    // Add new listeners.
    for(auto lItr: pipeData)
    {
      if(lItr.msg == ListenerAdd)
      {
        ListenerState &lState = listeners[lItr.fd];
        lState.addr = (*(sockaddr_in*)lItr.pAddr);
        lState.callback = (void (*)(Listener::AcceptState*))lItr.callback;
        lState.state = lItr.state;
        delete (sockaddr_in *)lItr.pAddr;
      }

      // Add new clients;
      if(lItr.msg == ConnectionAdd)
      {
        clients[lItr.fd] = ClientState();
        clients[lItr.fd].addr = *((sockaddr *)lItr.pAddr);
        delete (sockaddr *)lItr.pAddr;
      }

      if(lItr.msg == SocketRemove)
      {
        // Remove action depends on the type of socket.
        close(lItr.fd);
        auto itr = listeners.find(lItr.fd);
        if(itr != listeners.end())
        {
          listeners.erase(itr);
          continue;
        }

        auto itr2 = clients.find(lItr.fd);
        if(itr2 != clients.end())
        {
          clients.erase(itr2);
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

bool Networking::HandleListener(const pollfd &pfd, std::map<int, sockaddr> &incClients, const ListenerState &lState)
{
  if(!(pfd.revents & POLLIN))
    return true;

  sockaddr clientAddr;
  socklen_t clilen = sizeof(clientAddr);
  // Get the incoming socket.
  int newfd = 0;
  do
  {
    newfd = accept(pfd.fd, &clientAddr, &clilen);
    if(newfd >= 0)
    {
      incClients[newfd] = clientAddr;
    }
  } while(newfd >= 0);

  // At this point, if we have no sockets, we have an error on the listener and it needs to die.
  if(incClients.size() == 0)
    return false;

  return true;
}

bool Networking::HandleClient(const pollfd &pfd)
{
  int readyData = 0;
  int rc = ioctl(pfd.fd, FIONREAD, &readyData);

  // If no ready data, the trigger was the socket closing on the other end.
  if(readyData == 0)
  {
    return false;
  }

  // Fetch socket state
  ClientState &state = clients[pfd.fd];

  // Receive the data.  First make room in the state data vector
  size_t prevSize = state.dataBuf.size();
  size_t recieved = 0;
  state.dataBuf.resize(state.dataBuf.size() + readyData);
  // Now receive while there is data on the socket.
  while(recieved < readyData)
  {
    recieved += read(pfd.fd, &state.dataBuf[prevSize + recieved], readyData - recieved);
  }
  
  // Now that we have data, check if we have a complete message.
  if(state.dataBuf.size() > sizeof(unsigned int32_t) && *((unsigned int32_t *)&state.dataBuf[0]) <= state.dataBuf.size())
  {
    // We do.  Pack it up in a neat little package and send it off to the message handler.
    MessagePack *msgPack = new MessagePack();
    msgPack->messageData.resize(*((unsigned int32_t *)&state.dataBuf[0]) - 4);
    memcpy(&msgPack->messageData[0], &state.dataBuf[4], msgPack->messageData.size());
    msgPack->fd = pfd.fd;
    msgThreadpool.Push(MessageHandler, (void *)msgPack);

    // Erase the handled data.
    state.dataBuf.erase(state.dataBuf.begin(), state.dataBuf.begin() + *((unsigned int32_t *)&state.dataBuf[0]));
  }

  return true;
}

void Networking::MessageHandler(void * pack)
{
  MessagePack * mPack = (MessagePack *)pack;
  std::cout << "Recieved data on socket " << mPack->fd << std::endl;
  std::cout << "Recieved data: " << (char *)&mPack->messageData[0] << std::endl;
}

int Networking::Listen(const int &port, void (*callback)(Listener::AcceptState *), void * callbackState)
{
  if(port == 0)
    return -1;

  // Get the fd
  int sockFd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockFd < 0)
    return -1;

  // Bind the socket.
  sockaddr_in *lAddr = new sockaddr_in();
  memset(lAddr, sizeof(sockaddr_in), 0);
  lAddr->sin_family = AF_INET;
  lAddr->sin_addr.s_addr = INADDR_ANY;
  lAddr->sin_port = htons(port);
  if(bind(sockFd, (sockaddr *)lAddr, sizeof(sockaddr)) < 0)
  {
    delete lAddr;
    return -1;
  }

  // Actually listen
  listen(sockFd, 10);

  // Message the listener to the select thread.
  PipeMessagePack data;
  data.msg = ListenerAdd;
  data.fd = sockFd;
  data.pAddr = (void *)lAddr;
  data.callback = (void *)callback;
  data.state = callbackState;

  write(pipe_fds[1], (char *)&data, sizeof(data));
  return sockFd;
}

void Networking::SignalCloseSocket(const int &fd)
{
  if(fd < 0)
    return;

  PipeMessagePack data;
  data.msg = SocketRemove;
  data.fd = fd;
  write(pipe_fds[1], (char *)&data, sizeof(data));
}

void Networking::Cleanup()
{
  close(pipe_fds[1]);
  selectThread.join();
}



}
