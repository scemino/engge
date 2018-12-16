#pragma once
#include "Function.h"
#include "NGActor.h"

namespace ng
{
class _SayFunction : public Function
{
  public:
    _SayFunction(NGActor &actor, const int id)
        : _actor(actor), _id(id), _done(false)
    {
    }

    bool isElapsed() { return _done && !_actor.isTalking(); }

    virtual void operator()()
    {
        if (_done)
            return;
        _actor.say(_id);
        _done = true;
    }

  private:
    NGActor &_actor;
    const int _id;
    bool _done;
};
}
