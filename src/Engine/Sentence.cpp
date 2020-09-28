#include "engge/Engine/Sentence.hpp"

namespace ng {
Sentence &Sentence::push_back(std::unique_ptr<Function> func) {
  _functions.push_back(std::move(func));
  return *this;
}

void Sentence::stop() { _stopped = true; }

bool Sentence::isElapsed() { return _functions.empty(); }

void Sentence::operator()(const sf::Time &elapsed) {
  if (_functions.empty())
    return;
  if (_stopped) {
    _functions.clear();
    return;
  }
  if (_functions[0]->isElapsed()) {
    _functions.erase(_functions.begin());
  } else {
    (*_functions[0])(elapsed);
  }
}
} // namespace ng
