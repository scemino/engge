#pragma once
#include <optional>
#include <functional>
#include "ConsoleTools.hpp"
#include "TextureTools.hpp"
#include "DebugControls.hpp"
#include "ActorTools.hpp"
#include "ObjectTools.hpp"
#include "RoomTools.hpp"
#include "SoundTools.hpp"
#include "ThreadTools.hpp"
#include "GeneralTools.hpp"
#include "CameraTools.hpp"
#include "PreferencesTools.hpp"

namespace ng {
class Engine;

class DebugTools final {
public:
  explicit DebugTools(Engine &engine);

  void render();

private:
  struct Plot {
    float values[16];
    int offset{0};
  } m_renderTimes, m_updateTimes;

  void showPerformance();
  static void renderTimes(const char *label, Plot &plot, const std::function<ngf::TimeSpan()> &func);
  void showRoomTable();
  void showGlobalsTable();
  void showInputState();

public:
  bool visible{false};

private:
  Engine &m_engine;
  bool m_showRoomTable{false};
  bool m_showGlobalsTable{false};
  TextureTools m_texturesTools;
  ConsoleTools m_consoleTools;
  ActorTools m_actorTools;
  ObjectTools m_objectTools;
  RoomTools m_roomTools;
  SoundTools m_soundTools;
  ThreadTools m_threadTools;
  GeneralTools m_generalTools;
  CameraTools m_cameraTools;
  PreferencesTools m_preferencesTools;
};
} // namespace ng
