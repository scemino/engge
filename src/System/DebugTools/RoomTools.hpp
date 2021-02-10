#pragma once
#include <string>
#include <vector>

namespace ng {
class Engine;
class Room;

class RoomTools final {
public:
  explicit RoomTools(Engine &engine);

  void render();
  void updateWalkboxInfos(Room *pRoom);

public:
  bool m_showRoomTable{false};

private:
  Engine &m_engine;
  int m_selectedWalkbox{0};
  int m_fadeEffect{0};
  float m_fadeDuration{3.f};
  std::vector<std::string> m_walkboxInfos;
  std::vector<std::string> m_roomInfos;
};
}