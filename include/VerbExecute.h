#pragma once

namespace ng
{
class Entity;
struct Verb;

class VerbExecute
{
public:
  virtual ~VerbExecute() = default;
  virtual void execute(const Verb *pVerb, Entity* pObject1, Entity* pObject2) = 0;
};
}