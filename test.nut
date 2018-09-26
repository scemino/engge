function hideAll(r) {
    foreach (obj in r) {
        if (isObject(obj)) {
            objectHidden(obj, YES)
        }
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

function animateBirdObject(dist, delay = 0) {
    delay = delay + random(0.1, 5.0)
    breaktime(delay)
    local bird = createObject("VistaSheet", ["rfly_body1", "rfly_body2", "rfly_body3", "rfly_body4"])
    local speed = 0
    local yOffset = 0
    // objectLit(bird, YES)
    objectSort(bird, 250)
    objectAt(bird, -20, 0)
    if (dist == 1) {
        speed = 8
        objectScale(bird, 0.5)
    } else if (dist == 2) {
        speed = 15
        objectScale(bird, 0.25)
    }
    do {
        local yStart = random(100, 160)

        objectOffset(bird, 0, yStart)
        playObjectState(bird, 0)
        yOffset = random(-80, 80)
        if (dist == 2) {
            yOffset = yOffset * 0.25
        }
        objectOffsetTo(bird, 430, (yStart + yOffset), random((speed * 0.90), (speed * 1.10)))
        breaktime(speed * 1.10)
        breaktime(random(5.0, 10.0))
    } while (1)
}

function animateLock() {
    local lock = createObject("OpeningSheet", ["lock1", "lock2", "lock3", "lock4", "lock5", "lock4", "lock3", "lock2", "lock1"])
    print("lock: " + NO + "\n");
    print("lock: " + YES + "\n");
    do {
        lock.playObjectState(0);
        breaktime(2);
        lock.playObjectState(0);
        breaktime(0.5);
        lock.objectHidden(YES)
        breaktime(1);
        lock.objectHidden(NO)
        breaktime(random(2.0, 5.0));
    } while (1);
}

function flashRadioLight(room) {
    objectAlpha(room.startScreenRadioLight, 1.0)
    objectState(room.startScreenRadioLight, ON)
    do {
        breaktime(1)
        objectAlphaTo(room.startScreenRadioLight, 1.0, 0.25)
        breaktime(1)
        objectAlphaTo(room.startScreenRadioLight, 0.0, 0.25)
    } while (1)
}

function playTowerLight(openingLight) {
    do {
        objectState(openingLight, 1)
        playSound("TowerLight.wav")
        breaktime(2.5)
        objectState(openingLight, 0)
        playSound("TowerLight2.wav")
        breaktime(1.0)
    } while (isRunning)
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

function doOpening() {
    local r = loadRoom("Opening")
    hideAll(r)

    breaktime(2.0)

    objectHidden(r.opening1987, NO)
    scale(r.opening1987, 0.5)
    roomFade(FADE_IN, 2.0)
    local sid = loopSound("TowerHum.wav")
    breaktime(4.0)
    roomFade(FADE_OUT, 2.0)
    breaktime(2.0)
    hideAll(r)

    breaktime(2.0)

    objectHidden(r.openingLightBackground, NO)
    objectHidden(r.openingLight, NO)

    print("Show stars\n");
    for (local i = 1; i <= 3; i += 1) {
        local star = r["openingStar" + i]
        objectHidden(star, NO)
        print("start openingStar" + i + "\n")
        star.tid <- startthread(twinkleStar, star, 0.1, 0.5, random(0.5, 1.0), random(0.5, 1))
        print("star: " + star.tid + "\n");
    }
    for (local i = 4; i <= 16; i += 1) {
        local star = r["openingStar" + i]
        objectHidden(star, NO)
        print("start openingStar" + i + "\n")
        star.tid <- startthread(twinkleStar, star, 0.1, 0.5, random(0.5, 1.0), random(0.1, 0.5))
        print("star: " + star.tid + "\n");
    }

    print("Show stars done !\n");
    roomFade(FADE_IN, 3.0)
    breaktime(3.0)

    local tid = startthread(playTowerLight, r.openingLight)
    breaktime(10.0)

    fadeOutSound(sid, 3.0)
    //local isRunning=false
    roomFade(FADE_OUT, 3.0)
    breaktime(3.0)
    hideAll(r)

    for (local i = 1; i <= 16; i += 1) {
        local star = r["openingStar" + i]
        print("star2: " + star.tid + "\n");
        objectHidden(star, YES)
        stopthread(star.tid)
    }

    objectHidden(r.openingFenceBackground, NO)
    objectHidden(r.openingChain, NO)
    objectHidden(r.openingLock, NO)
    objectState(r.openingLock, 0)

    sid = loopSound("WindBirds.ogg", 3.0)
    roomFade(FADE_IN, 3.0)
    breaktime(1.0)

    playObjectState(r.openingLock, 1)

    local soundFenceLockRattle = playSound("FenceLockRattle.ogg")
    breaktime(1.0)

    playObjectState(r.openingLock, 1)

    breaktime(1.0)

    breaktime(1.0)

    playObjectState(r.openingLock, 1)

    breaktime(1.0)

    playObjectState(r.openingLock, 1)

    breaktime(1.0)

    fadeOutSound(sid, 3.0)
    fadeOutSound(soundFenceLockRattle, 3.0)
    roomFade(FADE_OUT, 3.0)

    breaktime(1.0)

    playObjectState(r.openingLock, 1)

    breaktime(1.0)

    playObjectState(r.openingLock, 1)

    breaktime(1.0)

    breaktime(1.0)
    hideAll(r)

    objectHidden(r.openingSignBackground, NO)
    objectHidden(r.openingSign, NO)
    objectHidden(r.openingPop, NO)
    objectHidden(r.openingThimbleweedParkText, NO)
    objectHidden(r.openingCityLimitText, NO)
    objectHidden(r.openingElevationText, NO)

    objectHidden(r.openingReeds1, NO)
    objectHidden(r.openingReeds2, NO)
    objectHidden(r.openingReeds3, NO)
    objectHidden(r.openingReeds4, NO)
    objectHidden(r.openingReeds5, NO)
    objectHidden(r.openingReeds6, NO)
    objectHidden(r.openingReeds7, NO)
    objectHidden(r.openingReeds8, NO)
    objectHidden(r.openingReeds9, NO)

    roomFade(FADE_IN, 5.0)
    breaktime(1.0)
    local soundCricketsLoop = loopSound("AmbNightCrickets_Loop.ogg", 2.0)
    breaktime(2.0)
    objectHidden(r.openingBulletHole, YES)
    objectState(r.openingPop, 0)
    breaktime(5.0)
    playSound("Gunshot.wav")
    // stopSound(soundCricketsLoop)
    fadeOutSound(soundCricketsLoop, 0.1)

    objectHidden(r.openingBulletHole, NO)
    breaktime(3.0)
    playSound("MetalClank.wav")
    objectState(r.openingPop, 1);
    breaktime(3.0)
    roomFade(FADE_OUT, 2.0)
    breaktime(3.0)
    hideAll(r)
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
    } while (1)
}

function animateFirefly(obj) {
    startthread(flashAlphaObject, obj, 1, 4, 0.5, 2, 0.1, 0.35)
}

function createFirefly(x) {
    local firefly = 0
    local zsort = 68
    local y = random(78, 168)
    local direction = randomfrom(-360, 360)
    if (y < 108) {
        firefly = createObject("firefly_large")
        zsort = random(68, 78)
    } else if (y < 218) {
        firefly = createObject("firefly_small")
        zsort = 117
    } else if (x > 628 && x < 874) {
        firefly = createObject("firefly_tiny")
        zsort = 668
    }
    if (firefly) {
        objectRotateTo(firefly, direction, 12)
        // objectRotateTo(firefly, direction, 12, LOOPING)
        objectAt(firefly, x, y)
        objectSort(firefly, zsort)
        return firefly
    }
}

function intro() {
    local r = loadRoom("StartScreen")
    startthread(flashRadioLight, r)

    for (local i = 1; i <= 3; i += 1) {
        startthread(animateBirdObject, 1, 15)
    }
    for (local i = 1; i <= 5; i += 1) {
        startthread(animateBirdObject, 2)
    }
}

soundBridgeTrain <- defineSound("BridgeTrain.ogg")				

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

 trainPassby = function() {
   objectOffset(Bridge.bridgeTrain, -100, 0)
   objectOffsetTo(Bridge.bridgeTrain, 2000, 0, 10)
   playSound(soundBridgeTrain)
 }

 enter = function() {
   print("hello "+Bridge.bridgeDragMark.name+"\n")
 }

}

defineRoom(Bridge)

willie <- { 
 _key = "willie"
}
createActor(willie)
actorRenderOffset(willie, 0, 45)

boris <- {
 _key = "boris"
}
createActor(boris)
actorRenderOffset(boris, 0, 45)

function newOpeningScene() {
    
    cameraInRoom(Bridge)
    objectState(Bridge.bridgeBody, GONE)
    objectState(Bridge.bridgeBottle, GONE)
    objectState(Bridge.bridgeChainsaw, GONE)
    objectParallaxLayer(Bridge.bridgeTrain, 2)
    objectParallaxLayer(Bridge.frontWavingReeds1, -2)
    objectParallaxLayer(Bridge.frontWavingReeds2, -2)
    objectParallaxLayer(Bridge.frontWavingReeds3, -2)


    // local star = 0
    // for (local i = 1; i <= 28; i += 1) {
    //     star = Bridge["bridgeStar" + i]
    //     objectParallaxLayer(star, 5)

    //     startthread(twinkleStar, star, 0.01, 0.1, random(0, 0.3), random(0.6, 1))
    // }
    // for (local i = 1; i <= 5; i += 1) {
    //     star = Bridge["bridgeStarB" + i]
    //     objectParallaxLayer(star, 5)

    //     startthread(twinkleStar, star, 0.05, 0.3, 0, 1)
    // }

    // for (local x = 0; x < 960; x += random(20, 40)) {
    //     local firefly = createFirefly(x)
    //     if (firefly) {
    //         startthread(animateFirefly, firefly)
    //     }
    // }
    actorAt(willie, Bridge.willieSpot)
    actorCostume(willie, "WilliePassedOutAnimation")
    actorLockFacing(willie, FACE_RIGHT)
    actorPlayAnimation(willie, "awake")
    actorAt(boris, Bridge.borisStartSpot)
    actorCostume(boris, "BorisAnimation")
    actorLockFacing(boris, FACE_RIGHT)
    // actorFace(boris, FACE_RIGHT)

    // cameraAt(700, 86)
    // roomFade(FADE_IN, 2)
    // breaktime(6)
    // cameraPanTo(210, 86, 12)
    // startthread(Bridge.trainPassby)

    breaktime(2)
    // breaktime(12.0)
    actorPlayAnimation(willie, "drink")
    breakwhileanimating(willie)
    actorPlayAnimation(willie, "awake")
    breaktime(2)
    // selectActor(boris)
    actorWalkTo(boris, Bridge.bridgeGateBack)
    // breakwhilewalking(boris)
    // cameraFollow(boris)

    breaktime(200.0)
}

startthread(newOpeningScene)

// intro();
// startthread(doOpening)
// loadRoom("AStreet")
// loadRoom("Bridge")
// loadRoom("MansionEntry")
// loadRoom("MansionExterior")
// loadRoom("SheriffsOffice")
// loadRoom("QuickiePal")
// loadRoom("QuickiePalOutside")
// loadRoom("Cemetery")
// loadRoom("HotelLobby")
// loadRoom("Alleyway")
