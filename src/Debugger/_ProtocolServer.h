#include <regex>
#include "nlohmann/json.hpp"
#include "_ByteBuffer.h"

#pragma once

namespace ng
{
class _ProtocolServer
{
private:
    static constexpr const char *TWO_CRLF{"\r\n\r\n"};
    int32_t _sequenceNumber{-1};
    int32_t _bodyLength{-1};
    sf::TcpSocket *_pOutputStream{nullptr};
    _ByteBuffer _rawData;
    std::regex CONTENT_LENGTH_MATCHER{"Content-Length: (\\d+)"};

public:
    void start(sf::TcpSocket &tcpSocket)
    {
        _pOutputStream = &tcpSocket;

        std::array<char, 4096> buffer;
        while (true)
        {
            size_t read;
            tcpSocket.receive(buffer.data(), buffer.size(), read);
            if (read == 0)
            {
                // end of stream
                break;
            }

            if (read > 0)
            {
                _rawData.append(buffer.data(), read);
                processData();
            }
        }
    }

    void sendEvent(const std::string &name)
    {
        nlohmann::json body;
        sendEvent(name, body);
    }

    void sendEvent(const std::string &name, nlohmann::json &body)
    {
        nlohmann::json event;
        event["seq"] = 0;
        event["type"] = "event";
        event["event"] = name;
        event["body"] = body;
        sendMessage(event);
    }

private:
    void processData()
    {
        while (true)
        {
            if (_bodyLength >= 0)
            {
                if (_rawData.size() >= _bodyLength)
                {
                    auto buf = _rawData.removeFirst(_bodyLength);
                    _bodyLength = -1;
                    dispatch(buf);
                    continue; // there may be more complete messages to process
                }
            }
            else
            {
                auto s = _rawData.getString();
                auto idx = s.find(TWO_CRLF);
                if (idx != std::string::npos)
                {
                    std::smatch matches;
                    if (std::regex_search(s, matches, CONTENT_LENGTH_MATCHER))
                    {
                        _bodyLength = std::atoi(matches[1].str().c_str());
                        _rawData.removeFirst(idx + 4);
                        continue; // try to handle a complete message
                    }
                }
            }
            break;
        }
    }

    void dispatch(std::string req)
    {
        auto json = nlohmann::json::parse(req);
        auto type = json["type"].get<std::string>();
        if (!type.empty())
        {
            if (type == "request")
            {
                dispatchRequest(json);
            }
            else if (type == "response")
            {
            }
        }
    }

protected:
    virtual void dispatchRequest(nlohmann::json &request) = 0;

    void sendMessage(nlohmann::json &message)
    {
        if (message["seq"].get<int>() == 0)
        {
            message["seq"] = _sequenceNumber++;
        }

        std::cout << message << std::endl;
        convertToBytes(message, *_pOutputStream);
    }

    static void convertToBytes(const nlohmann::json &request, sf::TcpSocket &os)
    {
        std::stringstream content;
        content << request;

        std::stringstream header;
        header << "Content-Length: " << size(content) << TWO_CRLF;

        os.send(header.str().data(), header.str().size());
        os.send(content.str().data(), content.str().size());
    }

    static size_t size(std::stringstream &ss)
    {
        ss.seekg(0, std::ios::end);
        return ss.tellg();
    }
};
}
