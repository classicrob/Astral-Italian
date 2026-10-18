#include "Game/Player/MarioAnimator.hpp"
#include "Game/Util.hpp"
#include "Game/Animation/XanimeCore.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Player/J3DModelX.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioState.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/JMath/JMATrigonometric.hpp>
#include <cstring>
#include "Game/Util/HashUtil.hpp"
#include "Game/Player/Yoshi.hpp"

inline Mario* MarioAnimator::getPlayer() const { return mActor->getMario(); }

// ---- enum names missing from the SMG2 ActorSensorUtil.hpp ----
static const s32 ACTMES_STAR_PIECE_GIFT_1 = 0x10;  // Petari value, SMG2 unverified


struct MarioAnimationCallback {
    const char* mAnimation;
    s32 mType;
    void (MarioAnimator::*mEntry)();
    void (MarioAnimator::*mUpdate)();
    void (MarioAnimator::*mClose)();
    u32 _2C;
};

MarioAnimationCallback marioCallbackTable[] = {
    {"空中ひねり", 0, &MarioAnimator::spinEntry, &MarioAnimator::spinUpdate, &MarioAnimator::spinClose, 0},
    {"地上ひねり", 0, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"アイスひねり", 1, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"アイスひねり静止", 1, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ファイアスピン", 2, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ファイアスピン空中", 2, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ハチスピン", 3, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ハチスピン空中", 3, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"雲スピン空中", 0, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"雲スピン", 0, &MarioAnimator::spinEntry, nullptr, &MarioAnimator::spinClose, 0},
    {"ステージインA", 0, nullptr, &MarioAnimator::stageInCheck, nullptr, 0},
    {"投げ", 0, &MarioAnimator::throwEntry, &MarioAnimator::throwCheck, &MarioAnimator::throwClose, 0},
    {"ファイア投げ", 1, &MarioAnimator::throwEntry, nullptr, &MarioAnimator::throwClose, 0},
    {"サマーソルト", 0, nullptr, &MarioAnimator::squatSpinCheck, nullptr, 0},
    {"ウォークイン", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"見る", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"ResultWait", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"ResultWaitGrandStar", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"WatchUpMore", 0, nullptr, nullptr, &MarioAnimator::walkinClose, 0},
    {"", 0, nullptr, nullptr, nullptr, 0},
};

void MarioAnimator::initCallbackTable() {
    u32 count = 0;
    MarioAnimationCallback* callback = marioCallbackTable;
    for (;; count++, callback++) {
        if (!*callback->mAnimation) {
            break;
        }
    }

    mCallbackTable = new HashSortTable(count);
    callback = marioCallbackTable;
    for (u32 i = 0; i < count; callback++, i++) {
        mCallbackTable->add(callback->mAnimation, i, false);
    }

    mCallbackTable->sort();
    mCallbackId = -1;
    mCallbackEnded = false;
}

void MarioAnimator::entryCallback(const char* pName) {
    mCallbackEnded = false;
    closeCallback();
    u32 index;
    if (mCallbackTable->search(pName, &index)) {
        mCallbackId = index;
        if (marioCallbackTable[mCallbackId].mEntry != nullptr) {
            (this->*marioCallbackTable[mCallbackId].mEntry)();
        }
    }
}

void MarioAnimator::runningCallback() {
    if (mCallbackId == -1) {
        return;
    }

    mCallbackEnded = true;
    if (isAnimationStop() || isAnimationTerminate(nullptr)) {
        closeCallback();
        return;
    }

    mCallbackEnded = false;
    if (!isAnimationRun(marioCallbackTable[mCallbackId].mAnimation)) {
        closeCallback();
        return;
    }

    if (marioCallbackTable[mCallbackId].mUpdate != nullptr) {
        (this->*marioCallbackTable[mCallbackId].mUpdate)();
    }
}

void MarioAnimator::closeCallback() {
    if (mCallbackId != -1 && marioCallbackTable[mCallbackId].mClose != nullptr) {
        (this->*marioCallbackTable[mCallbackId].mClose)();
    }

    mCallbackId = -1;
}

void MarioAnimator::spinEntry() {
    switch (marioCallbackTable[mCallbackId].mType) {
    case 0:
        if (getPlayer()->isPlayerModeCloud()) mActor->playEffect("雲スピン");
        else mActor->playEffect("スピンライト");
        break;
    case 1:
        mActor->playEffect("アイススピン");
        break;
    case 2:
        mActor->playEffect("ファイアスピン");
        break;
    case 3:
        if (gIsLuigi) {
            mActor->playEffect("ハチルイージスピン");
        } else {
            mActor->playEffect("ハチスピン");
        }

        break;
    }
}

void MarioAnimator::spinUpdate() {
    if (getFrame() > 30.0f) {
        getPlayer()->MarioModule::stopEffect("スピンライト");
    }
}

void MarioAnimator::spinClose() {
    switch (marioCallbackTable[mCallbackId].mType) {
    case 0:
        if (getPlayer()->isPlayerModeCloud()) mActor->stopEffect("雲スピン");
        else mActor->stopEffect("スピンライト");
        break;
    case 1:
        mActor->stopEffect("アイススピン");
        break;
    case 2:
        mActor->stopEffect("ファイアスピン");
        break;
    case 3:
        if (gIsLuigi) {
            mActor->stopEffect("ハチルイージスピン");
        } else {
            mActor->stopEffect("ハチスピン");
        }

        break;
    }
}

void MarioAnimator::stageInCheck() {
    if (static_cast< s32 >(getFrame()) == 50) {
        Mario* player = getPlayer();
        getPlayer()->MarioModule::playEffectRT("属性ステージイン", player->_368, getPlayer()->MarioModule::getTrans());
    }
}

void MarioAnimator::throwCheck() {
    if (mActor->_38C == 0 && getPlayer()->MarioModule::getStickP() && !getPlayer()->mMovementStates.jumping) {
        stopAnimation(nullptr, nullptr);
    }
}

void MarioAnimator::throwEntry() {
    switch (marioCallbackTable[mCallbackId].mType) {
    case 0:
        getPlayer()->MarioModule::playEffect("こうら投げ");
        break;
    case 1:
        getPlayer()->MarioModule::playEffect("ファイアボール投げ");
        break;
    }
}

void MarioAnimator::throwClose() {
    switch (marioCallbackTable[mCallbackId].mType) {
    case 0:
        getPlayer()->MarioModule::stopEffect("こうら投げ");
        break;
    case 1:
        getPlayer()->MarioModule::stopEffect("ファイアボール投げ");
        break;
    }
}

void MarioAnimator::squatSpinCheck() {
    if (!getPlayer()->mMovementStates._A && getFrame() >= 40.0f) {
        stopAnimation(nullptr, nullptr);
    }
}

void MarioAnimator::walkinClose() {
    if (mCallbackEnded) {
        stopAnimation(nullptr, nullptr);
        getPlayer()->changeAnimationInterpoleFrame(16);
    }
}
extern XanimeBckTable4 quadAnimeTable[];
extern XanimeBckTable3 tripleAnimeTable[];
extern XanimeBckTable2 doubleAnimeTable[];
extern XanimeBckTable1 singleAnimeTable[];
extern XanimeGroupInfo marioAnimeTable[];
extern XanimeAuxInfo marioAnimeAuxTable[];
extern XanimeOfsInfo marioAnimeOfsTable[];
extern XanimeSwapTable luigiAnimeSwapTable[];
extern XanimeGroupInfo marioRideAnimeTable[];
extern XanimeAuxInfo marioRideAnimeAuxTable[];
extern XanimeOfsInfo marioRideAnimeOfsTable[];
extern XanimeBckTable1 marioRideSingleAnimeTable[];
extern XanimeBckTable2 marioRideDoubleAnimeTable[];
extern XanimeBckTable3 marioRideTripleAnimeTable[];

MarioAnimator::MarioAnimator(MarioActor* pActor) : mActor(pActor), mExtraAnimationEnabled(false) {
    init();
}

void MarioAnimator::stopAnimation(const char* name, const char* defaultName) {
    if (defaultName) changeDefault(defaultName);
    if (name) mXanimePlayer->stopAnimation(name);
    else mXanimePlayer->stopAnimation();
    if (getPlayer()->isPlayerModeYoshi()) {
        mActor->fn_803B97F0()->fn_8040EDE0(name);
        if (name) mXanimePlayerExtra->stopAnimation(name);
        else mXanimePlayerExtra->stopAnimation();
    }
}

void MarioAnimator::init() {
    XanimeSwapTable* luigiAnimations = nullptr;
    if (gIsLuigi) {
        luigiAnimations = luigiAnimeSwapTable;
    }

    mResourceTable = new XanimeResourceTable(MR::getResourceHolder(mActor), marioAnimeTable, marioAnimeAuxTable, marioAnimeOfsTable,
                                             reinterpret_cast< XanimeBckTable* >(singleAnimeTable), doubleAnimeTable, tripleAnimeTable,
                                             luigiAnimations);

    _15 = 0;
    _14 = 0;
    _16 = 0;

    _18 = 0.0f;
    _1C = 0.0f;
    _20 = 0.0f;
    _24 = 1.0f;

    PSMTXIdentity(_28.toMtxPtr());
    PSMTXIdentity(_7C.toMtxPtr());
    PSMTXIdentity(_AC.toMtxPtr());

    _58 = 0.0f;
    _5C = 0.0f;
    _60.zero();

    _6C = false;
    _10C = 0;
    _10D = 0;
    mUpperDefaultSet = false;
    mCurrBck = nullptr;
    _118 = 0.0f;
    for (s32 index = 0; index < 32; ++index) mAnimationFlags[index] = 0;
    _70 = 0.0f;
    _74 = 0;
    _78 = 0;
    _110 = 0.0f;

    initCallbackTable();

    mXanimePlayer = new XanimePlayer(MR::getJ3DModel(mActor), mResourceTable);

    changeDefault("基本");
    change("基本");

    mXanimePlayer->getCore()->enableJointTransform(MR::getJ3DModelData(mActor));

    mActor->mModelManager->mXanimePlayer = mXanimePlayer;
    mXanimePlayerUpper = new XanimePlayer(MR::getJ3DModel(mActor), mResourceTable, mXanimePlayer);
    changeDefaultUpper("基本");
    mXanimePlayerUpper->changeAnimationBck("基本");
    mXanimePlayerUpper->mCore->shareJointTransform(mXanimePlayer->mCore);
    XanimeResourceTable* rideResources = new XanimeResourceTable(MR::getResourceHolder(mActor), marioRideAnimeTable,
        marioRideAnimeAuxTable, marioRideAnimeOfsTable, reinterpret_cast<XanimeBckTable*>(marioRideSingleAnimeTable),
        marioRideDoubleAnimeTable, marioRideTripleAnimeTable, nullptr);
    mXanimePlayerExtra = new XanimePlayer(MR::getJ3DModel(mActor), rideResources);
    mXanimePlayerExtra->setDefaultAnimation("基本");
    mXanimePlayerExtra->changeAnimationBck("基本");
    mXanimePlayerExtra->mCore->enableJointTransform(MR::getJ3DModelData(mActor));
    PSMTXCopy(MR::tmpMtxRotYRad(PI), _DC.toMtxPtr());
}

bool MarioAnimator::isAnimationStop() const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->fn_803B97F0()->fn_8040F570();
    return mXanimePlayer->mCurrentAnimation == mXanimePlayer->mDefaultAnimation;
}

#pragma dont_inline on
void MarioAnimator::targetWeight(f32* weight, f32 target, f32 rate) {
    *weight = *weight * (1.0f - rate) + target * rate;
}
#pragma dont_inline reset

void MarioAnimator::setWalkWeight(const f32* pWeights) {
    f32 rate = mActor->mConst->getTable()->mWeightBlendRatio;
    if (getPlayer()->_3FA != 0) {
        rate = 0.5f;
    }

    setBlendWeight(pWeights, rate);
}

void MarioAnimator::setBlendWeight(const f32* pWeights, f32 rate) {
    if (getPlayer()->isSwimming()) rate = mActor->mConst->getTable()->mWeightBlendRatioSwim;
    else if (getPlayer()->_3FA != 0) rate = 0.5f;
    targetWeight(&_18, pWeights[0], rate);
    targetWeight(&_1C, pWeights[1], rate);
    targetWeight(&_20, pWeights[2], rate);
    targetWeight(&_24, pWeights[3], rate);
    for (u32 index = 0; index < 4; ++index) {
        mXanimePlayer->changeTrackWeight(index, (&_18)[index]);
        if (getPlayer()->isPlayerModeYoshi()) {
            getYoshiAnimationPlayer()->changeTrackWeight(index, (&_18)[index]);
            mXanimePlayerExtra->changeTrackWeight(index, (&_18)[index]);
        }
    }
}

void MarioAnimator::forceSetBlendWeight(const f32* pWeights) {
    for (u32 index = 0; index < 4; ++index) {
        (&_18)[index] = pWeights[index];
        mXanimePlayer->changeTrackWeight(index, (&_18)[index]);
        if (getPlayer()->isPlayerModeYoshi()) {
            getYoshiAnimationPlayer()->changeTrackWeight(index, (&_18)[index]);
            mXanimePlayerExtra->changeTrackWeight(index, (&_18)[index]);
        }
    }
}

void MarioAnimator::initWalkWeight() {
    _18 = 0.0f;
    _1C = 0.0f;
    _20 = 0.0f;
    _24 = 1.0f;
}

bool MarioAnimator::isLandingAnimationRun() const {
    if (getPlayer()->MarioModule::isPlayerModeTeresa()) {
        return false;
    }

    if (isAnimationRun("着地") || isAnimationRun("着地B") || isAnimationRun("着地C") || isAnimationRun("着地ターン") ||
        isAnimationRun("着地幅とび")) {
        return true;
    }

    if (isAnimationRun("すべり着地")) {
        return true;
    }

    if (isAnimationRun(getPlayer()->_728)) {
        return true;
    }

    if (isAnimationRun("ハード着地")) {
        return true;
    }

    if (isAnimationRun("逆着地")) {
        return true;
    }

    if (isAnimationRun("ショート着地")) {
        return true;
    }

    if (MR::isBckPlaying(mActor, "pulloutgraspend")) {
        return true;
    }

    return false;
}

bool MarioAnimator::isCancelableAnimationRun() const {
    if (isAnimationRun("着地") || isAnimationRun("着地B") || isAnimationRun("着地C") || isAnimationRun("着地ターン") ||
        isAnimationRun("着地幅とび")) {
        return true;
    }

    if (isAnimationRun("すべり着地")) {
        return true;
    }

    if (MR::isBckPlaying(mActor, "pulloutgraspend")) {
        return true;
    }

    return false;
}

bool MarioAnimator::isWalkOrWaitingMotion() const {
    if (isAnimationStop()) {
        if (isAnimationRun("基本") || isAnimationRun("テニス基本")) {
            return true;
        }
    }

    if (isAnimationRun("坂前後ウエイト")) {
        return true;
    }

    if (isAnimationRun("坂左右ウエイト")) {
        return true;
    }

    if (isAnimationRun("カリカリ限界")) {
        return true;
    }

    if (isAnimationRun("飛び込み準備")) {
        return true;
    }

    return false;
}

void MarioAnimator::update() {
    fn_803E0A60();
    runningCallback();
    bool specialMode = false;
    if (mActor->_482 || mActor->_483) specialMode = true;
    if (specialMode) {
        mActor->fn_803BC5C0();
        return;
    }
    if (!mActor->fn_803B9AB0()) {
        fn_803E0AF0();
        fn_803E1060();
        setHoming();
        setHand();
        if (getPlayer()->isPlayerModeYoshi()) mActor->fn_803B97F0()->fn_8040CA10();
        fn_803E0F80();
        updateJointRumble();
        mActor->fn_803BC5C0();
        fn_803E1020();
    }
}

void MarioAnimator::updateWalkBas(const char* pAnimName, f32 speed) {
    bool shouldStart = false;

    // Check if player has a special animation queued
    if (getPlayer()->_970) {
        Mario* player = getPlayer();
        const char* currentBck = mXanimePlayer->getCurrentBckName();

        if (strcmp(currentBck, player->_970) != 0) {
            shouldStart = true;
            if (MR::strcasecmp("DamageWait", getPlayer()->_970) == 0) {
                if (mActor->mHealth == 1) {
                    shouldStart = false;
                }
            }
        }
    }

    // Check animation state transition
    if (_14 == _15) {
        if (_14 != 0) {
            if (!getPlayer()->fn_803A9690(pAnimName)) {
                goto startBas;
            }
        }

        if (_14 != 0) {
            goto updatePrevState;
        }

        if (!shouldStart) {
            goto updatePrevState;
        }
    }

    // Start or continue animation
startBas:
    switch (_14) {
    case 0:
        getPlayer()->startBas(nullptr, false, 0.0f, 0.0f);
        break;
    default:
        getPlayer()->startBas(pAnimName, false, 0.0f, speed);
        break;
    }

    f32 frame = getFrame();
    getPlayer()->fn_803A96D0(frame);

updatePrevState:
    _15 = _14;
}

void MarioAnimator::setHand() {
    TVec3f scale;
    scale.x = getPlayer()->_3F0;
    scale.y = 1.0f;
    scale.z = 1.0f;
    XanimeCore* core = mXanimePlayer->mCore;
    u8 armR = MR::getJointIndex(mActor, "ArmR1");
    XjointTransform* armRJoint = core->getJointTransform(armR);
    armRJoint->_14 = scale;
    core = mXanimePlayer->mCore;
    u8 armL = MR::getJointIndex(mActor, "ArmL1");
    XjointTransform* armLJoint = core->getJointTransform(armL);
    armLJoint->_14 = scale;
}

extern const char* jname_chest;

void MarioAnimator::setTilt() {
    f32 tiltAngle = 0.0f;

    if (!MR::isNearZero(_60)) {
        f32 frontDot = _60.dot(getPlayer()->MarioModule::getFrontVec());
        tiltAngle = 1.0f - frontDot;

        if (tiltAngle > 0.001f) {
            TVec3f cross = _60.cross(getPlayer()->MarioModule::getFrontVec());

            f32 gravDot = cross.dot(*getPlayer()->getGravityVec());
            if (gravDot < 0.0f) {
                tiltAngle = -tiltAngle;
            }
        } else {
            tiltAngle = 0.0f;
        }
    }

    _60 = getPlayer()->MarioModule::getFrontVec();

    Mario* player = getPlayer();
    MarioConstTable* table = mActor->mConst->getTable();

    f32 speed = player->mWalkSpeed;
    f32 ratio = table->mTiltRatio * speed;
    tiltAngle *= ratio;

    const f32 maxTilt = HALF_PI;
    f32 absTilt = MR::abs(tiltAngle);
    if (absTilt >= maxTilt) {
        tiltAngle *= (maxTilt / absTilt);
    }

    f32 absOld = MR::abs(_58);
    f32 absNew = MR::abs(tiltAngle);

    if (absNew > absOld) {
        _58 = 0.9f * _58 + 0.1f * tiltAngle;
    } else {
        _58 = 0.95f * _58 + 0.05f * tiltAngle;
    }

    table = mActor->mConst->getTable();
    Mario* player2 = getPlayer();
    f32 lookDown = player2->mWalkSpeed * table->mLookDownRatio;

    Mario* player3 = getPlayer();
    f32 vertAngle = -player3->_3F4;
    vertAngle *= PI;
    f32 f4 = 0.9f * _5C + 0.1f * lookDown;
    f32 f5 = vertAngle / 4.0f;
    _5C = 0.97f * f4 + 0.03f * f5;

    TVec3f rotation(0.0f, _58, _5C);
    MR::orderRotateMtx(0, rotation, _28.toMtxPtr());

    XanimeCore* core = mXanimePlayer->mCore;
    u8 chestIdx = MR::getJointIndex(mActor, jname_chest);
    XjointTransform* jt = core->getJointTransform(chestIdx);
    jt->_64 = _28.toMtxPtr();
}

void MarioAnimator::resetTilt() {
    PSMTXIdentity(_28.toMtxPtr());
    _58 = 0.0f;
    _5C = 0.0f;
    _60.zero();
}

static u8 sAnimatorHadLookTarget;

// NONMATCHING: register allocation and joint-name string-pool materialization.
void MarioAnimator::setHoming() {
    f32 angleH = 0.0f;
    s32 hasTarget = 0;
    TVec3f targetPos;
    if (mActor->getLookTargetSensor()) {
        HitSensor* lookTarget = mActor->getLookTargetSensor();
        targetPos = lookTarget->mPosition;
        hasTarget = 1;
        if (sAnimatorHadLookTarget == 0) sAnimatorHadLookTarget = 1;
    } else if (MR::isNormalTalking()) {
        if (sAnimatorHadLookTarget == 1) sAnimatorHadLookTarget = 0;
        LiveActor* talkActor = MR::getTalkingActor();
        if (talkActor != nullptr) {
            hasTarget = 1;
            f32 height = mActor->mActionTable1080->getFaceLookHeight(talkActor->mName);
            targetPos = talkActor->mPosition - *getPlayer()->getGravityVec() * height;
        }
    } else if (sAnimatorHadLookTarget == 1) sAnimatorHadLookTarget = 0;

    if (getPlayer()->isPlayerModeYoshi()) {
        if (MR::isNormalTalking()) {
            mLookTarget.x = targetPos.x;
            mLookTarget.y = targetPos.y;
            mLookTarget.z = targetPos.z;
            if (!mActor->fn_803B97F0()->mHeadCtrl->fn_8041AD70(0)) {
                mActor->fn_803B97F0()->mHeadCtrl->fn_8041AA70(0, &mLookTarget);
            }
        } else mActor->fn_803B97F0()->mHeadCtrl->fn_8041AB50(0);
    }

    if (hasTarget) {
        TVec3f toTarget = targetPos - mActor->_24C;

        Mario* player = getPlayer();
        angleH = MR::vecKillElement(toTarget, player->_1FC, &toTarget);

        f32 dist = toTarget.length();
        f32 vAngle;

        if (dist < 10.0f) {
            vAngle = 0.0f;
        } else {
            f32 dist2 = toTarget.length();
            vAngle = JMath::sAtanTable.atan2_(angleH, dist2);

            if (getPlayer()->MarioModule::isStatusActiveID(MarioStatus_Talk)) {
                if (vAngle > 1.0f) {
                    vAngle = 1.0f;
                }
            } else {
                MarioConstTable* table = mActor->mConst->getTable();
                if (vAngle > table->mLookMaxAngleVP) {
                    vAngle = table->mLookMaxAngleVP;
                }
            }

            MarioConstTable* table = mActor->mConst->getTable();
            f32 lowerLimit = -table->mLookMaxAngleVM;
            if (vAngle < lowerLimit) {
                vAngle = lowerLimit;
            }
        }

        angleH = MR::diffAngleAbs(toTarget, getPlayer()->mFrontVec);

        MarioConstTable* table = mActor->mConst->getTable();
        if (angleH > table->mLookMaxAngleH) {
            angleH = table->mLookMaxAngleH;
        }

        TVec3f cross = toTarget.cross(getPlayer()->mFrontVec);
        Mario* player2 = getPlayer();
        f32 gravDot = cross.dot(player2->_1FC);
        if (gravDot < 0.0f) {
            angleH = -angleH;
        }

        Mtx rotMtx;
        MR::orderRotateMtx(5, TVec3f(0.0f, angleH, -vAngle), rotMtx);

        MR::blendMtx(_7C.toMtxPtr(), rotMtx, 0.1f, _7C.toMtxPtr());

        XanimeCore* core = mXanimePlayer->getCore();
        u8 headIdx = MR::getJointIndex(mActor, "Head");
        XjointTransform* jt = core->getJointTransform(headIdx);
        jt->_64 = _7C.toMtxPtr();
    } else {
        Mtx identMtx;
        PSMTXIdentity(identMtx);
        MR::blendMtx(_7C.toMtxPtr(), identMtx, 0.1f, _7C.toMtxPtr());
    }

    MarioConstTable* table = mActor->mConst->getTable();

    f32 blend = 0.9f * _110 + 0.1f * angleH;
    _110 = blend;

    f32 leftShoulderRot = 0.0f;
    f32 rightShoulderRot = 0.0f;

    if (angleH > 0.0f) {
        table = mActor->mConst->getTable();
        f32 ratio = blend / table->mLookMaxAngleH;
        rightShoulderRot = ratio * table->mLookShoulderMoveMax;
    } else if (angleH < 0.0f) {
        table = mActor->mConst->getTable();
        f32 ratio = -blend / table->mLookMaxAngleH;
        leftShoulderRot = ratio * table->mLookShoulderMoveMax;
    }

    XanimeCore* core = mXanimePlayer->getCore();
    u8 leftIdx = MR::getJointIndex(mActor, "ShoulderL");
    core->getJointTransform(leftIdx)->_2C.x = leftShoulderRot;

    core = mXanimePlayer->getCore();
    u8 rightIdx = MR::getJointIndex(mActor, "ShoulderR");
    core->getJointTransform(rightIdx)->_2C.x = rightShoulderRot;
}

void MarioAnimator::setHipSlidingTilt(f32 stickX, f32 stickY) {
    TVec2f input(stickX, stickY);
    f32 weights[4];

    weights[0] = (TVec2f(0.0f, 1.0f) - input).length();

    weights[1] = (TVec2f(0.0f, -1.0f) - input).length();

    weights[2] = (TVec2f(-1.0f, 0.0f) - input).length();

    weights[3] = (TVec2f(1.0f, 0.0f) - input).length();

    const f32 maxDist = 1.414213f;
    for (s32 i = 0; i < 4; i++) {
        if (weights[i] >= maxDist) {
            weights[i] = 0.0f;
        } else {
            weights[i] = (maxDist - weights[i]) / maxDist;
        }
    }

    for (s32 i = 0; i < 4; i++) {
        f32 w = weights[i];
        f32 w2 = w * w;
        if (w < 0.0f) {
            weights[i] = -w2;
        } else {
            weights[i] = w2;
        }
    }

    MarioConstTable* table = mActor->mConst->getTable();
    f32 speed = table->mSliderTiltRatio;

    setBlendWeight(weights, speed);
}

void MarioAnimator::setHipSliderTilt() {
    TVec3f cameraDirection = MR::getCamZdir();
    if (cameraDirection.dot(getPlayer()->mFrontVec) >= 0.0f) {
        Mario* player = getPlayer();
        f32 stickY = player->getStickY();
        f32 stickX = player->getStickX();
        setHipSlidingTilt(stickX, stickY);
    } else {
        Mario* player = getPlayer();
        f32 stickY = -player->getStickY();
        f32 stickX = -player->getStickX();
        setHipSlidingTilt(stickX, stickY);
    }
}

void MarioAnimator::setHipSlipTilt() {
    TVec3f worldPadDir(getPlayer()->MarioModule::getWorldPadDir());
    f32 frontDot = MR::vecKillElement(worldPadDir, getPlayer()->MarioModule::getFrontVec(), &worldPadDir);
    Mario* player = getPlayer();
    f32 gravDot = MR::vecKillElement(worldPadDir, player->mSideVec, &worldPadDir);
    setHipSlidingTilt(-gravDot, frontDot);
}

void MarioAnimator::setSpeed(f32 speed) {
    mXanimePlayer->changeSpeed(speed);
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeSpeed(speed);
        mXanimePlayerExtra->changeSpeed(speed);
    }
}

f32 MarioAnimator::getFrame() const {
    if (getPlayer()->isPlayerModeYoshi()) return getYoshiAnimationPlayer()->getAnimationFrame();
    return mXanimePlayer->getAnimationFrame();
}

f32 MarioAnimator::getUpperFrame() const {
    return mXanimePlayerUpper->getAnimationFrame();
}

void MarioAnimator::setWalkMode() {
    if (!getPlayer()->mMovementStates._F && getPlayer()->mMovementStates._10 && !getPlayer()->mMovementStates._A && 0.0f == getPlayer()->_8F0 &&
        static_cast< u8 >(getPlayer()->checkStickFrontBack()) == 2) {
        if (!getPlayer()->MarioModule::isStatusActiveID(MarioStatus_Magic) && !getPlayer()->MarioModule::isStatusActiveID(MarioStatus_Skate)) {
            if (getPlayer()->mMovementStates._35) {
                change("ブレーキ滑り床");
            } else {
                change("ブレーキ");
            }
        }
    } else if (isAnimationRun("ブレーキ")) {
        getPlayer()->doBrakingAnimation();
        getPlayer()->mTargetWalkSpeedIndex = 0;
    } else {
        mXanimePlayer->stopAnimation("ブレーキ滑り床");
    }

    if (getPlayer()->mMovementStates._4 && !getPlayer()->mMovementStates._23) {
        getPlayer();
    }
}

void MarioAnimator::calc() {
    bool specialMode = false;
    if (mActor->_482 || mActor->_483) specialMode = true;
    if (!specialMode || getPlayer()->isPlayerModeYoshi()) {
        fn_803E1890();
        fn_803E13D0();
        fn_803E1620();
        fn_803E17C0();
    }
}

#pragma dont_inline on
void MarioAnimator::fn_803E0E40(const char* pName) {
    const XanimeGroupInfo* info = mXanimePlayer->mCurrentAnimation;
    if (info->mAttribute == 2) {
        getPlayer()->startBas(pName, false, info->mLoop, info->mEnd);
    } else {
        getPlayer()->startBas(pName, false, 0.0f, 0.0f);
    }
}
#pragma dont_inline reset

void MarioAnimator::change(const char* pName) {
    if (mActor->_B90) {
        return;
    }

    if (isTeresaClear()) {
        mXanimePlayer->changeAnimationBck(pName);
        if (getPlayer()->isPlayerModeYoshi()) {
            mActor->fn_803B97F0()->fn_8040EDA0(pName);
            if (!mExtraAnimationEnabled) mXanimePlayerExtra->changeAnimationBck(pName);
        }
    }

    const char* bck = mXanimePlayer->getCurrentBckName();
    if (bck != nullptr) {
        fn_803E0E40(bck);
        mActor->fn_803BBFD0(bck);
    } else {
        getPlayer()->startBas(nullptr, false, 0.0f, 0.0f);
        mActor->fn_803BBFD0(nullptr);
    }

    mActor->changeSpecialModeAnimation(pName);
    mCurrBck = bck;
    entryCallback(pName);
}

void MarioAnimator::changeUpper(const char* pName) {
    mXanimePlayerUpper->changeAnimationBck(pName);
    if (getPlayer()->isPlayerModeYoshi()) mActor->fn_803B97F0()->fn_8040EE30(pName);
    _6C = true;
}

// NONMATCHING: stop-condition boolean uses a different register.
void MarioAnimator::stopUpper(const char* pName) {
    if (!_6C) {
        return;
    }

    bool shouldStop = false;
    if (pName == nullptr || mXanimePlayerUpper->isRun(pName)) {
        shouldStop = true;
    }

    if (shouldStop) {
        u16 jointID = MR::getJointIndex(mActor, "Spine1");
        XanimeCore* upperCore = mXanimePlayerUpper->getCore();
        J3DModelData* modelData = MR::getJ3DModelData(mActor);
        XanimeCore* lowerCore = mXanimePlayer->getCore();
        upperCore->freezeCopy(modelData, lowerCore, (u16)jointID, 8);
        mXanimePlayerUpper->stopAnimation();
        if (getPlayer()->isPlayerModeYoshi()) mActor->fn_803B97F0()->fn_8040EE70(pName);
        _6C = false;
    }
}

#pragma dont_inline on
void MarioAnimator::changeDefault(const char* pName) {
    getPlayer()->startBas(nullptr, false, 0.0f, 0.0f);

    mXanimePlayer->setDefaultAnimation(pName);
    if (getPlayer()->isPlayerModeYoshi()) {
        mActor->fn_803B97F0()->fn_8040ED90(pName);
        mXanimePlayerExtra->setDefaultAnimation(pName);
    }
}

#pragma dont_inline reset
bool MarioAnimator::isDefaultAnimationRun(const char* pName) const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->fn_803B97F0()->fn_8040F450(pName);
    return strcmp(mXanimePlayer->getDefaultAnimationName(), pName) == 0;
}

void MarioAnimator::changeDefaultUpper(const char* pName) {
    if (pName != nullptr) {
        mUpperDefaultSet = true;
        mXanimePlayerUpper->setDefaultAnimation(pName);
        if (getPlayer()->isPlayerModeYoshi()) mActor->fn_803B97F0()->fn_8040EE00(pName);
    } else {
        mUpperDefaultSet = false;
    }
}

u16 MarioAnimator::getUpperJointID() const {
    return MR::getJointIndex(mActor, "Spine1");
}

const char* jname_chest = "Spine1";
#pragma push
#pragma opt_propagation off
void MarioAnimator::updateJointRumble() {
    u32 timer = _74;
    if (timer == 0) {
        _118 = 0.0f;
        return;
    }

    u32 newTimer = timer - 1;
    _74 = newTimer;

    f32 t = (f32)newTimer * JMath::TAngleConstant_< f32 >::RADIAN_DEG360() / 30.0f;
    f32 sineVal = JMASinRadian(t);

    f32 power = -_70;

    if (_74 < 30) {
        f32 fade = (f32)_74 / 30.0f;
        sineVal *= fade;
    }

    MarioActor* actor = mActor;
    XanimeCore* core = mXanimePlayer->mCore;
    XjointTransform* larmJt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(actor, "ArmL1")));
    larmJt->_2C.y = sineVal * power;

    core = mXanimePlayer->mCore;
    XjointTransform* rarmJt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "ArmR2")));
    rarmJt->_2C.y = sineVal * power;

    f32 hipRot = 0.7f * ((f32)_74 / 60.0f);

    if (getPlayer()->mMovementStates._1 && !getPlayer()->mMovementStates._37 && getPlayer()->mTargetWalkSpeedIndex == 0 &&
        getPlayer()->calcAngleD(getPlayer()->_368) > 5.0f) {
        TVec3f dir;
        const TVec3f* airGrav = &getPlayer()->getAirGravityVec();
        Mario* playerVec = getPlayer();
        MR::vecKillElement(playerVec->_368, *airGrav, &dir);
        MR::normalizeOrZero(&dir);

        if (getPlayer()->MarioModule::getFrontVec().dot(dir) < -0.707f) {
            hipRot = -hipRot;
        } else {
            hipRot = getPlayer()->MarioModule::getFrontVec().dot(dir) > 0.707f ? hipRot : 0.9f * _118;
        }

        PSMTXRotRad(_AC.toMtxPtr(), 'Z', hipRot);
    } else {
        PSMTXIdentity(_AC.toMtxPtr());
    }

    _118 = hipRot;

    core = mXanimePlayer->mCore;
    XjointTransform* hipJt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "Spine2")));
    hipJt->_64 = _AC.toMtxPtr();
}
#pragma pop

void MarioAnimator::addRumblePower(f32 power, u32 time) {
    if (_70 < power) {
        _70 = power;
    }

    _74 = time;
}

void MarioAnimator::setUpperRotateY(f32 angle) {
    PSMTXRotRad(_AC.toMtxPtr(), 'X', angle);
    XanimeCore* core = mXanimePlayer->mCore;
    XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "Spine2")));
    jt->_64 = _AC.toMtxPtr();
}

void MarioAnimator::clearAllJointTransform() {
    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "ShoulderL")));
        jt->_64 = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "ShoulderR")));
        jt->_64 = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "Head")));
        jt->_64 = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "Spine2")));
        jt->_64 = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "ArmL1")));
        jt->_2C.y = 0.0f;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, "ArmR2")));
        jt->_2C.y = 0.0f;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        u8 chestIdx = MR::getJointIndex(mActor, jname_chest);
        XjointTransform* jt = core->getJointTransform(chestIdx);
        jt->_64 = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        u8 hipIdx = MR::getJointIndex(mActor, "Hip");
        XjointTransform* jt = core->getJointTransform(hipIdx);
        jt->_64 = nullptr;
    }

    TVec3f zeroVec(0.0f, 0.0f, 0.0f);
    XjointTransform* jt = mXanimePlayer->mCore->getJointTransform(0);
    jt->_38 = zeroVec;

    _74 = 0;
    _70 = 0.0f;
}

bool MarioAnimator::isMirrorAnimation() {
    if (mActor->_468) {
        return false;
    }

    TVec3f camDir = getPlayer()->MarioModule::getCamDirX();
    Mario* player = getPlayer();
    f32 dot = player->_1FC.dot(getPlayer()->MarioModule::getCamDirY());
    if (dot < 0.0f) {
        camDir = -camDir;
    }

    if (isAnimationRun("壁すべり") || isAnimationRun("壁くっつき")) {
        if (!_10C) {
            const TVec3f& wallNorm = getPlayer()->getWallNorm();
            f32 wallDot = camDir.dot(wallNorm);
            if (wallDot < -0.5f) {
                _10D = true;
            } else {
                _10D = false;
            }

            _10C = true;
        }

        return _10D;
    }

    // Check if walling (player mMovementStates._37 bit check)
    if (getPlayer()->mMovementStates._37) {
        if (isAnimationRun("ブレーキ") || isAnimationRun("ターンブレーキ") || isAnimationRun("ターンジャンプ")) {
            if (!_10C) {
                f32 frontDot = camDir.dot(getPlayer()->mFrontVec);
                if (frontDot > 0.0f) {
                    _10D = true;
                } else {
                    _10D = false;
                }

                _10C = true;
            }

            return _10D;
        }
    }

    // Clear cache flag
    _10C = false;
    if (isAnimationRun("左空パンチ")) {
        return true;
    }

    return false;
}

void MarioAnimator::switchMirrorMode() {
    f32 scale = 1.0f;
    J3DModelX* model = static_cast< J3DModelX* >(MR::getJ3DModel(mActor));
    if (isMirrorAnimation()) {
        u32* modelFlags = (u32*)&model->mFlags;
        *modelFlags |= 1;
        XjointTransform* jt = mXanimePlayer->mCore->getJointTransform(0);
        TVec3f mirrorScale;
        mirrorScale.x = scale;
        mirrorScale.y = scale;
        mirrorScale.z = -scale;
        jt->_14 = mirrorScale;

        Mtx invBase;
        MtxPtr base = mActor->getBaseMtx();
        PSMTXInverse(base, invBase);
        MR::multMtx(_DC, invBase, MR::tmpMtxRotYRad(PI));

        base = mActor->getBaseMtx();
        MR::multMtx(_DC, _DC, base);

        jt->_6C = _DC;
    } else {
        u32* modelFlags = (u32*)&model->mFlags;
        *modelFlags &= ~1;
        XjointTransform* jt = mXanimePlayer->mCore->getJointTransform(0);
        TVec3f normalScale;
        normalScale.x = scale;
        normalScale.y = scale;
        normalScale.z = scale;
        jt->_14 = normalScale;
        jt->_6C = 0;
    }
}

void MarioAnimator::changePickupAnimation(const HitSensor* pSensor) {
    u32 type = pSensor->mType;
    switch (type) {
    case ACTMES_STAR_PIECE_GIFT_MAX:
        mActor->_494 = mActor->_49C;
        getPlayer()->MarioModule::changeAnimation("カブ抜き");
        mActor->changeNullAnimation("PullOut", -2);
        getPlayer()->stopWalk();
        break;
    case ACTMES_STAR_PIECE_GIFT:
    case ACTMES_STAR_PIECE_GIFT_1:
        mActor->_494 = mActor->_498;
        if (!getPlayer()->isSwimming()) {
            if (mActor->_424 == pSensor) {
                getPlayer()->MarioModule::changeAnimationUpper("ひろいクイック");
                getPlayer()->MarioModule::playEffect("ひろいクイック");
                mActor->clearNullAnimation(-3);
                getPlayer()->MarioModule::startPadVib(2);
            } else if (getPlayer()->mMovementStates.jumping && !getPlayer()->mMovementStates._B) {
                getPlayer()->MarioModule::changeAnimation("ひろい空中");
                mActor->clearNullAnimation(-3);
                getPlayer()->stopWalk();
            } else {
                getPlayer()->MarioModule::changeAnimation("ひろい");
                mActor->changeNullAnimation("CarryStart", -2);
                getPlayer()->stopWalk();
            }
        } else {
            mActor->clearNullAnimation(0);
            getPlayer()->MarioModule::startPadVib(2);
        }

        break;
    }
}

void MarioAnimator::updateTakingAnimation(const HitSensor* pSensor) {
    if (pSensor == nullptr) {
        return;
    }

    LiveActor* sensorActor = pSensor->mHost;
    if (MR::isDead(sensorActor)) {
        if (mActor->_468 != 0) {
            mActor->rushDropThrowMemoSensor();
        }

        return;
    }

    u32 type = pSensor->mType;
    switch (type) {
    case ACTMES_STAR_PIECE_GIFT_MAX:
        stopAnimation(nullptr, nullptr);
        getPlayer()->MarioModule::changeAnimationUpper("カブウエイト");
        mActor->clearNullAnimation(0);
        mActor->offTakingFlag();
        break;
    case ACTMES_STAR_PIECE_GIFT:
    case ACTMES_STAR_PIECE_GIFT_1:
        if (!getPlayer()->isSwimming()) {
            s32 shouldChange = 1;
            if (isAnimationRun("ひろいクイック")) {
                shouldChange = isUpperAnimationTerminated(nullptr);
            }

            if (shouldChange) {
                if (!isAnimationRun("ひろい空中") && mActor->isStopNullAnimation()) {
                    getPlayer()->MarioModule::changeAnimationUpper("ひろいウエイト");
                    getPlayer()->MarioModule::startPadVib(2);
                    mActor->clearNullAnimation(0);
                    mActor->offTakingFlag();
                }
            }
        } else {
            MR::emitEffect(pSensor->mHost, "SwimBubble");
        }

        break;
    }
}

void MarioAnimator::changeThrowAnimation(const HitSensor* pSensor) {
    u32 type = pSensor->mType;
    switch (type) {
    case ACTMES_STAR_PIECE_GIFT_MAX:
        getPlayer()->MarioModule::stopAnimationUpper(nullptr);
        getPlayer()->MarioModule::changeAnimation("両手投げ");
        getPlayer()->MarioModule::startPadVib("マリオ[亀投げ]");
        break;
    case ACTMES_STAR_PIECE_GIFT:
    case ACTMES_STAR_PIECE_GIFT_1:
        getPlayer()->MarioModule::stopAnimationUpper(nullptr);
        if (getPlayer()->isSwimming()) {
            getPlayer()->MarioModule::changeAnimation("水泳亀投げ", "水泳基本");
        } else {
            getPlayer()->MarioModule::changeAnimation("投げ");
        }

        getPlayer()->MarioModule::startPadVib("マリオ[亀投げ]");
        break;
    }
}

void MarioAnimator::waterToGround() {
    HitSensor* swimSensor;
    if (mActor->_468 == 0) {
        swimSensor = nullptr;
    } else {
        swimSensor = mActor->_428[0];
    }

    if (swimSensor == nullptr) {
        return;
    }

    u32 state = swimSensor->mType;
    switch (state) {
    case ACTMES_STAR_PIECE_GIFT:
    case ACTMES_STAR_PIECE_GIFT_1:
        getPlayer()->MarioModule::changeAnimationUpper("ひろいウエイト");
        mActor->clearNullAnimation(0);
        MR::deleteEffect(swimSensor->mHost, "SwimBubble");
        break;
    case ACTMES_STAR_PIECE_GIFT_MAX:
        getPlayer()->MarioModule::changeAnimationUpper("カブウエイト");
        mActor->clearNullAnimation(0);
        break;
    }
}

HitSensor* MarioActor::getLookTargetSensor() const {
    if (_934) {
        return nullptr;
    }

    if (isSleeping()) {
        return nullptr;
    }

    return _46C;
}
void MarioAnimator::setAnimationFlag(u32 index) { mAnimationFlags[index] = 1; }
void MarioAnimator::resetAnimationFlag(u32 index) { mAnimationFlags[index] = 0; }
bool MarioAnimator::isAnimationFlag(u32 index) const { return mAnimationFlags[index] != 0; }
void MarioAnimator::enableExtraAnimation() { mExtraAnimationEnabled = true; }
void MarioAnimator::disableExtraAnimation() { mExtraAnimationEnabled = false; }
void MarioAnimator::changeExtraAnimation(const char* name) {
    if (mExtraAnimationEnabled) mXanimePlayerExtra->changeSimpleBck(name);
}
XanimePlayer* MarioAnimator::fn_803DFBF0() const { return mXanimePlayerExtra; }
const char* MarioAnimator::fn_803DF0D0() const { return mXanimePlayer->getCurrentBckName(); }
f32 MarioAnimator::fn_803DF0E0() const { return mXanimePlayer->_20->mRate; }
const char* MarioAnimator::getUpperAnimationName() const { return mXanimePlayerUpper->getCurrentAnimationName(); }
const char* MarioAnimator::getAnimationStringPointer(const char* name) const { return mXanimePlayer->getNameStringPointer(name); }
bool MarioAnimator::isUpperAnimationTerminated(const char* name) const {
    if (name) return mXanimePlayerUpper->isTerminate(name);
    return mXanimePlayerUpper->isTerminate();
}
f32 MarioAnimator::getWalkWeight(u32 index) const { return (&_18)[index]; }
f32 MarioAnimator::getAnimationInterpoleRatio() const { return mXanimePlayer->_0C; }
bool MarioAnimator::isFrameCtrlAttribute(u32 attribute) const { return mXanimePlayer->_20->getAttribute() == attribute; }

// NONMATCHING: equivalent null-return block ordering.
XanimePlayer* MarioAnimator::getYoshiAnimationPlayer() const {
    XanimePlayer* result;
    if (!mActor->fn_803B97F0()) result = nullptr;
    else result = mActor->fn_803B97F0()->mXanimePlayer;
    return result;
}
// NONMATCHING: equivalent null-return block ordering.
XanimePlayer* MarioAnimator::getYoshiUpperAnimationPlayer() const {
    XanimePlayer* result;
    if (!mActor->fn_803B97F0()) result = nullptr;
    else result = mActor->fn_803B97F0()->mXanimePlayerUpper;
    return result;
}
f32 XanimePlayer::getAnimationFrame() const {
    if (isTerminate()) return _20->mEnd;
    return _20->mFrame;
}
// NONMATCHING: redundant rounding instruction and pointer-load scheduling.
void XanimePlayer::setAnimationFrame(f32 frame) {
    _20->mFrame = _84 = frame;
}
void MarioAnimator::changeTrackAnim(u8 index, const char* name) {
    mXanimePlayer->changeTrackAnimation(index, name);
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeTrackAnimation(index, name);
        mXanimePlayerExtra->changeTrackAnimation(index, name);
    }
}
void MarioAnimator::setAnimationPaused(bool paused) {
    mXanimePlayer->_7E = paused;
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->_7E = paused;
        mXanimePlayerExtra->_7E = paused;
    }
}
void MarioAnimator::setAnimationFrameCtrlAttribute(u8 attribute) {
    mXanimePlayer->_20->mAttribute = attribute;
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->_20->mAttribute = attribute;
        mXanimePlayerExtra->_20->mAttribute = attribute;
    }
}
void MarioAnimator::setAnimationInterpoleFrame(s32 frame) {
    mXanimePlayer->changeInterpoleFrame(frame);
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeInterpoleFrame(frame);
        mXanimePlayerExtra->changeInterpoleFrame(frame);
    }
}
void MarioAnimator::setAnimationFrame(f32 frame) {
    if (mActor->_B90) return;
    mXanimePlayer->setAnimationFrame(frame);
    if (getPlayer()->isPlayerModeYoshi()) {
        mActor->fn_803B97F0()->mXanimePlayer->setAnimationFrame(frame);
        if (!mExtraAnimationEnabled) mXanimePlayerExtra->setAnimationFrame(frame);
    }
}
void MarioAnimator::resetAnimationUpdateFlags() {
    XanimePlayer* player = mXanimePlayer;
    player->_88 = false;
    player->_89 = false;
    player = mXanimePlayerUpper;
    player->_88 = false;
    player->_89 = false;
    if (getPlayer()->isPlayerModeYoshi()) {
        XanimePlayer* player = getYoshiAnimationPlayer();
        player->_88 = false;
        player->_89 = false;
        player = getYoshiUpperAnimationPlayer();
        player->_88 = false;
        player->_89 = false;
        player = mXanimePlayerExtra;
        player->_88 = false;
        player->_89 = false;
    }
}
f32 MarioAnimator::getAnimationTrackWeight(u32 index) {
    if (getPlayer()->isPlayerModeYoshi()) return getYoshiAnimationPlayer()->mWeights[index];
    return mXanimePlayer->mWeights[index];
}

void MarioAnimator::setMainJointMatrix(u32 index, MtxPtr matrix) {
    mXanimePlayer->mCore->getJointTransform(index)->_64 = matrix;
}
void MarioAnimator::setYoshiJointMatrix(const char* joint, MtxPtr matrix) {
    if (getPlayer()->isPlayerModeYoshi()) {
        Yoshi* actor = mActor->fn_803B97F0();
        XanimeCore* core = getYoshiAnimationPlayer()->mCore;
        core->getJointTransform(MR::getJointIndex(actor, joint))->_64 = matrix;
    }
}
void MarioAnimator::setMainJointOffset(u32 index, const TVec3f& offset) {
    mXanimePlayer->mCore->getJointTransform(index)->_2C = offset;
}
void MarioAnimator::setYoshiJointOffset(const char* joint, const TVec3f& offset) {
    if (getPlayer()->isPlayerModeYoshi()) {
        Yoshi* actor = mActor->fn_803B97F0();
        XanimeCore* core = getYoshiAnimationPlayer()->mCore;
        core->getJointTransform(MR::getJointIndex(actor, joint))->_2C = offset;
    }
}
void MarioAnimator::setAnimationModel(J3DModel* model) {
    mXanimePlayer->setModel(model);
    mXanimePlayerUpper->setModel(model);
    if (getPlayer()->isPlayerModeYoshi()) mXanimePlayerExtra->setModel(model);
}
void MarioAnimator::setAnimationInterpoleRatio(f32 ratio) {
    mXanimePlayer->_0C = ratio;
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->_0C = ratio;
        mXanimePlayerExtra->_0C = ratio;
    }
}

bool MarioAnimator::isUpperAnimationActive() const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->fn_803B97F0()->mUpperAnimationActive;
    return _6C;
}
bool MarioAnimator::isAnimationRun(const char* name) const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->fn_803B97F0()->fn_8040F490(name);
    if (!name) return mXanimePlayer->mCurrentAnimation != mXanimePlayer->mDefaultAnimation;
    if (mXanimePlayer->isRun(name)) return true;
    if (isUpperAnimationActive() && mXanimePlayerUpper->isRun(name)) return true;
    return false;
}
bool MarioAnimator::isMainAnimationRun(const char* name) const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->fn_803B97F0()->fn_8040F540(name);
    if (!name) return mXanimePlayer->mCurrentAnimation != mXanimePlayer->mDefaultAnimation;
    return mXanimePlayer->isRun(name);
}
bool MarioAnimator::isAnimationTerminate(const char* name) const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->fn_803B97F0()->fn_8040F590(name);
    if (name) return mXanimePlayer->isTerminate(name);
    return mXanimePlayer->isTerminate();
}
bool MarioAnimator::isAnimationRun(const char* name, u32 index) {
    if (!mAnimationFlags[index]) return false;
    if (!isAnimationRun(name)) {
        mAnimationFlags[index] = 0;
        return false;
    }
    return true;
}
f32 MarioAnimator::fn_803DD530() {
    if (getPlayer()->isPlayerModeYoshi()) return getYoshiAnimationPlayer()->_20->mEnd;
    return mXanimePlayer->_20->mEnd;
}

#pragma auto_inline off
void MarioAnimator::fn_803E0A60() {
    if (mXanimePlayer->isAnimationRunSimple()) {
        if (!mActor->_EA4 && !mActor->_3C0) {
            if (!getPlayer()->mMovementStates._22 && !mActor->_934 && getPlayer()->mMovementStates._1) {
                getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
            }
        }
    }
}

void MarioAnimator::fn_803E0AF0() {
    static const u8 walkStateTable[] = {0, 1, 1, 2, 2, 2, 3, 3};
    getPlayer()->_71D = getPlayer()->mTargetWalkSpeedIndex;
    if (isWalkOrWaitingMotion() || isAnimationRun("しゃがみ基本")) _14 = walkStateTable[getPlayer()->mTargetWalkSpeedIndex];
    else _15 = 0xFF;
    if (getPlayer()->mDrawStates._A) { fn_803E0C20(); return; }
    fn_803E0D30();
    if (isCancelableAnimationRun()) { fn_803E0DC0(); return; }
    if (fn_803E1110()) { fn_803E1160(); return; }
    if (fn_803E11D0()) { fn_803E11F0(); return; }
    if (isWalkOrWaitingMotion()) { fn_803E1230(); return; }
    fn_803E12B0();
}

void MarioAnimator::fn_803E0C20() {
    if (!getPlayer()->isPlayerModeHopper() && !getPlayer()->MarioModule::isStatusActiveID(MarioStatus_Freeze)) {
        getPlayer()->fn_8038CE40();
        fn_803E0C90();
        fn_803E0CE0();
    }
}

void MarioAnimator::fn_803E0C90() {
    if (getPlayer()->mMovementStates._B) {
        getPlayer()->mMovementStates.jumping = false;
        getPlayer()->mMovementStates._B = false;
        getPlayer()->MarioModule::stopAnimation(nullptr, "基本");
    }
}

void MarioAnimator::fn_803E0CE0() {
    if (isAnimationStop()) getPlayer()->MarioModule::changeAnimation("崖ふんばり", static_cast<const char*>(nullptr));
}

void MarioAnimator::fn_803E0D30() {
    if (isAnimationRun("崖ふんばり")) {
        if (!getPlayer()->mMovementStates._1) {
            getPlayer()->MarioModule::stopAnimation("崖ふんばり", "落下");
            getPlayer()->_414 = 15;
        } else getPlayer()->MarioModule::stopAnimation("崖ふんばり", static_cast<const char*>(nullptr));
    }
}

void MarioAnimator::fn_803E0DC0() {
    getPlayer()->decideWalkSpeed();
    if (getPlayer()->mTargetWalkSpeedIndex != 0) getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
    if (getPlayer()->mMovementStates._A) getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
}

void MarioAnimator::fn_803E0E90() {
    if (_14 == 0) getPlayer()->startBas(nullptr, false, 0.0f, 0.0f);
    else if (!isAnimationRun("幅とび")) getPlayer()->startBas("SquatWalk", false, 0.0f, 79.0f);
    else getPlayer()->startBas(nullptr, false, 0.0f, 0.0f);
}

#pragma dont_inline on
void MarioAnimator::fn_803E0F40() {
    f32 frame = getFrame();
    mActor->mMario->fn_803A96D0(frame);
}
#pragma dont_inline reset

void MarioAnimator::fn_803E0F80() {
    if (_6C) {
        XanimePlayer* upper = mXanimePlayerUpper;
        upper->updateBeforeMovement();
        upper->updateAfterMovement();
        XanimePlayer* player = mXanimePlayerUpper;
        if ((player->_24[player->_54].mState & 1) && player->_20->mAttribute != 1 && !mUpperDefaultSet) stopUpper(nullptr);
    }
}

void MarioAnimator::fn_803E1020() { if (isAnimationStop()) mCurrBck = mXanimePlayer->getCurrentBckName(); }

void MarioAnimator::fn_803E1060() {
    if (isAnimationRun("基本") || isAnimationRun("がんばり走り")) setTilt();
    else resetTilt();
    if (isAnimationRun("スライダー尻")) setHipSliderTilt();
    if (isAnimationRun("坂すべり下向きあおむけ", 3)) setHipSlipTilt();
}

// NONMATCHING: equivalent short-circuit branch layout.
bool MarioAnimator::fn_803E1110() const {
    if (!getPlayer()->mMovementStates._A) {
        bool squat = false;
        if (getPlayer()->_20._A && !getPlayer()->mMovementStates.jumping && getPlayer()->mMovementStates._1 == 1) squat = true;
        if (!squat) return false;
        return true;
    }
    return true;
}

void MarioAnimator::fn_803E1160() {
    if (!getPlayer()->mMovementStates._23) {
        getPlayer()->fn_803ACC40();
        if (_14 != _15) { fn_803E0E90(); fn_803E0F40(); }
        _15 = _14;
    }
}

bool MarioAnimator::fn_803E11D0() const { return getPlayer()->mMovementStates._34; }
void MarioAnimator::fn_803E11F0() { getPlayer()->decideWalkSpeed(); getPlayer()->decideOnIceAnimation(); }

void MarioAnimator::fn_803E1230() {
    getPlayer()->decideWalkSpeed();
    getPlayer()->decideWalkAnimation();
    if (mActor->mBeeWallWalk) updateWalkBas("BeeCreepWalk", 59.0f);
    else updateWalkBas("Walk", 59.0f);
}

void MarioAnimator::fn_803E12B0() {
    if (isAnimationRun("氷上慣性走行")) getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
    else if (isAnimationRun("壁押し", 0)) {
        getPlayer()->decideWalkSpeed();
        getPlayer()->checkWallPush();
    } else {
        if (getPlayer()->mMovementStates.turning || getPlayer()->mMovementStates._4) getPlayer()->decideWalkSpeed();
        else if (!isAnimationRun("ハード着地")) {
            if (isAnimationRun("がんばり走り")) {
                if (getPlayer()->_8F0 < 1.0f) getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
            } else _15 = 0;
        }
    }
    getPlayer()->updateBrakeAnimation();
}

bool MarioAnimator::isAnimationRunSimple() const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->fn_803B97F0()->fn_8040F530();
    return mXanimePlayer->isAnimationRunSimple();
}

void MarioAnimator::fn_803E13D0() {
    mXanimePlayer->calcAnm(0);
    if (getPlayer()->isPlayerModeYoshi()) getYoshiAnimationPlayer()->calcAnm(0);
    if (_6C) {
        mXanimePlayerUpper->calcAnm(MR::getJointIndex(mActor, "Spine1"));
        XanimePlayer* upper = mXanimePlayerUpper;
        upper->overWriteMtxCalc(MR::getJointIndex(mActor, "PartsControl"));
        if (getPlayer()->isPlayerModeYoshi()) {
            u16 index = mActor->fn_803B97F0()->fn_8040F440();
            getYoshiUpperAnimationPlayer()->calcAnm(index);
        }
    } else {
        fn_803E1570();
        if (getPlayer()->isPlayerModeYoshi() && mActor->fn_803B97F0()->mUpperAnimationActive) {
            u16 index = mActor->fn_803B97F0()->fn_8040F440();
            getYoshiUpperAnimationPlayer()->calcAnm(index);
        }
    }
    if (getPlayer()->isPlayerModeYoshi()) {
        mXanimePlayer->clearAnm(MR::getJointIndex(mActor, "Spine1"));
        XanimePlayer* upper = mXanimePlayerUpper;
        upper->clearMtxCalc(MR::getJointIndex(mActor, "PartsControl"));
        mXanimePlayerExtra->calcAnm(0);
    }
}

void MarioAnimator::fn_803E1570() {
    mXanimePlayer->clearAnm(MR::getJointIndex(mActor, "Spine1"));
    XanimePlayer* upper = mXanimePlayerUpper;
    upper->clearMtxCalc(MR::getJointIndex(mActor, "PartsControl"));
    if (getPlayer()->isPlayerModeYoshi()) {
        u16 index = mActor->fn_803B97F0()->fn_8040F440();
        getYoshiAnimationPlayer()->clearAnm(index);
    }
}

void MarioAnimator::fn_803E1620() {
    if (!getPlayer()->isPlayerModeYoshi()) {
        mXanimePlayer->mCore->_6 = 1;
        MR::calcJ3DModel(mActor);
        mXanimePlayer->mCore->_6 = 2;
        MR::calcJ3DModel(mActor);
        return;
    }
    mActor->fn_803B97F0()->fn_8040CB30();
    LiveActor* renderActor = mActor->fn_803B97F0();
    if (mActor->fn_803B9AB0()) {
        MtxPtr matrix = mActor->fn_803B97F0()->getBaseMtx();
        PSMTXCopy(matrix, static_cast<J3DModel*>(mActor->fn_803BBA20())->getBaseTRMtx());
        renderActor = mActor->fn_803B9830()->mRideModelActor;
        MR::onCalcAnim(renderActor);
        renderActor->calcAnim();
        MR::offCalcAnim(renderActor);
    }
    if (MR::isPowerStarGetDemoActive()) {
        MtxPtr matrix = mActor->getBaseMtx();
        PSMTXCopy(matrix, static_cast<J3DModel*>(mActor->fn_803BBA20())->getBaseTRMtx());
    } else {
        MtxPtr matrix = MR::getJointMtx(renderActor, "MarioPos");
        PSMTXCopy(matrix, static_cast<J3DModel*>(mActor->fn_803BBA20())->getBaseTRMtx());
    }
    mXanimePlayerExtra->mCore->_6 = 1;
    MR::calcJ3DModel(mActor);
    mXanimePlayerExtra->mCore->_6 = 2;
    MR::calcJ3DModel(mActor);
}

void MarioAnimator::fn_803E17C0() {
    mXanimePlayer->clearAnm(0);
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->clearAnm(0);
        mXanimePlayerExtra->clearAnm(0);
    }
    if (_6C) { fn_803E1570(); return; }
    if (getPlayer()->isPlayerModeYoshi() && mActor->fn_803B97F0()->mUpperAnimationActive) {
        u16 index = mActor->fn_803B97F0()->fn_8040F440();
        getYoshiAnimationPlayer()->clearAnm(index);
    }
}

void MarioAnimator::fn_803E1890() {
    TVec3f position(getPlayer()->_13C);
    mXanimePlayer->mCore->getJointTransform(0)->_38 = position;
    if (getPlayer()->isPlayerModeYoshi()) {
        TVec3f yoshiPosition(getPlayer()->_13C);
        getYoshiAnimationPlayer()->mCore->getJointTransform(0)->_38 = yoshiPosition;
    }
}

void MarioAnimator::fn_803DEFD0(u32 joint,TVec3f* out) {
    if (mActor->mMario->isPlayerModeYoshi()) {mXanimePlayerExtra->getMainAnimationTrans(joint,out);return;}
    if (isUpperAnimationActive()) {mXanimePlayerUpper->getMainAnimationTrans(joint,out);return;}
    mXanimePlayer->getMainAnimationTrans(joint,out);
}
void MarioAnimator::fn_803DEC10() {
    if (mActor->mMario->isPlayerModeInvincible() && mActor->mMario->mWalkSpeed>=1.5f) return;
    if (_78) {--_78;return;}
    if (isAnimationRun("基本")) {
        mXanimePlayer->changeTrackAnimation(2,"ラン");
        if (mActor->mMario->isPlayerModeYoshi() && mActor->fn_803B97F0()->mDashTimer<=0) {
            getYoshiAnimationPlayer()->changeTrackAnimation(2,"ラン");
            mXanimePlayerExtra->changeTrackAnimation(2,"ラン");
        }
    }
}
void MarioAnimator::fn_803DF0F0() {
    mXanimePlayer->changeTrackAnimation(0,"泥低速歩行");mXanimePlayer->changeTrackAnimation(1,"泥低速歩行");
    if (mActor->mMario->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeTrackAnimation(0,"泥低速歩行");getYoshiAnimationPlayer()->changeTrackAnimation(1,"泥高速歩行");
        mXanimePlayerExtra->changeTrackAnimation(0,"泥低速歩行");mXanimePlayerExtra->changeTrackAnimation(1,"泥高速歩行");
    }
}
void MarioAnimator::fn_803DF1B0() {
    mXanimePlayer->changeTrackAnimation(0,"ハチ匍匐前進");mXanimePlayer->changeTrackAnimation(1,"ハチ匍匐前進");mXanimePlayer->changeTrackAnimation(2,"ハチ匍匐前進");mXanimePlayer->changeTrackAnimation(3,"ハチ匍匐ウエイト");
    if (mActor->mMario->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeTrackAnimation(0,"ハチ匍匐前進");getYoshiAnimationPlayer()->changeTrackAnimation(1,"ハチ匍匐前進");getYoshiAnimationPlayer()->changeTrackAnimation(2,"ハチ匍匐前進");getYoshiAnimationPlayer()->changeTrackAnimation(3,"ハチ匍匐ウエイト");
        mXanimePlayerExtra->changeTrackAnimation(0,"ハチ匍匐前進");mXanimePlayerExtra->changeTrackAnimation(1,"ハチ匍匐前進");mXanimePlayerExtra->changeTrackAnimation(2,"ハチ匍匐前進");mXanimePlayerExtra->changeTrackAnimation(3,"ハチ匍匐ウエイト");
    }
}
void MarioAnimator::fn_803DF2D0() {
    mXanimePlayer->changeTrackAnimation(0,"鈍行");mXanimePlayer->changeTrackAnimation(1,"歩行");
    if (mActor->mMario->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeTrackAnimation(0,"鈍行");getYoshiAnimationPlayer()->changeTrackAnimation(1,"歩行");
        mXanimePlayerExtra->changeTrackAnimation(0,"鈍行");mXanimePlayerExtra->changeTrackAnimation(1,"歩行");
    }
}
void MarioAnimator::fn_803DF8E0() {
    for (s32 i=0;i<MR::getJointNum(mActor);++i) mXanimePlayerExtra->mCore->mJointList[i]=mXanimePlayer->mCore->mJointList[i];
}
void MarioAnimator::fn_803DF9D0() {
    setYoshiJointMatrix("AllRoot",nullptr);setYoshiJointOffset("Center",TVec3f(0.0f,0.0f,0.0f));
    getYoshiAnimationPlayer()->mCore->getJointTransform(0)->_38=TVec3f(0.0f,0.0f,0.0f);
    for (s32 i=0;i<MR::getJointNum(mActor);++i) mXanimePlayer->mCore->mJointList[i]=mXanimePlayerExtra->mCore->mJointList[i];
}
void MarioAnimator::fn_803DFAC0() {
    if (mActor->mMario->isPlayerModeYoshi() && !mExtraAnimationEnabled && !mActor->fn_803B97F0()->fn_8040F490(nullptr)) {
        const char* name=mXanimePlayerExtra->getDefaultAnimationName();
        if (!MR::isEqualString(mXanimePlayerExtra->getCurrentAnimationName(),name)) {
            mXanimePlayerExtra->stopAnimation();mXanimePlayerExtra->setAnimationFrame(mActor->fn_803B97F0()->fn_8040F620());
        }
    }
}
void MarioAnimator::fn_803DFB70() {
    if (mActor->mMario->isPlayerModeYoshi()) {
        if (!MR::isStopAnimFrame(mActor)) {XanimePlayer* player=mXanimePlayerExtra;player->updateBeforeMovement();player->updateAfterMovement();}
        MarioActor* actor=mActor;actor->updateHand();actor->updateFace();
    }
}
void MarioAnimator::fn_803DE880() {
    if (_78) {--_78;return;}
    TVec3f side;PSVECCrossProduct(&mActor->mMario->mFrontVec,&mActor->mMario->_368,&side);MR::normalizeOrZero(&side);
    if (MR::isNearZero(side,0.001f)) return;
    TVec3f front;PSVECCrossProduct(&mActor->mMario->_368,&side,&front);MR::normalizeOrZero(&front);
    f32 sideDot=side.dot(mActor->mMario->getAirGravityVec());f32 frontDot=front.dot(mActor->mMario->getAirGravityVec());
    f32 weights[4]={0.0f,0.0f,0.0f,0.0f};f32 sideBias=0.0f,frontBias=0.0f;
    if (_16==1) sideBias=0.1f;else if (_16==2) frontBias=0.1f;
    f32 absFront=MR::abs(frontDot),absSide=MR::abs(sideDot);
    if (sideBias+absSide>frontBias+absFront) {
        f32 angle=1.5707964f-mActor->mMario->marioAcos(absSide);if (angle>=0.7853982f) angle=0.7853982f;
        f32 ratio=angle/0.7853982f;weights[2]=ratio;weights[3]=1.0f-ratio;
        if (sideDot<0.0f) {
            mXanimePlayer->changeTrackAnimation(2,"坂右ウエイト");
            if (mActor->mMario->isPlayerModeYoshi()) {getYoshiAnimationPlayer()->changeTrackAnimation(2,"坂右ウエイト");mXanimePlayerExtra->changeTrackAnimation(2,"坂右ウエイト");}
        } else {
            mXanimePlayer->changeTrackAnimation(2,"坂左ウエイト");
            if (mActor->mMario->isPlayerModeYoshi()) {getYoshiAnimationPlayer()->changeTrackAnimation(2,"坂左ウエイト");mXanimePlayerExtra->changeTrackAnimation(2,"坂左ウエイト");}
        }
        _16=1;
    } else {
        f32 angle=1.5707964f-mActor->mMario->marioAcos(absFront);if (angle>=0.7853982f) angle=0.7853982f;
        f32 ratio=angle/0.7853982f;weights[2]=ratio;weights[3]=1.0f-ratio;
        if (frontDot<0.0f) {
            mXanimePlayer->changeTrackAnimation(2,"坂前ウエイト");
            if (mActor->mMario->isPlayerModeYoshi()) {getYoshiAnimationPlayer()->changeTrackAnimation(2,"坂前ウエイト");mXanimePlayerExtra->changeTrackAnimation(2,"坂前ウエイト");}
        } else {
            mXanimePlayer->changeTrackAnimation(2,"坂後ウエイト");
            if (mActor->mMario->isPlayerModeYoshi()) {getYoshiAnimationPlayer()->changeTrackAnimation(2,"坂後ウエイト");mXanimePlayerExtra->changeTrackAnimation(2,"坂後ウエイト");}
        }
        _16=2;
    }
    if (mActor->mMario->_10._F) {forceSetBlendWeight(weights);mActor->mMario->_10._F=false;}
    else setBlendWeight(weights,mActor->mConst->getTable()->mSlopeAnimBlendRatio);
    _78=4;
}

XjointInfo& XjointInfo::operator=(const XjointInfo& other) {
    _0=other._0;_28=other._28;_50=other._50;_5C=other._5C;_60=other._60;return *this;
}
XjointTransform* MarioAnimator::getJointCtrl(const char* name) const {
    XanimeCore* core=mXanimePlayer->mCore;
    return core->getJointTransform(MR::getJointIndex(mActor,name));
}
void MarioAnimator::fn_803DDF60() {
    mActor->_494=mActor->mDrillFixedPosition788;
    mActor->mMario->fn_803B4630("ドリルひろいクイック");mActor->mMario->stopWalk();
}
void MarioAnimator::fn_803DDD80(const HitSensor* sensor) {
    LiveActor* host=MR::getSensorHost(sensor);
    if (host && MR::isEqualString(MR::getSensorHost(sensor)->mName,"アイテムドリル")) {
        mActor->_494=mActor->mDrillFixedPosition788;
        Mario* player=mActor->mMario;
        if (player->mMovementStates.jumping && !player->mMovementStates._B) {
            player->fn_803B4630("ドリルひろいクイック");mActor->mMario->stopWalk();return;
        }
        player->fn_803B45E0("ドリルひろい","CarryDrillStart");mActor->mMario->stopWalk();return;
    }
    u32 type=sensor->mType;
    if (static_cast<u32>(type-0x12)>1) {
        if (type==0x1D) {
        mActor->_494=mActor->mTurnipFixedPosition784;
        mActor->mMario->fn_803B45E0("カブ抜き","PullOut");mActor->mMario->stopWalk();
        }
    } else {
        mActor->_494=mActor->mCarryFixedPosition780;
        if (!mActor->mMario->isSwimming()) {
            if (mActor->_424==sensor) {
                mActor->mMario->fn_803B4670("ひろいクイック");mActor->playEffect("ひろいクイック");mActor->mMario->startPadVib(2);return;
            }
            Mario* player=mActor->mMario;
            if (player->mMovementStates.jumping && !player->mMovementStates._B) {
                player->fn_803B4630("ひろい空中");mActor->mMario->stopWalk();return;
            }
            player->fn_803B45E0("ひろい","CarryStart");mActor->mMario->stopWalk();return;
        }
        mActor->clearNullAnimation(0);mActor->mMario->startPadVib(2);
    }
}
