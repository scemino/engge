#pragma once
#include <memory>

namespace ng {
class Logger;
class EntityManager;

template<typename TService>
struct Locator {
  Locator() = delete;
  ~Locator() = delete;

  inline static void set(std::shared_ptr<TService> pService) {
    m_pService = std::move(pService);
  }

  template<class ..._Args>
  inline static TService &create(_Args &&...__args) {
    m_pService = std::move(std::make_shared<TService>(std::forward<_Args>(__args)...));
    return *m_pService;
  }

  inline static TService &get() {
    return *m_pService;
  }

  static void reset() {
    m_pService.reset();
  }

private:
  static std::shared_ptr<TService> m_pService;
};

template<typename TService>
std::shared_ptr<TService> Locator<TService>::m_pService{};
}
