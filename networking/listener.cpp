#include "networking.hpp"
namespace FirnLibs {

bool Networking::Listener::Listen(const int &port, const std::function<void (const std::shared_ptr<Client> &)> &callback)
{
  // If this already has an fd, don't overwrite it with a new 
  if(identifier != 0)
    return false;

  // We need a callback.  Not necessarily a state.
  if(callback == nullptr)
    return false;

  identifier = Networking::GetInstance().Listen(port, callback);

  if(identifier == 0)
    return false;

  return true;
}


Networking::Listener::~Listener()
{
  Stop();
}

void Networking::Listener::Stop()
{
  Networking::GetInstance().SignalCloseSocket(identifier);
  identifier = 0;
}

}
