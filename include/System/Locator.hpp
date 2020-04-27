#pragma once
#include <memory>

namespace ng {
class Logger;
class ResourceManager;

template<typename TService>
struct Locator {
  Locator() = delete;
  ~Locator() = delete;

  inline static void set(std::shared_ptr<TService> pService) {
    _pService = std::move(pService);
  }

  template<class ..._Args>
  inline static TService &create(_Args &&...__args) {
    _pService = std::move(std::make_shared<TService>(std::forward<_Args>(__args)...));
    return *_pService;
  }

  inline static TService &get() {
    return *_pService;
  }

  static void reset() {
    _pService.reset();
  }

private:
  static std::shared_ptr<TService> _pService;
};

template<typename TService>
std::shared_ptr<TService> Locator<TService>::_pService{};
}
