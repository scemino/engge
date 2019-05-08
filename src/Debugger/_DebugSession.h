#include "_ProtocolServer.h"

namespace ng
{
class _DebugSession : public _ProtocolServer, public DebuggerListener
{
private:
    bool _clientLinesStartAt1{true};
    Debugger &_debugger;

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
            std::vector<StackFrame> stackTrace;
            _debugger.getStackTrace(stackTrace);
            nlohmann::json response;
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
                    nlohmann::json source{{"name", frame.functionName}, {"path", "./" + frame.source}};
                    stackFrame["source"] = source;
                }
                stackFrames.push_back(stackFrame);
            }

            response["stackFrames"] = stackFrames;
            response["totalFrames"] = stackFrames.size();
            sendResponse(request, response);
        }
        else if (cmd == "scopes")
        {
            nlohmann::json response;
            std::vector<nlohmann::json> scopes;
            response["scopes"] = scopes;
            sendResponse(request, response);
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
    }
};
}
