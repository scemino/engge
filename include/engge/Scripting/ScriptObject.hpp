#pragma once

namespace ng {
class ScriptObject {
public:
  virtual ~ScriptObject() = default;

  [[nodiscard]] int getId() const { return m_id; }

protected:
  int m_id{0};
};
}