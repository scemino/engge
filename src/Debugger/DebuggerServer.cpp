#include <iostream>
#include "Debugger.h"
#include "DebuggerServer.h"
#include "_DebugSession.h"

namespace ng
{
DebuggerServer::DebuggerServer(Debugger &debugger, int port)
    : _serverThread(runServer, std::ref(*this)), _port(port), _debugger(debugger)
{
}

void DebuggerServer::runServer(DebuggerServer &server)
{
    sf::TcpListener listener;
    listener.listen(server._port);
    while (true)
    {
        sf::TcpSocket client;
        if (listener.accept(client) == sf::Socket::Done)
        {
            std::cout << "New connection received from " << client.getRemoteAddress() << std::endl;
            server.runSession(client);
        }
    }
}

void DebuggerServer::runSession(sf::TcpSocket &socket)
{
    _DebugSession debugSession(_debugger);
    _debugger.add(&debugSession);
    debugSession.start(socket);
    _debugger.remove(&debugSession);
}
}
