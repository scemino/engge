#include "LipAnimation.hpp"
#include <engge/Entities/Actor.hpp>
#include <engge/Entities/Costume.hpp>

namespace ng {
void LipAnimation::load(const std::string &path) {
  m_lip.load(path);
  m_index = 0;
  m_elapsed = ngf::TimeSpan::seconds(0);
  updateHead();
}

void LipAnimation::clear() {
  m_lip.clear();
  m_index = 0;
}

void LipAnimation::setActor(Actor *pActor) {
  m_pActor = pActor;
}

void LipAnimation::update(const ngf::TimeSpan &elapsed) {
  if (m_lip.getData().empty() || m_index == static_cast<int>(m_lip.getData().size()))
    return;

  auto time = m_lip.getData().at(m_index).time;
  m_elapsed += elapsed;
  const auto lipSize = static_cast<int>(m_lip.getData().size());
  if ((m_elapsed > time) && (m_index < lipSize)) {
    m_index++;
  }
  if (m_index == static_cast<int>(m_lip.getData().size())) {
    end();
    return;
  }
  updateHead();
}

void LipAnimation::end() {
  if (!m_pActor)
    return;
  m_pActor->getCostume().setHeadIndex(0);
}

void LipAnimation::updateHead() {
  if (m_lip.getData().empty() && m_index >= static_cast<int>(m_lip.getData().size()))
    return;
  auto letter = m_lip.getData().at(m_index).letter;
  if (letter == 'X' || letter == 'G')
    letter = 'A';
  if (letter == 'H')
    letter = 'D';
  auto index = letter - 'A';
//    trace("lip: {} {}", _lip.getData().at(_index).time.asSeconds(), _lip.getData().at(_index).letter);
  // TODO: what is the correspondence between letter and head index ?
  m_pActor->getCostume().setHeadIndex(index);
}

ngf::TimeSpan LipAnimation::getDuration() const {
  if (m_lip.getData().empty())
    return ngf::TimeSpan(0);
  return m_lip.getData().back().time;
}
}
