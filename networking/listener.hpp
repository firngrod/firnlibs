#pragma once

#ifndef _Networking_h_
#error Included listener.hpp directly!  Include networking.hpp instead.
#endif

    class Listener
    {
    public:
      Listener() { identifier = 0;}
      ~Listener();

      bool Listen(const int &port, const std::function<void (const std::shared_ptr<Client> &)> &callback);
      void Stop();
      // Prevent copying to avoid issues with deconstruction;
      Listener(const Listener &) = delete;
      void operator=(const Listener &) = delete;
    protected:
      int identifier;
      friend Networking;
    };


