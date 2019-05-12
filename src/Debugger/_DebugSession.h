#include "_ProtocolServer.h"
#include "_Handles.h"
#include "../../extlibs/squirrel/squirrel/sqpcheader.h"
#include "../../extlibs/squirrel/squirrel/sqvm.h"
#include "../../extlibs/squirrel/squirrel/sqstring.h"
#include "../../extlibs/squirrel/squirrel/sqtable.h"
#include "../../extlibs/squirrel/squirrel/sqarray.h"
#include "../../extlibs/squirrel/squirrel/sqfuncproto.h"
#include "../../extlibs/squirrel/squirrel/sqclosure.h"
#include "../../extlibs/squirrel/squirrel/squserdata.h"
#include "../../extlibs/squirrel/squirrel/sqcompiler.h"
#include "../../extlibs/squirrel/squirrel/sqfuncstate.h"
#include "../../extlibs/squirrel/squirrel/sqclass.h"

namespace ng
{
class _DebugSession : public _ProtocolServer, public DebuggerListener
{
private:
    bool _clientLinesStartAt1{true};
    Debugger &_debugger;
    _Handles<std::vector<Variable>> _variablesHandles;

public:
    explicit _DebugSession(Debugger &debugger)
        : _debugger(debugger)
    {
    }

    void sendResponse(nlohmann::json &request, nlohmann::json &body)
    {
        nlohmann::json response;
        response["type"] = "response";
        response["success"] = true;
        response["seq"] = request["seq"].get<int>();
        response["request_seq"] = request["seq"].get<int>();
        response["command"] = request["command"].get<std::string>();
        response["body"] = body;
        sendMessage(response);
    }

protected:
    void dispatchRequest(nlohmann::json &request) override
    {
        std::cout << request << std::endl;

        auto cmd = request["command"].get<std::string>();
        if (cmd == "initialize")
        {
            if (!request["linesStartAt1"].is_null())
            {
                _clientLinesStartAt1 = request["linesStartAt1"].get<bool>();
            }
            initialize(request);
        }
        else if (cmd == "setBreakpoints")
        {
            if (!request["linesStartAt1"].is_null())
            {
                _clientLinesStartAt1 = request["linesStartAt1"].get<bool>();
            }
            setBreakpoints(request);
        }
        else if (cmd == "setExceptionBreakpoints")
        {
            nlohmann::json response;
            sendResponse(request, response);
        }
        else if (cmd == "threads")
        {
            nlohmann::json response;
            nlohmann::json thread{{"id", 1}, {"name", "main"}};
            std::vector<nlohmann::json> threads{thread};
            response["threads"] = threads;
            sendResponse(request, response);
        }
        else if (cmd == "stackTrace")
        {
            stackTrace(request);
        }
        else if (cmd == "scopes")
        {
            scopes(request);
        }
        else if (cmd == "variables")
        {
            variables(request);
        }
        else if (cmd == "loadedSources")
        {
            nlohmann::json response;
            std::vector<nlohmann::json> sources;
            response["sources"] = sources;
            sendResponse(request, response);
        }
        else if (cmd == "source")
        {
            auto path = request["arguments"]["source"]["path"];
            auto source = _debugger.getSource(path);
            nlohmann::json response;
            response["content"] = source;
            sendResponse(request, response);
        }
        else if (cmd == "continue")
        {
            nlohmann::json response;
            sendResponse(request, response);
            _debugger.resume();
        }
        else if (cmd == "launch")
        {
            nlohmann::json response;
            sendResponse(request, response);
        }
        else if (cmd == "pause")
        {
            nlohmann::json response;
            sendResponse(request, response);
            _debugger.pause();
        }
        else if (cmd == "next" || cmd == "stepIn" || cmd == "stepOut")
        {
            nlohmann::json response;
            sendResponse(request, response);
            _debugger.step();
        }
    }

    void stackTrace(nlohmann::json &request)
    {
        std::vector<StackFrame> stackTrace;
        _debugger.getStackTrace(stackTrace);
        std::vector<nlohmann::json> stackFrames;
        int id = 0;
        for (auto &&frame : stackTrace)
        {
            nlohmann::json stackFrame{
                {"id", id++},
                {"name", frame.functionName},
                {"line", frame.line},
                {"column", 0}};
            if (!frame.source.empty())
            {
                nlohmann::json source{{"name", frame.functionName}, {"path", frame.source}};
                stackFrame["source"] = source;
            }
            stackFrames.push_back(stackFrame);
        }

        nlohmann::json response{
            {"stackFrames", stackFrames},
            {"totalFrames", stackFrames.size()}};
        sendResponse(request, response);
    }

    void scopes(nlohmann::json &request)
    {
        auto frameId = request["arguments"]["frameId"].get<int>();
        std::vector<Variable> vars;
        _debugger.getVariables(frameId, vars);

        std::vector<nlohmann::json> scopes;
        if (!vars.empty())
        {
            auto id = _variablesHandles.create(vars);
            nlohmann::json scope{
                {"name", "Locals"},
                {"variablesReference", id},
                {"expensive", false}};
            scopes.push_back(scope);
        }
        nlohmann::json response{{"scopes", scopes}};
        sendResponse(request, response);
    }

    void variables(nlohmann::json &request)
    {
        auto varId = request["arguments"]["variablesReference"].get<int>();
        auto it = _variablesHandles.find(varId);

        std::vector<nlohmann::json> variables;
        if (it != _variablesHandles.end())
        {
            auto &vars = it->second;
            std::string type, value;
            for (auto &&var : vars)
            {
                int32_t id = 0;
                switch (sq_type(var.object))
                {
                case OT_NULL:
                    type = "null";
                    value = "null";
                    break;
                case OT_INTEGER:
                {
                    SQInteger i = sq_objtointeger(&var.object);
                    type = "int";
                    std::stringstream s;
                    s << i;
                    value = s.str();
                    break;
                }
                case OT_FLOAT:
                {
                    SQFloat f = sq_objtofloat(&var.object);
                    type = "float";
                    std::stringstream s;
                    s << f;
                    value = s.str();
                    break;
                }
                case OT_USERPOINTER:
                {
                    type = "userpointer";
                    value = "?";
                    break;
                }
                case OT_STRING:
                {
                    const SQChar *s = sq_objtostring(&var.object);
                    type = "string";
                    value = s;
                    break;
                }
                case OT_TABLE:
                {
                    auto size = _table(var.object)->CountUsed();
                    if (size > 0)
                    {
                        SQObjectPtr refidx, key, val;
                        SQInteger idx;
                        std::vector<Variable> vars;
                        while ((idx = _table(var.object)->Next(false, refidx, key, val)) != -1)
                        {
                            refidx = idx;
                            Variable v{_stringval(key), val};
                            vars.push_back(v);
                        }

                        id = _variablesHandles.create(vars);
                    }

                    std::stringstream s;
                    s << "{size=" << size << "}";
                    type = "table";
                    value = s.str();
                    break;
                }
                case OT_ARRAY:
                {
                    auto size = _array(var.object)->Size();
                    if (size > 0)
                    {
                        SQObjectPtr refidx, key, val;
                        SQInteger idx;
                        std::vector<Variable> vars;
                        while ((idx = _array(var.object)->Next(refidx, key, val)) != -1)
                        {
                            std::stringstream keyIndex;
                            keyIndex << '[' << _integer(key) << ']';
                            refidx = idx;
                            Variable v{keyIndex.str(), val};
                            vars.push_back(v);
                        }

                        id = _variablesHandles.create(vars);
                    }

                    std::stringstream s;
                    s << "(size=" << size << ")";
                    type = "[]";
                    value = s.str();
                    break;
                }
                case OT_CLOSURE:
                {
                    type = "closure";
                    value = "()";
                    break;
                }
                case OT_NATIVECLOSURE:
                {
                    type = "native closure";
                    value = "()";
                    break;
                }
                case OT_GENERATOR:
                {
                    type = "generator";
                    value = "()";
                    break;
                }
                case OT_USERDATA:
                {
                    type = "userdata";
                    value = "?";
                    break;
                }
                case OT_THREAD:
                {
                    type = "thread";
                    value = "{}";
                    break;
                }
                case OT_CLASS:
                {
                    type = "class";
                    value = "{}";
                    break;
                }
                case OT_INSTANCE:
                {
                    type = "instance";
                    value = "{}";
                    break;
                }
                case OT_WEAKREF:
                {
                    type = "weakref";
                    value = "*";
                    break;
                }
                case OT_BOOL:
                {
                    SQBool bval = sq_objtobool(&var.object);
                    type = "bool";
                    value = bval == SQTrue ? "true" : "false";
                }
                break;
                default:
                    assert(0);
                    break;
                }

                nlohmann::json variable{
                    {"name", var.name},
                    {"type", type},
                    {"value", value}};
                if (id != 0)
                {
                    variable["variablesReference"] = id;
                }
                variables.push_back(variable);
            }
        }

        nlohmann::json response{
            {"variables", variables}};
        sendResponse(request, response);
    }

    void initialize(nlohmann::json &request)
    {
        nlohmann::json capabilities = {
            // This debug adapter does not need the configurationDoneRequest.
            {"supportsConfigurationDoneRequest", false},
            // This debug adapter does not support function breakpoints.
            {"supportsFunctionBreakpoints", false},
            // This debug adapter doesn't support conditional breakpoints.
            {"supportsConditionalBreakpoints", false},
            // This debug adapter does not support a side effect free evaluate request for data hovers.
            {"supportsEvaluateForHovers", false},
            {"supportsExceptionOptions", {}},
            {"supportsLoadedSourcesRequest", true}};
        sendResponse(request, capabilities);
        // engge Debug is ready to accept breakpoints immediately
        sendEvent("initialized");
    }

    void setBreakpoints(nlohmann::json &request)
    {
        auto &args = request["arguments"];
        auto &source = args["source"];
        std::string path;
        if (!source.is_null() && !source["path"].is_null())
        {
            path = source["path"].get<std::string>();
        }
        if (path.empty())
        {
            // TODO:
            //sendErrorResponse(response, 3010, "setBreakpoints: property 'source' is empty or misformed", null, false, true);
            return;
        }

        for (auto &&bp : _debugger.getBreakpoints())
        {
            if (bp.path != path)
                continue;
            _debugger.removeBreakpoint(bp);
        }

        for (auto &&bp : args["breakpoints"])
        {
            auto line = bp["line"].get<int>();
            _debugger.addBreakpoint(path, line);
        }

        nlohmann::json jBreakpoints;
        for (auto &&bp : _debugger.getBreakpoints())
        {
            if (bp.path != path)
                continue;
            nlohmann::json jBreakpoint;
            jBreakpoint["line"] = bp.line;
            jBreakpoint["verified"] = true;
            jBreakpoints.push_back(jBreakpoint);
        }

        nlohmann::json jBody;
        jBody["breakpoints"] = jBreakpoints;
        sendResponse(request, jBody);
    }

    void onBreakpointAdded(const Breakpoint &breakpoint) override
    {
    }

    void onBreakpointHit(const Breakpoint &breakpoint) override
    {
        nlohmann::json body{
            {"reason", "breakpoint"},
            {"description", "Breakpoint hit"},
            {"threadId", 1},
            {"allThreadsStopped", true}};
        sendEvent("stopped", body);
        _variablesHandles.reset();
    }

    void onPaused() override
    {
        nlohmann::json body{
            {"reason", "step"},
            {"description", "Pause"},
            {"threadId", 1},
            {"allThreadsStopped", true}};
        sendEvent("stopped", body);
        _variablesHandles.reset();
    }
};
} // namespace ng
