const talkColorBoris		= 0x3ea4b5
const talkColorWillie		= 0xc69c6d

// Defines.nut
const HANDLED 		= 1			// This should returned from functions that override system behavior.
const NOT_HANDLED 	= 0			// This should returned from functions that override system behavior.
const animStand = "stand"

// DefineSounds.nut
soundTowerHum <- defineSound("TowerHum.wav")
soundTowerLight <- defineSound("TowerLight.wav")
soundFenceLockRattle <- defineSound("FenceLockRattle.ogg")
soundCricketsLoop <- defineSound("AmbNightCrickets_Loop.ogg")	
soundGunshot <- defineSound("Gunshot.wav")
soundMetalClank <- defineSound("MetalClank.wav")
soundBridgeTrain <- defineSound("BridgeTrain.ogg")				
soundTowerLight <- defineSound("TowerLight.wav")
soundTowerLight2 <- defineSound("TowerLight2.wav")
soundWindBirds <- defineSound("WindBirds.ogg")
musicStartScreen <- defineSound("GenTown_StartScreen_LOOP.ogg")
musicBridgeA <- defineSound("Highway_Bridge_A.ogg")
musicBridgeB <- defineSound("Highway_Bridge_B.ogg")
musicBridgeC <- defineSound("Highway_Bridge_C.ogg")
musicBridgeD <- defineSound("Highway_Bridge_D.ogg")
musicBridgeE <- defineSound("Highway_Bridge_E.ogg")
soundDrip1 <- defineSound("Drip1.wav")					
soundDrip2 <- defineSound("Drip2.wav")					
soundDrip3 <- defineSound("Drip3.wav")		
soundBridgeAmbienceFrog1 <- defineSound("BridgeAmbienceFrog1.wav")
soundBridgeAmbienceFrog2 <- defineSound("BridgeAmbienceFrog2.wav")
soundBridgeAmbienceFrog3 <- defineSound("BridgeAmbienceFrog3.wav")
soundBridgeAmbienceFrog4 <- defineSound("BridgeAmbienceFrog4.wav")
soundGateSlidingOpen <- defineSound("GateSlidingOpen.wav")
soundGateSlidingClosed <- defineSound("GateSlidingClosed.wav")			
soundFootstepWater1 <- defineSound("FootstepWater1.wav")
soundFootstepWater2 <- defineSound("FootstepWater2.wav")
soundFootstepNormal1 <- defineSound("FootstepNormal1.wav")
soundFootstepNormal2 <- defineSound("FootstepNormal2.wav")
soundFootstepNormal3 <- defineSound("FootstepNormal3.wav")
soundTitleStinger1 <- defineSound("TitleCardStab1.ogg")
soundTitleStinger2 <- defineSound("TitleCardStab2.ogg")
soundTitleStinger3 <- defineSound("TitleCardStab3.ogg")
soundTitleStinger4 <- defineSound("TitleCardStab4.ogg")
soundPunchHit <- defineSound("PunchHit.wav")
soundScrape <- defineSound("Scrape.wav")
soundPolaroidFilmIntoCamera <-	defineSound("PolaroidFilmIntoCamera.wav")	
soundDrinkWhisky <- defineSound("DrinkWhisky.wav")	
soundBreakLightbulb <- defineSound("BreakLightbulb.wav")		
soundSpark1 <- defineSound("Spark1.wav")						
soundSpark2 <- defineSound("Spark2.wav")						

// Boot.nut
function objectName(obj, name) {
 if (!is_table(obj)) return name
 if (!is_string(name)) return "???"

 obj.name <- name
 return name
}

// DefineActors.nut
const defaultTextOffset = 90

function defineVerbs(slot) {
 setVerb(slot, 0, { verb = VERB_WALKTO, image = "walkto", func = "verbWalkTo", text = "@30011", key = getUserPref("keyWalkTo", "") })
 setVerb(slot, 1, { verb = VERB_OPEN, image = "open",  func = "verbOpen", text = "@30012", key = getUserPref("keyOpen", "@30013") })
 setVerb(slot, 2, { verb = VERB_CLOSE, image = "close",  func = "verbClose", text = "@30014", key = getUserPref("keyClose", "@30015") })
 setVerb(slot, 3, { verb = VERB_GIVE, image = "give",  func = "verbGive", text = "@30016", key = getUserPref("keyGiveTo", "@30017") })
 setVerb(slot, 4, { verb = VERB_PICKUP, image = "pickup",  func = "verbPickUp", text = "@30018", key = getUserPref("keyPickup", "@30019") })
 setVerb(slot, 5, { verb = VERB_LOOKAT, image = "lookat",  func = "verbLookAt", text = "@30020", key = getUserPref("keyLookAt", "@30021") })
 setVerb(slot, 6, { verb = VERB_TALKTO, image = "talkto",  func = "verbTalkTo", text = "@30022", key = getUserPref("keyTalkTo", "@30023") })
 setVerb(slot, 7, { verb = VERB_PUSH, image = "push",  func = "verbPush", text = "@30024", key = getUserPref("keyPush", "@30025") })
 setVerb(slot, 8, { verb = VERB_PULL, image = "pull",  func = "verbPull", text = "@30026", key = getUserPref("keyPull", "@30027") })
 setVerb(slot, 9, { verb = VERB_USE, image = "use",  func = "verbUse", text = "@30028", key = getUserPref("keyUse", "@30029") })
}

// Boot.nut
settings <- { 
    playFootsteps = YES
    actorIdleAnimations = YES
    idleTime = 3*60
    demo = NO	
    demoQP = NO
    selectActorHintTime = 60
    preloadMusic = NO
    ambianceSounds = YES
    starTwinkleRate = 1.0
    isRon = NO
    showMusicInfo = YES
    showAmbianceInfo = YES
    toilet_paper_over = YES
}

// Inventory.nut
Inventory <-
{
    background = "Inventory"

 enter = function()
 {
 }

 exit = function()
 {
 }

camera =
 {
 icon = "polaroid_camera"
 name = objectName(this, "@28131")
 flags = USE_ON
 noAgentGiveText = "@28132"
 throwawayable = "@28133"
 filmCounter = 0
 verbLookAt = function()
 {
 if (filmCounter) {
 sayLine("@28134")
 } else {
 sayLine("@28135")
 }
 }
 verbUse = function(obj=null)
 {
 if (obj == polaroidFilm) {
 if (obj.rawin("verbUse")) { obj.verbUse(this) } else { defaultObject.verbUse(obj,this) }
 } else 
 if (filmCounter) {
 noReach()
 sayLine("@28136")
 } else {
 noReach()
 sayLine("@28132")
 }
 }
 verbOpen = function()
 {
 if (filmCounter) {
 sayLine("@28137")
 } else {
 sayLine("@28138")
 }
 }
 verbClose = function()
 {
 sayLineAlreadyClosed(this)
 }
 objectPreWalk = function(verb, obj1, obj2) {
 
 if (verb == VERB_USE && obj2 == Bridge.bridgeBody && filmCounter) {
 filmCounter--
 stopSentence(currentActor)
 startthread(Bridge.takePhoto)
 return HANDLED
 }
 return NOT_HANDLED
 }
 }

polaroidFilm =
 {
 icon = "polaroid_film"
 name = objectName(this, "@28139")
 flags = USE_IN
 verbLookAt = function()
 {
 sayLine(agent, "@28140",
 "@28141")
 }
 verbUse = function(obj=null)
 {
 if (obj == camera) {
 removeInventory(this)
 popInventory(camera)
 camera.filmCounter = 1
 playSound(soundPolaroidFilmIntoCamera)
 } else {
 noReach()
 sayLine(agent, "@28142")
 }
 }
 verbOpen = function()
 {
 sayLine(agent, "@28143")
 }
 }

 reyesNotebook =
 {
 icon = "notebook"
 inventory_slot = 2
 name = objectName(this, "@28110")
 function onPickUp(actor) {
 name = (actor == reyes) ? "@28110" : "@28111"
 }
 verbLookAt = function()
 {
 if (isReyes()) {
 cameraInRoom(Notebook)
 Notebook.loadNotebookFor(reyes)
 } else 
 if (isRay()) {
 sayLine(ray, "@28112","@28113", "@28114")
 } else 
 if (isRansome()) {
 sayLine(ransome, "@28115","@28116")
 } else {
 sayLine("@28117","@28118")
 }
 }
 verbUse = function(obj=null)
 {
 verbLookAt()
 }
 verbOpen = function()
 {
 verbLookAt()			
 }
 verbClose = function()
 {
 sayLineAlreadyClosed(this)
 }

 objectPreWalk = function(verb, obj1, obj2) {
 
 if (verb == VERB_GIVE) {
 if (obj2 == reyes) {
 return NOT_HANDLED
 } else 
 if (isRay()) {
 sayLine(ray, "@28119")
 } else {
 sayLine(reyes, "@28120")
 }
 return HANDLED
 }
 return NOT_HANDLED
 }
 }
}
defineRoom(Inventory)

// Helpers.nut
function footstepsNormal(actor) {
 if (settings.playFootsteps) {
 if (getroottable().rawin("ransome")		 && actor == ransome) {
 actorSound(actor, 1, soundRansomeFootstep1, soundRansomeFootstep2, soundRansomeFootstep3, soundRansomeFootstep4)
 } else
 if (getroottable().rawin("pigeon1")		 && actor == pigeon1) {
 actorSound(actor, 1, 0)		
 } else {
 actorSound(actor, 1, soundFootstepNormal1, soundFootstepNormal2, soundFootstepNormal3)
 }
 }
 actorHideLayer(actor, "splash")
}

function footstepsWater(actor) {
    print("footstepsWater "+actor+"\n")
 if (settings.playFootsteps) {
 actorSound(actor, 1, soundFootstepWater1, soundFootstepWater2)
 }
 actorShowLayer(actor, "splash")
}

function doOpening() {

 cameraInRoom(TitleCards)
 roomFade(FADE_IN, 2.0)

 local text = createTextObject("sayline", translate("@25545"), ALIGN_CENTER | 1000)
 objectScale(text, 0.5)
 objectColor(text, 0x30AAFF)
 objectAlpha(text, 0.0)
 objectAt(text, 320,180)
 
 objectAlphaTo(text, 1.0, 1.0, LINEAR)
 breaktime(3.0)
 objectAlphaTo(text, 0.0, 1.0, LINEAR)
 breaktime(1.0)
 
 deleteObject(text)

 text = createTextObject("sayline", translate("@25546"), ALIGN_CENTER | 1000)
 objectScale(text, 0.5)
 objectColor(text, 0x30AAFF)
 objectAlpha(text, 0.0)
 objectAt(text, 320,180)

 objectAlphaTo(text, 1.0, 1.0, LINEAR)
 breaktime(4.0)
 objectAlphaTo(text, 0.0, 1.0, LINEAR)
 breaktime(1.0)

//  breakwhilerunning(TitleCards.showPartBody())

























//  startMusic(musicTempTheme, bridgeMusicPool)

 selectActor(ray)		
 enterRoomFromDoor(Bridge.startRight)
 actorAt(ray, Bridge.startLeft)
 actorAt(reyes, Bridge.startRight)
 cameraAt(Bridge.bridgeBody)
 roomOverlayColor(0xFF000000, 0x00000000, 5.0)
 startDialog("Opening")
 breakwhiledialog()
 if (randomOdds(0.5)) {
 pickupObject(Inventory.polaroidFilm, reyes)
 pickupObject(Inventory.camera, ray)
 selectActor(ray)
 cameraFollow(ray)
 } else {
 pickupObject(Inventory.polaroidFilm, ray)
 pickupObject(Inventory.camera, reyes)
 selectActor(reyes)
 cameraFollow(reyes)
 }
 actorSlotSelectable(ray, ON)
 actorSlotSelectable(reyes, ON)
 actorSlotSelectable(ON)
//  Bridge.flashSelectActorIcon(1)
 breakhere(1)
//  exCommand(EX_AUTOSAVE)
 Bridge.speck_of_dust <- YES	
}

boris <- {
    _key = "boris"
    name = "@30072"
    fullname = "@30073"
    icon = "icon_boris"
    detective = NO
    function showHideLayers() {
        actorHideLayer(boris, "splash")
    }
    function defaultClickedAt(x, y) {	
    }
}
createActor(boris)
actorRenderOffset(boris, 0, 45)
defineVerbs(6)
verbUIColors(6, {	nameid = "boris", sentence = 0xffffff, 
 verbNormal = 0x3ea4b5, verbHighlight = 0x4fd0e6,
 verbNormalTint = 0x4ebbb5, verbHighlightTint = 0x96ece0, 
 inventoryFrame = 0x009fdb, inventoryBackground = 0x002432 })


function borisCostume()
{
 actorCostume(boris, "BorisAnimation")
 actorWalkSpeed(boris, 30, 15)
 actorRenderOffset(boris, 0, 45)
 actorTalkColors(boris, talkColorBoris)
actorTalkOffset(boris, 0, defaultTextOffset)
 actorHidden(boris, OFF)
//  objectLit(boris, 1)
 footstepsNormal(boris)
 boris.showHideLayers()
}
borisCostume()

currentActor <- boris
 function isBoris(actor = null) {
 return ((actor?actor:currentActor) == boris)
return true
}

function noReach(actor = null) {
 if (actor == null) actor = currentActor
//  if (!actorWalking(actor) && !actor.onLadder) actorStand(actor)	
}

// soundhelpers.nut
const AMBIENT_VOLUME = 0.25

_soundAmbianceSID <- 0
_soundAmbianceArraySID <- []	
_soundAmbianceTID <- 0
_soundAmbianceBedVolume <- 0
_soundAmbianceSfxVolume <- 0

function watchAmbianceSounds(time, sound_array) {
 local sid
 if (sound_array == null) return
 do {
 if (time >= 0) {
 breaktime(random(time*0.5, time*1.5))
 } else {
 breakwhilesound(sid)
 breaktime(random(-time*0.5, -time*1.5))
 }
 local sound = randomfrom(sound_array)
 if (sound) {
 sid = playSound(sound)
 soundVolume(sid, _soundAmbianceSfxVolume)
 if (settings.showAmbianceInfo) ""
 if (sid) {
 _soundAmbianceArraySID.push(sid)
 }
 }	
 }while(1)
}

function startSoundAmbiance(sound, bed_volume = AMBIENT_VOLUME, sfx_volume = 1.0, time = 0, sound_array = null) {
 if (settings.ambianceSounds == NO) return
 _soundAmbianceBedVolume = bed_volume
 _soundAmbianceSfxVolume = sfx_volume
 if (_soundAmbianceSID) {
 fadeOutSound(_soundAmbianceSID, 0.25)
 }
 foreach (sid in _soundAmbianceArraySID) {
 fadeOutSound(sid, 0.25)
 }
 _soundAmbianceArraySID = []
 _soundAmbianceTID = stopthread(_soundAmbianceTID)
 _soundAmbianceSID = loopSound(sound)
 soundVolume(_soundAmbianceSID, _soundAmbianceBedVolume)
 if (settings.showAmbianceInfo) ""
 if (time && sound_array) {
 //TODO: _soundAmbianceTID = startglobalthread(watchAmbianceSounds, time, sound_array)
 threadpauseable(_soundAmbianceTID, NO)
 }
}

function startSoundBridgeAmbiance() {
 startSoundAmbiance(soundCricketsLoop, AMBIENT_VOLUME, 1.0, 5.0, 
 [ soundBridgeAmbienceFrog1, soundBridgeAmbienceFrog2, soundBridgeAmbienceFrog3, soundBridgeAmbienceFrog4 ])
}


function actorBlinks(actor, state) {
 if (state) {
 actorBlinkRate(actor, 2.0,5.0)
 } else {
 actorBlinkRate(actor, 0.0,0.0)
 }
}

function bounceImage() {
    local image = createObject("RaySheet", ["bstand_body1"]);
    local x = random(0, 1280);
    local y = random(0, 720);
    scale(image, 0.5);

    // objectAlpha(image, 0.0, 2);

    do {
        local steps = random(100, 150);

        local end_x = random(0, 320);
        local end_y = random(0, 180);

        local dx = (end_x - x) / steps;
        local dy = (end_y - y) / steps;

        for (local i = 0; i < steps; i++) {
            x += dx;
            y += dy;
            image.at(x, y);
            breaktime(0.01);
        }
    } while (1);
}

function twinkleStar(obj, fadeRange1, fadeRange2, objectAlphaRange1, objectAlphaRange2) {
    local timeOff, timeOn, fadeIn, fadeOut
    objectAlpha(obj, randomfrom(objectAlphaRange1, objectAlphaRange2))
    if (randomOdds(1.0)) {
        do {
            fadeIn = random(fadeRange1, fadeRange2)
            objectAlpha(obj, objectAlphaRange2, fadeIn)
            breaktime(fadeIn)
            fadeOut = random(fadeRange1, fadeRange2)
            objectAlpha(obj, objectAlphaRange1, fadeOut)
            breaktime(fadeOut)
        } while (1)
    }
}

function flashAlphaObject(obj, offRange1, offRange2, onRange1, onRange2, fadeRange1, fadeRange2, maxFade = 1.0, minFade = 0.0) {
 local timeOff, timeOn, fadeIn, fadeOut
 objectAlpha(obj, randomfrom(0.0, 1.0))
 do {
 timeOff = random(offRange1, offRange2)
 breaktime(timeOff)
 fadeIn = random(fadeRange1, fadeRange2)
 objectAlphaTo(obj, maxFade, fadeIn)
 breaktime(fadeIn)
 timeOn = random(onRange1, onRange2)
 breaktime(timeOn)
 fadeOut = random(fadeRange1, fadeRange2)
 objectAlphaTo(obj, minFade, fadeOut)
 breaktime(fadeOut)
 } while(1)
}

function animateFirefly(obj) {
 startthread(flashAlphaObject, obj, 1, 4, 0.5, 2, 0.1, 0.35)
 }

function createFirefly(x) {
 local firefly = 0
 local zsort = 68
 local y = random(78,168)
 local direction = randomfrom(-360,360)
 if (y < 108) {
 firefly = createObject("firefly_large")
 zsort = random(68,78)
 } else
 if (y < 218) {
 firefly = createObject("firefly_small")
 zsort = 117
 } else
 if (x > 628 && x < 874) {		
 firefly = createObject("firefly_tiny")
 zsort = 668
 }
 if (firefly) {
 objectRotateTo(firefly, direction, 12, LOOPING)
 objectAt(firefly, x, y)
 objectSort(firefly, zsort)
 return firefly
 }
 }

Bridge <- 
{
 name = "Bridge"
 background = "Bridge"
 speck_of_dust = YES, speck_of_dust_collected = NO

 no_cell_reception = YES

 _lightObject1 = 0

 _returnEntryNewOpening = NO
 _watchForIdleTID = 0
 _watchSwitchIconTID = 0
 _willieSnoringTID = 0
 _spookySoundSID = 0

 selectActorHint = NO

 agent_takes_picture = 0
 otheragent_save_x = 0
 otheragent_save_y = 0
 otheragent_save_dir = 0

 function willieSnoring() {
 //TODO: Bridge._willieSnoringTID = threadid()
 g.willie_sleeping = YES
 actorAnimationNames(willie, { stand = "asleep", walk = "asleep", reach = "asleep" })
 actorPlayAnimation(willie, "asleep")
 actorVolume(willie, 0.25)		
 do {
    if (isActorOnScreen(willie)) 
    {
    breakwhiletalking(willie)
    breakwhiletalking(currentActor)
    mumbleLine(willie, "@24839")
    breakwhiletalking(willie)
    }
    breaktime(4)
 }while(1)
 }

 function openGate() {
//  Tutorial.stopHints(1)
 inputOff()
 Bridge.bridgeGate.gate_opening = YES
 objectOffsetTo(Bridge.bridgeGate, -60, 0, 2.0, EASE_INOUT)
 objectTouchable(Bridge.bridgeGate, NO)
 objectOffsetTo(Bridge.bridgeGateBack, -60, 0, 2.0, EASE_INOUT)
 objectTouchable(Bridge.bridgeGateBack, NO)
 playObjectSound(soundGateSlidingOpen, Bridge.bridgeGate)
 Bridge.bridgeGate.gate_state = OPEN
 breaktime(1)
 walkboxHidden("gate", NO)
 Bridge.bridgeGate.gate_opening = NO
 inputOn()
 breaktime(1)
 objectTouchable(Bridge.bridgeGate, YES)

//  Tutorial.completeHint(1)
 }

 function closeGate() {
     print("closeGate\n")
 if (Bridge.bridgeGate.gate_state == CLOSED) {
     print("closed\n")
 return	
 }
 Bridge.bridgeGate.gate_closing = YES
 Bridge.bridgeGate.gate_state = CLOSED
 walkboxHidden("gate", YES)
 playObjectSound(soundGateSlidingClosed, Bridge.bridgeGate)
 objectOffsetTo(Bridge.bridgeGate, 0, 0, 2.0, EASE_INOUT)
 objectTouchable(Bridge.bridgeGate, NO)
 objectOffsetTo(Bridge.bridgeGateBack, 0, 0, 2.0, EASE_INOUT)
 objectTouchable(Bridge.bridgeGateBack, NO)
 breaktime(2.0)
 Bridge.bridgeGate.gate_closing = NO
 objectTouchable(Bridge.bridgeGate, YES)

 }

 function trigCloseGate()
 {
     startthread(Bridge.closeGate)
     removeTrigger(Bridge.triggerCloseGate);
 }

 startLeft = { name = objectName(this, "@25553") }
 startRight = { name = objectName(this, "@25554") }

borisWallet =
{
 icon = "boris_wallet"
 name = objectName(this, "@25555")
 verbLookAt = function()
 {
 sayLine(boris, "@25556")
 }
 verbOpen = function()
 {
 sayLine(boris, "@25557")
 }
 verbUse = function(obj=null)
 {
 verbOpen()
 }
 verbClose = function()
 {
 sayLine(boris, "@25558")
 }
 verbGive = function(actor)
 {
 sayLine(boris, "@25559")
 }
}

function useRock(obj=null){
inputOff()
if (obj == bridgeTownSign) {
actorWalkTo(currentActor, bridgeLight)
breakwhilewalking(currentActor)
}
actorPlayAnimation(currentActor, "break_light")
breaktime(1.0/12.0*30)	
objectState(bridgeLight, OFF)
breaktime(0.5)
breakwhileanimating(currentActor)
breaktime(0.5)
removeTrigger(Bridge.triggerUnderbrush)

// Notes.checkBorisNotes(3)
inputOn()
sayLine(boris, "@25569",
"@25570")
}

rock =
{
 icon = "rock"
 name = objectName(this, "@25567")
 flags = USE_ON
 verbLookAt = function()
 {
  sayLine(boris, "@25568")
 }
 verbUse = function(obj=null)
 {
 if (obj == bridgeLight || obj == bridgeTownSign) {
 if (objectState(bridgeLight) == ON) {
//  Tutorial.completeHint(6)
 startthread(useRock, obj)					
 } else {
 noReach()
 sayLine(boris, "@25571")
 }
 } else
 if (obj == borisNote) {
 noReach()
 sayLine(boris, "@25572")
 } else
 if (obj == borisPrototypeToy) {
 noReach()
 sayLine(boris, "@25573")
 } else {
 noReach()
 sayLine(boris, "@25574")
 }
 }
 verbGive = function(actor)
 {
 sayLine(boris, "@25575")
 }
}

borisNote =
{
 icon = "safe_combination_note"
 name = objectName(this, "@25345")
}
borisHotelKeycard =
{
 icon = "key_card_mauve"
 name = objectName(this, "@25578")
}
bridgeGrateEntryDoor =
 {
 name = objectName(this, "@25661")
 initTouchable = NO
 }
bridgeGrateTree =
 {
    name = objectName(this, "@25668")
 }
  bridgeStump =
 {
 name = objectName(this, "@25671")
 initTouchable = NO
 }
 bridgeBottle =
 {
 name = objectName(this, "@25675")
 }
 bridgeFireflyTiny = 
 {
 initState = GONE
 }
 bridgeFireflySmall = 
 {
 initState = GONE
 }
 bridgeFireflyLarge = 
 {
 initState = GONE
 }
 bridgeHighwayDoor = 
 {
 name = objectName(this, "@25625")
 }
 bridgeGate =
 {
    gate_state = CLOSED
    gate_opening = NO
    gate_closing = NO
    name = objectName(this, "@25207")
    defaultVerb = VERB_OPEN
    verbLookAt = function()
    {
        if (g.openingScene == 1) {
            sayLine(boris, "@25677",
            "@25678")
        } else {
            sayLine("@25679")
        }
    }
    verbOpen = function() 
    {
        // if (g.taken_photo || incutscene()) {
        if (g.taken_photo) {
            if (gate_state == OPEN) {
                //  if (isBoris()) {
                sayLine(boris, "@25680")
                //  } else {
                //  sayLineAlreadyOpen(this)
                //  }
            } else {
                startthread(openGate)
                defaultVerb = VERB_CLOSE
            }
        }
    }
    verbClose = function() 
    {
        if (gate_state == CLOSED && gate_closing == NO) {
            noReach()
            if (isBoris()) {
                sayLine(boris, "@25681")
            } else {
                sayLineAlreadyClosed(this)
            }
        } else {
            startthread(closeGate)
            defaultVerb = VERB_OPEN
        }
    }
    verbPush = function()
    {
        if (gate_state == CLOSED) {
        verbOpen()
        } else {
        verbClose()
        }
    }
    verbPull = function()
    {
        verbPush()
    }
    verbUse = function(obj=null)
    {
        verbPush()
    }
    objectPreWalk = function(verb, obj1, obj2) {
        if (verb == VERB_OPEN || verb == VERB_PUSH || verb == VERB_PULL || verb == VERB_USE) {
            // if (isTesterTronRunning()) return NOT_HANDLED
            if (g.openingScene == 1) {
                if (gate_state == OPEN) {
                    sayLine(boris, "@25682")
                } else {
                    sayLine(boris, "@25683")
                }
                return HANDLED
            } else 
                if (!g.taken_photo) {
                    sayLine("@25684")
                return HANDLED
            }
        }
        return NOT_HANDLED
    }
 }
 bridgeGateBack =
 {
    name = objectName(this, "@25207")
    defaultVerb = VERB_OPEN
    verbLookAt = function()
    {
        if (isBoris()) {
        sayLine(boris, "@25685")
        } else {
        sayLine("@25686")
        }
    }
    verbOpen = function() 
    {
        startthread(openGate)
        defaultVerb = VERB_CLOSE
    }
    verbClose = function() 
    {
        if (bridgeGate.gate_state == CLOSED) {
            if (isBoris()) {
                sayLine(boris, "@25681")
                } else {
                sayLineAlreadyClosed(this)
            }
        } else {
            startthread(closeGate)
            defaultVerb = VERB_OPEN
        }
    }
 verbPush = function()
 {
 if (bridgeGate.gate_state == CLOSED) {
 bridgeGateBack.verbOpen()
 } else {
 bridgeGateBack.verbClose()
 }
 }
 verbPull = function()
 {
 verbPush()
 }
 verbUse = function(obj=null)
 {
 verbPush()
 }
 }
bridgeChainsaw = 
 {
 name = objectName(this, "@25687")
 }
 bridgeLight =
 {
   name = "@25694"
   initState = ON
   count = 0
    verbLookAt = function()
    {
        if (objectState(this) == ON) {
            if (g.openingScene == 1 && borisNote.readNote) {
                switch(++count) {
                case 1:
                sayLine(boris, "@25695")
                break;
                case 2:
                sayLine(boris, "@25696")
                break;
                case 3:
                sayLine(boris, "@25697")
                break;
                default:
                sayLine(boris, "@25698")
                }
                } else {
                if (isBoris()) {
                sayLine(boris, "@25699")
                } else {
                sayLine("@25704")
                }
            }				
        } else {
            if (isBoris()) {
            sayLine(boris, "@25700")
            } else {
            sayLine("@25701")
            }
        }
    }
    verbUse = function(obj=null)
    {
        noReach()
        if (g.openingScene == 1) {
        if (objectState(this) == ON) {
        sayLine(boris, "@25702")
        } else {
        sayLine(boris, "@25703")
        }
        } else {
        if (objectState(this) == ON) {
        sayLine("@25704")
        } else {
        sayLine("@25705")
        }
        }
    }
    verbOpen = function()
    {
        noReach()
        if (isBoris()) {
        sayLine(boris, "@25706")
        } else {
        sayLineCantOpen(this)
        }
    }
    verbClose = function()
    {
        verbUse()
    }
    verbPull = function()
    {
        if (g.openingScene == 1) {
            if (objectState(this) == ON) {
                sayLine(boris, "@30520")
            } else {
                verbUse()
            }
        } else {
            verbUse()
        }
    }
    verbPush = function()
    {
        verbPull()
    }
 }
bridgeRock =
 {
 name = objectName(this, "@25567")
 verbLookAt = function()
 {
 sayLine(boris, "@40125")
 }
 function bridgeRockPickUp()
 {
 inputSilentOff()
 breaktime(0.1)
 pickupObject(rock)
 objectState(this, GONE)
 breaktime(0.1)
 actorPlayAnimation(currentActor, "pocket_rock")
 breakwhileanimating(currentActor)
 if (g.easy_mode) {
 sayLine(boris, "@40125")
 }
 inputOn()
 }
 verbPickUp = function()
 {
//  Tutorial.completeHint(5)
 startthread(bridgeRockPickUp)
 }
 verbPush = function()
 {
 sayLine(boris, "@25707")
 }
 verbPull = function()
 {
 verbPickUp()
 }
 verbOpen = function()
 {
 sayLine(boris, "@25708")
 }
 verbClose = function()
 {
 sayLine(boris, "@25709")
 }
 }
 bridgeDragMark =
 {
   name = "@25710"
   initState = GONE
 }
 reedsLeft = { name = "" }
 reedsRight = { name = "" }
 willieObject =
 {
 name = objectName(this, "@30087")
 verbWalkTo = function()
 {
 }
 verbDefault = function()
 {
 noReach()
 startDialog("WillieBorisDialog")
 }
 }
bridgeTownSign = 
{
    name = objectName(this, "@25147")
}
borisPrototypeToy =
{
 icon = "prototype_toy_orange"
 name = objectName(this, "@25560")
 verbLookAt = function()
 {
 sayLine(boris, "@25561",
 "@25562")
 }
 verbOpen = function()
 {
 sayLine(boris, "@25563")
 }
 verbClose = function()
 {
 sayLine(boris, "@25564")
 }
 verbPull = function()
 {
 verbOpen()
 }
 verbPush = function()
 {
 verbOpen()
 }
 verbUse = function(obj=null)
 {
 sayLine(boris, "@25565")
 }
 verbGive = function(actor)
 {
 sayLine(boris, "@25566")
 }
}

 trainPassby = function() {
   objectOffset(Bridge.bridgeTrain, -100, 0)
   objectOffsetTo(Bridge.bridgeTrain, 2000, 0, 10, LINEAR)
   playSound(soundBridgeTrain)
 }

function waitUntilLightOut(actor) {
 inputOff()
 actorStopWalking(currentActor)
 actorWalkTo(currentActor, Bridge.borisRefuseSpot)
 sayLine(boris, "@25552")
 breakwhilewalking(currentActor)
 inputOn()
 }
 
 attackBoris = function() {
//  Tutorial.completeHint(8)
//  cutscene( 
//  @()
//  {
 
 actorWalkTo(boris, Bridge.attackSpot)
 stopthread(Bridge._willieSnoringTID)	
//  breakwhilewalking(boris)
 local shadowAttacker = createObject("AlleywaySheet", [ "shadow_attacking_1", "shadow_attacking_2", "shadow_attacking_3", "shadow_attacking_4", "shadow_attacking_5", "shadow_attacking_6", "shadow_attacking_5", "shadow_attacking_4", "shadow_attacking_3", "shadow_attacking_2", "shadow_attacking_1" ])
 objectAt(shadowAttacker, objectPosX(Bridge.attackerSpot), objectPosY(Bridge.attackerSpot))
 objectHidden(shadowAttacker, NO)
 playObjectState(shadowAttacker, 0)
 objectAlpha(shadowAttacker, 1.0)
 objectAlphaTo(shadowAttacker, 0.0, 2.0, SLOW_EASE_OUT)
 actorFace(boris, FACE_RIGHT)
 stopTalking(boris)
 breakhere(4)
 actorPlayAnimation(boris, "get_hit")
 // TODO: screenShake()
 playSound(soundPunchHit)
//  TODO: stopMusic()
 breakwhileanimating(boris)
 breaktime(1.0)
 objectState(Bridge.bridgeDragMark, HERE)
 objectAlpha(Bridge.bridgeDragMark, 0)
 objectAlphaTo(Bridge.bridgeDragMark, 1, 2)
 playSoundVolume(soundScrape, 1.0)
 objectOffsetTo(boris, -10, 0, 0.5)
 breaktime(1.0)
 playSoundVolume(soundScrape, 0.75)
 objectOffsetTo(boris, -25, 0, 0.5)
 breaktime(1.0)
 roomFade(FADE_OUT, 4.0)
 playSoundVolume(soundScrape, 0.50)
 objectOffsetTo(boris, -40, 0, 0.5)
 breaktime(1.0)
 playSoundVolume(soundScrape, 0.25)
 objectOffsetTo(boris, -50, 0, 0.5)
 breaktime(3)
 objectState(Bridge.bridgeRock, GONE)
 objectState(Bridge.bridgeBody, HERE)
 objectTouchable(Bridge.bridgeBody, YES)
 objectState(Bridge.bridgeBottle, HERE)
 objectTouchable(Bridge.bridgeBottle, YES)
 if ((g.easy_mode == NO)) {
 objectState(Bridge.bridgeChainsaw, HERE)
 objectTouchable(Bridge.bridgeChainsaw, YES)
 }
 // TODO: actorAt(willie, Void)
 // TODO: actorAt(boris, Void)
 objectState(Bridge.willieObject, GONE)
 // TODO: removeInventory(boris)
 removeTrigger(Bridge.triggerAttack)
 g.openingScene = NO
 actorSlotSelectable(ON)
 doOpening()
//  })
 }

 enter = function(door)
 {
     print("enter\n")
     print("bridgeGrateTree: "+Bridge.bridgeGrateTree+"\n")
     print("state: "+objectState(Bridge.bridgeGrateTree)+"\n")
 // TODO: setMapLocation( CountyMap.mapBridge )
 // TODO: if (!settings.demo) {
 // TODO: musicBridge()
 // TODO: }
 
 // TODO: setAmbientLight(0x999999);	
 
 // TODO: _lightObject1 = lightSetUp(0xAAAAAA, 719, 43, 0.8, 0, 210, 0.7, 200, 0.85, null, null)
 if (g.openingScene == 1) {
walkboxHidden("body", NO)
 addTrigger(Bridge.triggerCloseGate, trigCloseGate)
 objectTouchable(bridgeHighwayDoorOpening, YES)
 williePassedOutCostume()
 actorVolume(willie, 1.0)
 objectState(Bridge.willieObject, HERE)
 objectTouchable(Bridge.willieObject, YES)
 addTrigger(Bridge.triggerAttack, Bridge.attackBoris)
 addTrigger(Bridge.triggerUnderbrush, Bridge.waitUntilLightOut)
 if (g.willie_sleeping) {	
 startthread(Bridge.willieSnoring)
 actorPlayAnimation(willie, "asleep")
 actorAnimationNames(willie, { stand = "asleep", walk = "asleep", reach = "asleep" })
 } else {
 actorPlayAnimation(willie, "awake")
 }
 if (_returnEntryNewOpening) {



 }
 _returnEntryNewOpening = YES
 } else {
 objectTouchable(bridgeHighwayDoorOpening, NO)
 objectTouchable(bridgeGateBack, NO)
 walkboxHidden("body", YES)
 objectState(bridgeRock, GONE)
 objectState(bridgeBody, HERE)
 objectTouchable(bridgeBody, YES)
 objectState(bridgeDragMark, HERE)
 objectTouchable(bridgeDragMark, YES)
 objectState(Bridge.willieObject, GONE)
 }
 objectSort(Bridge.bridgeStump, 86)									
 if ((g.easy_mode == YES)) {
 objectState(bridgeChainsaw, GONE)
 objectOffset(Bridge.bridgeGrateTree, 27, -34)
 objectState(Bridge.bridgeGrateTree, ON)
 objectState(Bridge.bridgeGrateTree, ON)
 objectTouchable(Bridge.bridgeGrateTree, NO)
 objectTouchable(Bridge.bridgeStump, YES)
 objectTouchable(Bridge.bridgeGrateEntryDoor, YES)
 }

 if (!g.in_video_flashback) {
 startSoundBridgeAmbiance()
 // TODO: loopObjectSound(soundBridgeAmbienceWaterLoop, Bridge.triggerWater, -1, 0.25)
 }
 for (local x = 0; x < 960; x += random(20, 40)) {		
 local firefly = createFirefly(x)
 if (firefly) {
 startthread(animateFirefly, firefly)
 }		
 }
 for (local x = 1150; x < 2140; x += random(30, 50)) {		
 local firefly = createFirefly(x)
 if (firefly) {
 startthread(animateFirefly, firefly)
 }		
 }

objectTouchable(Bridge.bridgeGate, YES)
 Bridge.bridgeGate.gate_opening = NO
 if (Bridge.bridgeGate.gate_state == OPEN) {
walkboxHidden("gate", NO)
 objectOffset(Bridge.bridgeGate, -60, 0)
 objectOffset(Bridge.bridgeGateBack, -60, 0)
 } else {
walkboxHidden("gate", YES)
 objectOffset(Bridge.bridgeGate, 0, 0)
 objectOffset(Bridge.bridgeGateBack, 0, 0)
 }
 if (door == bridgeHighwayDoor) {
 actorWalkTo(currentActor, Bridge.pathSpot)
 }
 if (objectState(Bridge.bridgeGrateTree) == ON) {		
 walkboxHidden("fallen_tree", YES)
  objectSort(Bridge.bridgeGrateTree, 73)
  objectOffset(Bridge.bridgeGrateTree, 27, -34)
  } else {																						
  walkboxHidden("fallen_tree", NO)
  objectSort(Bridge.bridgeGrateTree, 123)
  objectOffset(Bridge.bridgeGrateTree, 0, 0)
  }

 // TODO: addTrigger(Bridge.triggerSound1, @() { Bridge.spookySound(Bridge.triggerSound1, randomfrom(soundOwls, soundBridgeAmbienceOwlHoot, soundBridgeAmbienceOwlHoot, soundBridgeAmbienceOwlHoot), 5.0) })
 // TODO: addTrigger(Bridge.triggerSound4, @() { Bridge.spookySound(Bridge.triggerSound4, randomfrom(soundWolf, soundBridgeAmbienceDogHowl1, soundBridgeAmbienceDogHowl2, soundBridgeAmbienceDogHowl3, soundBridgeAmbienceDogHowl4), 5.0) })

 addTrigger(triggerWater, footstepsWater, footstepsNormal)
 
 objectHidden(bridgeEyes, YES)
 objectParallaxLayer(bridgeWater, 1)
 loopObjectState(bridgeWater, 0)
 loopObjectState(bridgeShoreline, 0)
 actorSound(bridgeSewerDrip, 2, soundDrip1, soundDrip2, soundDrip3)
 loopObjectState(bridgeSewerDrip, 0)
 objectParallaxLayer(bridgeTrain, 2)

 // TODO: objectShader(reedsLeft, YES, GRASS_BACKANDFORTH, 3, 0.5, YES)
 // TODO: objectShader(reedsRight, YES, GRASS_BACKANDFORTH, 3, 0.5, YES)

 objectParallaxLayer(frontWavingReeds1, -2)
 // TODO: objectShader(frontWavingReeds1, YES, GRASS_BACKANDFORTH, 5, 1, YES)
 objectParallaxLayer(frontWavingReeds2, -2)
 // TODO: objectShader(frontWavingReeds2, YES, GRASS_BACKANDFORTH, 6, 1, YES)
 objectParallaxLayer(frontWavingReeds3, -2)
 // TODO: objectShader(frontWavingReeds3, YES, GRASS_BACKANDFORTH, 4, 2, YES)
 
 local star = 0
 for (local i = 1; i <= 28; i += 1) {
 star = Bridge["bridgeStar"+i]
 objectParallaxLayer(star, 5)
 
 startthread(twinkleStar, star, 0.01, 0.1, random(0,0.3), random(0.6, 1))
 }	
 for (local i = 1; i <= 5; i += 1) {
 star = Bridge["bridgeStarB"+i]
 objectParallaxLayer(star, 5)
 
 startthread(twinkleStar, star, 0.05, 0.3, 0, 1)
 }
 objectOffset(Bridge.bridgeTrain, -100, 0)	

 // TODO: flashSelectActorIcon(selectActorHint)

 if (g.act4) {
 objectTouchable(bridgeGrateEntryDoor, NO)
 }

 if ((g.easy_mode == YES) && !g.openingScene && g.sheriff_counter == 0) {
 startthread(bodySheriffNag)	
 }
 
 if (g.openingScene == 1) {
 addTrigger(Bridge.triggerAttack, Bridge.attackBoris)
 if (objectState(bridgeLight) == OFF) {
 removeTrigger(Bridge.triggerUnderbrush)
 }
 }

 if ((g.easy_mode == YES) && g.openingScene) {
 Tutorial.triggerHint()
 if (g.hint_stage <= 4) {
 // TODO: addTrigger(Bridge.trigger1Hints, @(){ Tutorial.completeHint(4) })
 }
 if (g.hint_stage <= 5) {
 // TODO: addTrigger(Bridge.triggerRockHint, @(){ removeTrigger(Bridge.triggerRockHint)
 sayLine(boris, "@40123","@40124") 
 }
 }
 if (g.hint_stage == 7) {
 // TODO: addTrigger(Bridge.trigger2Hints, @(){ Tutorial.completeHint(7) })
 }
 }
}


willie <- { 
 _key = "willie"
 dialog = null
 rambleTID = 0
}
createActor(willie)
actorRenderOffset(willie, 0, 45)
actorTalkColors(willie, talkColorWillie)

ray <- { 
 _key = "ray"
  talkColor = 0x30AAFF
  icon = "icon_ray"
  _pause_looking = NO
  function pauseLooking(state) {
 this._pause_looking = state
 }
 function showHideLayers() {
 actorHideLayer(ray, "nose_glasses")
//  ray.pauseBlinks(NO)
 actorHideLayer(ray, "pirate_hat")
 actorHideLayer(ray, "foil_hat")
 actorHideLayer(ray, "splash")
 }
}
createActor(ray)

reyes <- {
 _key = "reyes"
 talkColor = 0xc2b49f
 name = "@30045"
 icon = "icon_reyes"
 _pause_looking = NO
 notebookOpen = NO
 _writingTID = 0
 function pauseLooking(state) {
 this._pause_looking = state
 }
function showHideLayers() {
 actorHideLayer(reyes, "nose_glasses")
//  reyes.pauseBlinks(NO)
 actorHideLayer(reyes, "pirate_hat")
 actorHideLayer(reyes, "foil_hat")
 actorHideLayer(reyes, "splash")
 }
 function _startWriting() {
 if (!actorWalking(this)) {
 if (notebookOpen == NO) {
 actorPlayAnimation(reyes, "start_writing", NO)
 breaktime(0.30)
 }
 if (!actorWalking(this)) {
 actorPlayAnimation(reyes, "writing", YES)
 }
 }
 }
  function _stopWriting() {
 if (notebookOpen && !actorWalking(this)) {
 actorPlayAnimation(reyes, "stop_writing", NO)
 breaktime(0.30)
 if (!actorWalking(this)) {
 actorPlayAnimation(reyes, animStand)
 }
 }
 _writingTID = 0
 }

function startWriting() {
//  stopActorIdle(reyes)
 if (_writingTID) stopthread(_writingTID)
//  if (objectOwner(Inventory.reyesNotebook) != reyes) {
//  return
//  }
 _writingTID = startthread(_startWriting)
 notebookOpen = YES
 }

  function stopWriting() {
 if (_writingTID) stopthread(_writingTID)
//  if (objectOwner(Inventory.reyesNotebook) != reyes) {
//  return
//  }
 _writingTID = startthread(_stopWriting)
 notebookOpen = NO
//  startReyesIdle()
 }
}
createActor(reyes)

function rayCostume()
{
 actorCostume(ray, "RayAnimation")
 actorWalkSpeed(ray, 30, 15)
 actorRenderOffset(ray, 0, 45)
//  objectOffset(ray, 0, 0)
 actorTalkColors(ray, ray.talkColor)
 actorTalkOffset(ray, 0,defaultTextOffset)
//  objectLit(ray, 1)
 footstepsNormal(ray)
 ray.showHideLayers()
}

function reyesCostume()
{
 actorCostume(reyes, "ReyesAnimation")
 actorWalkSpeed(reyes, 30, 15)
 actorRenderOffset(reyes, 0, 45)
//  objectOffset(reyes, 0, 0)
 actorTalkColors(reyes, reyes.talkColor)
 actorTalkOffset(reyes, 0,defaultTextOffset)
//  objectLit(reyes, 1)
 footstepsNormal(reyes)
 reyes.showHideLayers()
}

addSelectableActor(1, ray)
defineVerbs(1)
verbUIColors(1, {	nameid = "ray", sentence = 0xffffff, 
 verbNormal = 0x2173d8, verbHighlight = 0x4c9cff,
 verbNormalTint = 0x0077ff, verbHighlightTint = 0x95ecff, 
 dialogNormal = 0x4c9cff, dialogHighlight = 0xa6ceff,
 inventoryFrame = 0x005dcf, inventoryBackground = 0x001e51 })
rayCostume()

addSelectableActor(2, reyes)
defineVerbs(2)
verbUIColors(2, {	nameid = "reyes", sentence = 0xffffff, 
 verbNormal = 0xaca192, verbHighlight = 0xbba88c,
 retroNormal = 0x958064, retroHighlight = 0xecc285,
 verbNormalTint = 0x95754f, verbHighlightTint = 0xf6e0c6, 
 dialogNormal = 0xc2b49f, dialogHighlight = 0xf6e0bf,
 inventoryFrame = 0xaca08c, inventoryBackground = 0x454441 })
 verbUIColors(6, {	nameid = "boris", sentence = 0xffffff, 
 verbNormal = 0x3ea4b5, verbHighlight = 0x4fd0e6,
 verbNormalTint = 0x4ebbb5, verbHighlightTint = 0x96ece0, 
 inventoryFrame = 0x009fdb, inventoryBackground = 0x002432 })
reyesCostume()

addSelectableActor(6, boris)

function williePassedOutCostume()
{
 actorCostume(willie, "WilliePassedOutAnimation")
 actorUseWalkboxes(willie, NO)
 actorLockFacing(willie, FACE_RIGHT)
 objectHotspot(willie, -28,0,20,50)
}

function actorEyesLook(actor, dir) {
 if (actor) {
 if (is_actor(dir)) {
 local x = actorPosX(actor) - actorPosX(dir)
 if (x > -10 && x < 10) {
 dir = FACE_FRONT
 } else
 if (x < 0) {
 dir = FACE_RIGHT
 } else {
 dir = FACE_LEFT
 }
 } else	
 if (is_object(dir)) {
 local x = actorPosX(actor) - objectPosX(dir)
 if (x > -10 && x < 10) {
 dir = FACE_FRONT
 } else
 if (x < 0) {
 dir = FACE_RIGHT
 } else {
 dir = FACE_LEFT
 }
 }	
 if (dir == FACE_RIGHT) actorPlayAnimation(actor, "eyes_right")
 if (dir == FACE_LEFT) actorPlayAnimation(actor, "eyes_left")
 if (dir == FACE_FRONT) actorPlayAnimation(actor, "eyes_front")
 }
}

// Globals.nut
g <- {
    openingScene = 1
    willie_sleeping = NO
    easy_mode = NO
    in_video_flashback = NO
    act4 = NO
    hint_stage = 1	
    taken_photo = NO		
}

// MusicHelpers.nut
_watchMusicTID <- 0
_playingMusicSID <- 0
_playingMusic <- 0
_musicPool <- null
_nextMusic <- 0
g.musicSuspended <- NO
g.musicPlaying <- NO
g.musicSuspended <- NO

bridgeMusicPool <- [ musicBridgeA, musicBridgeB, musicBridgeC musicBridgeD, musicBridgeE ]

function _startWatchMusicCallback(){
do {
 breakwhilesound(_playingMusicSID)
 if (g.musicSuspended) return
 if (_musicPool == null) {
 stopMusic()
 return
 }
 startMusic(0, null, NO)
 }while(1)
}

function _startWatchMusic() {
 _watchMusicTID = stopthread(_watchMusicTID)		
 if (_musicPool) {
     _watchMusicTID = startthread(_startWatchMusicCallback)
 // TODO: _watchMusicTID = startglobalthread(_startWatchMusicCallback)
 threadpauseable(_watchMusicTID, NO)
 g.musicPlaying = YES
 }
}

function startMusic(music = 0, pool = null, cross_fade = YES) {
 if (g.musicSuspended == YES) {
 return
 }
 
 if (music == 0 && pool == _musicPool && _playingMusic && _watchMusicTID) {
 return
 }
 if (pool && pool != _musicPool) {
 _nextMusic = 0		
 }
 if (pool) {
 _musicPool = pool
 }
 
 if (music == 0 && _musicPool) {
 music = _nextMusic ? _nextMusic : randomfrom(_musicPool)
 } else {
 
 cross_fade = NO
 }
 if (settings.preloadMusic) {
 
 _nextMusic = randomfrom(_musicPool)
 if (music == _nextMusic) {
 _nextMusic = randomfrom(_musicPool)
 }
 if (music == _nextMusic) {
 _nextMusic = randomfrom(_musicPool)
 }
 if (music == _nextMusic) {
 _nextMusic = 0		
 }
 if (_nextMusic) {
 if (settings.showMusicInfo) ""
 loadSound(_nextMusic,YES)
 }
 }
 
 if (_playingMusicSID) {
 if (cross_fade) {
 fadeOutSound(_playingMusicSID, MUSIC_FADE_TIME)
 } else {
 stopSound(_playingMusicSID)
 }
 }
 _playingMusic = 0
 if (music) {
 if (cross_fade && _playingMusicSID) {	
 _playingMusicSID = playMusic(music, 0, MUSIC_FADE_TIME)
 } else {
// TODO: _playingMusicSID = playMusic(music)
 _playingMusicSID = playSound(music)
 }
 _playingMusic = music
 if (settings.showMusicInfo) ""
 _startWatchMusic()
 }
}

AStreet <- 
{
 background = "AStreet"
 enter = function() {
 }
}

MansionEntry <- 
{
 background = "MansionEntry"
 enter = function() {
 }
}

SheriffsOffice <- 
{
 background = "SheriffsOffice"
 enter = function() {
 }
}
QuickiePal <- 
{
 background = "QuickiePal"
 enter = function() {
 }
}

QuickiePalOutside <- 
{
 background = "QuickiePalOutside"
 enter = function() {
 }
}

Cemetery <- 
{
 background = "Cemetery"
 enter = function() {
 }
}

HotelLobby <- 
{
 background = "HotelLobby"
 enter = function() {
 }
}

Alleyway <- 
{
 background = "Alleyway"
 enter = function() {
 }
}

MansionLibrary <- 
{
 background = "MansionLibrary"
 enter = function() {
 }
}

ChucksOffice <- 
{
 background = "ChucksOffice"
 enter = function() {
 }
}

CircusEntrance <- 
{
 background = "CircusEntrance"
 enter = function() {
 }
}

CoronersOffice <- 
{
 background = "CoronersOffice"
 enter = function() {
 }
}

StartScreen <-
{
 background = "StartScreen"
 _music = 0

 function animateBirdObject(dist, delay = 0) {
    delay = delay + random(0.1, 5.0)
    breaktime(delay)
    local bird = createObject("VistaSheet", [ "rfly_body1", "rfly_body2", "rfly_body3", "rfly_body4" ])
    local speed = 0
    local yOffset = 0
    objectLit(bird, YES)
    objectSort(bird, 250)
    objectAt(bird, -20, 0)
    if (dist == 1) {
        speed = 8
        objectScale(bird, 0.5)
    } else 
    if (dist == 2) {
        speed = 15
        objectScale(bird, 0.25)
    }
    do {
        local yStart = random(100,160)
        
        objectOffset(bird, 0, yStart)
        loopObjectState(bird, 0)
        yOffset = random(-80,80)
        if (dist == 2) {
        yOffset = yOffset*0.25
        }
        objectOffsetTo(bird, 430, (yStart + yOffset), random((speed*0.90), (speed*1.10)))
        breaktime(speed*1.10)	
        breaktime(random(5.0, 10.0))
    } while(1)
 }

 function flashRadioLight() {
    objectAlpha(startScreenRadioLight, 0.0)
    objectState(startScreenRadioLight, ON)
    do {
        breaktime(1)
        objectAlphaTo(startScreenRadioLight, 1.0, 0.25)
        breaktime(1)
        objectAlphaTo(startScreenRadioLight, 0.0, 0.25)
    } while(1)
 }

 enter = function()
 {
    ""
    // TODO: stopMusic()
    _music = loopMusic(musicStartScreen)
    actorSlotSelectable(OFF)

    startthread(flashRadioLight)

    for (local i = 1; i <= 3; i += 1) {
    startthread(animateBirdObject, 1, 15)
    }
    for (local i = 1; i <= 5; i += 1) {
    startthread(animateBirdObject, 2)
    }
    // setProgress("mainmenu")
 }

 exit = function()
 {
    fadeOutSound(_music, 1.0)
 }
}

Opening <-
{
 background = "Opening"
//  _dont_hero_track = 1

 function waveReed(reed) {
 local speed_min = 0.75
 local speed_max = 1.0
 objectHidden(reed, NO)
//  objectShader(reed, YES, GRASS_BACKANDFORTH, random(3.0,5.0), random(speed_min,speed_max), YES)
 }

 function playOpening(){
    return startthread(playOpening2);
 }

 function playOpening3(){
 do {
 objectState(openingLight, 1)
 playSound(soundTowerLight)
 breaktime(2.5)
 objectState(openingLight, 0)
 playSound(soundTowerLight2)
 breaktime(1.0)
 }while(1)
 }

 function playOpening2() {
 cameraInRoom(Opening)

 breaktime(2.0)

 objectHidden(opening1987, NO)
 objectScale(opening1987, 0.5)
 roomFade(FADE_IN, 2.0)
 local sid = loopSound(soundTowerHum, -1, 1.0)
 breaktime(4.0)
 roomFade(FADE_OUT, 2.0)
 breaktime(2.0)
 hideAll()

 breaktime(2.0)

 
 objectHidden(openingLightBackground, NO)
 objectHidden(openingLight, NO)

 for (local i = 1; i <= 3; i += 1) {
 local star = Opening["openingStar"+i]
 objectHidden(star, NO)
 
 star.tid <- startthread(twinkleStar, star, 0.1, 0.5, random(0.5,1.0), random(0.5, 1))
 }	
 for (local i = 4; i <= 16; i += 1) {
 local star = Opening["openingStar"+i]
 objectHidden(star, NO)
 
 star.tid <- startthread(twinkleStar, star, 0.1, 0.5, random(0.5,1.0), random(0.1, 0.5))
 }	

 roomFade(FADE_IN, 3.0)
 breaktime(3.0)

 local tid = startthread(playOpening3)
 breaktime(10.0)

 fadeOutSound(sid, 3.0)
 stopthread(tid)
 roomFade(FADE_OUT, 3.0)
 breaktime(3.0)
 hideAll()

 for (local i = 1; i <= 16; i += 1) {
 local star = Opening["openingStar"+i]
 objectHidden(star, YES)
 stopthread(star.tid)
 }	

 
 objectHidden(openingFenceBackground, NO)
 objectHidden(openingChain, NO)
 objectHidden(openingLock, NO)
 objectState(openingLock, 0)

 sid = loopSound(soundWindBirds, -1, 3.0)
 roomFade(FADE_IN, 3.0)
 breaktime(1.0)

 playObjectState(openingLock, 1)

 playSound(soundFenceLockRattle)
 breaktime(1.0)

 playObjectState(openingLock, 1)

 breaktime(1.0)

 breaktime(1.0)

 playObjectState(openingLock, 1)

 breaktime(1.0)

 playObjectState(openingLock, 1)

 breaktime(1.0)

 fadeOutSound(sid, 3.0)
 fadeOutSound(soundFenceLockRattle, 3.0)
 roomFade(FADE_OUT, 3.0)

 breaktime(1.0)

 playObjectState(openingLock, 1)

 breaktime(1.0)

 playObjectState(openingLock, 1)

 breaktime(1.0)

 breaktime(1.0)
 hideAll()

 
 objectHidden(openingSignBackground, NO)
 objectHidden(openingSign, NO)
 objectHidden(openingPop, NO)
 objectHidden(openingThimbleweedParkText, NO)
 objectHidden(openingCityLimitText, NO)
 objectHidden(openingElevationText, NO)

 for (local i = 1; i <= 3; i += 1) {
 local star = Opening["openingStarA"+i]
 objectHidden(star, NO)
 
 star.tid <- startthread(twinkleStar, star, 0.01, 0.1, random(0,0.3), random(0.6, 1))
 }	
 for (local i = 1; i <= 1; i += 1) {
 local star = Opening["openingStarAB"+i]
 objectHidden(star, NO)
 
 star.tid <- startthread(twinkleStar, star, 0.05, 0.3, 0, 1)
 }	

 waveReed(openingReeds1)
 waveReed(openingReeds2)
 waveReed(openingReeds3)
 waveReed(openingReeds4)
 waveReed(openingReeds5)
 waveReed(openingReeds6)
 waveReed(openingReeds7)
 waveReed(openingReeds8)
 waveReed(openingReeds9)

 roomFade(FADE_IN, 5.0)
 breaktime(1.0)
 loopSound(soundCricketsLoop, -1, 2.0)
 breaktime(2.0)
 objectHidden(openingBulletHole, YES)
 objectState(openingPop, 0)
 breaktime(5.0)
 playSound(soundGunshot)
 stopSound(soundCricketsLoop)
 objectHidden(openingBulletHole, NO)
 breaktime(3.0)
 playSound(soundMetalClank)
 objectState(openingPop, 1);
 breaktime(3.0)
 roomFade(FADE_OUT, 2.0)
 breaktime(3.0)
 hideAll()
 }

 function hideAll() {
    foreach(obj in this) { 
        if (isObject(obj)) { 
            objectHidden(obj, YES) 
        }
    }
 }

 enter = function()
 {
    ""
    hideAll()
 }

 exit = function()
 {
 }

 openingSign = { name = "" }
 openingPop = { name = "" }
 openingBulletHole = { name = "" }
 opening1987 = { name = "" }
}

TitleCards <-
{
 background = "TitleCards"
//  _dont_hero_track = TRUE

 enter = function()
 {
 ""
 foreach(obj in this) { if (isObject(obj)) { objectHidden(obj, YES) }}
 }

 exit = function()
 {
 }

 function displayCard(part, title) {
 if (part != 1) {	
//  local count = getPrivatePref("heroPartCount", 0)+1
//  setPrivatePref("heroPartCount", count)
//  markStat("part", count) 
 }
//  cameraInRoom(TitleCards)
//  stopAllSounds()
//  stopMusic(0.10)
//  stopSoundAmbiance()
//  local state = inputState()
 inputOff()
 inputVerbs(OFF)
 objectHidden(pressPreview, YES)
 objectHidden(part, NO)
 objectHidden(title, NO)
 objectHidden(line, NO)
 objectAlpha(part, 1.0)
 objectAlpha(title, 1.0)
 objectAlpha(line, 1.0)
 playSound(randomfrom(soundTitleStinger1, soundTitleStinger2, soundTitleStinger3, soundTitleStinger4))
 breaktime(5.0)
 objectAlphaTo(part, 0.0, 2.0)
 objectAlphaTo(title, 0.0, 2.0)
 objectAlphaTo(line, 0.0, 2.0)
 breaktime(4.0)
 objectHidden(part, YES)
 objectHidden(title, YES)
 objectHidden(line, YES)
//  inputState(state)
 }
  function showPartMeeting() {
//  achievementPart(1)
//  setProgress("part1")
//  logEvent("part1")
 return startthread(displayCard, part1, part1Title)
 }
}

Void <- { background = "Void" }

defineRoom(Bridge)
defineRoom(StartScreen)
// defineRoom(AStreet)
// defineRoom(MansionEntry)
// defineRoom(SheriffsOffice)
// defineRoom(QuickiePal)
// defineRoom(QuickiePalOutside)
// defineRoom(Cemetery)
// defineRoom(HotelLobby)
// defineRoom(Alleyway)
// defineRoom(MansionLibrary)
// defineRoom(ChucksOffice)
// defineRoom(CircusEntrance)
// defineRoom(CoronersOffice)
// defineRoom(Opening)
defineRoom(Void)
defineRoom(TitleCards)
// Opening.playOpening()
// TitleCards.showPartMeeting()
// cameraInRoom(StartScreen)
// cameraInRoom(TitleCards)
// roomFade(FADE_IN, 2.0)

function newOpeningScene() {
    // inputOn()
    // inputVerbs(ON)
    // selectActor(boris)
    // cameraFollow(boris)
    // roomFade(FADE_OUT, 0)
    actorSlotSelectable(OFF)
    // TODO: exCommand(EX_AUTOSAVE_STATE, (NO))
    actorAt(boris, Bridge.borisStartSpot)
    actorFace(boris, FACE_RIGHT)
    pickupObject(Bridge.borisNote, boris)
    pickupObject(Bridge.borisWallet, boris)
    pickupObject(Bridge.borisHotelKeycard, boris)
    pickupObject(Bridge.borisPrototypeToy, boris)
    // TODO: setRoomNumber(borisHotelKeycard)
    // TODO: setKeycardName(borisHotelKeycard)
    Bridge.speck_of_dust <- NO	
    actorSlotSelectable(ray, OFF)
    actorSlotSelectable(reyes, OFF)
    
    // TODO: lot of code

    // startMusic(musicBridgeA, bridgeMusicPool)
    cameraInRoom(Bridge)

    Bridge.bridgeGate.gate_state = CLOSED
    objectState(Bridge.bridgeBody, GONE)
    objectState(Bridge.bridgeBottle, GONE)
    objectState(Bridge.bridgeChainsaw, GONE)
    objectTouchable(Bridge.bridgeGateBack, YES)
    objectTouchable(Bridge.bridgeGate, NO)
    williePassedOutCostume()
    actorAt(willie, Bridge.willieSpot)
    actorUsePos(willie, Bridge.willieTalkSpot)
    willie.dialog = "WillieBorisDialog"
    actorPlayAnimation(willie, "awake")
    objectState(Bridge.willieObject, HERE)
    objectTouchable(Bridge.willieObject, YES)
    cameraAt(700,86)
    roomFade(FADE_IN, 2)
    breaktime(6)
    cameraPanTo(210, 86, 12, EASE_INOUT)
    startthread(Bridge.trainPassby)
    breaktime(2)
    breaktime(12.0)
    actorPlayAnimation(willie, "drink")
    breakwhileanimating(willie)
    actorPlayAnimation(willie, "awake")
    breaktime(2)
    selectActor(boris)
    actorWalkTo(boris, Bridge.bridgeGateBack)
    breakwhilewalking(boris)
    cameraFollow(boris)
    breaktime(1.0)
    sayLine(boris, "@25541", 
    "@25542")
    breakwhiletalking(boris)

    actorBlinks(boris, OFF)
    actorEyesLook(boris, DIR_RIGHT)
    breaktime(0.5)
    actorEyesLook(boris, DIR_FRONT)
    breaktime(0.15)
    actorEyesLook(boris, DIR_LEFT)
    breaktime(0.5)
    actorEyesLook(boris, DIR_FRONT)
    breaktime(0.15)
    actorEyesLook(boris, DIR_RIGHT)
    breaktime(0.5)
    actorEyesLook(boris, DIR_FRONT)
    breaktime(0.25)
    actorBlinks(boris, ON)
    
    sayLine(boris, "@25543")
    breakwhiletalking(boris)
    breaktime(0.5)
    sayLine(boris, "@25544")

    inputVerbs(ON)
    inputOn()
    print("end :D\n")
}

g.openingScene = 1
local tid = startthread(newOpeningScene)
// local tid = startthread(doOpening)
