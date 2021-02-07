#pragma once
#include <imgui.h>
#include <utility>
#include <vector>
#include <engge/Entities/Entity.hpp>
#include <engge/Entities/Actor/Costume.hpp>

namespace ng {
class Actor;
class Engine;

class ActorTools final {
public:
  explicit ActorTools(Engine &engine);
  void render();

private:
  void showInventory(Actor *actor);
  void showGeneral(Actor *actor);
  void showCostume(Actor *actor);

private:
  static std::string getFlags(Actor &actor);
  static int facingToInt(Facing facing);
  static Facing intToFacing(int facing);
  static int directionToInt(UseDirection dir);
  static UseDirection intToDirection(int dir);

public:
  bool showActorTable{false};

private:
  Engine &m_engine;
  int m_selectedActor{0};
  ImGuiTextFilter m_filterCostume;
  std::vector<std::string> m_actorInfos;
};
}