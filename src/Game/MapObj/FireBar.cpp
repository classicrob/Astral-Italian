#include "Game/Util/SceneUtil.hpp"
#include "Game/MapObj/FireBar.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/Util/ActorStateUtil.hpp"
#include "Game/NameObj/NameObjExecuteHolder.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Util.hpp"
#include <JSystem/JMath/JMath.hpp>


namespace NrvFireBar {
    NEW_NERVE(FireBarNrvA, FireBarBall, NerveA);

    class FireBarNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            MR::updateActorStateAndNextNerve(reinterpret_cast< FireBarBall* >(pSpine->mExecutor), GET_NERVE_DIRECT(NrvFireBar, FireBarNrvA));
        }
        static FireBarNrvB sInstance;
    };
    FireBarNrvB FireBarNrvB::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(FireBarNrvWait, FireBar, Wait);
    NEW_NERVE(FireBarNrvC, FireBar, NerveC);
};  // namespace NrvFireBar

FireBarBall::~FireBarBall() {
}

FireBar::~FireBar() {
}

FireBarBall::FireBarBall(LiveActor* pParent) : LiveActor("ファイアバー玉"), mFireBarParent(pParent), mBindStarPointer(nullptr), mIsClipped(false) {
}

void FireBarBall::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "FireBarBall", false);

    f32 shadowDropDist = 100.0f;
    MR::getJMapInfoArg6NoInit(rIter, &shadowDropDist);

    if (0.0f != shadowDropDist) {
        MR::setShadowDropLength(this, nullptr, shadowDropDist);
        s32 shadowDropRotate = -1;
        MR::getJMapInfoArg4NoInit(rIter, &shadowDropRotate);

        if (shadowDropRotate == 0) {
            TVec3f upVec;
            MR::calcUpVec(&upVec, mFireBarParent);
            upVec.negate();
            MR::setShadowDropDirection(this, nullptr, upVec);
        }

        s32 cutShadows = -1;
        MR::getJMapInfoArg2NoInit(rIter, &cutShadows);

        if (cutShadows == 0) {
            MR::setShadowVolumeEndDropOffset(this, nullptr, 20.0f);
            MR::onShadowVolumeCutDropLength(this, nullptr);
            MR::onCalcShadow(this, nullptr);
        }
    } else {
        MR::invalidateShadow(this, nullptr);
    }

    MR::invalidateClipping(this);
    initNerve(GET_NERVE_DIRECT(NrvFireBar, FireBarNrvA), 1);
    mBindStarPointer = new WalkerStateBindStarPointer(this, nullptr);
    MR::initActorState(this, mBindStarPointer, GET_NERVE_DIRECT(NrvFireBar, FireBarNrvB), nullptr);
    makeActorDead();
}

void FireBarBall::initAfterPlacement() {
    MR::calcGravity(this);
}

void FireBarBall::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    setNerve(GET_NERVE_DIRECT(NrvFireBar, FireBarNrvA));
}

void FireBarBall::movement() {
    if (MR::isClipped(mFireBarParent)) {
        if (!mIsClipped) {
            mIsClipped = true;
            MR::invalidateHitSensors(this);
            MR::deleteEffect(this, "FireBarBall");
            MR::disconnectToDrawTemporarily(this);
        }
    } else {
        if (mIsClipped) {
            mIsClipped = false;
            MR::validateHitSensors(this);
            MR::emitEffect(this, "FireBarBall");
            MR::connectToDrawTemporarily(this);
        }

        LiveActor::movement();
    }
}

void FireBarBall::exeNerveA() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "FireBarBall");
    }

    if (mBindStarPointer->tryStartPointBind()) {
        setNerve(GET_NERVE_DIRECT(NrvFireBar, FireBarNrvB));
    }
}

void FireBarBall::startClipped() {
    MR::deleteEffect(this, "FireBarBall");
    LiveActor::startClipped();
}

void FireBarBall::endClipped() {
    MR::emitEffect(this, "FireBarBall");
    LiveActor::endClipped();
}

FireBar::FireBar(const char* pName) : LiveActor(pName) {
    mFireBalls = nullptr;
    mFireBallCount = 5;
    mChildBaseMtx = 0;
    mFrontVec.x = 0.0f;
    mFrontVec.y = 0.0f;
    mFrontVec.z = 1.0f;
    mFireBarSpeed = -5.0f;
    mStickCount = 1;
    mStickDistance = 140.0f;
    mIsStopped = false;
    mIsSpherical = false;
    mSphericalSpeed = 150.0f;
}

void FireBar::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "FireBarCore", false);

    const char* pObjName = nullptr;
    MR::getObjectName(&pObjName, rIter);
    if (MR::isEqualString(pObjName, "FireBarSpherical")) {
        mIsSpherical = true;
    }

    f32 shadowDropDist = 100.0f;
    MR::getJMapInfoArg6NoInit(rIter, &shadowDropDist);
    MR::getJMapInfoArg3NoInit(rIter, &shadowDropDist);
    shadowDropDist -= 55.0f;

    if (shadowDropDist > 0.0f) {
        TVec3f up_vec;
        MR::calcUpVec(&up_vec, this);
        TVec3f drop_pos;
        drop_pos.scaleAdd(50.0f, up_vec, mPosition);
        MR::setShadowDropPosition(this, nullptr, drop_pos);
        MR::setShadowVolumeStartDropOffset(this, nullptr, 100.0f);
        MR::setShadowDropLength(this, nullptr, shadowDropDist);

        s32 rotateShadow = -1;
        MR::getJMapInfoArg4NoInit(rIter, &rotateShadow);

        if (rotateShadow == 0) {
            TVec3f drop_direction;
            MR::calcUpVec(&drop_direction, this);
            drop_direction.negate();
            MR::setShadowDropDirection(this, nullptr, drop_direction);
        }
    } else {
        MR::invalidateShadowAll(this);
    }

    s32 numFireBalls = 5;
    f32 fireBallSpeed = -5.0f;
    MR::getJMapInfoArg0NoInit(rIter, &numFireBalls);
    MR::getJMapInfoArg1NoInit(rIter, &fireBallSpeed);
    MR::getJMapInfoArg5NoInit(rIter, &mStickDistance);
    MR::getJMapInfoArg7NoInit(rIter, &mStickCount);

    mFireBarSpeed = 0.1f * fireBallSpeed;
    mFireBallCount = mStickCount * numFireBalls;

    MR::joinToGroupArray(this, rIter, mName, 0x10);
    MR::calcFrontVec(&mFrontVec, this);

    initFireBarBall(rIter);

    s32 numChildObj = MR::getChildObjNum(rIter);
    if (numChildObj > 0) {
        mChildBaseMtx = new TPos3f();
        PSMTXIdentity(mChildBaseMtx->toMtxPtr());
    }

    for (s32 i = 0; i < numChildObj; i++) {
        const char* pChildName = nullptr;
        MR::getChildObjName(&pChildName, rIter, i);

        PartsModel* pParts = new PartsModel(this, MR::getChildObjJapaneseName(rIter, i), pChildName, mChildBaseMtx->mMtx, -1, false);
        pParts->initWithoutIter();
    }

    MR::setClippingTypeSphere(this, (mFireBallCount + 1) * 100.0f);
    MR::addEffect(this, "PointerTouch");
    initNerve(GET_NERVE(FireBar, FireBarNrvWait));
    MR::useStageSwitchSyncAppear(this, rIter);
}

void FireBar::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
    }

    MR::startActionSound(this, "OjLvFireBarCenter", -1, -1, -1);

    if (!mIsStopped) {
        TVec3f up_vec;
        MR::calcUpVec(&up_vec, this);
        MR::rotateVecDegree(&mFrontVec, up_vec, mFireBarSpeed);
        fixFireBarBall();

        if (isAnyFireBallInNerveB()) {
            MR::sendMsgToGroupMember(0x7B, this, getSensor("Body"), "Body");
            setNerve(GET_NERVE(FireBar, FireBarNrvC));
        }
    }
}

void FireBar::exeNerveC() {
    if (!isAnyFireBallInNerveB()) {
        MR::sendMsgToGroupMember(0x7A, this, getSensor("Body"), "Body");
        setNerve(GET_NERVE(FireBar, FireBarNrvWait));
    }
}

bool FireBar::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0x7B) {
        mIsStopped = true;
        return true;
    }
    if (msg == 0x7A) {
        mIsStopped = false;
        return true;
    }
    return false;
}

bool FireBar::isAnyFireBallInNerveB() const {
    for (s32 i = 0; i < mFireBallCount; i++) {
        if (mFireBalls[i]->isNerve(GET_NERVE_DIRECT(NrvFireBar, FireBarNrvB))) {
            return true;
        }
    }
    return false;
}

void FireBar::makeActorAppeared() {
    for (s32 i = 0; i < mFireBallCount; i++) {
        mFireBalls[i]->makeActorAppeared();
    }

    LiveActor::makeActorAppeared();
}

void FireBar::makeActorDead() {
    MR::tryDeleteEffect(this, "PointerTouch");

    for (s32 i = 0; i < mFireBallCount; i++) {
        mFireBalls[i]->makeActorDead();
    }

    LiveActor::makeActorDead();
}

void FireBar::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrRide(pReceiver) && MR::isSensorEnemyAttack(pSender)) {
        MR::sendMsgEnemyAttackFire(pReceiver, pSender);
    }
}

void FireBar::updateHitSensor(HitSensor* pSensor) {
    if (mIsSpherical) {
        setSensorToNearestFireBall(pSensor);
        return;
    }

    f32 minDistance = FLOAT_MAX;

    TVec3f position = mPosition;

    s32 fireBallCount = mFireBallCount / mStickCount;

    TVec3f up;
    MR::calcUpVec(&up, this);

    TVec3f start;
    start.scale(mStickDistance, mFrontVec);
    start.scaleAdd(50.0f, up, start);

    TVec3f end;
    end.scale(mStickDistance + 100.0f * (fireBallCount - 1), mFrontVec);
    end.scaleAdd(50.0f, up, end);

    for (s32 i = 0; i < mStickCount; i++) {
        TVec3f startWorld;
        startWorld.add(start, *getPosition());
        TVec3f endWorld;
        endWorld.add(end, *getPosition());

        TVec3f footPos;
        MR::calcPerpendicFootToLineInside(&footPos, *MR::getPlayerCenterPos(), startWorld, endWorld);

        f32 distance = footPos.distance(*MR::getPlayerCenterPos());

        if (minDistance > distance) {
            position.set(footPos);
            minDistance = distance;
        }

        if (i + 1 != mStickCount) {
            TVec3f up;
            MR::calcUpVec(&up, this);
            MR::rotateVecDegree(&start, up, 360.0f / mStickCount);
            MR::rotateVecDegree(&end, up, 360.0f / mStickCount);
        }
    }

    MR::setSensorPos(pSensor, position);
}

void FireBar::setSensorToNearestFireBall(HitSensor* pSensor) {
    if (mFireBalls == nullptr) {
        MR::setSensorPos(pSensor, mPosition);
        return;
    }

    f32 minDistance = 0.0f;
    s32 nearest = 0;
    TVec3f unusedPos(mPosition);

    for (s32 i = 0; i < mFireBallCount; i++) {
        f32 distance = MR::calcDistanceToPlayer(mFireBalls[i]);

        if (distance < minDistance || i == 0) {
            minDistance = distance;
            nearest = i;
        }
    }

    MR::setSensorPos(pSensor, mFireBalls[nearest]->mPosition);
}

void FireBar::initFireBarBall(const JMapInfoIter& rIter) {
    mFireBalls = new FireBarBall*[mFireBallCount];
    s32 totalNum = mFireBallCount / mStickCount;
    f32 startVal = 0.0f;

    for (s32 i = 0; i < mFireBallCount; i++) {
        mFireBalls[i] = new FireBarBall(this);
        mFireBalls[i]->init(rIter);
        MR::startBtk(mFireBalls[i], "FireBarBall");

        f32 frame = (i % totalNum == 0) ? 0.0f : 25.0f + startVal;
        startVal = static_cast< s32 >(frame) % static_cast< s32 >(MR::getBtkFrameMax(mFireBalls[i]));
        MR::setBtkFrame(mFireBalls[i], startVal);
    }

    fixFireBarBall();
}

void FireBar::fixFireBarBall() {
    TVec3f scaled;
    scaled.scale(100.0f, mFrontVec);

    TVec3f final_pos;
    final_pos.z = 0.0f;
    final_pos.y = 0.0f;
    final_pos.x = 0.0f;

    s32 totalNum = mFireBallCount / mStickCount;
    TVec3f prev = scaled;

    for (s32 i = 0; i < mFireBallCount; i++) {
        if (i % totalNum == 0) {
            TVec3f up_vec;
            MR::calcUpVec(&up_vec, this);
            MR::rotateVecDegree(&scaled, up_vec, 360.0f / mStickCount);
            TVec3f norm;
            MR::normalize(&norm, scaled);
            final_pos.scaleAdd(mStickDistance, norm, mPosition);
            final_pos.scaleAdd(50.0f, up_vec, final_pos);
            prev.set(scaled);
        } else if (mIsSpherical) {
            TVec3f up_vec;
            MR::calcUpVec(&up_vec, this);
            TVec3f cross;
            PSVECCrossProduct(&up_vec, &prev, &cross);
            MR::rotateVecDegree(&prev, cross, mSphericalSpeed / 100.0f);
            final_pos.add(prev);
        } else {
            final_pos.add(scaled);
        }

        MR::setPosition(mFireBalls[i], final_pos);
    }

    if (mChildBaseMtx != nullptr) {
        TVec3f up_vec(0.0f);
        MR::calcUpVec(&up_vec, this);
        MR::makeMtxFrontUpPos(mChildBaseMtx, mFrontVec, up_vec, mPosition);
    }
}
