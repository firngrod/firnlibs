#pragma once
#include <mutex>

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
        TT Copy() const { return child; }
        TT &Copy(TT& out) const { return out = child; }
      private:
        Token(TT &input, std::recursive_mutex &mute) : child(input), mut(mute) {lock = std::unique_lock<std::recursive_mutex>(mut);};
        TT &child;
        std::recursive_mutex &mut;
        std::unique_lock<std::recursive_mutex> lock;
      };

      GuardedVar() {};
      GuardedVar(const T &input) { child = input; }
      Token<T> Get() { return Token<T>(child, mut); }
      GuardedVar<T> operator=(const GuardedVar<T> &other) = delete;
      GuardedVar<T> (const GuardedVar<T> &other) = delete;

    private:
      T child;
      std::recursive_mutex mut;
    };
  }
}
