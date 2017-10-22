#pragma once

#ifndef _Networking_h_
#error Included listener.hpp directly!  Include networking.hpp instead.
#endif

    class Listener
    {
    public:
      Listener();
      ~Listener();

      bool Listen(const int &port, const std::function<void (const std::shared_ptr<Client> &)> &callback,
                  const std::function<void (const int &)> &errorCallback = nullptr);
      void Stop();
      // Prevent copying to avoid issues with deconstruction;
      Listener(const Listener &) = delete;
      void operator=(const Listener &) = delete;
    protected:

      // Our own callback functions.
      void ErrorCallback(const int &errorNo);
      void Callback(const std::shared_ptr<Client> &client);

      // Callback functions from outside.
      std::function<void (const std::shared_ptr<Client> &client)> callback;
      std::function<void (const int &)> errorCallback;

      int errorNo;
      unsigned int identifier;
      friend Networking;
    };


