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
      bool Commence(void (*callback)(DataReceivedState *), void * asyncState);
    protected:
      int identifier;

      void HandleIncData(const std::vector<unsigned char> &message);
      PipeMessagePack * limboState;
      friend Networking;
    };
