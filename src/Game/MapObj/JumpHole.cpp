#include "Game/MapObj/JumpHole.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    const f32 sGravityAcc = 1.2f;
    const f32 sHoleRadius = 200.0f;
    const f32 sHoleDepth = 400.0f;
    const f32 sHoleRefrecRate = 0.9f;
    const s32 sForceSetStartTime = 40;
    const s32 sForceSetEndTime = 90;
    const f32 sBallToCenterFreq = 0.98f;
    const f32 sBallToCenterEndFreq = 0.7f;
    const s32 sForceSetCenterTime = 180;
    const s32 sLaunchFixTime = 10;
    const s32 sSetUpTime = 45;
    const f32 sLaunchValidHeight = 160.0f;

    static TVec3f sTestBindSpherePos(0, 0, 0);
    static TVec3f sTestBindSphereVelocity(0, 0, 0);
};  // namespace

namespace NrvJumpHole {
    NEW_NERVE(JumpHoleNrvWait, JumpHole, Wait);
    NEW_NERVE(JumpHoleNrvSetCenter, JumpHole, SetCenter);
    NEW_NERVE(JumpHoleNrvSetUp, JumpHole, SetUp);
    NEW_NERVE(JumpHoleNrvRailMove, JumpHole, RailMove);
};  // namespace NrvJumpHole

JumpHole::JumpHole(const char* pName)
    : LiveActor(pName), mBoundSensor(), mCameraInfo(), mLaunchStart(0, 0, 0), mBindVelocity(0, 0, 0), mHoleGravity(0, 0, 0), mSettledFrames() {
    mCone = new BindCone(mPosition, TVec3f(0.0f, 1.0f, 0.0f), ::sHoleDepth, ::sHoleRadius);
    mBaseMtx.identity();
}

namespace {
    // Follow validater whose PMF takes no argument; the isValid thunk forwards the id untouched.
    class JumpHoleFollowValidater : public BaseMatrixFollowValidater {
    public:
        JumpHoleFollowValidater(JumpHole* pHost, bool (JumpHole::*pFunc)()) : mHost(pHost), mFunc(pFunc) {
        }

        virtual bool isValid(s32) const {
            return (mHost->*mFunc)();
        }

        /* 0x4 */ JumpHole* mHost;
        /* 0x8 */ bool (JumpHole::*mFunc)();
    };
}  // namespace

void JumpHole::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, false);
    mCone->setPosition(mPosition);
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "\x94\xAD\x8E\xCB\x83\x4A\x83\x81\x83\x89");
    MR::makeMtxTR(reinterpret_cast< MtxPtr >(&mBaseMtx), this);
    bool (JumpHole::*pFunc)() = &JumpHole::isNerveTypeNotWait;
    MR::addBaseMatrixFollowTarget(this, rIter, &mBaseMtx, new JumpHoleFollowValidater(this, pFunc));
    initNerve(GET_NERVE(JumpHole, JumpHoleNrvWait), 0);
    MR::useStageSwitchSyncAppear(this, rIter);
}

bool JumpHole::isNerveTypeNotWait() {
    return !isNerve(GET_NERVE(JumpHole, JumpHoleNrvWait));
}

void JumpHole::initAfterPlacement() {
    MR::calcGravityVector(this, &mHoleGravity, nullptr, 0);
    mCone->setDirection(mHoleGravity);
}

void JumpHole::control() {
    return;
}

bool JumpHole::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xA4 /* ACTMES_SPHERE_PLAYER_BINDED */) {
        f32 radiusSquared = MR::getSensorRadius(pReceiver) * MR::getSensorRadius(pReceiver);
        TVec3f offset = *MR::getSensorPos(pReceiver) - *MR::getSensorPos(pSender);
        TVec3f lateral;
        lateral.killElement2(offset, mHoleGravity);

        if (lateral.squared() <= radiusSquared && offset.squared(lateral) <= ::sLaunchValidHeight * ::sLaunchValidHeight) {
            mBoundSensor = pSender;
            mBindVelocity = MR::getSensorHost(pSender)->mVelocity;
            MR::zeroVelocity(MR::getSensorHost(mBoundSensor));
            return true;
        }
    }

    return false;
}

void JumpHole::exeWait() {
    if (mBoundSensor != nullptr) {
        bindHole();
        setNerve(GET_NERVE(JumpHole, JumpHoleNrvSetCenter));
        MR::invalidateClipping(this);
    }
}

void JumpHole::exeSetCenter() {
    if (MR::isFirstStep(this)) {
        mSettledFrames = 0;
        PSMTXCopy(getBaseMtx(), mBaseMtx.mMtx);
    }

    bindHole();

    if (mBindVelocity.squared() < 0.0001f) {
        mSettledFrames++;
    } else {
        mSettledFrames = 0;
    }

    MR::startActionSound(this, "OjLvJumpHoleSetting", -1, -1, -1);

    if (mSettledFrames > ::sLaunchFixTime || MR::isGreaterStep(this, ::sForceSetCenterTime)) {
        setNerve(GET_NERVE(JumpHole, JumpHoleNrvSetUp));
        mBindVelocity.zero();
    }
}

void JumpHole::exeSetUp() {
    if (MR::isFirstStep(this)) {
        MR::sendArbitraryMsg(0xA5 /* ACTMES_SET_UP_JUMP_HOLE */, mBoundSensor, getSensor("Body"));
    }

    if (MR::isGreaterStep(this, ::sSetUpTime)) {
        setNerve(GET_NERVE(JumpHole, JumpHoleNrvRailMove));
    }
}

void JumpHole::exeRailMove() {
    if (MR::isFirstStep(this)) {
        mLaunchTime = 0.0f;
        initParabola(MR::getSensorHost(mBoundSensor)->mPosition);
        MR::shakeCameraNormal();
        MR::startActionSound(this, "OjJumpHoleFlip", -1, -1, -1);
        MR::startLevelSoundPlayer("JumpObj", -1, -1);
        MR::startLevelSoundPlayer("JumpLongV", -1, -1);
        MR::sendArbitraryMsg(0xA6 /* ACTMES_SHOOT_JUMP_HOLE */, mBoundSensor, getSensor("Body"));
        MR::startMultiActorCameraTargetPlayer(this, mCameraInfo, "\x94\xAD\x8E\xCB\x83\x4A\x83\x81\x83\x89", -1);
    }

    f32 horizontal = mLaunchDistance * mLaunchTime;
    f32 vertical = mLaunchTime * (mLaunchLinear + mLaunchQuadratic * mLaunchTime);
    TVec3f position = mLaunchStart + mLaunchGravity * vertical + mLaunchDirection * horizontal;
    TVec3f previous(MR::getSensorHost(mBoundSensor)->mPosition);
    MR::setPosition(MR::getSensorHost(mBoundSensor), position);
    mLaunchTime += 1.0f / mLaunchDuration;

    if (mLaunchTime >= 1.0f) {
        MR::getSensorHost(mBoundSensor)->mVelocity.set<f32>(position - previous);
        MR::sendArbitraryMsg(0xA7 /* ACTMES_END_JUMP_HOLE */, mBoundSensor, getSensor("Body"));
        mBoundSensor = nullptr;
        MR::endMultiActorCamera(this, mCameraInfo, "\x94\xAD\x8E\xCB\x83\x4A\x83\x81\x83\x89", false, -1);
        setNerve(GET_NERVE(JumpHole, JumpHoleNrvWait));
        MR::validateClipping(this);
    }

    MR::makeMtxFrontUpPos(&mBaseMtx, mLaunchDirection, mLaunchGravity, position);
}

void JumpHole::bindHole() {
    f32 radius = MR::getSensorRadius(mBoundSensor);
    TVec3f previous(*MR::getSensorPos(mBoundSensor));
    TVec3f position(previous);
    mBindVelocity += mHoleGravity * ::sGravityAcc;

    BindResult result;
    BindSphere sphere(previous, radius);
    MR::bindSpereToCone(&result, mBindVelocity, sphere, *mCone);
    MR::updateBindPositionAndVelocity(&position, &mBindVelocity, result, ::sHoleRefrecRate);

    TVec3f lateralVelocity;
    lateralVelocity.killElement2(mBindVelocity, mHoleGravity);
    TVec3f lateralOffset;
    lateralOffset.killElement2(position - mPosition, mHoleGravity);
    f32 limit = ::sHoleRadius - radius;
    if (lateralVelocity.dot(lateralOffset) > 0.0f && lateralOffset.squared() > limit * limit) {
        f32 distance;
        MR::separateScalarAndDirection(&distance, &lateralOffset, lateralOffset);
        position -= lateralOffset * (distance - limit);

        TVec3f tangent = lateralVelocity - lateralOffset * lateralVelocity.dot(lateralOffset);
        if (tangent.isZero()) {
            tangent = mHoleGravity;
        } else {
            MR::normalizeOrZero(&tangent);
        }

        mBindVelocity -= lateralVelocity;
        mBindVelocity += tangent * lateralVelocity.length();
    }

    mBindVelocity *= MR::calcNerveValue(this, ::sForceSetStartTime, ::sForceSetEndTime, ::sBallToCenterFreq, ::sBallToCenterEndFreq);
    MR::getSensorHost(mBoundSensor)->mVelocity.set<f32>(position - previous);

    if (position.squared(previous) < 0.1f * 0.1f) {
        mBindVelocity.zero();
    }
}

void JumpHole::initParabola(const TVec3f& rPosition) {
    MR::moveCoordToStartPos(this);
    TVec3f start(MR::getRailPos(this));
    MR::moveCoordToEndPos(this);
    TVec3f end(MR::getRailPos(this));
    MR::calcGravityVector(this, end, &mLaunchGravity, nullptr, 0);

    if (MR::isNearZero(mLaunchGravity)) {
        mLaunchGravity.set(0.0f, -1.0f, 0.0f);
    }

    f32 startHeight = mLaunchGravity.dot(start - rPosition);
    f32 endHeight = mLaunchGravity.dot(end - rPosition);
    mLaunchDirection = end - rPosition - mLaunchGravity * endHeight;
    MR::separateScalarAndDirection(&mLaunchDistance, &mLaunchDirection, mLaunchDirection);
    MR::calcParabolicFunctionParam(&mLaunchQuadratic, &mLaunchLinear, startHeight, endHeight);
    mLaunchStart = rPosition;
    mLaunchDuration = MR::fastSqrtf(MR::abs(2.0f * mLaunchQuadratic / ::sGravityAcc));
}
