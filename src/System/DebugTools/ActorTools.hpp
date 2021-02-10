#pragma once
#include <imgui.h>
#include <utility>
#include <vector>
#include <engge/Entities/Entity.hpp>
#include <engge/Entities/Costume.hpp>

namespace ng {
class Actor;
class Engine;

class ActorTools final {
public:
  explicit ActorTools(Engine &engine);
  void render();

private:
  void showInventory(Actor *actor);
  void showProperties(Actor *actor);
  void showCostume(Actor *actor);
  void showActorTable(Actor *actor);

private:
  static std::string getFlags(Actor &actor);
  static int facingToInt(Facing facing);
  static Facing intToFacing(int facing);
  static int directionToInt(UseDirection dir);
  static UseDirection intToDirection(int dir);

public:
  bool actorsVisible{false};

private:
  Engine &m_engine;
  Actor* m_selectedActor{nullptr};
  ImGuiTextFilter m_filterCostume;
  bool m_showActorTable{false};
  bool m_showInventory{false};
  bool m_showProperties{false};
  bool m_showCostume{false};
};
}