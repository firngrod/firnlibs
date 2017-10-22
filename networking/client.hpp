#pragma once

#ifndef _Networking_h_
#error Included client.hpp directly!  Include networking.hpp instead.
#endif



    class Client
    {
    public:
      Client();
      ~Client();

      class DataReceivedState
      {
        void * asyncState;
        std::vector<unsigned char> message;
        std::vector<unsigned char> reply;
      };
      bool Commence(const std::function<void (const std::vector<unsigned char> &)> callback,
                    const std::function<void (const int &)> errorCallback = nullptr);
      void Send(const std::vector<unsigned char> &data);
    protected:
      int identifier;
      int errorNo;

      void HandleIncData(const std::vector<unsigned char> &message);
      void ErrorCallback(const int &errorNo);

      // Callbacks from outside;
      std::function<void (const std::vector<unsigned char> &)> callback;
      std::function<void (const int &)> errorCallback;

      PipeMessagePack * limboState;
      friend Networking;
    };
