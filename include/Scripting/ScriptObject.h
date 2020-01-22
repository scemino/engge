#pragma once

namespace ng
{
class ScriptObject
{
public:
  virtual ~ScriptObject() = default;

  [[nodiscard]] int getId() const { return _id; }

protected:
  int _id{0};
};
}