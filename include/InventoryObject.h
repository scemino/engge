#pragma once
#include <string>
#include <memory>
#include <squirrel3/squirrel.h>

namespace ng
{
class Room;
class InventoryObject
{
  public:
    void setRoom(const Room *pRoom) { _pRoom = pRoom; }
    const Room *getRoom() const { return _pRoom; }
    void setName(const std::string &name) { _name = name; }
    const std::string &getName() const { return _name; }
    void setIcon(const std::string &icon) { _icon = icon; }
    const std::string &getIcon() const { return _icon; }
    void setHandle(std::unique_ptr<HSQOBJECT> pHandle) { _pHandle = std::move(pHandle); }
    HSQOBJECT *getHandle() const { return _pHandle.get(); }

  private:
    const Room *_pRoom;
    std::string _icon, _name;
    std::unique_ptr<HSQOBJECT> _pHandle;
};
} // namespace ng
