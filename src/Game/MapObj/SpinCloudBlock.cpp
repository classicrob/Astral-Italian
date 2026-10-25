#include "Game/MapObj/PlayerFollowPass.hpp"
#include "Game/MapObj/SpinCloudBlock.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/AreaObj/SpinSwitchArea.hpp"
#include "Game/Boss/KoopaJrCastle.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"


namespace MR {
    void getAreaFrontVec(TVec3f*, const AreaObj*, const TVec3f&);
}

namespace {
    const f32 cBlockScales[5] = {0.7f, 0.85f, 0.7f, 0.5f, 0.5f};
}  // namespace

// ---- nerves (sinit order: Wait, Transform, Move, Disperse, Disappear) ----

class SpinCloudBlockNrvWait;
class SpinCloudBlockNrvTransform;
class SpinCloudBlockNrvMove;
class SpinCloudBlockNrvDisperse;
class SpinCloudBlockNrvDisappear;

class SpinCloudBlockNrvWait : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SpinCloudBlock* pBlock = reinterpret_cast<SpinCloudBlock*>(pSpine->mExecutor);
        pBlock->exeWait();
    }
    static SpinCloudBlockNrvWait sInstance;
};
SpinCloudBlockNrvWait SpinCloudBlockNrvWait::sInstance ATTRIBUTE_WEAK;

class SpinCloudBlockNrvTransform : public Nerve {
public:
    virtual void execute(Spine* pSpine) const;
    static SpinCloudBlockNrvTransform sInstance;
};
SpinCloudBlockNrvTransform SpinCloudBlockNrvTransform::sInstance ATTRIBUTE_WEAK;

class SpinCloudBlockNrvMove : public Nerve {
public:
    virtual void execute(Spine* pSpine) const;
    static SpinCloudBlockNrvMove sInstance;
};
SpinCloudBlockNrvMove SpinCloudBlockNrvMove::sInstance ATTRIBUTE_WEAK;

class SpinCloudBlockNrvDisperse : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SpinCloudBlock* pBlock = reinterpret_cast<SpinCloudBlock*>(pSpine->mExecutor);
        pBlock->exeDisperse();
    }
    static SpinCloudBlockNrvDisperse sInstance;
};
SpinCloudBlockNrvDisperse SpinCloudBlockNrvDisperse::sInstance ATTRIBUTE_WEAK;

class SpinCloudBlockNrvDisappear : public Nerve {
public:
    virtual void execute(Spine* pSpine) const {
        SpinCloudBlock* pBlock = reinterpret_cast<SpinCloudBlock*>(pSpine->mExecutor);
        pBlock->exeDisappear();
    }
    static SpinCloudBlockNrvDisappear sInstance;
};
SpinCloudBlockNrvDisappear SpinCloudBlockNrvDisappear::sInstance ATTRIBUTE_WEAK;

void SpinCloudBlockNrvTransform::execute(Spine* pSpine) const {
    SpinCloudBlock* pBlock = reinterpret_cast<SpinCloudBlock*>(pSpine->mExecutor);
    if (pBlock->exeTransform()) {
        pBlock->setNerve(&SpinCloudBlockNrvMove::sInstance);
    }
}

void SpinCloudBlockNrvMove::execute(Spine* pSpine) const {
    SpinCloudBlock* pBlock = reinterpret_cast<SpinCloudBlock*>(pSpine->mExecutor);
    if (pBlock->exeMove()) {
        pBlock->setNerve(&SpinCloudBlockNrvDisperse::sInstance);
    }
}

// ---- SpinCloudBlock ----

SpinCloudBlock::SpinCloudBlock(const char* pName) : LiveActor(pName) {
    mSpring = nullptr;
    mQuat.x = 0.0f;
    mQuat.y = 0.0f;
    mQuat.z = 0.0f;
    mQuat.w = 1.0f;
    mSavedPos.set(0.0f);
    mAnchorPos.set(0.0f);
    mSwayVel.set(0.0f);
    mUnkC8 = 0;
    mPushBack = 0.0f;
    mHitTimer = 0;

    MR::createSwingActionWatcher();
    mSpring = new SpringValue(0.0f, 0.0f, 0.03f, 0.93f, 0.0f);
    MR::createPlayerFollowPass();
}

SpinCloudBlock::~SpinCloudBlock() {
}

void SpinCloudBlock::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "SpinCloudBlock", false);
    MR::invalidateShadow(this, "\x83\x75\x83\x8D\x83\x62\x83\x4E");
    MR::validateShadow(this, "雲");
    MR::invalidateClipping(this);
    MR::resetCollisionPartsBelongZone(this);
    initNerve(&SpinCloudBlockNrvMove::sInstance, 0);
    makeActorDead();
}

void SpinCloudBlock::control() {
    if (mHitTimer > 0) {
        mHitTimer--;
    }
    mSpring->update();
    mSpring->clampValueMax(60.0f);
    if (!MR::isNearZero(mSwayVel, 0.001f)) {
        MR::restrictVelocityToPlaneModeArea(&mSwayVel, mPosition);
        mPosition.add(mSwayVel);
    }
    MR::fn_800145C0(this);
}

// NONMATCHING: quaternion arithmetic scheduling, FPR allocation and temporary stack order.
void SpinCloudBlock::calcAndSetBaseMtx() {
    LiveActor::calcAndSetBaseMtx();

    TPos3f mtx;
    mtx.setQT(mQuat, mPosition);

    f32 springOffset = mSpring->mSpringValue;
    TVec3f offset(mGravity);
    offset *= springOffset;
    MR::addTransMtx(mtx, offset);
    MR::setBaseTRMtx(this, mtx);

    f32 springRatio = mSpring->mSpringValue / 60.0f;
    f32 scale = 1.0f - 0.25f * springRatio;
    if (scale < 0.05f) {
        scale = 0.05f;
    } else if (scale > 2.0f) {
        scale = 2.0f;
    }

    TVec3f baseScale(mScale);
    f32 inv = 1.0f / scale;
    f32 root = MR::fastSqrtf(inv);
    baseScale.mul(TVec3f(root, scale, root));
    MR::setBaseScale(this, baseScale);
}

bool SpinCloudBlock::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Break")) {
        return false;
    }
    if (MR::isMsgPlayerHipDropFloor(msg)) {
        MR::tryHitReactionBreak(this);
        kill();
        return true;
    }
    return false;
}

bool SpinCloudBlock::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isEqualSensor(pReceiver, this, "Break")) {
        return false;
    }
    if (MR::isMsgEnemyAttack(msg) || MR::isMsgExplosionAttack(msg)) {
        MR::tryHitReactionBreak(this);
        kill();
        return true;
    }
    if (MR::isMsgEnemyAttackElectric(msg) &&
        (isNerve(&SpinCloudBlockNrvMove::sInstance) || isNerve(&SpinCloudBlockNrvTransform::sInstance))) {
        setNerve(&SpinCloudBlockNrvDisappear::sInstance);
        return true;
    }
    return false;
}

bool SpinCloudBlock::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xD6) {
        if (MR::isDead(this) || isNerve(&SpinCloudBlockNrvWait::sInstance)) {
            return false;
        }
        MR::invalidateCollisionParts(this);
        MR::tryHitReactionBreak(this);
        kill();
    }

    if (MR::isEqualSensor(pReceiver, this, "Break")) {
        if (msg == 0xF4 && isNerve(&SpinCloudBlockNrvMove::sInstance)) {
            setNerve(&SpinCloudBlockNrvDisappear::sInstance);
            return true;
        }
        return false;
    }

    if (MR::isMsgFloorTouch(msg)) {
        if (MR::isPlayerElementModeBee() || MR::isPlayerElementModeCloud()) {
            if (mHitTimer == 0) {
                mSpring->mVelocity += 8.0f;
            }
            mHitTimer = 10;
        } else {
            MR::tryHitReactionBreak(this);
            kill();
        }
    }
    return false;
}

// NONMATCHING: vector temporary stack slots and load/store scheduling differ.
namespace {
    inline const TVec3f& getSpinCloudGravity(const LiveActor& actor) {
        return actor.mGravity;
    }

    inline TVec3f scaleSpinCloudVector(const TVec3f& vector, f32 scale) {
        TVec3f offset(vector);
        offset *= scale;
        return offset;
    }


    inline TVec3f calcSpinCloudTraceOrigin(const SpinCloudBlock& block, const TVec3f& offset) {
        TVec3f origin(block.mPosition);
        origin -= offset;
        return origin;
    }
}

bool SpinCloudBlock::exeTransform() {
    if (MR::isFirstStep(this)) {
        MR::validateShadow(this, "\x83\x75\x83\x8D\x83\x62\x83\x4E");
        MR::invalidateShadow(this, "雲");
        MR::startAction(this, "Transform");
        MR::zeroVelocity(this);
        MR::validateCollisionParts(this);
        mSwayVel.zero();
        placeAt(mAnchorPos);

        TVec3f hit;
        if (MR::getFirstPolyOnLineToMap(&hit, nullptr,
                calcSpinCloudTraceOrigin(*this, scaleSpinCloudVector(getSpinCloudGravity(*this), 50.0f)),
                scaleSpinCloudVector(getSpinCloudGravity(*this), 210.0f))) {
            f32 dist = 160.0f - PSVECDistance(&hit, &mPosition);
            mPushBack = dist;
            if (mPushBack < 0.0f) {
                mPushBack = 0.0f;
            }
        } else {
            mPushBack = 0.0f;
        }
    } else {
        if (!MR::isOnGroundPlayer()) {
            TVec3f fall;
            MR::calcVecToPlayerHorizon(&fall, this, 0);
            MR::clampLength(&fall, fall, 10.0f);
            mPosition.add(fall);
            if (MR::isGreaterStep(this, 15)) {
                TVec3f velKilled(*MR::getPlayerVelocity());
                MR::vecKillElement(velKilled, mGravity, &velKilled);
                MR::pushPlayer(scaleSpinCloudVector(velKilled, 0.003f));
            }
        }

        if (mPushBack > 0.0f) {
            f32 step = 10.0f;
            if (mPushBack < step) {
                step = mPushBack;
                mPushBack = 0.0f;
            } else {
                mPushBack -= step;
            }
            mPosition.add(scaleSpinCloudVector(getSpinCloudGravity(*this), -step));
        }
    }
    adjustOnPlayer();
    return MR::isActionEnd(this);
}

bool SpinCloudBlock::exeMove() {
    if (MR::isFirstStep(this)) {
        MR::validateShadow(this, "\x83\x75\x83\x8D\x83\x62\x83\x4E");
        MR::invalidateShadow(this, "雲");
        MR::startAction(this, "Wait");
        MR::setScale(this, 1.0f);
    }
    calcWind();
    adjustOnPlayer();
    if (PSVECMag(&mSwayVel) > 6.0f) {
        MR::tryStartActionNotPlaying(this, "Move");
    } else {
        MR::tryStartActionNotPlaying(this, "Wait");
    }
    if (MR::isGreaterStep(this, 300)) {
        return true;
    }
    return false;
}

bool SpinCloudBlock::updateScaleEase(s32 step) {
    calcWind();
    adjustOnPlayer();
    f32 scale = MR::calcNerveEaseInValue(this, step, 1.0f, 0.1f);
    MR::setScale(this, scale);
    return MR::isGreaterStep(this, step);
}

void SpinCloudBlock::calcWind() {
    TVec3f windDir(0.0f);
    f32 windStrength = -1.0f;
    FluffFunction::calcFluffWindInfo(mPosition, &windDir, &windStrength);
    if (windStrength > 0.0f) {
        f32 k = MR::getEaseInOutValue(windStrength, 200.0f, 1300.0f, 0.45f, 0.0f);
        TVec3f force(windDir);
        force.x *= k;
        force.y *= k;
        force.z *= k;
        mSwayVel.add(force);
    }
    if (KoopaJrCastleWindFunction::calcWindInfo(&mPosition, &windDir, &windStrength) != 0) {
        mSwayVel.add(windDir);
    }
    mSwayVel.x *= 0.95f;
    mSwayVel.y *= 0.95f;
    mSwayVel.z *= 0.95f;
}

void SpinCloudBlock::adjustOnPlayer() {
    if (MR::isActorOnPlayer(this)) {
        f32 t = MR::getPlayerCeilingDistance();
        if (!(t >= 160.0f)) {
            f32 dot = mSwayVel.dot(mGravity);
            if (dot < 0.0f) {
                TVec3f cancel(mGravity);
                cancel *= -dot;
                mSwayVel.add(cancel);
            }
            f32 rest = 160.0f - t;
            TVec3f sink(mGravity);
            sink *= rest;
            mPosition.add(sink);
        }
    }
}

void SpinCloudBlock::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::invalidateShadow(this, "\x83\x75\x83\x8D\x83\x62\x83\x4E");
        MR::validateShadow(this, "雲");
        MR::startAction(this, "MiniWait");
    }
    mGravity.set(*MR::getPlayerGravity());
    MR::calcGravityOrZero(this);

    TVec3f up;
    up = -mGravity;
    TVec3f front;
    MR::getPlayerFrontVec(&front);
    MR::blendQuatUpFront(&mQuat, up, front, 0.2f, 0.2f);
    MR::setPosition(this, mAnchorPos);
}

void SpinCloudBlock::exeDisperse() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "Disperse", -1, -1, -1);
    }
    if (updateScaleEase(0x3C)) {
        setNerve(&SpinCloudBlockNrvWait::sInstance);
        MR::tryHitReactionDead(this);
        kill();
    }
}

void SpinCloudBlock::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::emitEffect(this, "Disappear");
    }
    if (updateScaleEase(0xF)) {
        setNerve(&SpinCloudBlockNrvWait::sInstance);
        MR::tryHitReactionBreak(this);
        kill();
    }
}

// NONMATCHING: position and velocity store order and vector temporary stack slots differ.
void SpinCloudBlock::placeAt(const TVec3f& rPos) {
    TQuat4f identity;
    identity.x = 0.0f;
    identity.y = 0.0f;
    identity.z = 0.0f;
    identity.w = 1.0f;
    MR::setPosition(this, rPos);
    MR::calcGravity(this);

    AreaObj* pArea = MR::getAreaObj("SpinCloudFrontDefineArea", rPos);
    if (pArea == nullptr) {
        MR::turnQuatUpToGravity(&mQuat, identity, this);
    } else {
        TVec3f front;
        MR::getAreaFrontVec(&front, pArea, rPos);
        TVec3f up(-mGravity);
        MR::makeQuatUpFront(&mQuat, up, front);
    }
    MR::setScale(this, 1.0f);
    MR::resetPosition(this, rPos);
}

void SpinCloudBlock::spawnAt(const TVec3f& rPos) {
    MR::invalidateShadow(this, "\x83\x75\x83\x8D\x83\x62\x83\x4E");
    MR::validateShadow(this, "雲");
    placeAt(rPos);
    setNerve(&SpinCloudBlockNrvWait::sInstance);
    appear();
    MR::invalidateCollisionParts(this);
    mAnchorPos.set(rPos);
    MR::startAction(this, "MiniWait");
}

void SpinCloudBlock::setAnchorPos(const TVec3f& rPos) {
    mAnchorPos.set<f32>(rPos);
}

bool SpinCloudBlock::tryStartTransform(const TVec3f& rPlayerPos) {
    if (isNerve(&SpinCloudBlockNrvWait::sInstance)) {
        mSavedPos.set<f32>(mPosition);
        MR::setScale(this, 1.0f);
        setNerve(&SpinCloudBlockNrvTransform::sInstance);
        mAnchorPos.set<f32>(rPlayerPos);
        return true;
    }
    return false;
}

static NO_INLINE void killIfWaiting(LiveActor* pActor) {
    if (pActor->isNerve(&SpinCloudBlockNrvWait::sInstance)) {
        pActor->kill();
    }
}

// ---- SpinCloudBlockGroup ----

SpinCloudBlockGroup::SpinCloudBlockGroup() : LiveActorGroup("\x83\x58\x83\x73\x83\x93\x82\xC5\x94\xAD\x90\xB6\x82\xB7\x82\xE9\x89\x5F\x83\x75\x83\x8D\x83\x62\x83\x4E\x95\xDB\x8E\x9D\x8A\xC7\x97\x9D", 0x10) {
    mBlocks = nullptr;
    mBlockNum = 0;
    mRespawnStep = 0;
    mSpinSpeed = 0.0f;

    MR::createSwingActionWatcher();
    MR::createPlayerFollowPass();
    MR::connectToSceneMapObjMovement(this);

    for (s32 i = 0; i < 0x10; i++) {
        SpinCloudBlock* pBlock = new SpinCloudBlock("\x83\x58\x83\x73\x83\x93\x82\xC5\x94\xAD\x90\xB6\x82\xB7\x82\xE9\x89\x5F\x83\x75\x83\x8D\x83\x62\x83\x4E");
        pBlock->initWithoutIter();
        registerActor(pBlock);
    }

    mBlocks = new LiveActor*[10];
    for (s32 i = 0; i < 10; i++) {
        mBlocks[i] = nullptr;
    }
}

#pragma dont_inline on
SpinCloudBlockGroup::~SpinCloudBlockGroup() {
}

#pragma dont_inline reset
void SpinCloudBlockGroup::movement() {
    if (mRespawnStep > 0) {
        mRespawnStep--;
    }
    mSpinSpeed = 0.1f * (100.0f * (f32) mBlockNum) + 0.9f * mSpinSpeed;

    if (mBlockNum > 0) {
        for (s32 i = 0; i < mBlockNum; i++) {
            TVec3f offset;
            MR::calcPlayerFollowPassPosition(&offset, 100.0f * (f32) (i + 1));
            static_cast<SpinCloudBlock*>(mBlocks[i])->setAnchorPos(offset);
        }
        if (mRespawnStep == 0 && MR::isPlayerSwingActionStartNow()) {
            static_cast<SpinCloudBlock*>(mBlocks[0])->tryStartTransform(*MR::getPlayerPos());
            for (s32 i = 0; i < mBlockNum - 1; i++) {
                mBlocks[i] = mBlocks[i + 1];
            }
            mRespawnStep = 20;
            mBlocks[mBlockNum - 1] = nullptr;
            mBlockNum--;
            updateBlockScale();
        }
    }
}

void SpinCloudBlockGroup::updateBlockScale() {
    for (s32 i = 0; i < mBlockNum; i++) {
        MR::setScale(mBlocks[mBlockNum - (i + 1)], cBlockScales[i]);
    }
}

// NONMATCHING: boolean/index registers and vector component store scheduling differ.
bool SpinCloudBlockGroup::spawnBlocks() {
    MR::resetPlayerFollowPass();
    s32 count;
    bool result = false;
    count = 3 - mBlockNum;
    if (count > 0) {
        result = true;
    }
    for (s32 i = 0; i < count; i++) {
        LiveActor* pBlock = getDeadBlock();
        if (pBlock == nullptr) {
            return false;
        }
        TVec3f pos;
        MR::calcPlayerFollowPassPosition(&pos, 60.0f * (f32) (i + 1));
        mBlocks[mBlockNum] = pBlock;
        static_cast<SpinCloudBlock*>(pBlock)->spawnAt(pos);
        mBlockNum++;
    }
    updateBlockScale();
    return result;
}

LiveActor* SpinCloudBlockGroup::getDeadBlock() {
    if (getDeadActor() != nullptr) {
        return getDeadActor();
    }
    return nullptr;
}

void SpinCloudBlockGroup::resetBlocks() {
    s32 num = getObjNum();
    for (s32 i = 0; i < num; i++) {
        killIfWaiting(getActor(i));
    }
    mBlockNum = 0;
}

namespace {
    SpinCloudBlockGroup* getBlockGroup() {
        return static_cast<SpinCloudBlockGroup*>(MR::getSceneObjHolder()->getObj(0x60));
    }
}  // namespace

namespace MR {
    void createSpinCloudBlockGroup() {
        createSceneObj(0x60);
    }

    bool spawnSpinCloudBlocks() {
        return getBlockGroup()->spawnBlocks();
    }

    void resetSpinCloudBlocks() {
        getBlockGroup()->resetBlocks();
    }

    bool isSpinCloudBlockGroupEmpty() {
        return getBlockGroup()->mBlockNum == 0;
    }

    f32 getSpinCloudBlockSpinSpeed() {
        if (!isExistSceneObj(SceneObj_SpinCloudBlockGroup)) {
            return 0.0f;
        }
        return getBlockGroup()->mSpinSpeed;
    }
}  // namespace MR
