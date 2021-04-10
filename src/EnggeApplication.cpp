#include "engge/EnggeApplication.hpp"
#include "engge/Input/InputMappings.hpp"
#include "Engine/DebugFeatures.hpp"
#include <ngf/Graphics/Colors.h>
#include "engge/Engine/EngineCommands.hpp"

namespace {
ng::InputConstants toKey(ngf::Scancode key) {
  if (key >= ngf::Scancode::A && key <= ngf::Scancode::Z) {
    return static_cast<ng::InputConstants>(static_cast<int>(ng::InputConstants::KEY_A)
        + (static_cast<int>(key) - static_cast<int>(ngf::Scancode::A)));
  }
  if (key >= ngf::Scancode::Keypad1 && key <= ngf::Scancode::Keypad9) {
    return static_cast<ng::InputConstants>(static_cast<int>(ng::InputConstants::KEY_1)
        + (static_cast<int>(key) - static_cast<int>(ngf::Scancode::Keypad1)));
  }
  if (key == ngf::Scancode::Keypad0) {
    return ng::InputConstants::KEY_0;
  }
  if (key >= ngf::Scancode::D1 && key <= ngf::Scancode::D9) {
    return static_cast<ng::InputConstants>(static_cast<int>(ng::InputConstants::KEY_PAD1)
        + (static_cast<int>(key) - static_cast<int>(ngf::Scancode::D1)));
  }
  if (key >= ngf::Scancode::F1 && key <= ngf::Scancode::F12) {
    return static_cast<ng::InputConstants>(static_cast<int>(ng::InputConstants::KEY_F1)
        + (static_cast<int>(key) - static_cast<int>(ngf::Scancode::F1)));
  }
  switch (key) {
  case ngf::Scancode::Space:return ng::InputConstants::KEY_SPACE;
  case ngf::Scancode::Escape:return ng::InputConstants::KEY_ESCAPE;
  case ngf::Scancode::Left:return ng::InputConstants::KEY_LEFT;
  case ngf::Scancode::Right:return ng::InputConstants::KEY_RIGHT;
  case ngf::Scancode::Up:return ng::InputConstants::KEY_UP;
  case ngf::Scancode::Down:return ng::InputConstants::KEY_DOWN;
  case ngf::Scancode::Tab:return ng::InputConstants::KEY_TAB;
  case ngf::Scancode::Return:return ng::InputConstants::KEY_RETURN;
  case ngf::Scancode::Backspace:return ng::InputConstants::KEY_BACKSPACE;
  default:return ng::InputConstants::NONE;
  }
}

ng::MetaKeys toMetaKeys(const ngf::KeyModifiers &modifiers) {
  auto metaKey = ((static_cast<int>(modifiers) & static_cast<int>(ngf::KeyModifiers::Alt)) ? ng::MetaKeys::Alt
                                                                                           : ng::MetaKeys::None) |
      ((static_cast<int>(modifiers) & static_cast<int>(ngf::KeyModifiers::Control)) ? ng::MetaKeys::Control
                                                                                    : ng::MetaKeys::None) |
      ((static_cast<int>(modifiers) & static_cast<int>(ngf::KeyModifiers::Shift)) ? ng::MetaKeys::Shift
                                                                                  : ng::MetaKeys::None) |
      ((static_cast<int>(modifiers) & static_cast<int>(ngf::KeyModifiers::Gui)) ? ng::MetaKeys::System
                                                                                : ng::MetaKeys::None);
  return metaKey;
}
}

namespace ng {
void EnggeApplication::onInit() {
  m_window.init({"Engge", {ng::Screen::Width, ng::Screen::Height}});
  ng::Services::init();

  // read achievements if any
  auto achievementsPath = ng::Locator<ng::EngineSettings>::get().getPath();
  achievementsPath.append("save.dat");
  if (std::filesystem::exists(achievementsPath)) {
    ng::Locator<ng::AchievementManager>::get().load(achievementsPath);
  }

  // detect if we have any ggpack file
  if (ng::Locator<ng::EngineSettings>::get().getPackCount() == 0) {
    std::string s;
    s = "No ggpack files detected.";
    throw std::logic_error(s);
  }

  auto &scriptEngine = ng::Locator<ng::ScriptEngine>::create();
  m_engine = &ng::Locator<ng::Engine>::create();
  m_engine->setApplication(this);
  scriptEngine.setEngine(*m_engine);
  m_debugTools = std::make_unique<ng::DebugTools>(*m_engine);

  Locator<CommandManager>::get().registerCommands({
                                                      {EngineCommands::ToggleDebug,
                                                       [this] { m_debugTools->visible = !m_debugTools->visible; }}
                                                  });

  ng::InputMappings::registerMappings();
  ng::info("Start game");
}

void EnggeApplication::onEvent(ngf::Event &event) {
  switch (event.type) {
  case ngf::EventType::Quit:
    ng::Locator<Engine>::reset();
    ng::Locator<SoundManager>::reset();
    m_engine = nullptr;
    break;
  case ngf::EventType::WindowResized:
    getRenderTarget()->setView(ngf::View{
        ngf::frect::fromPositionSize({0, 0}, glm::vec2(event.resize.size) * ngf::Window::getDpiScale())});
    break;
  case ngf::EventType::MouseButtonPressed:
    if (event.mouseButton.button == 0) {
      m_isMousePressed = true;
      m_pos = event.mouseButton.position;
    }
    break;
  case ngf::EventType::MouseButtonReleased:
    if (event.mouseButton.button == 0) {
      m_isMousePressed = false;
    }
    break;
  case ngf::EventType::MouseMoved: {
    if (!m_isMousePressed || !m_isKeyPressed)
      break;
    auto pos2 = event.mouseMoved.position;
    auto delta = pos2 - m_pos;
    if (abs(delta.x) < 50) {
      m_engine->getCamera().move(-(glm::vec2) delta);
    }
    m_pos = pos2;
  }
    break;
  case ngf::EventType::KeyReleased: {
    auto key = toKey(event.key.scancode);
    auto metaKey = toMetaKeys(event.key.modifiers);
    if (key != ng::InputConstants::NONE) {
      m_engine->keyUp({metaKey, key});
    }
    if (event.key.scancode == ngf::Scancode::Space) {
      m_isKeyPressed = false;
    }
  }
    break;
  case ngf::EventType::KeyPressed: {
    auto key = toKey(event.key.scancode);
    auto metaKey = toMetaKeys(event.key.modifiers);
    if (key != ng::InputConstants::NONE) {
      m_engine->keyDown({metaKey, key});
    }
    if (event.key.scancode == ngf::Scancode::Space) {
      m_isKeyPressed = true;
    }
    break;
  }
  default:break;
  }
}

void EnggeApplication::onRender(ngf::RenderTarget &target) {
  ngf::StopWatch clock;
  target.clear();
  if (m_engine)
    m_engine->draw(target);
  Application::onRender(target);
  ng::DebugFeatures::renderTime = clock.getElapsedTime();
}

void EnggeApplication::onImGuiRender() {
  m_debugTools->render();
}

void EnggeApplication::onUpdate(const ngf::TimeSpan &elapsed) {
  if (!m_init) {
    m_engine->run();
    m_init = true;
  }
  ngf::StopWatch clock;
  m_engine->update(elapsed);
  ng::DebugFeatures::updateTime = clock.getElapsedTime();
}

void EnggeApplication::onQuit() {
  auto achievementsPath = ng::Locator<ng::EngineSettings>::get().getPath();
  achievementsPath.append("save.dat");
  ng::Locator<ng::AchievementManager>::get().save(achievementsPath);
  Application::onQuit();
}
}
