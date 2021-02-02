#pragma once
#include <ngf/System/TimeSpan.h>
#include "engge/Entities/Actor/Actor.hpp"
#include "engge/Parsers/Lip.hpp"

namespace ng {
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
  Lip _lip;
  Actor *_pActor{nullptr};
  int _index{0};
  ngf::TimeSpan _elapsed;
};
}
