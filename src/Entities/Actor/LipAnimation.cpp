#include "LipAnimation.hpp"

namespace ng {
void LipAnimation::load(const std::string &path) {
  _lip.load(path);
  _index = 0;
  _elapsed = ngf::TimeSpan(0);
  updateHead();
}

void LipAnimation::clear() {
  _lip.clear();
}

void LipAnimation::setActor(Actor *pActor) {
  _pActor = pActor;
}

void LipAnimation::update(const ngf::TimeSpan &elapsed) {
  if (_lip.getData().empty())
    return;

  auto time = _lip.getData()[_index].time;
  _elapsed += elapsed;
  if (_elapsed > time && _index < static_cast<int>(_lip.getData().size())) {
    _index++;
  }
  if (_index == static_cast<int>(_lip.getData().size())) {
    end();
    return;
  }
  updateHead();
}

void LipAnimation::end() {
  if (!_pActor)
    return;
  _pActor->getCostume().setHeadIndex(0);
}

void LipAnimation::updateHead() {
  if (_lip.getData().empty() && _index >= static_cast<int>(_lip.getData().size()))
    return;
  auto letter = _lip.getData()[_index].letter;
  if (letter == 'X' || letter == 'G')
    letter = 'A';
  if (letter == 'H')
    letter = 'D';
  auto index = letter - 'A';
//    trace("lip: {} {}", _lip.getData()[_index].time.asSeconds(), _lip.getData()[_index].letter);
  // TODO: what is the correspondance between letter and head index ?
  _pActor->getCostume().setHeadIndex(index);
}

ngf::TimeSpan LipAnimation::getDuration() const {
  if (_lip.getData().empty())
    return ngf::TimeSpan(0);
  return _lip.getData().back().time;
}
}
