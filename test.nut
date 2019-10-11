include("Defines.nut")
include("Boot.nut")

local doStart = false

if(doStart) {
  start(true)
} else {
  actorSlotSelectable(3, YES)
  actorSlotSelectable(4, YES)
  actorSlotSelectable(5, YES)
  inputOn()
  inputVerbs(ON)
  selectActor(ray)
  // g.sheriff_counter=4
  // enterRoomFromDoor(cityHallMainStreetDoor)
  enterRoomFromDoor(highwayMainStreetDoor)
}
