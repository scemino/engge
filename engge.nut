// if you want to start the game normally, remove or rename this file
include("Defines.nut")
include("Boot.nut")

actorSlotSelectable(3, YES)
actorSlotSelectable(4, YES)
actorSlotSelectable(5, YES)
inputOn()
inputVerbs(ON)
selectActor(ray)

enterRoomFromDoor(highwayMainStreetDoor)
