#include <thread>
#include "SFML/network.hpp"

#pragma once

namespace ng
{
class Debugger;
class DebuggerServer
{
public:
    explicit DebuggerServer(Debugger& debugger, int port);

private:
    static void runServer(DebuggerServer &server);
    void runSession(sf::TcpSocket &socket);

private:
    std::thread _serverThread;
    int _port{0};
    Debugger& _debugger;
};
}
