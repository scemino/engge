#include "Engine/Callback.hpp"
#include "System/Locator.hpp"
#include "System/Logger.hpp"
#include "Engine/ResourceManager.hpp"

namespace ng
{
Callback::Callback(HSQUIRRELVM v, sf::Time duration, HSQOBJECT method)
: TimeFunction(duration), _v(v), _done(false), _method(method)
{
    _id = Locator<ResourceManager>::get().getCallbackId();
    sq_addref(_v, &_method);
}

void Callback::onElapsed()
{
    if (_done)
        return;
    _done = true;

    sq_pushobject(_v, _method);
    sq_pushroottable(_v);
    if (SQ_FAILED(sq_call(_v, 1, SQFalse, SQTrue)))
    {
        error("failed to call callback");
    }
    sq_release(_v, &_method);
}
} // namespace ng
