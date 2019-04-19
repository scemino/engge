#pragma once

namespace ng
{
class InventoryObject;
class Object;
struct Verb;

class VerbExecute
{
public:
  virtual ~VerbExecute() = default;
  virtual void use(const InventoryObject *pObjectSource, Object *pObjectTarget) = 0;
  virtual void execute(Object *pObject, const Verb *pVerb) = 0;
  virtual void execute(const InventoryObject *pObject, const Verb *pVerb) = 0;
};
}