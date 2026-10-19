#include "Game/MapObj/KeySwitch.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <JSystem/JMath/JMath.hpp>

namespace {
    const char* cDemoName = "カギ出現";
};  // namespace

namespace NrvKeySwitch {
    NEW_NERVE(KeySwitchNrvDemoStart, KeySwitch, DemoStart);
    NEW_NERVE(KeySwitchNrvAppear, KeySwitch, Appear);
    NEW_NERVE(KeySwitchNrvWait, KeySwitch, Wait);
};  // namespace NrvKeySwitch

KeySwitch::~KeySwitch() {
}

KeySwitch::KeySwitch(const char* pName) : LiveActor(pName) {
    mCameraInfo = 0;
    mCurDemoFrame = -1;
}

void KeySwitch::init(const JMapInfoIter& rIter) {
    MR::initDefaultPos(this, rIter);
    MR::initActor(this, rIter, "KeySwitch", false);
    MR::connectToSceneMapObjDecoration(this);
    initHitSensor(1);
    TVec3f box;
    box.x = 0.0f;
    box.y = 105.0f;
    box.z = 0.0f;
    MR::addHitSensorMapObjSimple(this, "Body", 8, 105.0f, box);
    initBinder(105.0f, 105.0f, 0);
    initEffectKeeper(0, 0, false);
    initSound(4, "KeySwitch", nullptr, TVec3f(0.0f));
    MR::initShadowVolumeCylinder(this, 90.0f);
    f32 dropLength = -1.0f;
    MR::getJMapInfoArg0NoInit(rIter, &dropLength);

    if (dropLength >= 0.0f) {
        MR::setShadowDropLength(this, nullptr, dropLength);
    }

    MR::calcGravity(this);

    if (MR::isValidInfo(rIter)) {
        if (!MR::initActorCamera(this, rIter, &mCameraInfo)) {
            mCameraInfo = nullptr;
        }
    }

    if (MR::isValidInfo(rIter)) {
        MR::needStageSwitchWriteA(this, rIter);
    }

    bool usesSwitch = MR::useStageSwitchSyncAppear(this, rIter);

    if (usesSwitch || !MR::isValidInfo(rIter)) {
        initNerve(GET_NERVE(KeySwitch, KeySwitchNrvDemoStart));
    } else {
        initNerve(GET_NERVE(KeySwitch, KeySwitchNrvWait));
    }

    if (usesSwitch || !MR::isValidInfo(rIter)) {
        makeActorDead();
    } else {
        makeActorAppeared();
    }
}

void KeySwitch::initKeySwitchByOwner(const JMapInfoIter& rIter) {
    initWithoutIter();
    MR::needStageSwitchWriteA(this, rIter);
    MR::initActorCamera(this, rIter, &mCameraInfo);
}

void KeySwitch::prepareDemoStart() {
    setNerve(GET_NERVE(KeySwitch, KeySwitchNrvDemoStart));
    makeActorDead();
}

void KeySwitch::appearKeySwitch(const TVec3f& rVec) {
    MR::resetPosition(this, rVec);
    appear();
}

void KeySwitch::exeDemoStart() {
    if (!mCameraInfo) {
        setNerve(GET_NERVE(KeySwitch, KeySwitchNrvAppear));
        return;
    }

    if (MR::tryStartDemoWithoutCinemaFrame(this, ::cDemoName)) {
        MR::startActorCameraTargetSelf(this, mCameraInfo, -1);
        mCurDemoFrame = 0;
        setNerve(GET_NERVE(KeySwitch, KeySwitchNrvAppear));
    }
}

void KeySwitch::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Rotation");
        mVelocity.scale(-40.0f, mGravity);
        MR::invalidateClipping(this);
        MR::startActionSound(this, "OjKeySwitchAppear");
    }

    MR::setBckRate(this, MR::calcNerveValue(this, 0xB4, 3.0f, 1.5f));
    MR::addVelocityToGravity(this, 2.0f);

    bool isLanded = false;

    if (MR::isBindedGround(this)) {
        if (mVelocity.length() >= 10.0f) {
            s32 mag = mVelocity.length();
            mag *= 2;
            if (mag > 0x64) {
                mag = 0x64;
            }

            MR::startActionSound(this, "OjKeySwitchBound", mag);
            TVec3f upDir;
            upDir.negate(mGravity);
            MR::calcReboundVelocity(&mVelocity, upDir, 0.60f, 0.7f);
        } else {
            isLanded = true;
        }
    }

    if (!tryAvoid()) {
        if (isLanded) {
            MR::validateClipping(this);
            setNerve(GET_NERVE(KeySwitch, KeySwitchNrvWait));
        }
    }
}

void KeySwitch::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::tryStartBck(this, "Rotation", nullptr);
        MR::setBckRate(this, 1.0f);
        mVelocity.zero();
        MR::offCalcGravity(this);
    }
}

void KeySwitch::appear() {
    MR::onCalcGravity(this);
    MR::invalidateClipping(this);
    LiveActor::appear();
}

void KeySwitch::kill() {
    MR::onSwitchA(this);
    MR::startActionSound(this, "SyReadRiddleS", -1, -1, -1);
    MR::emitEffect(this, "Get");
    LiveActor::kill();
}

void KeySwitch::calcAndSetBaseMtx() {
    TVec3f front;
    MR::calcFrontVec(&front, this);
    TPos3f mtx;
    MR::calcMtxFromGravityAndZAxis(&mtx, this, mGravity, front);
    MR::setBaseTRMtx(this, mtx);
}

void KeySwitch::control() {
    if (mCurDemoFrame != -1 && mCameraInfo != nullptr) {
        s32 frames = MR::getActorCameraFrames(this, mCameraInfo);

        if (frames == -1) {
            frames = 40;
        }

        if (mCurDemoFrame >= frames) {
            MR::endActorCamera(this, mCameraInfo, false, -1);
            MR::endDemo(this, ::cDemoName);
            mCameraInfo = nullptr;
            mCurDemoFrame = -1;
        } else {
            mCurDemoFrame++;
        }
    }
}

bool KeySwitch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return MR::isMsgStarPieceReflect(msg);
}

bool KeySwitch::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return true;
    }

    if (isNerve(GET_NERVE(KeySwitch, KeySwitchNrvAppear)) && MR::isLessEqualStep(this, 60)) {
        return false;
    }

    if (MR::isMsgItemGet(msg)) {
        MR::startActionSound(this, "OjKeySwitchGet");
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        kill();
        return true;
    }

    return false;
}

bool KeySwitch::tryAvoid() {
    LiveActor* sensorActor;
    HitSensor* sensor = nullptr;

    if (MR::isBindedGround(this)) {
        sensor = MR::getGroundSensor(this);
    } else if (MR::isBindedWall(this)) {
        sensor = MR::getWallSensor(this);
    }

    if (sensor == nullptr) {
        return false;
    }

    if (!MR::isSensorType(sensor, 0x54 /* ATYPE_KEY_SWITCH_AVOID */)) {
        return false;
    }

    sensorActor = MR::getSensorHost(sensor);
    TVec3f up;
    MR::calcUpVec(&up, sensorActor);
    TVec3f fromHost;
    fromHost.sub(mPosition, sensorActor->mPosition);
    TVec3f avoidDir = fromHost.killElement(up);

    if (MR::normalizeOrZero(&avoidDir)) {
        MR::calcFrontVec(&avoidDir, sensorActor);
    }

    mVelocity.scale(10.0f, avoidDir);
    return true;
}
