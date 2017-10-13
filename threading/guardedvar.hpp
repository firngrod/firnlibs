#pragma once

namespace FirnLibs
{
  namespace Threading
  {
    template<class T> class GuardedVar
    {
    public:
      template<class TT> class Token
      {
      friend GuardedVar;
      public:
        TT *operator->() { return &child; }
        Token<TT> &operator=(const TT &other) { child = other; }
        operator TT&() { return child; };
        operator TT() const { return child; }
      private:
        Token(TT &input, std::mutex &mute) : child(input), mut(mute) {lock = std::unique_lock<std::mutex>(mut);};
        TT &child;
        std::mutex &mut;
        std::unique_lock<std::mutex> lock;
      };

      GuardedVar(const T &input = T()) { child = input; }
      Token<T> Get() { return Token<T>(child, mut); }

    private:
      GuardedVar<T> operator=(const GuardedVar<T> &other) {}
      T child;
      std::mutex mut;
    };
  }
}
