#pragma once
#include "squirrel.h"
#include "ScriptExecute.h"

namespace ng
{
class _DefaultScriptExecute : public ScriptExecute
{
  public:
    explicit _DefaultScriptExecute(HSQUIRRELVM vm) : _vm(vm) {}

  public:
    void execute(const std::string &code) override
    {
        sq_resetobject(&_result);
        _pos = 0;
        auto top = sq_gettop(_vm);
        // compile
        sq_pushroottable(_vm);
        if (SQ_FAILED(sq_compilebuffer(_vm, code.data(), code.size(), _SC("_DefaultScriptExecute"), SQTrue)))
        {
            error("Error executing code {}", code);
            return;
        }
        sq_push(_vm, -2);
        // call
        if (SQ_FAILED(sq_call(_vm, 1, SQTrue, SQTrue)))
        {
            error("Error calling code {}", code);
            return;
        }
        sq_getstackobj(_vm, -1, &_result);
        sq_settop(_vm, top);
    }

    bool executeCondition(const std::string &code) override
    {
        std::string c;
        c.append("return ");
        c.append(code);

        execute(c);
        if (_result._type == OT_BOOL)
        {
            trace("{} returns {}", code, sq_objtobool(&_result));
            return sq_objtobool(&_result);
        }

        if (_result._type == OT_INTEGER)
        {
            trace("{} return {}", code, sq_objtointeger(&_result));
            return sq_objtointeger(&_result) != 0;
        }

        error("Error getting result {}", code);
        return false;
    }

    std::string executeDollar(const std::string &code) override
    {
        std::string c;
        c.append("return ");
        c.append(code);

        execute(c);
        // get the result
        if (_result._type != OT_STRING)
        {
            error("Error getting result {}", code);
            return "";
        }
        trace("{} returns {}", code, sq_objtostring(&_result));
        return sq_objtostring(&_result);
    }

    SoundDefinition *getSoundDefinition(const std::string &name) override
    {
        sq_pushroottable(_vm);
        sq_pushstring(_vm, name.data(), -1);
        sq_get(_vm, -2);
        HSQOBJECT obj;
        sq_getstackobj(_vm, -1, &obj);
        sq_pop(_vm, 2);

        if (!sq_isuserpointer(obj))
        {
            error("getSoundDefinition: sound should be a userpointer");
            return nullptr;
        }

        SoundDefinition *pSound = static_cast<SoundDefinition *>(obj._unVal.pUserPointer);
        return pSound;
    }

  private:
    static int _pos;
    HSQUIRRELVM _vm{};
    HSQOBJECT _result{};
};
int _DefaultScriptExecute::_pos = 0;
}
