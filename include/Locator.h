#pragma once

namespace ng
{
class Logger;
class ResourceManager;

class Locator
{
public:
    static Logger &getLogger() { return *_logger; }
    static ResourceManager &getResourceManager() { return *_resourceManager; }
    static void registerService(Logger *service) { _logger = service; }
    static void registerService(ResourceManager *service) { _resourceManager = service; }

private:
  static Logger* _logger;
  static ResourceManager* _resourceManager;
};
}
