#pragma once
#include <ngf/System/TimeSpan.h>
#include <engge/Parsers/Lip.hpp>

namespace ng {
class Actor;

class LipAnimation final {
public:
  void load(const std::string &path);

  void clear();
  void setActor(Actor *pActor);
  void update(const ngf::TimeSpan &elapsed);
  void end();
  [[nodiscard]] ngf::TimeSpan getDuration() const;

private:
  void updateHead();

private:
  Lip m_lip;
  Actor *m_pActor{nullptr};
  int m_index{0};
  ngf::TimeSpan m_elapsed;
};
}
