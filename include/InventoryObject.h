#pragma once
#include <string>
#include <memory>
#include "squirrel.h"

namespace ng
{
class Room;
class InventoryObject
{
  public:
    void setRoom(const Room *pRoom) { _pRoom = pRoom; }
    const Room *getRoom() const { return _pRoom; }
    void setName(const std::wstring &name) { _name = name; }
    const std::wstring &getName() const { return _name; }
    void setIcon(const std::string &icon) { _icon = icon; }
    const std::string &getIcon() const { return _icon; }
    void setHandle(std::unique_ptr<HSQOBJECT> pHandle) { _pHandle = std::move(pHandle); }
    HSQOBJECT *getHandle() const { return _pHandle.get(); }

  private:
    const Room *_pRoom;
    std::string _icon;
    std::wstring _name;
    std::unique_ptr<HSQOBJECT> _pHandle;
};
} // namespace ng
