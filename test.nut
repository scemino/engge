include("Defines.nut")
include("Boot.nut")

local doStart = true

if(doStart) {
  start(true)
} else {
  inputOn()
  inputVerbs(ON)
  selectActor(ray)
  enterRoomFromDoor(highwayBridgeDoor)
}
