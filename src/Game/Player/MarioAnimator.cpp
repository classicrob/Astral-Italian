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

Mario* MarioAnimator::getPlayer() const { return mActor->getMario(); }

extern XanimeBckTable4 quadAnimeTable[];
extern XanimeBckTable4 tripleAnimeTable[];
extern XanimeBckTable3 doubleAnimeTable[];
extern XanimeBckTable2 singleAnimeTable[];
extern XanimeGroupInfo marioAnimeTable[];
extern XanimeOfsInfo marioAnimeAuxTable[];
extern XanimeBckTable1 marioAnimeOfsTable[];
extern XanimeSwapTable luigiAnimeSwapTable[];
extern XanimeGroupInfo marioRideAnimeTable[];
extern XanimeOfsInfo marioRideAnimeAuxTable[];
extern XanimeBckTable1 marioRideAnimeOfsTable[];
extern XanimeBckTable2 marioRideSingleAnimeTable[];
extern XanimeBckTable3 marioRideDoubleAnimeTable[];
extern XanimeBckTable4 marioRideTripleAnimeTable[];

#ifndef GALAXY_NATIVE
#include "../src/Game/Player/MarioAnimatorTables.inc"
#endif

namespace {
    // Original writable walk-state table at 0x806DC0F8.
    u8 walkStateTable[] = {0, 1, 1, 2, 2, 2, 3, 3};
    // Original writable joint/animation literals, 0x806DC100..0x806DC37F.
    // jname_chest retains its distinct later Spine1 literal.
    char animName806DC100[] = "Spine1";
    char animName806DC108[] = "基本";
    char animName806DC110[] = "テニス基本";
    char animName806DC11C[] = "坂前後ウエイト";
    char animName806DC12C[] = "坂左右ウエイト";
    char animName806DC13C[] = "カリカリ限界";
    char animName806DC14C[] = "飛び込み準備";
    char animName806DC15C[] = "Spine2";
    char animName806DC164[] = "ShoulderL";
    char animName806DC170[] = "ShoulderR";
    char animName806DC17C[] = "Head";
    char animName806DC184[] = "ArmL1";
    char animName806DC18C[] = "ArmR2";
    char animName806DC194[] = "着地";
    char animName806DC19C[] = "着地B";
    char animName806DC1A4[] = "着地C";
    char animName806DC1AC[] = "着地ターン";
    char animName806DC1B8[] = "着地幅とび";
    char animName806DC1C4[] = "すべり着地";
    char animName806DC1D0[] = "pulloutgraspend";
    char animName806DC1E0[] = "壁すべり";
    char animName806DC1EC[] = "壁くっつき";
    char animName806DC1F8[] = "ブレーキ";
    char animName806DC204[] = "ターンブレーキ";
    char animName806DC214[] = "ターンジャンプ";
    char animName806DC224[] = "左空パンチ";
    char animName806DC230[] = "アイテムドリル";
    char animName806DC240[] = "ドリルひろいクイック";
    char animName806DC258[] = "ドリルひろい";
    char animName806DC268[] = "CarryDrillStart";
    char animName806DC278[] = "カブ抜き";
    char animName806DC288[] = "PullOut";
    char animName806DC290[] = "ひろいクイック";
    char animName806DC2A0[] = "ひろい空中";
    char animName806DC2AC[] = "ひろい";
    char animName806DC2B4[] = "CarryStart";
    char animName806DC2C0[] = "ドリルウエイト";
    char animName806DC2D0[] = "カブウエイト";
    char animName806DC2E0[] = "ひろいウエイト";
    char animName806DC2F0[] = "SwimBubble";
    char animName806DC2FC[] = "両手投げ";
    char animName806DC308[] = "マリオ[亀投げ]";
    char animName806DC318[] = "水泳亀投げ";
    char animName806DC324[] = "水泳基本";
    char animName806DC330[] = "投げ";
    char animName806DC338[] = "坂右ウエイト";
    char animName806DC348[] = "坂左ウエイト";
    char animName806DC358[] = "坂前ウエイト";
    char animName806DC368[] = "坂後ウエイト";
    char animName806DC378[] = "ラン";
}

// Original unreferenced writable literals, 0x806DC380 and 0x806DC388.
char animName806DC380[] = "FootL";
char animName806DC388[] = "FootR";

namespace {
    char animName806DC390[] = "泥低速歩行";
    char animName806DC39C[] = "泥高速歩行";
    char animName806DC3A8[] = "ハチ匍匐前進";
    char animName806DC3B8[] = "ハチ匍匐ウエイト";
    char animName806DC3CC[] = "鈍行";
    char animName806DC3D4[] = "歩行";
    char animName806DC3E0[] = "AllRoot";
    char animName806DC3E8[] = "Center";
    char animName806DC3F0[] = "ArmR1";
    char animName806DC3F8[] = "Spine1";
    char animName806DC400[] = "DamageWait";
    char animName806DC40C[] = "しゃがみ基本";
    char animName806DC41C[] = "崖ふんばり";
    char animName806DC428[] = "落下";
    char animName806DC430[] = "幅とび";
    char animName806DC438[] = "SquatWalk";
    char animName806DC444[] = "がんばり走り";
    char animName806DC454[] = "スライダー尻";
    char animName806DC464[] = "坂すべり下向きあおむけ";
    char animName806DC47C[] = "BeeCreepWalk";
    char animName806DC48C[] = "Walk";
    char animName806DC494[] = "氷上慣性走行";
    char animName806DC4A4[] = "壁押し";
    char animName806DC4AC[] = "ハード着地";
    char animName806DC4B8[] = "PartsControl";
    char animName806DC4C8[] = "MarioPos";
}

MarioAnimator::MarioAnimator(MarioActor* pActor) : mActor(pActor), mExtraAnimationEnabled(false) {
    init();
}

void MarioAnimator::stopAnimation(const char* name, const char* defaultName) {
    if (defaultName) changeDefault(defaultName);
    if (name) mXanimePlayer->stopAnimation(name);
    else mXanimePlayer->stopAnimation();
    if (getPlayer()->isPlayerModeYoshi()) {
        mActor->getYoshi()->fn_8040EDE0(name);
        if (name) mXanimePlayerExtra->stopAnimation(name);
        else mXanimePlayerExtra->stopAnimation();
    }
}

// NONMATCHING: external table and string-pool address materialization; target addresses verified.
void MarioAnimator::init() {
    XanimeSwapTable* luigiAnimations = nullptr;
    if (gIsLuigi) {
        luigiAnimations = luigiAnimeSwapTable;
    }

    mResourceTable = new XanimeResourceTable(MR::getResourceHolder(mActor), marioAnimeTable, marioAnimeAuxTable, marioAnimeOfsTable,
                                             singleAnimeTable, doubleAnimeTable, tripleAnimeTable,
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

    changeDefault(animName806DC108);
    change(animName806DC108);

    mXanimePlayer->getCore()->enableJointTransform(MR::getJ3DModelData(mActor));

    mActor->mModelManager->mXanimePlayer = mXanimePlayer;
    mXanimePlayerUpper = new XanimePlayer(MR::getJ3DModel(mActor), mResourceTable, mXanimePlayer);
    changeDefaultUpper(animName806DC108);
    mXanimePlayerUpper->changeAnimationBck(animName806DC108);
    mXanimePlayerUpper->mCore->shareJointTransform(mXanimePlayer->mCore);
    XanimeResourceTable* rideResources = new XanimeResourceTable(MR::getResourceHolder(mActor), marioRideAnimeTable,
        marioRideAnimeAuxTable, marioRideAnimeOfsTable, marioRideSingleAnimeTable,
        marioRideDoubleAnimeTable, marioRideTripleAnimeTable, nullptr);
    mXanimePlayerExtra = new XanimePlayer(MR::getJ3DModel(mActor), rideResources);
    mXanimePlayerExtra->setDefaultAnimation(animName806DC108);
    mXanimePlayerExtra->changeAnimationBck(animName806DC108);
    XanimeCore* extraCore = mXanimePlayerExtra->mCore;
    extraCore->enableJointTransform(MR::getJ3DModelData(mActor));
    PSMTXCopy(MR::tmpMtxRotYRad(PI), _DC.toMtxPtr());
}

bool MarioAnimator::isAnimationStop() const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->getYoshi()->fn_8040F570();
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

    if (isAnimationRun(animName806DC194) || isAnimationRun(animName806DC19C) || isAnimationRun(animName806DC1A4) || isAnimationRun(animName806DC1AC) ||
        isAnimationRun(animName806DC1B8)) {
        return true;
    }

    if (isAnimationRun(animName806DC1C4)) {
        return true;
    }

    if (isAnimationRun(getPlayer()->_728)) {
        return true;
    }

    if (isAnimationRun(animName806DC4AC)) {
        return true;
    }

    if (isAnimationRun("逆着地")) {
        return true;
    }

    if (isAnimationRun("ショート着地")) {
        return true;
    }

    if (MR::isBckPlaying(mActor, animName806DC1D0)) {
        return true;
    }

    return false;
}

bool MarioAnimator::isCancelableAnimationRun() const {
    if (isAnimationRun(animName806DC194) || isAnimationRun(animName806DC19C) || isAnimationRun(animName806DC1A4) || isAnimationRun(animName806DC1AC) ||
        isAnimationRun(animName806DC1B8)) {
        return true;
    }

    if (isAnimationRun(animName806DC1C4)) {
        return true;
    }

    if (MR::isBckPlaying(mActor, animName806DC1D0)) {
        return true;
    }

    return false;
}

bool MarioAnimator::isWalkOrWaitingMotion() const {
    if (isAnimationStop()) {
        if (isAnimationRun(animName806DC108) || isAnimationRun(animName806DC110)) {
            return true;
        }
    }

    if (isAnimationRun(animName806DC11C)) {
        return true;
    }

    if (isAnimationRun(animName806DC12C)) {
        return true;
    }

    if (isAnimationRun(animName806DC13C)) {
        return true;
    }

    if (isAnimationRun(animName806DC14C)) {
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
        if (getPlayer()->isPlayerModeYoshi()) mActor->getYoshi()->fn_8040CA10();
        fn_803E0F80();
        updateJointRumble();
        mActor->fn_803BC5C0();
        fn_803E1020();
    }
}

// NONMATCHING: saved-register assignment and reuse of the unchanged animation-state byte.
inline bool isSameWalkAnimationState(const u8& current, const u8& previous) {
    return current == previous;
}

void MarioAnimator::updateWalkBas(const char* pAnimName, f32 speed) {
    const char* queuedAnimation;
    bool shouldStart = false;

    // Check if player has a special animation queued
    queuedAnimation = getPlayer()->_970;
    if (queuedAnimation) {
        const char* currentBck = mXanimePlayer->getCurrentBckName();

        if (strcmp(currentBck, queuedAnimation) != 0) {
            shouldStart = true;
            if (MR::strcasecmp(animName806DC400, getPlayer()->_970) == 0) {
                if (mActor->mHealth == 1) {
                    shouldStart = false;
                }
            }
        }
    }

    // Check animation state transition
    if (isSameWalkAnimationState(_14, _15)) {
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
#ifdef GALAXY_NATIVE
    {
#endif
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
#ifdef GALAXY_NATIVE
    }
#endif

updatePrevState:
    _15 = _14;
}

void MarioAnimator::setHand() {
    TVec3f scale;
    MarioActor* actor = mActor;
    scale.x = actor->mMario->_3F0;
    scale.y = 1.0f;
    scale.z = 1.0f;
    XanimeCore* core = mXanimePlayer->mCore;
    u8 armR = MR::getJointIndex(actor, animName806DC3F0);
    XjointTransform* armRJoint = core->getJointTransform(armR);
    armRJoint->mScaleModifier = scale;
    core = mXanimePlayer->mCore;
    u8 armL = MR::getJointIndex(mActor, animName806DC184);
    XjointTransform* armLJoint = core->getJointTransform(armL);
    armLJoint->mScaleModifier = scale;
}

extern const char* jname_chest;

// NONMATCHING: floating-point register allocation and scheduling.
void MarioAnimator::setTilt() {
    f32 tiltAngle = 0.0f;

    if (!MR::isNearZero(_60)) {
        f32 frontDot = _60.dot(getPlayer()->mFrontVec);
        tiltAngle = 1.0f - frontDot;

        if (tiltAngle > 0.001f) {
            TVec3f cross = _60.cross(getPlayer()->mFrontVec);

            f32 gravDot = cross.dot(*getPlayer()->getGravityVec());
            if (gravDot < 0.0f) {
                tiltAngle = -tiltAngle;
            }
        } else {
            tiltAngle = 0.0f;
        }
    }

    _60 = getPlayer()->mFrontVec;

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

    f32 f5;
    f32 f4;
    f32 vertAngle;
    f32 lookDown;
    table = mActor->mConst->getTable();
    Mario* player2 = getPlayer();
    lookDown = player2->mWalkSpeed * table->mLookDownRatio;

    Mario* player3 = getPlayer();
    vertAngle = -player3->_3F4;
    vertAngle *= PI;
    f4 = 0.9f * _5C + 0.1f * lookDown;
    f5 = vertAngle / 4.0f;
    _5C = 0.97f * f4 + 0.03f * f5;

    TVec3f rotation(0.0f, _58, _5C);
    MR::orderRotateMtx(0, rotation, _28.toMtxPtr());

    XanimeCore* core = mXanimePlayer->mCore;
    u8 chestIdx = MR::getJointIndex(mActor, jname_chest);
    XjointTransform* jt = core->getJointTransform(chestIdx);
    jt->mLocalRotationMtx = _28.toMtxPtr();
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
            if (!mActor->getYoshi()->mHeadCtrl->fn_8041AD70(0)) {
                mActor->getYoshi()->mHeadCtrl->fn_8041AA70(0, &mLookTarget);
            }
        } else mActor->getYoshi()->mHeadCtrl->fn_8041AB50(0);
    }

    if (hasTarget) {
        TVec3f toTarget = targetPos - mActor->mCapPosition;

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
        u8 headIdx = MR::getJointIndex(mActor, animName806DC17C);
        XjointTransform* jt = core->getJointTransform(headIdx);
        jt->mLocalRotationMtx = _7C.toMtxPtr();
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
    u8 leftIdx = MR::getJointIndex(mActor, animName806DC164);
    core->getJointTransform(leftIdx)->mLocalTranslationOffset.x = leftShoulderRot;

    core = mXanimePlayer->getCore();
    u8 rightIdx = MR::getJointIndex(mActor, animName806DC170);
    core->getJointTransform(rightIdx)->mLocalTranslationOffset.x = rightShoulderRot;
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
    f32 frontDot = MR::vecKillElement(worldPadDir, getPlayer()->mFrontVec, &worldPadDir);
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
    if (!((getPlayer()->mMovementStates.mLowWord >> 16) & 1U) && ((getPlayer()->mMovementStates.mLowWord >> 15) & 1U) && !((getPlayer()->mMovementStates.mLowWord >> 21) & 1U) && 0.0f == getPlayer()->_8F0 &&
        static_cast< u8 >(getPlayer()->checkStickFrontBack()) == 2) {
        if (!getPlayer()->MarioModule::isStatusActiveID(MarioStatus_Magic) && !getPlayer()->MarioModule::isStatusActiveID(MarioStatus_Skate)) {
            if (((getPlayer()->mMovementStates.mHighWord >> 10) & 1U)) {
                change("ブレーキ滑り床");
            } else {
                change(animName806DC1F8);
            }
        }
    } else if (isAnimationRun(animName806DC1F8)) {
        getPlayer()->doBrakingAnimation();
        getPlayer()->mTargetWalkSpeedIndex = 0;
    } else {
        mXanimePlayer->stopAnimation("ブレーキ滑り床");
    }

    if (((getPlayer()->mMovementStates.mLowWord >> 27) & 1U) && !((getPlayer()->mMovementStates.mHighWord >> 28) & 1U)) {
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
// NONMATCHING: player-pointer load scheduling.
void MarioAnimator::fn_803E0E40(const char* pName) {
    const XanimeGroupInfo* info = mXanimePlayer->mCurrentAnimation;
    if (info->mAttribute == 2) {
        MarioActor* actor = mActor;
        f32 loop = info->mLoop;
        f32 end = info->mEnd;
        actor->mMario->startBas(pName, false, loop, end);
    } else {
        mActor->mMario->startBas(pName, false, 0.0f, 0.0f);
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
            mActor->getYoshi()->fn_8040EDA0(pName);
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
    if (getPlayer()->isPlayerModeYoshi()) mActor->getYoshi()->fn_8040EE30(pName);
    _6C = true;
}

// NONMATCHING: stop-condition boolean uses a different register.
void MarioAnimator::stopUpper(const char* pName) {
    if (!_6C) {
        return;
    }

    bool shouldStop = pName == nullptr || mXanimePlayerUpper->isRun(pName);

    if (shouldStop) {
        u16 jointID = MR::getJointIndex(mActor, animName806DC100);
        XanimeCore* upperCore = mXanimePlayerUpper->getCore();
        J3DModelData* modelData = MR::getJ3DModelData(mActor);
        XanimeCore* lowerCore = mXanimePlayer->getCore();
        upperCore->freezeCopy(modelData, lowerCore, (u16)jointID, 8);
        mXanimePlayerUpper->stopAnimation();
        if (getPlayer()->isPlayerModeYoshi()) mActor->getYoshi()->fn_8040EE70(pName);
        _6C = false;
    }
}

#pragma dont_inline on
// NONMATCHING: zero-literal relocation label.
void MarioAnimator::changeDefault(const char* pName) {
    mActor->mMario->startBas(nullptr, false, 0.0f, 0.0f);

    mXanimePlayer->setDefaultAnimation(pName);
    if (mActor->mMario->isPlayerModeYoshi()) {
        mActor->getYoshi()->fn_8040ED90(pName);
        mXanimePlayerExtra->setDefaultAnimation(pName);
    }
}

#pragma dont_inline reset
bool MarioAnimator::isDefaultAnimationRun(const char* pName) const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->getYoshi()->fn_8040F450(pName);
    return strcmp(mXanimePlayer->getDefaultAnimationName(), pName) == 0;
}

void MarioAnimator::changeDefaultUpper(const char* pName) {
    if (pName != nullptr) {
        mUpperDefaultSet = true;
        mXanimePlayerUpper->setDefaultAnimation(pName);
        if (getPlayer()->isPlayerModeYoshi()) mActor->getYoshi()->fn_8040EE00(pName);
    } else {
        mUpperDefaultSet = false;
    }
}

u16 MarioAnimator::getUpperJointID() const {
    return MR::getJointIndex(mActor, animName806DC100);
}

const char* jname_chest = animName806DC3F8;
#pragma push
#pragma opt_propagation off
// NONMATCHING: string-pool materialization, saved registers, and scheduling.
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
    XjointTransform* larmJt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(actor, animName806DC184)));
    larmJt->mLocalTranslationOffset.y = sineVal * power;

    core = mXanimePlayer->mCore;
    XjointTransform* rarmJt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC18C)));
    rarmJt->mLocalTranslationOffset.y = sineVal * power;

    f32 hipRot = 0.7f * ((f32)_74 / 60.0f);

    if (((getPlayer()->mMovementStates.mLowWord >> 30) & 1U) && !((getPlayer()->mMovementStates.mHighWord >> 8) & 1U) && getPlayer()->mTargetWalkSpeedIndex == 0 &&
        getPlayer()->calcAngleD(getPlayer()->_368) > 5.0f) {
        TVec3f dir;
        MR::vecKillElement(getPlayer()->_368, getPlayer()->getAirGravityVec(), &dir);
        MR::normalizeOrZero(&dir);

        if (getPlayer()->mFrontVec.dot(dir) < -0.707f) {
            hipRot = -hipRot;
        } else {
            hipRot = getPlayer()->mFrontVec.dot(dir) > 0.707f ? hipRot : 0.9f * _118;
        }

        PSMTXRotRad(_AC.toMtxPtr(), 'Z', hipRot);
    } else {
        PSMTXIdentity(_AC.toMtxPtr());
    }

    _118 = hipRot;

    core = mXanimePlayer->mCore;
    XjointTransform* hipJt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC15C)));
    hipJt->mLocalRotationMtx = _AC.toMtxPtr();
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
    XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC15C)));
    jt->mLocalRotationMtx = _AC.toMtxPtr();
}

void MarioAnimator::clearAllJointTransform() {
    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC164)));
        jt->mLocalRotationMtx = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC170)));
        jt->mLocalRotationMtx = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC17C)));
        jt->mLocalRotationMtx = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC15C)));
        jt->mLocalRotationMtx = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC184)));
        jt->mLocalTranslationOffset.y = 0.0f;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        XjointTransform* jt = core->getJointTransform(static_cast< u16 >(MR::getJointIndex(mActor, animName806DC18C)));
        jt->mLocalTranslationOffset.y = 0.0f;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        u8 chestIdx = MR::getJointIndex(mActor, jname_chest);
        XjointTransform* jt = core->getJointTransform(chestIdx);
        jt->mLocalRotationMtx = nullptr;
    }

    {
        XanimeCore* core = mXanimePlayer->mCore;
        u8 hipIdx = MR::getJointIndex(mActor, "Hip");
        XjointTransform* jt = core->getJointTransform(hipIdx);
        jt->mLocalRotationMtx = nullptr;
    }

    TVec3f zeroVec(0.0f, 0.0f, 0.0f);
    XjointTransform* jt = mXanimePlayer->mCore->getJointTransform(0);
    jt->mWorldTranslationOffset = zeroVec;

    _70 = 0.0f;
    _74 = 0;
}

// NONMATCHING: string-pool materialization and vector-address scheduling.
bool MarioAnimator::isMirrorAnimation() {
    if (mActor->_468) {
        return false;
    }

    if (getPlayer()->isPlayerModeYoshi()) {
        return false;
    }

    TVec3f camDir = getPlayer()->MarioModule::getCamDirX();
    Mario* player = getPlayer();
    f32 dot = player->_1FC.dot(getPlayer()->MarioModule::getCamDirY());
    if (dot < 0.0f) {
        camDir = -camDir;
    }

    if (isAnimationRun(animName806DC1E0) || isAnimationRun(animName806DC1EC)) {
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
    if (((getPlayer()->mMovementStates.mHighWord >> 8) & 1U)) {
        if (isAnimationRun(animName806DC1F8) || isAnimationRun(animName806DC204) || isAnimationRun(animName806DC214)) {
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
    if (isAnimationRun(animName806DC224)) {
        return true;
    }

    return false;
}

void MarioAnimator::switchMirrorMode() {
    f32 scale = 1.0f;
    J3DModelX* model = static_cast< J3DModelX* >(MR::getJ3DModel(mActor));
    if (isMirrorAnimation()) {
        model->mDisplayListMask |= 1U;
        XjointTransform* jt = mXanimePlayer->mCore->getJointTransform(0);
        TVec3f mirrorScale;
        mirrorScale.x = scale;
        mirrorScale.y = scale;
        mirrorScale.z = -scale;
        jt->mScaleModifier = mirrorScale;

        Mtx invBase;
        MtxPtr base = mActor->getBaseMtx();
        PSMTXInverse(base, invBase);
        MR::multMtx(_DC, invBase, MR::tmpMtxRotYRad(PI));

        base = mActor->getBaseMtx();
        MR::multMtx(_DC, _DC, base);

        jt->mWorldRotationMtx = _DC;
    } else {
        model->mDisplayListMask &= ~1U;
        XjointTransform* jt = mXanimePlayer->mCore->getJointTransform(0);
        TVec3f normalScale;
        normalScale.x = scale;
        normalScale.y = scale;
        normalScale.z = scale;
        jt->mScaleModifier = normalScale;
        jt->mWorldRotationMtx = 0;
    }
}

// NONMATCHING: equivalent literal-pool base materialization and prologue scheduling.
void MarioAnimator::updateTakingAnimation(const HitSensor* pSensor) {
    if (pSensor == nullptr) {
        return;
    }
    if (MR::isDead(MR::getSensorHost(pSensor))) {
        if (mActor->_468 != 0) {
            mActor->rushDropThrowMemoSensor();
        }
        return;
    }
    if (MR::getSensorHost(pSensor) != nullptr && MR::isEqualString(MR::getSensorHost(pSensor)->mName, animName806DC230)) {
        mActor->mMario->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
        mActor->mMario->MarioModule::changeAnimationUpper(animName806DC2C0);
        mActor->clearNullAnimation(0);
        mActor->fn_803BA220();
        return;
    }
    switch (pSensor->mType) {
    case ATYPE_BOMBHEI:
        mActor->mMario->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
        mActor->mMario->MarioModule::changeAnimationUpper(animName806DC2D0);
        mActor->clearNullAnimation(0);
        mActor->fn_803BA220();
        break;
    case ATYPE_JET_TURTLE:
    case ATYPE_JET_TURTLE_SLOW:
        if (!mActor->mMario->isSwimming()) {
            s32 shouldChange = 1;
            if (isAnimationRun(animName806DC290)) {
                shouldChange = isUpperAnimationTerminated(nullptr);
            }
            if (shouldChange) {
                if (!isAnimationRun(animName806DC2A0) && mActor->isStopNullAnimation()) {
                    mActor->mMario->MarioModule::changeAnimationUpper(animName806DC2E0);
                    mActor->mMario->MarioModule::startPadVib(2);
                    mActor->clearNullAnimation(0);
                    mActor->fn_803BA220();
                }
            }
        } else {
            MR::emitEffect(MR::getSensorHost(pSensor), animName806DC2F0);
        }
        break;
    }
}

// NONMATCHING: string-pool materialization and branch scheduling.
void MarioAnimator::changeThrowAnimation(const HitSensor* pSensor) {
    u32 type = pSensor->mType;
    switch (type) {
    case 0x1D:
        getPlayer()->MarioModule::stopAnimationUpper(nullptr);
        getPlayer()->MarioModule::changeAnimation(animName806DC2FC);
        getPlayer()->MarioModule::startPadVib(animName806DC308);
        break;
    case 0x12:
    case 0x13:
        getPlayer()->MarioModule::stopAnimationUpper(nullptr);
        if (getPlayer()->isSwimming()) {
            getPlayer()->MarioModule::changeAnimation(animName806DC318, animName806DC324);
        } else {
            getPlayer()->MarioModule::changeAnimation(animName806DC330);
        }

        getPlayer()->MarioModule::startPadVib(animName806DC308);
        break;
    }
}

// NONMATCHING: string-pool materialization and saved registers.
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
    case 0x12:
    case 0x13:
        getPlayer()->MarioModule::changeAnimationUpper(animName806DC2E0);
        mActor->clearNullAnimation(0);
        MR::deleteEffect(MR::getSensorHost(swimSensor), animName806DC2F0);
        break;
    case 0x1D:
        getPlayer()->MarioModule::changeAnimationUpper(animName806DC2D0);
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
f32 MarioAnimator::fn_803DF0E0() const { return mXanimePlayer->mFrameCtrl->mRate; }
const char* MarioAnimator::getUpperAnimationName() const { return mXanimePlayerUpper->getCurrentAnimationName(); }
const char* MarioAnimator::getAnimationStringPointer(const char* name) const { return mXanimePlayer->getNameStringPointer(name); }
bool MarioAnimator::isUpperAnimationTerminated(const char* name) const {
    if (name) return mXanimePlayerUpper->isTerminate(name);
    return mXanimePlayerUpper->isTerminate();
}
f32 MarioAnimator::getWalkWeight(u32 index) const { return (&_18)[index]; }
f32 MarioAnimator::getAnimationInterpoleRatio() const { return mXanimePlayer->mFrameBlendRatio; }
bool MarioAnimator::isFrameCtrlAttribute(u32 attribute) const { return mXanimePlayer->mFrameCtrl->getAttribute() == attribute; }

namespace {
inline XanimePlayer* getYoshiBodyAnimation(MarioActor* const& actor) {
    if (actor->getYoshi() == nullptr) return nullptr;
    return actor->getYoshi()->mXanimePlayer;
}
inline XanimePlayer* getYoshiUpperAnimation(MarioActor* const& actor) {
    if (actor->getYoshi() == nullptr) return nullptr;
    return actor->getYoshi()->mXanimePlayerUpper;
}
}
XanimePlayer* MarioAnimator::getYoshiAnimationPlayer() const {
    return getYoshiBodyAnimation(mActor);
}
XanimePlayer* MarioAnimator::getYoshiUpperAnimationPlayer() const {
    return getYoshiUpperAnimation(mActor);
}
f32 XanimePlayer::getAnimationFrame() const {
    if (isTerminate()) return mFrameCtrl->mEnd;
    return mFrameCtrl->mFrame;
}
void XanimePlayer::setAnimationFrame(f32 frame) {
    mCachedFrame = frame;
    mFrameCtrl->mFrame = mCachedFrame;
}
void MarioAnimator::changeTrackAnim(u8 index, const char* name) {
    mXanimePlayer->changeTrackAnimation(index, name);
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeTrackAnimation(index, name);
        mXanimePlayerExtra->changeTrackAnimation(index, name);
    }
}
void MarioAnimator::setAnimationPaused(bool paused) {
    mXanimePlayer->mAutoRunDefaultAnimation = paused;
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->mAutoRunDefaultAnimation = paused;
        mXanimePlayerExtra->mAutoRunDefaultAnimation = paused;
    }
}
void MarioAnimator::setAnimationFrameCtrlAttribute(u8 attribute) {
    mXanimePlayer->mFrameCtrl->mAttribute = attribute;
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->mFrameCtrl->mAttribute = attribute;
        mXanimePlayerExtra->mFrameCtrl->mAttribute = attribute;
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
        mActor->getYoshi()->mXanimePlayer->setAnimationFrame(frame);
        if (!mExtraAnimationEnabled) mXanimePlayerExtra->setAnimationFrame(frame);
    }
}
void MarioAnimator::resetAnimationUpdateFlags() {
    XanimePlayer* player = mXanimePlayer;
    player->mHasUpdatedFrame = false;
    player->mAnimationChanged = false;
    player = mXanimePlayerUpper;
    player->mHasUpdatedFrame = false;
    player->mAnimationChanged = false;
    if (getPlayer()->isPlayerModeYoshi()) {
        XanimePlayer* player = getYoshiAnimationPlayer();
        player->mHasUpdatedFrame = false;
        player->mAnimationChanged = false;
        player = getYoshiUpperAnimationPlayer();
        player->mHasUpdatedFrame = false;
        player->mAnimationChanged = false;
        player = mXanimePlayerExtra;
        player->mHasUpdatedFrame = false;
        player->mAnimationChanged = false;
    }
}
f32 MarioAnimator::getAnimationTrackWeight(u32 index) {
    if (getPlayer()->isPlayerModeYoshi()) return getYoshiAnimationPlayer()->mWeights[index];
    return mXanimePlayer->mWeights[index];
}

void MarioAnimator::setMainJointMatrix(u32 index, MtxPtr matrix) {
    mXanimePlayer->mCore->getJointTransform(index)->mLocalRotationMtx = matrix;
}
void MarioAnimator::setYoshiJointMatrix(const char* joint, MtxPtr matrix) {
    if (getPlayer()->isPlayerModeYoshi()) {
        Yoshi* actor = mActor->getYoshi();
        XanimeCore* core = getYoshiAnimationPlayer()->mCore;
        core->getJointTransform(MR::getJointIndex(actor, joint))->mLocalRotationMtx = matrix;
    }
}
void MarioAnimator::setMainJointOffset(u32 index, const TVec3f& offset) {
    mXanimePlayer->mCore->getJointTransform(index)->mLocalTranslationOffset = offset;
}
void MarioAnimator::setYoshiJointOffset(const char* joint, const TVec3f& offset) {
    if (getPlayer()->isPlayerModeYoshi()) {
        Yoshi* actor = mActor->getYoshi();
        XanimeCore* core = getYoshiAnimationPlayer()->mCore;
        core->getJointTransform(MR::getJointIndex(actor, joint))->mLocalTranslationOffset = offset;
    }
}
void MarioAnimator::setAnimationModel(J3DModel* model) {
    mXanimePlayer->setModel(model);
    mXanimePlayerUpper->setModel(model);
    if (getPlayer()->isPlayerModeYoshi()) mXanimePlayerExtra->setModel(model);
}
void MarioAnimator::setAnimationInterpoleRatio(f32 ratio) {
    mXanimePlayer->mFrameBlendRatio = ratio;
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->mFrameBlendRatio = ratio;
        mXanimePlayerExtra->mFrameBlendRatio = ratio;
    }
}

bool MarioAnimator::isUpperAnimationActive() const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->getYoshi()->mUpperAnimationActive;
    return _6C;
}
bool MarioAnimator::isAnimationRun(const char* name) const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->getYoshi()->fn_8040F490(name);
    if (!name) return mXanimePlayer->mCurrentAnimation != mXanimePlayer->mDefaultAnimation;
    if (mXanimePlayer->isRun(name)) return true;
    if (isUpperAnimationActive() && mXanimePlayerUpper->isRun(name)) return true;
    return false;
}
bool MarioAnimator::isMainAnimationRun(const char* name) const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->getYoshi()->fn_8040F540(name);
    if (!name) return mXanimePlayer->mCurrentAnimation != mXanimePlayer->mDefaultAnimation;
    return mXanimePlayer->isRun(name);
}
bool MarioAnimator::isAnimationTerminate(const char* name) const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->getYoshi()->fn_8040F590(name);
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
namespace {
inline f32 getAnimationEndFrame(const XanimePlayer* player) {
    return player->mFrameCtrl->mEnd;
}
}

f32 MarioAnimator::fn_803DD530() {
    if (getPlayer()->isPlayerModeYoshi()) return getYoshiAnimationPlayer()->mFrameCtrl->mEnd;
    return getAnimationEndFrame(mXanimePlayer);
}

#pragma auto_inline off
void MarioAnimator::fn_803E0A60() {
    if (mXanimePlayer->isAnimationRunSimple()) {
        if (!mActor->_EA4 && !mActor->_3C0) {
            if (!((getPlayer()->mMovementStates.mHighWord >> 29) & 1U) && !mActor->_934 && ((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
                getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
            }
        }
    }
}

void MarioAnimator::fn_803E0AF0() {
    getPlayer()->_71D = getPlayer()->mTargetWalkSpeedIndex;
    if (isWalkOrWaitingMotion() || isAnimationRun(animName806DC40C)) _14 = walkStateTable[getPlayer()->mTargetWalkSpeedIndex];
    else _15 = 0xFF;
    if (((getPlayer()->mDrawStates.mWord >> 21) & 1U)) { fn_803E0C20(); return; }
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
    if (((getPlayer()->mMovementStates.mLowWord >> 20) & 1U)) {
        getPlayer()->mMovementStates.mLowWord &= ~Mario::MovementStates::Jumping;
        getPlayer()->mMovementStates.mLowWord &= 0xFFEFFFFFU;
        getPlayer()->MarioModule::stopAnimation(nullptr, animName806DC108);
    }
}

void MarioAnimator::fn_803E0CE0() {
    if (isAnimationStop()) getPlayer()->MarioModule::changeAnimation(animName806DC41C, static_cast<const char*>(nullptr));
}

void MarioAnimator::fn_803E0D30() {
    if (isAnimationRun(animName806DC41C)) {
        if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
            getPlayer()->MarioModule::stopAnimation(animName806DC41C, animName806DC428);
            getPlayer()->_414 = 15;
        } else getPlayer()->MarioModule::stopAnimation(animName806DC41C, static_cast<const char*>(nullptr));
    }
}

void MarioAnimator::fn_803E0DC0() {
    getPlayer()->decideWalkSpeed();
    if (getPlayer()->mTargetWalkSpeedIndex != 0) getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
    if (((getPlayer()->mMovementStates.mLowWord >> 21) & 1U)) getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
}

void MarioAnimator::fn_803E0E90() {
    if (_14 == 0) getPlayer()->startBas(nullptr, false, 0.0f, 0.0f);
    else if (!isAnimationRun(animName806DC430)) getPlayer()->startBas(animName806DC438, false, 0.0f, 79.0f);
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
        if ((player->mFrameCtrls[player->mCurrentFrameCtrlIndex].mState & 1) && player->mFrameCtrl->mAttribute != 1 && !mUpperDefaultSet) stopUpper(nullptr);
    }
}

void MarioAnimator::fn_803E1020() { if (isAnimationStop()) mCurrBck = mXanimePlayer->getCurrentBckName(); }

void MarioAnimator::fn_803E1060() {
    if (isAnimationRun(animName806DC108) || isAnimationRun(animName806DC444)) setTilt();
    else resetTilt();
    if (isAnimationRun(animName806DC454)) setHipSliderTilt();
    if (isAnimationRun(animName806DC464, 3)) setHipSlipTilt();
}

// NONMATCHING: equivalent short-circuit branch layout.
bool MarioAnimator::fn_803E1110() const {
    bool result = true;
    if (!((getPlayer()->mMovementStates.mLowWord >> 21) & 1U)) {
        bool squat = false;
        if (((getPlayer()->mFlags24.mLowWord >> 21) & 1U) && !((getPlayer()->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && ((getPlayer()->mMovementStates.mLowWord >> 30) & 1U) == 1) squat = true;
        if (!squat) result = false;
    }
    return result;
}

void MarioAnimator::fn_803E1160() {
    if (!((getPlayer()->mMovementStates.mHighWord >> 28) & 1U)) {
        getPlayer()->fn_803ACC40();
        if (_14 != _15) { fn_803E0E90(); fn_803E0F40(); }
        _15 = _14;
    }
}

bool MarioAnimator::fn_803E11D0() const { return ((getPlayer()->mMovementStates.mHighWord >> 11) & 1U); }
void MarioAnimator::fn_803E11F0() { getPlayer()->decideWalkSpeed(); getPlayer()->decideOnIceAnimation(); }

void MarioAnimator::fn_803E1230() {
    getPlayer()->decideWalkSpeed();
    getPlayer()->decideWalkAnimation();
    if (mActor->mBeeWallWalk) updateWalkBas(animName806DC47C, 59.0f);
    else updateWalkBas(animName806DC48C, 59.0f);
}

void MarioAnimator::fn_803E12B0() {
    if (isAnimationRun(animName806DC494)) getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
    else if (isAnimationRun(animName806DC4A4, 0)) {
        getPlayer()->decideWalkSpeed();
        getPlayer()->checkWallPush();
    } else {
        if (((getPlayer()->mMovementStates.mLowWord & Mario::MovementStates::Turning) >> 28) || ((getPlayer()->mMovementStates.mLowWord >> 27) & 1U)) getPlayer()->decideWalkSpeed();
        else if (!isAnimationRun(animName806DC4AC)) {
            if (isAnimationRun(animName806DC444)) {
                if (getPlayer()->_8F0 < 1.0f) getPlayer()->MarioModule::stopAnimation(nullptr, static_cast<const char*>(nullptr));
            } else _15 = 0;
        }
    }
    getPlayer()->updateBrakeAnimation();
}

bool MarioAnimator::isAnimationRunSimple() const {
    if (getPlayer()->isPlayerModeYoshi()) return mActor->getYoshi()->fn_8040F530();
    return mXanimePlayer->isAnimationRunSimple();
}

void MarioAnimator::fn_803E13D0() {
    mXanimePlayer->calcAnm(0);
    if (getPlayer()->isPlayerModeYoshi()) getYoshiAnimationPlayer()->calcAnm(0);
    if (_6C) {
        mXanimePlayerUpper->calcAnm(MR::getJointIndex(mActor, animName806DC100));
        XanimePlayer* upper = mXanimePlayerUpper;
        upper->overWriteMtxCalc(MR::getJointIndex(mActor, animName806DC4B8));
        if (getPlayer()->isPlayerModeYoshi()) {
            u16 index = mActor->getYoshi()->fn_8040F440();
            getYoshiUpperAnimationPlayer()->calcAnm(index);
        }
    } else {
        fn_803E1570();
        if (getPlayer()->isPlayerModeYoshi() && mActor->getYoshi()->mUpperAnimationActive) {
            u16 index = mActor->getYoshi()->fn_8040F440();
            getYoshiUpperAnimationPlayer()->calcAnm(index);
        }
    }
    if (getPlayer()->isPlayerModeYoshi()) {
        mXanimePlayer->clearAnm(MR::getJointIndex(mActor, animName806DC100));
        XanimePlayer* upper = mXanimePlayerUpper;
        upper->clearMtxCalc(MR::getJointIndex(mActor, animName806DC4B8));
        mXanimePlayerExtra->calcAnm(0);
    }
}

void MarioAnimator::fn_803E1570() {
    mXanimePlayer->clearAnm(MR::getJointIndex(mActor, animName806DC100));
    XanimePlayer* upper = mXanimePlayerUpper;
    upper->clearMtxCalc(MR::getJointIndex(mActor, animName806DC4B8));
    if (getPlayer()->isPlayerModeYoshi()) {
        u16 index = mActor->getYoshi()->fn_8040F440();
        getYoshiAnimationPlayer()->clearAnm(index);
    }
}

void MarioAnimator::fn_803E1620() {
    if (!getPlayer()->isPlayerModeYoshi()) {
        mXanimePlayer->mCore->mCalcMode = 1;
        MR::calcJ3DModel(mActor);
        mXanimePlayer->mCore->mCalcMode = 2;
        MR::calcJ3DModel(mActor);
        return;
    }
    mActor->getYoshi()->fn_8040CB30();
    LiveActor* renderActor = mActor->getYoshi();
    if (mActor->fn_803B9AB0()) {
        MtxPtr matrix = mActor->getYoshi()->getBaseMtx();
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
        MtxPtr matrix = MR::getJointMtx(renderActor, animName806DC4C8);
        PSMTXCopy(matrix, static_cast<J3DModel*>(mActor->fn_803BBA20())->getBaseTRMtx());
    }
    mXanimePlayerExtra->mCore->mCalcMode = 1;
    MR::calcJ3DModel(mActor);
    mXanimePlayerExtra->mCore->mCalcMode = 2;
    MR::calcJ3DModel(mActor);
}

void MarioAnimator::fn_803E17C0() {
    mXanimePlayer->clearAnm(0);
    if (getPlayer()->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->clearAnm(0);
        mXanimePlayerExtra->clearAnm(0);
    }
    if (_6C) { fn_803E1570(); return; }
    if (getPlayer()->isPlayerModeYoshi() && mActor->getYoshi()->mUpperAnimationActive) {
        u16 index = mActor->getYoshi()->fn_8040F440();
        getYoshiAnimationPlayer()->clearAnm(index);
    }
}

void MarioAnimator::fn_803E1890() {
    TVec3f position(getPlayer()->_13C);
    mXanimePlayer->mCore->getJointTransform(0)->mWorldTranslationOffset = position;
    if (getPlayer()->isPlayerModeYoshi()) {
        TVec3f yoshiPosition(getPlayer()->_13C);
        getYoshiAnimationPlayer()->mCore->getJointTransform(0)->mWorldTranslationOffset = yoshiPosition;
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
    if (isAnimationRun(animName806DC108)) {
        mXanimePlayer->changeTrackAnimation(2,animName806DC378);
        if (mActor->mMario->isPlayerModeYoshi() && mActor->getYoshi()->mDashTimer<=0) {
            getYoshiAnimationPlayer()->changeTrackAnimation(2,animName806DC378);
            mXanimePlayerExtra->changeTrackAnimation(2,animName806DC378);
        }
    }
}
void MarioAnimator::fn_803DF0F0() {
    mXanimePlayer->changeTrackAnimation(0,animName806DC390);mXanimePlayer->changeTrackAnimation(1,animName806DC390);
    if (mActor->mMario->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeTrackAnimation(0,animName806DC390);getYoshiAnimationPlayer()->changeTrackAnimation(1,animName806DC39C);
        mXanimePlayerExtra->changeTrackAnimation(0,animName806DC390);mXanimePlayerExtra->changeTrackAnimation(1,animName806DC39C);
    }
}
void MarioAnimator::fn_803DF1B0() {
    mXanimePlayer->changeTrackAnimation(0,animName806DC3A8);mXanimePlayer->changeTrackAnimation(1,animName806DC3A8);mXanimePlayer->changeTrackAnimation(2,animName806DC3A8);mXanimePlayer->changeTrackAnimation(3,animName806DC3B8);
    if (mActor->mMario->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeTrackAnimation(0,animName806DC3A8);getYoshiAnimationPlayer()->changeTrackAnimation(1,animName806DC3A8);getYoshiAnimationPlayer()->changeTrackAnimation(2,animName806DC3A8);getYoshiAnimationPlayer()->changeTrackAnimation(3,animName806DC3B8);
        mXanimePlayerExtra->changeTrackAnimation(0,animName806DC3A8);mXanimePlayerExtra->changeTrackAnimation(1,animName806DC3A8);mXanimePlayerExtra->changeTrackAnimation(2,animName806DC3A8);mXanimePlayerExtra->changeTrackAnimation(3,animName806DC3B8);
    }
}
void MarioAnimator::fn_803DF2D0() {
    mXanimePlayer->changeTrackAnimation(0,animName806DC3CC);mXanimePlayer->changeTrackAnimation(1,animName806DC3D4);
    if (mActor->mMario->isPlayerModeYoshi()) {
        getYoshiAnimationPlayer()->changeTrackAnimation(0,animName806DC3CC);getYoshiAnimationPlayer()->changeTrackAnimation(1,animName806DC3D4);
        mXanimePlayerExtra->changeTrackAnimation(0,animName806DC3CC);mXanimePlayerExtra->changeTrackAnimation(1,animName806DC3D4);
    }
}
void MarioAnimator::fn_803DF8E0() {
    for (s32 i=0;i<MR::getJointNum(mActor);++i) mXanimePlayerExtra->mCore->mJointList[i]=mXanimePlayer->mCore->mJointList[i];
}
namespace {
inline void clearYoshiWorldOffset(MarioAnimator* animator) {
    TVec3f offset(0.0f,0.0f,0.0f);
    animator->getYoshiAnimationPlayer()->mCore->getJointTransform(0)->mWorldTranslationOffset=offset;
}
}

void MarioAnimator::fn_803DF9D0() {
    setYoshiJointMatrix(animName806DC3E0,nullptr);setYoshiJointOffset(animName806DC3E8,TVec3f(0.0f,0.0f,0.0f));
    clearYoshiWorldOffset(this);
    for (s32 i=0;i<MR::getJointNum(mActor);++i) mXanimePlayer->mCore->mJointList[i]=mXanimePlayerExtra->mCore->mJointList[i];
}
void MarioAnimator::fn_803DFAC0() {
    if (mActor->mMario->isPlayerModeYoshi() && !mExtraAnimationEnabled && !mActor->getYoshi()->fn_8040F490(nullptr)) {
        const char* name=mXanimePlayerExtra->getDefaultAnimationName();
        if (!MR::isEqualString(mXanimePlayerExtra->getCurrentAnimationName(),name)) {
            mXanimePlayerExtra->stopAnimation();mXanimePlayerExtra->setAnimationFrame(mActor->getYoshi()->fn_8040F620());
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
    f32 weights[4];
    weights[3]=0.0f;weights[2]=0.0f;weights[1]=0.0f;weights[0]=0.0f;
    f32 sideBias=0.0f,frontBias=0.0f;
    if (_16==1) sideBias=0.1f;else if (_16==2) frontBias=0.1f;
    f32 absFront=MR::abs(frontDot),absSide=MR::abs(sideDot);
    if (sideBias+absSide>frontBias+absFront) {
        f32 angle=1.5707964f-mActor->mMario->marioAcos(absSide);if (angle>=0.7853982f) angle=0.7853982f;
        f32 ratio=angle/0.7853982f;weights[2]=ratio;weights[3]=1.0f-ratio;
        if (sideDot<0.0f) {
            mXanimePlayer->changeTrackAnimation(2,animName806DC338);
            if (mActor->mMario->isPlayerModeYoshi()) {getYoshiAnimationPlayer()->changeTrackAnimation(2,animName806DC338);mXanimePlayerExtra->changeTrackAnimation(2,animName806DC338);}
        } else {
            mXanimePlayer->changeTrackAnimation(2,animName806DC348);
            if (mActor->mMario->isPlayerModeYoshi()) {getYoshiAnimationPlayer()->changeTrackAnimation(2,animName806DC348);mXanimePlayerExtra->changeTrackAnimation(2,animName806DC348);}
        }
        _16=1;
    } else {
        f32 angle=1.5707964f-mActor->mMario->marioAcos(absFront);if (angle>=0.7853982f) angle=0.7853982f;
        f32 ratio=angle/0.7853982f;weights[2]=ratio;weights[3]=1.0f-ratio;
        if (frontDot<0.0f) {
            mXanimePlayer->changeTrackAnimation(2,animName806DC358);
            if (mActor->mMario->isPlayerModeYoshi()) {getYoshiAnimationPlayer()->changeTrackAnimation(2,animName806DC358);mXanimePlayerExtra->changeTrackAnimation(2,animName806DC358);}
        } else {
            mXanimePlayer->changeTrackAnimation(2,animName806DC368);
            if (mActor->mMario->isPlayerModeYoshi()) {getYoshiAnimationPlayer()->changeTrackAnimation(2,animName806DC368);mXanimePlayerExtra->changeTrackAnimation(2,animName806DC368);}
        }
        _16=2;
    }
    if (((mActor->mMario->mFlags14.mLowWord >> 16) & 1U)) {forceSetBlendWeight(weights);mActor->mMario->mFlags14.mLowWord &= 0xFFFEFFFFU;}
    else setBlendWeight(weights,mActor->mConst->getTable()->mSlopeAnimBlendRatio);
    _78=4;
}

XjointInfo& XjointInfo::operator=(const XjointInfo& other) {
    mFrozenTransform=other.mFrozenTransform;mCurrentTransform=other.mCurrentTransform;mUnblendedTranslation=other.mUnblendedTranslation;mInterpoleRatio=other.mInterpoleRatio;mInterpoleStep=other.mInterpoleStep;return *this;
}
XjointTransform* MarioAnimator::getJointCtrl(const char* name) const {
    XanimeCore* core=mXanimePlayer->mCore;
    return core->getJointTransform(MR::getJointIndex(mActor,name));
}
void MarioAnimator::fn_803DDF60() {
    mActor->_494=mActor->mDrillFixedPosition788;
    mActor->mMario->fn_803B4630(animName806DC240);mActor->mMario->stopWalk();
}
void MarioAnimator::fn_803DDD80(const HitSensor* sensor) {
    LiveActor* host=MR::getSensorHost(sensor);
    if (host && MR::isEqualString(MR::getSensorHost(sensor)->mName,animName806DC230)) {
        mActor->_494=mActor->mDrillFixedPosition788;
        Mario* player=mActor->mMario;
        if (((player->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && !((player->mMovementStates.mLowWord >> 20) & 1U)) {
            player->fn_803B4630(animName806DC240);mActor->mMario->stopWalk();return;
        }
        player->fn_803B45E0(animName806DC258,animName806DC268);mActor->mMario->stopWalk();return;
    }
    u32 type=sensor->mType;
    if (static_cast<u32>(type-0x12)>1) {
        if (type==0x1D) {
        mActor->_494=mActor->mTurnipFixedPosition784;
        mActor->mMario->fn_803B45E0(animName806DC278,animName806DC288);mActor->mMario->stopWalk();
        }
    } else {
        mActor->_494=mActor->mCarryFixedPosition780;
        if (!mActor->mMario->isSwimming()) {
            if (mActor->_424==sensor) {
                mActor->mMario->fn_803B4670(animName806DC290);mActor->playEffect(animName806DC290);mActor->mMario->startPadVib(2);return;
            }
            Mario* player=mActor->mMario;
            if (((player->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && !((player->mMovementStates.mLowWord >> 20) & 1U)) {
                player->fn_803B4630(animName806DC2A0);mActor->mMario->stopWalk();return;
            }
            player->fn_803B45E0(animName806DC2AC,animName806DC2B4);mActor->mMario->stopWalk();return;
        }
        mActor->clearNullAnimation(0);mActor->mMario->startPadVib(2);
    }
}
