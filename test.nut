include("Defines.nut")
include("Boot.nut")

local doStart = false

if(doStart) {
  start(true)
} else {
  inputOn()
  inputVerbs(ON)
  selectActor(ray)
  enterRoomFromDoor(dinerMainStreetDoor)
}
