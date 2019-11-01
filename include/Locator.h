#pragma once

namespace ng
{
class Logger;

class Locator
{
public:
    static Logger &getLogger() { return *_logger; }
    static void registerService(Logger *service) { _logger = service; }

private:
  static Logger* _logger;
};
}
