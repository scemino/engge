const talkColorBoris		= 0x3ea4b5
const talkColorWillie		= 0xc69c6d

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

// sound helpers.nut
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
 // TODO: soundVolume(_soundAmbianceSID, _soundAmbianceBedVolume)
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
 //TODO: objectRotateTo(firefly, direction, 12, LOOPING)
 objectRotateTo(firefly, direction, 12)
 objectAt(firefly, x, y)
 objectSort(firefly, zsort)
 return firefly
 }
 }

Bridge <- 
{
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

 bridgeDragMark =
 {
   name = "@25710"
   initState = GONE
 }

 bridgeGate =
 {
    name="bridgeGate"
    gate_state = CLOSED
    gate_opening = NO
 }

 bridgeLight =
 {
   name = "@25694"
   initState = ON
 }

borisNote =
{
 icon = "safe_combination_note"
}
borisWallet =
{
 icon = "boris_wallet"
}
borisHotelKeycard =
{
 icon = "key_card_mauve"
}
borisPrototypeToy =
{
 icon = "prototype_toy_orange"
}

 trainPassby = function() {
   objectOffset(Bridge.bridgeTrain, -100, 0)
   objectOffsetTo(Bridge.bridgeTrain, 2000, 0, 10, LINEAR)
   playSound(soundBridgeTrain)
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
// TODO: walkboxHidden("body", NO)
 // TODO: addTrigger(Bridge.triggerCloseGate, @() { startthread(Bridge.closeGate); removeTrigger(Bridge.triggerCloseGate); })
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
 // TODO: walkboxHidden("body", YES)
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
// TODO:  walkboxHidden("gate", NO)
 objectOffset(Bridge.bridgeGate, -60, 0)
 objectOffset(Bridge.bridgeGateBack, -60, 0)
 } else {
// TODO:  walkboxHidden("gate", YES)
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
  // TODO: walkboxHidden("fallen_tree", NO)
  objectSort(Bridge.bridgeGrateTree, 123)
  objectOffset(Bridge.bridgeGrateTree, 0, 0)
  }

 // TODO: addTrigger(Bridge.triggerSound1, @() { Bridge.spookySound(Bridge.triggerSound1, randomfrom(soundOwls, soundBridgeAmbienceOwlHoot, soundBridgeAmbienceOwlHoot, soundBridgeAmbienceOwlHoot), 5.0) })
 // TODO: addTrigger(Bridge.triggerSound4, @() { Bridge.spookySound(Bridge.triggerSound4, randomfrom(soundWolf, soundBridgeAmbienceDogHowl1, soundBridgeAmbienceDogHowl2, soundBridgeAmbienceDogHowl3, soundBridgeAmbienceDogHowl4), 5.0) })

 // TODO: addTrigger(triggerWater, footstepsWater, footstepsNormal)
 
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
 // TODO: addTrigger(Bridge.triggerAttack, Bridge.attackBoris)
 if (objectState(bridgeLight) == OFF) {
 // TODO: removeTrigger(Bridge.triggerUnderbrush)
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

defineRoom(Bridge)
foreach(obj in Bridge) { 
    if("name" in obj){
        print("$$$$$$ obj: "+obj.name+"\n")
    }
}

willie <- { 
 _key = "willie"
 dialog = null
}
createActor(willie)
actorRenderOffset(willie, 0, 45)

function williePassedOutCostume()
{
 actorCostume(willie, "WilliePassedOutAnimation")
 actorUseWalkboxes(willie, NO)
 actorLockFacing(willie, FACE_RIGHT)
 objectHotspot(willie, -28,0,20,50)
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
defineVerbs(1)

function borisCostume()
{
 actorCostume(boris, "BorisAnimation")
//  actorWalkSpeed(boris, 30, 15)
 actorRenderOffset(boris, 0, 45)
 actorTalkColors(boris, talkColorBoris)
//  actorTalkOffset(boris, 0, defaultTextOffset)
 actorHidden(boris, OFF)
//  objectLit(boris, 1)
//  footstepsNormal(boris)
 boris.showHideLayers()
}
borisCostume()

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

// Globals.nut
g <- {
    openingScene = 0
    easy_mode = NO
    in_video_flashback = NO
    act4 = NO
    hint_stage = 1		
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

// Bridge.nut
function newOpeningScene() {
    roomFade(FADE_OUT, 0)
    // TODO: actorSlotSelectable(OFF)
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
    // TODO: actorSlotSelectable(ray, OFF)
    // TODO: actorSlotSelectable(reyes, OFF)
    
    // TODO: lot of code

    startMusic(musicBridgeA, bridgeMusicPool)
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
    // TODO: cameraFollow(boris)
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

local tid = startthread(newOpeningScene)
// print("thread: "+tid+"\n")
// breakhere(1000)
// print("stop thread\n")
// stopthread(tid)

AStreet <- 
{
 background = "AStreet"
 enter = function() {
 }
}
defineRoom(AStreet)

MansionEntry <- 
{
 background = "MansionEntry"
 enter = function() {
 }
}
defineRoom(MansionEntry)

SheriffsOffice <- 
{
 background = "SheriffsOffice"
 enter = function() {
 }
}
defineRoom(SheriffsOffice)

QuickiePal <- 
{
 background = "QuickiePal"
 enter = function() {
 }
}
defineRoom(QuickiePal)

QuickiePalOutside <- 
{
 background = "QuickiePalOutside"
 enter = function() {
 }
}
defineRoom(QuickiePalOutside)

Cemetery <- 
{
 background = "Cemetery"
 enter = function() {
 }
}
defineRoom(Cemetery)

HotelLobby <- 
{
 background = "HotelLobby"
 enter = function() {
 }
}
defineRoom(HotelLobby)

Alleyway <- 
{
 background = "Alleyway"
 enter = function() {
 }
}
defineRoom(Alleyway)

MansionLibrary <- 
{
 background = "MansionLibrary"
 enter = function() {
 }
}
defineRoom(MansionLibrary)

ChucksOffice <- 
{
 background = "ChucksOffice"
 enter = function() {
 }
}
defineRoom(ChucksOffice)

CircusEntrance <- 
{
 background = "CircusEntrance"
 enter = function() {
 }
}
defineRoom(CircusEntrance)

CoronersOffice <- 
{
 background = "CoronersOffice"
 enter = function() {
 }
}
defineRoom(CoronersOffice)

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
    stopMusic()
    _music = loopMusic(musicStartScreen)
    // actorSlotSelectable(OFF)

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
defineRoom(StartScreen)

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
        print("obj: "+obj+"\n")
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
defineRoom(Opening)

// Opening.playOpening()

// cameraInRoom(QuickiePal)
// roomFade(FADE_IN, 2)

