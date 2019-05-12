#pragma once
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include "SFML/Network.hpp"
#include "squirrel.h"

namespace ng
{
class Engine;
class ScriptEngine;

struct Breakpoint
{
    std::string name;
    std::string path;
    SQInteger line;
};

class DebuggerListener
{
public:
    virtual void onBreakpointAdded(const Breakpoint &breakpoint) = 0;
    virtual void onBreakpointHit(const Breakpoint &breakpoint) = 0;
    virtual void onPaused() = 0;
};

struct StackFrame
{
    std::string source;
    std::string functionName;
    int32_t line;
};

struct Variable
{
    std::string name;
    HSQOBJECT object;
};

enum class DebuggerState
{
    Running,
    Pause
};

class Debugger
{
public:
    Debugger(Engine &engine, ScriptEngine &scriptEngine);

    void pause();
    void resume();
    void step();

    static void execute(std::string code);

    void addBreakpoint(std::string filename, SQInteger line);
    void removeBreakpoint(const Breakpoint &breakpoint);
    const std::vector<Breakpoint> &getBreakpoints() const;

    void getStackTrace(std::vector<StackFrame> &stackFrames);
    void getVariables(int level, std::vector<Variable> &variables);
    std::string getSource(std::string path);

    void add(DebuggerListener *listener);
    void remove(DebuggerListener *listener);

private:
    static void debugHook(HSQUIRRELVM v, SQInteger type,
                          const SQChar *sourcename, SQInteger line, const SQChar *funcname);

private:
    ScriptEngine &_scriptEngine;
    std::mutex _mutex;
    std::vector<Breakpoint> _breakpoints;
    std::vector<DebuggerListener *> _listeners;
    HSQUIRRELVM _vm{nullptr};
    DebuggerState _state{DebuggerState::Running};
};
} // namespace ng