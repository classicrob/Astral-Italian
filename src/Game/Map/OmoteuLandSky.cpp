#include "Game/Map/OmoteuLandSky.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SceneUtil.hpp"

namespace NrvOmoteuLandSky {
    NEW_NERVE(OmoteuLandSkyNrvFirst, OmoteuLandSky, FirstWait);
    NEW_NERVE(OmoteuLandSkyNrvFaceWait, OmoteuLandSky, FaceWait);
    NEW_NERVE(OmoteuLandSkyNrvFaceToBack, OmoteuLandSky, FaceToBack);
    NEW_NERVE(OmoteuLandSkyNrvBackWait, OmoteuLandSky, BackWait);
    NEW_NERVE(OmoteuLandSkyNrvBackToFace, OmoteuLandSky, BackToFace);
}  // namespace NrvOmoteuLandSky

OmoteuLandSky::OmoteuLandSky(const char* pName) : LiveActor(pName) {
}

void OmoteuLandSky::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, "OmoteuLandSky", false);
    MR::connectToSceneSky(this);
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    initNerve(&NrvOmoteuLandSky::OmoteuLandSkyNrvFirst::sInstance, 0);
    MR::needStageSwitchReadA(this, rIter);
    MR::useStageSwitchAwake(this, rIter);
    makeActorAppeared();
}

void OmoteuLandSky::calcAnim() {
    TVec3f camPos = MR::getCamPos();
    MR::setPosition(this, camPos);
    LiveActor::calcAnim();
}

void OmoteuLandSky::exeFirstWait() {
    if (MR::isFirstStep(this)) {
        if (isPlayerGravitySameDirection()) {
            MR::startAction(this, "FaceToBack");
            MR::setBckFrameEndAndStop(this);
            setNerve(&NrvOmoteuLandSky::OmoteuLandSkyNrvBackWait::sInstance);
        } else {
            MR::startAction(this, "BackToFace");
            MR::setBckFrameEndAndStop(this);
            setNerve(&NrvOmoteuLandSky::OmoteuLandSkyNrvFaceWait::sInstance);
        }
    }
}

void OmoteuLandSky::exeFaceWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FaceWait");
    }

    if (isPlayerGravitySameDirection()) {
        if (!MR::isActionEnd(this)) {
            MR::setBckFrameEndAndStop(this);
        }
        setNerve(&NrvOmoteuLandSky::OmoteuLandSkyNrvFaceToBack::sInstance);
    }
}

void OmoteuLandSky::exeFaceToBack() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FaceToBack");
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvOmoteuLandSky::OmoteuLandSkyNrvBackWait::sInstance);
    }
}

void OmoteuLandSky::exeBackWait() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BackWait");
    }

    if (!isPlayerGravitySameDirection()) {
        setNerve(&NrvOmoteuLandSky::OmoteuLandSkyNrvBackToFace::sInstance);
    }
}

void OmoteuLandSky::exeBackToFace() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "BackToFace");
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvOmoteuLandSky::OmoteuLandSkyNrvFaceWait::sInstance);
    }
}

bool OmoteuLandSky::isPlayerGravitySameDirection() {
    TVec3f up(0.0f, 1.0f, 0.0f);
    return !MR::isOppositeDirection(*MR::getPlayerGravity(), up, 0.01f);
}

OmoteuLandSky::~OmoteuLandSky() {
}
