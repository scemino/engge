#pragma once
#include <memory>

namespace ng
{
class Logger;
class ResourceManager;

template<typename TService>
struct Locator
{
  Locator() = delete;
  ~Locator() = delete;

  inline static void set(std::shared_ptr<TService> pService) {
        _pService = std::move(pService);
    }

  inline static TService& get() {
      return *_pService;
  }

  private:
    static std::shared_ptr<TService> _pService;
};

template<typename TService>
std::shared_ptr<TService> Locator<TService>::_pService{};
}
