#define JGEOMETRY_VEC3_SET_NOINLINE
#include "Game/System/MiniPlayerRocket.hpp"
#include "Game/System/MiniRoutePoint.hpp"
#include "Game/System/MiniStarCheckPoint.hpp"
#include "Game/System/MiniWorldMapMark.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/Effect/PlayerRocketSmokeMoveParticleCallBack.hpp"
#include "Game/Effect/PlayerRocketSmokeWaitParticleCallBack.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/LiveActorGroup.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace {
    class ScatteredStarPieceGroup : public LiveActorGroup {
    public:
        ScatteredStarPieceGroup(const char* pName, s32 num) : LiveActorGroup(pName, num) {}
    };
}

MiniPlayerRocket::MiniPlayerRocket() : LiveActor("プレイヤーロケット") {
    mDirState = 0;
    mMoveSpeed = 0.0f;
    mWaitCount = 0;
    mBindHost = nullptr;
    mStarPieces = nullptr;
    mScaleCtrl = nullptr;
    mScaleParam = nullptr;
    mActionTimer = 0;
    mSmokeMoveCb = nullptr;
    mSmokeWaitCb = nullptr;
    mStarPieceNum = 0;
    mPlayerMtx.identity();
}

// NONMATCHING: regalloc; PlayerRocketSmoke*ParticleCallBack ctor args r5/r6 are unset in the asm (passed as nullptr, 0)
void MiniPlayerRocket::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "MiniPlayerRocket", false);
    initNerve(&NrvMiniPlayerRocket::NrvBind::sInstance, 0);
    MR::addEffectHitNormal(this, "Hit");
    MR::setEffectBaseScale(this, "Hit", 10.0f);

    mStarPieces = new ScatteredStarPieceGroup("ばら撒きスターピースグループ", 10);
    mStarPieces->initWithoutIter();
    for (s32 i = 0; i < 10; i++) {
        LiveActor* pPiece = new ScatteredStarPiece("ばら撒きスターピース[ダメージ用]");
        pPiece->initWithoutIter();
        mStarPieces->registerActor(pPiece);
    }

    mScaleParam = new AnimScaleParam();
    mScaleParam->_0 = 0.5f;
    mScaleParam->_4 = 0.7f;
    mScaleCtrl = new AnimScaleController(mScaleParam);

    mSmokeMoveCb = new PlayerRocketSmokeMoveParticleCallBack(this, nullptr, 0);
    mSmokeWaitCb = new PlayerRocketSmokeWaitParticleCallBack(this, nullptr, 0);

    MR::startAction(this, "Wait");
    MR::startBckNoInterpole(this, "Wait");
    mStarPieceNum = MR::getStarPieceNum();
    makeActorAppeared();
}

void MiniPlayerRocket::appear() {
    LiveActor::appear();
    MR::validateHitSensor(this, "Binder");
}

void MiniPlayerRocket::kill() {
    if (mBindHost != nullptr) {
        mBindHost = nullptr;
        MR::endBindAndPlayerWait(this);
    }
    LiveActor::kill();
}

// NONMATCHING: regalloc
void MiniPlayerRocket::calcAndSetBaseMtx() {
    TVec3f scale = mScaleCtrl->_C * mScale;
    J3DModel* pModel = MR::getJ3DModel(this);
    pModel->mBaseScale.x = scale.x;
    pModel->mBaseScale.y = scale.y;
    pModel->mBaseScale.z = scale.z;
    LiveActor::calcAndSetBaseMtx();
}

void MiniPlayerRocket::resetAction() {
    changeAction("Move");
    mActionTimer = 8;
}

bool MiniPlayerRocket::changeAction(const char* pName) {
    if (MR::tryStartActionNotPlaying(this, pName)) {
        mActionTimer = 0;
        if (MR::isEqualString(pName, "Wait") || MR::isEqualString(pName, "DemoIntoGalaxy") ||
            MR::isEqualString(pName, "TurnEndL") || MR::isEqualString(pName, "TurnEndR")) {
            emitChimneySmoke("ChimneySmokeWait");
            MR::deleteEffect(this, "ChimneySmokeMove");
        } else if (MR::isEqualString(pName, "Move")) {
            emitChimneySmoke("ChimneySmokeMove");
            MR::deleteEffect(this, "ChimneySmokeWait");
        } else {
            MR::deleteEffect(this, "ChimneySmokeMove");
            MR::deleteEffect(this, "ChimneySmokeWait");
        }
        return true;
    }
    return false;
}

bool MiniPlayerRocket::emitChimneySmoke(const char* pName) {
    if (MR::isEqualString(pName, "ChimneySmokeWait")) {
        if (mSmokeWaitCb != nullptr) {
            MR::emitEffectWithParticleCallBack(this, pName, mSmokeWaitCb);
        } else {
            MR::emitEffect(this, pName);
        }
    } else if (MR::isEqualString(pName, "ChimneySmokeMove")) {
        if (mSmokeMoveCb != nullptr) {
            MR::emitEffectWithParticleCallBack(this, pName, mSmokeMoveCb);
        } else {
            MR::emitEffect(this, pName);
        }
    } else {
        MR::emitEffect(this, pName);
    }
    return true;
}

void MiniPlayerRocket::updateChimneySmoke() {
    if (MR::isAnyAnimPlaying(this, "Wait")) {
        if (!MR::isEffectValid(this, "ChimneySmokeWait")) {
            emitChimneySmoke("ChimneySmokeWait");
        }
    } else if (MR::isAnyAnimPlaying(this, "Move")) {
        if (!MR::isEffectValid(this, "ChimneySmokeMove")) {
            emitChimneySmoke("ChimneySmokeMove");
        }
    }
}

void MiniPlayerRocket::control() {
    if (!MR::isHiddenModel(this)) {
        updateChimneySmoke();
        mScaleCtrl->update();
        updateAction();
        updateMove();
    }
}

void MiniPlayerRocket::updateAction() {
    if (mStarPieceNum != MR::getStarPieceNum()) {
        mStarPieceNum = MR::getStarPieceNum();
        MR::startBrk(this, "Flash");
    }

    if (mActionTimer > 0) {
        mActionTimer--;
        if (mActionTimer == 0) {
            if (mDirState == 1) {
                changeAction("TurnEndR");
            } else {
                changeAction("TurnEndL");
            }
            mDirState = 0;
        }
    }

    const char* pName = MR::getPlayingBckName(this);
    const char* names[3] = {"EarthenPipeOut", "TurnEndL", "TurnEndR"};
    if (MR::isExistString(pName, names, 3) && MR::isBckOneTimeAndStopped(this)) {
        changeAction("Wait");
    }
}

// NONMATCHING: regalloc; constructor argument scheduling
void MiniPlayerRocket::updateMove() {
    if (mWaitCount > 0) {
        mWaitCount--;
        return;
    }

    TVec3f up(0.0f, 0.0f, 1.0f);
    updateRotation(up, mMoveSpeed);
    mMoveSpeed = 0.25f;
}

// NONMATCHING: regalloc; vector temporary stack allocation
void MiniPlayerRocket::updateRotation(const TVec3f& rDir, f32 blendRate) {
    if (MR::isNearZero(rDir, 0.001f)) {
        return;
    }

    TVec3f dir;
    MR::normalize(&dir, rDir);

    TQuat4f quat;
    MR::makeQuatRotateDegree(&quat, mRotation);

    // Z axis of the current rotation
    TVec3f front;
    front.set<f32>(2.0f * (quat.x * quat.z) + 2.0f * (quat.w * quat.y),
              2.0f * (quat.y * quat.z) - 2.0f * (quat.w * quat.x),
              1.0f - 2.0f * (quat.x * quat.x) - 2.0f * (quat.y * quat.y));

    if (MR::isOppositeDirection(front, dir, 0.01f)) {
        TVec3f adjust(-0.1f, 0.0f, 0.1f);
        TVec3f sum = dir + adjust;
        MR::normalize(&dir, sum);
    }

    if (blendRate < 1.0f) {
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::blendQuatUpFront(&quat, up, dir, 1.0f, blendRate);
    } else {
        TVec3f up(0.0f, 1.0f, 0.0f);
        MR::makeQuatUpFront(&quat, up, dir);
    }
    MR::setRotation(this, quat);
}

void MiniPlayerRocket::setMoveDirection(const TVec3f& rDir) {
    mWaitCount = 2;
    updateRotation(rDir, mMoveSpeed);
    mMoveSpeed = 0.3f;

    if (rDir.x > 0.1f) {
        mDirState = 1;
    } else if (rDir.x < 0.1f) {
        mDirState = 2;
    }
}

void MiniPlayerRocket::setMoveDirectionInstant(const TVec3f& rDir) {
    mWaitCount = 2;
    updateRotation(rDir, 1.0f);

    if (rDir.x > 0.1f) {
        mDirState = 1;
    } else if (rDir.x < 0.1f) {
        mDirState = 2;
    }
}

void MiniPlayerRocket::disablePlayerBind() {
    setNerve(&NrvMiniPlayerRocket::NrvNone::sInstance);
}

void MiniPlayerRocket::enablePlayerBind() {
    setNerve(&NrvMiniPlayerRocket::NrvBind::sInstance);
}

void MiniPlayerRocket::setPlayerMtx(const f32 (*pMtx)[4]) {
    mPlayerMtx.set(pMtx);
}

void MiniPlayerRocket::startBounce() {
    mScaleCtrl->startScaleVelocityY(-0.1f);
}

void MiniPlayerRocket::updateHitSensor(HitSensor* pSensor) {
    MR::setSensorPos(pSensor, *MR::getPlayerPos());
}

bool MiniPlayerRocket::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgEnemyAttack(msg)) {
        MR::emitEffectHit(this, mPosition, "Hit");
        MR::shakeCameraNormal();
        MR::tryRumblePadMiddle(this, 0);
        MR::stopSceneForDefaultHit(15);
        updateStarPieceScatter(-1);
        return true;
    }
    return false;
}

bool MiniPlayerRocket::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg)) {
        mBindHost = MR::getSensorHost(pSender);
        MR::invalidateHitSensor(this, "Binder");
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        MR::setBaseTRMtx(mBindHost, mPlayerMtx);
        MR::setPlayerBaseMtx(mPlayerMtx);
        return true;
    }

    if (MR::isMsgRushCancel(msg) || MR::isMsgRushForceCancel(msg)) {
        mBindHost = nullptr;
        MR::validateHitSensor(this, "Binder");
        return true;
    }

    return false;
}

// NONMATCHING: regalloc; constructor argument scheduling
bool MiniPlayerRocket::updateStarPieceScatter(s32 count) {
    s32 num = count;
    if (count < 0) {
        num = 10;
    }

    for (s32 i = 0; i < mStarPieces->getObjNum(); i++) {
        if (MR::getStarPieceNum() <= 0) {
            break;
        }

        LiveActor* pPiece = mStarPieces->getActor(i);
        if (MR::isDead(pPiece)) {
            TVec3f up(0.0f, 1.0f, 0.0f);
            static_cast< ScatteredStarPiece* >(pPiece)->scatter(mPosition, up, 5.0f, 100.0f);
            num--;
        }

        if (num <= 0) {
            break;
        }
    }

    return num < count;
}

MiniPlayerRocket::~MiniPlayerRocket() {}

namespace NrvMiniPlayerRocket {
    void NrvBind::execute(Spine* pSpine) const {
        MiniPlayerRocket* pRocket = reinterpret_cast<MiniPlayerRocket*>(pSpine->mExecutor);
        pRocket->mPlayerMtx.set(MR::getJointMtx(pRocket, static_cast<s32>(0)));
        MR::makeMtxWithoutScale(&pRocket->mPlayerMtx, pRocket->mPlayerMtx);
    }

    void NrvNone::execute(Spine*) const {}

    NrvBind NrvBind::sInstance ATTRIBUTE_WEAK;
    NrvNone NrvNone::sInstance ATTRIBUTE_WEAK;
}

// ---- MiniRoutePoint ----

MiniRoutePoint::MiniRoutePoint() : LiveActor("中継点") {
    mNo = -1;
    mIsSmall = false;
    mIsTicoBuild = false;
}

MiniRoutePoint::~MiniRoutePoint() {}

// NONMATCHING: regalloc
void MiniRoutePoint::init(const JMapInfoIter&) {
    MR::initActor(this, "MiniRoutePoint", false);
    MR::setClippingFarMax(this);
    MR::registerDemoSimpleCastAll(this);

    mShadow = new LiveActor("中継点影");
    MR::initActor(mShadow, "MiniRoutePointShadow", false);
    MR::setClippingFarMax(mShadow);
    MR::invalidateClipping(this);
    MR::invalidateClipping(mShadow);
    makeActorDead();
}

void MiniRoutePoint::makeActorAppeared() {
    MR::setScale(this, mIsSmall ? 0.4f : 1.0f);
    mScale.y = 1.0f;
    MR::setScale(mShadow, mScale);
    MR::setRotation(mShadow, mRotation);
    LiveActor::makeActorAppeared();
    mShadow->makeActorAppeared();
}

void MiniRoutePoint::makeActorDead() {
    LiveActor::makeActorDead();
    mShadow->makeActorDead();
}

bool MiniRoutePoint::isOpening() const {
    if (MR::isDead(this)) {
        return false;
    }
    return WorldMapFunction::isPointing(const_cast<MiniRoutePoint*>(this));
}

bool MiniRoutePoint::tryOpen() {
    if (MR::isDead(this)) {
        return false;
    }

    WorldMapFunction::approachScale(this, 0.1f, mIsSmall ? 0.7f : 1.2f);
    mScale.y = 1.0f;
    return true;
}

// NONMATCHING: regalloc and store order
void MiniRoutePoint::setPosition(const TVec3f& rPos) {
    MR::setPosition(this, rPos);

    TVec3f shadowPos;
    shadowPos.x = rPos.x;
    shadowPos.y = -700.0f + rPos.y;
    shadowPos.z = rPos.z;
    MR::setPosition(mShadow, shadowPos);
}

// NONMATCHING: regalloc
void MiniRoutePoint::setReachable(bool isReachable) {
    if (isReachable) {
        MR::startAction(this, "Open");
        if (mIsTicoBuild) {
            MR::startBrk(this, "TicoBuild");
        } else {
            MR::startBrk(this, "Normal");
        }
    } else {
        MR::startAction(this, "Close");
    }
}

bool MiniRoutePoint::isStarting() const {
    return MR::isAnyAnimPlaying(this, "Open");
}

bool MiniRoutePoint::start() {
    MR::startActionSoundOneShot(this, "RoadExtend", -1, -1, -1);
    return true;
}

bool MiniRoutePoint::startOpenSound() {
    MR::startActionSoundOneShot(this, "PointOn", -1, -1, -1);
    return true;
}

bool MiniRoutePoint::startCloseSound() {
    MR::startActionSoundOneShot(this, "PointOff", -1, -1, -1);
    return true;
}

void MiniRoutePoint::control() {
    WorldMapFunction::returnScale(this, 0.05f, mIsSmall ? 0.4f : 1.0f);
    mScale.y = 1.0f;
    MR::setScale(mShadow, mScale);
    MR::setRotation(mShadow, mRotation);
}

// ---- MiniWorldMapMark ----

MiniWorldMapMark::MiniWorldMapMark(LiveActor* pOwner, const char* pName) : LiveActor("ミニ跡地"), mOwner(pOwner), mName(pName) {}

void MiniWorldMapMark::init(const JMapInfoIter&) {
    if (!MR::initActor(this, mName, false)) {
        MR::connectToSceneWorldMapMiniObj(this);
    }
    MR::setClippingFarMax(this);
    MR::setClippingTypeSphereContainsModelBoundingBox(this, 100.0f);
    MR::copyTransRotateScale(mOwner, this);
    makeActorDead();
}

MiniWorldMapMark::~MiniWorldMapMark() {}

bool isStarCheckPointGrand(const char* pName) {
    return MR::isEqualString(pName, "StarCheckPointGrand");
}
