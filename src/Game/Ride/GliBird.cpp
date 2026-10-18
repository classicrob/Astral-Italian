#pragma auto_inline off
#include "Game/LiveActor/ActorParam.hpp"
#include "Game/Ride/GliBird.hpp"
#include "Game/Camera/CameraTargetMtx.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Map/RaceManager.hpp"
#include "Game/NPC/GliBirdNpc.hpp"
#include "Game/NPC/Pichan.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Screen/GliBirdGuidance.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Util/Color.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include <revolution/wpad.h>

// Unnamed SMG2 helpers (plain C symbols).
// PLACEHOLDER(objects): extern "C" block of unnamed/odd target symbols (relocs must match); resolve when they get real names in symbols.txt
extern "C" {
    s32 fn_804B8960();
    void fn_800126F0(LiveActor*, const TVec3f*, const f32*);
    void fn_800095D0(LiveActor*, f32, f32, f32);
    void fn_8004F030();
    void fn_8004FE50();
    void fn_80051910();
    void fn_80051940();
    s32 fn_80038770(TVec3f*, TVec3f*, HitSensor**, const TVec3f&, bool*, bool*, f32);
    s32 fn_80038870(const TVec3f&, TVec3f*, const TVec3f&, TVec3f*, HitSensor**, bool*, bool*, f32);
    LiveActor* fn_80235370(void*, LiveActor*, const char*, f32, const f32*);
}

inline void GliBird::exeDeadLava() {
    if (MR::isStep(this, 90)) {
        fn_8004F030();
    }
}

namespace NrvGliBird {
    NEW_NERVE(GliBirdNrvHide, GliBird, Hide);
    NEW_NERVE(GliBirdNrvReady, GliBird, Ready);
    NEW_NERVE(GliBirdNrvStart, GliBird, Start);
    NEW_NERVE(GliBirdNrvTakeOff, GliBird, TakeOff);
    NEW_NERVE(GliBirdNrvGlide, GliBird, Glide);
    NEW_NERVE(GliBirdNrvDamage, GliBird, Damage);
    NEW_NERVE(GliBirdNrvDeadDamage, GliBird, DeadDamage);
    NEW_NERVE(GliBirdNrvDeadWall, GliBird, DeadWall);
    NEW_NERVE(GliBirdNrvDeadWater, GliBird, DeadWater);
    NEW_NERVE(GliBirdNrvDeadLava, GliBird, DeadLava);
};  // namespace NrvGliBird

GliBirdParam::GliBirdParam(const LiveActor* pHost) {
    mOffsetFront = MR::getActorParamF32(pHost, "プレイヤー地形チェック[OffsetFront]");
    mOffsetUp = MR::getActorParamF32(pHost, "プレイヤー地形チェック[OffsetUp]");
    mRadius = MR::getActorParamF32(pHost, "プレイヤー地形チェック[Radius]");
    mWaterFriction = MR::getActorParamF32(pHost, "水の摩擦");
    mWaterLiftMin = MR::getActorParamF32(pHost, "水の押し上げ加速Min");
    mWaterLiftRate = MR::getActorParamF32(pHost, "水の押し上げ加速率");
    mDeadGravity = MR::getActorParamF32(pHost, "死亡重力");
    mDamageDeadBirdUpFront = MR::getActorParamF32(pHost, "ダメージ死亡（グライバード）ジャンプ速度UpFront");
    mDamageDeadBirdSide = MR::getActorParamF32(pHost, "ダメージ死亡（グライバード）ジャンプ速度Side");
    mDamageDeadPlayerUpFront = MR::getActorParamF32(pHost, "ダメージ死亡（プレイヤー）ジャンプ速度UpFront");
    mDamageDeadPlayerSide = MR::getActorParamF32(pHost, "ダメージ死亡（プレイヤー）ジャンプ速度Side");
    mWallDeadBirdUpFront = MR::getActorParamF32(pHost, "壁ヒット死亡（グライバード）ジャンプ速度UpFront");
    mWallDeadBirdSide = MR::getActorParamF32(pHost, "壁ヒット死亡（グライバード）ジャンプ速度Side");
    mWallDeadPlayerUpFront = MR::getActorParamF32(pHost, "壁ヒット死亡（プレイヤー）ジャンプ速度UpFront");
    mWallDeadPlayerSide = MR::getActorParamF32(pHost, "壁ヒット死亡（プレイヤー）ジャンプ速度Side");
}

GliBird::GliBird(const char* pName)
    : LiveActor(pName), mInitPos(0.0f, 0.0f, 0.0f), mFront(0.0f, 0.0f, 1.0f), mUp(0.0f, 1.0f, 0.0f), mSide(1.0f, 0.0f, 0.0f),
      mDirFront(0.0f, 0.0f, 1.0f), mDirUp(0.0f, 1.0f, 0.0f), mSpeed(0.0f), mFriction(0.0f), mSideAccel(0.0f), mPitchAngle(0.0f), mPitchAccel(0.0f),
      mYawAngle(0.0f), mYawAccel(0.0f), mRollAngle(0.0f), mRollAccel(0.0f), mBlend(0.0f), _100(0.0f), mRider(nullptr), mDamageTimer(0),
      mJointCtrl(nullptr), mFootJointCtrl(nullptr), mCameraInfo(nullptr), mParam(nullptr), mRumbleCount(-1), mIsWaterEffect(false),
      mWaterCount(0), mBirdBreakModel(nullptr), mPlayerBreakModel(nullptr), mDeadBody(nullptr), mCameraTarget(nullptr), mLastSpeed(0.0f),
      mIsNearGround(false), mGroundHitPos(0.0f, 0.0f, 0.0f), mGroundSmoothPos(0.0f, 0.0f, 0.0f), mGroundNormal(0.0f, 0.0f, 1.0f),
      mVolumeDrawer(nullptr), mNpc(nullptr), mIsTutorial(false), mNoRumble(false) {
    mWaterMtx.identity();
    mLavaBigMtx.identity();
    mLavaSmallMtx.identity();
    mLavaPlayerMtx.identity();
    mPlayerDeadMtx.identity();
    mApproachMtx.identity();
    mVolumeMtx.identity();
    mSpareMtx.identity();
}

static BreakModel* createBreakModel(LiveActor* pHost, MtxPtr pMtx) NO_INLINE;
static BreakModel* createBreakModel(LiveActor* pHost, MtxPtr pMtx) {
    BreakModel* pModel = new BreakModel(pHost, "水柱", "WaterColumn", pMtx, "Splash");
    MR::setScale(pModel, 1.5f);
    pModel->initWithoutIter();
    return pModel;
}

void GliBird::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, "GliBird", "Ride", false);
    mInitPos.set(mPosition);
    MR::calcActorAxis(&mSide, &mUp, &mFront, this);
    mDirFront.set(mFront);
    mDirUp.set(mUp);

    GliBirdParam* pParam = new GliBirdParam(this);
    mParam = pParam;

    MR::setEffectHostMtx(this, "GroundApproach", mApproachMtx);
    MR::setEffectHostMtx(this, "WaterApproach", mApproachMtx);
    MR::setEffectHostMtx(this, "NearWater", mWaterMtx);
    MR::setEffectHostMtx(this, "GliBirdLavaInBig", mLavaBigMtx);
    MR::setEffectHostMtx(this, "GliBirdLavaInSmall", mLavaSmallMtx);
    MR::setEffectHostMtx(this, "GliBirdLavaInPlayer", mLavaPlayerMtx);

    mJointCtrl = new ActorJointCtrl(this);
    mFootJointCtrl = new GliBirdFootJointCtrl(this);
    mFootJointCtrl->initWithoutIter();
    mBirdBreakModel = createBreakModel(this, mWaterMtx);
    mPlayerBreakModel = createBreakModel(this, mPlayerDeadMtx);

    LiveActor* pDeadBody = static_cast< LiveActor* >(operator new(0x98));
    if (pDeadBody != nullptr) {
        pDeadBody = fn_80235370(pDeadBody, this, "グライバード死亡挙動", 30.0f, mParam->mDeadGravity);
    }
    mDeadBody = pDeadBody;
    mDeadBody->initWithoutIter();

    mCameraTarget = new CameraTargetMtx("カメラターゲットダミー");
    MR::initMultiActorCamera(this, rIter, &mCameraInfo, "死亡");

    s32 drawPasses = 3;
    MR::getJMapInfoArg6NoInit(rIter, &drawPasses);
    if (drawPasses > 0) {
        mVolumeDrawer = MR::createVolumeDrawer("エリア描画", "VolumeSphere", mVolumeMtx);
        Color8 color(0xF0, 0x78, 0x50, 0x50);
        mVolumeDrawer->setColor(color);
        mVolumeDrawer->setDrawPasses(drawPasses);
        mVolumeDrawer->setScale(TVec3f(0.05f));
        MR::setScale(reinterpret_cast< LiveActor* >(mVolumeDrawer), 0.85f);
        mVolumeDrawer->setScaleDownRate(-1);
    }

    RaceManagerFunction::entryPlayerRide(this);
    MR::invalidateHitSensor(this, "Bind");
    MR::registerGliBirdForTutorial(this);

    mNpc = new GliBirdNpc("グライバードＮＰＣ");
    MR::initChildObj(mNpc, rIter, 0);
    MR::initUseStageSwitchWriteA(this, rIter);
    initNerve(GET_NERVE(GliBird, GliBirdNrvHide), 0);
    makeActorAppeared();
}

void GliBird::showBird() {
    if (!MR::isDead(mNpc)) {
        MR::validateHitSensors(this);
        MR::showModel(this);
        mNpc->makeActorDead();
    }
}

bool GliBird::isEnableFootJointCtrl() const {
    if (mRider != nullptr && !isNerve(GET_NERVE(GliBird, GliBirdNrvDeadWall)) && !isNerve(GET_NERVE(GliBird, GliBirdNrvDeadDamage))) {
        return true;
    }

    return false;
}

void GliBird::movement() {
    LiveActor::movement();
}

void GliBird::control() {
    if (fn_804B8960() != 0) {
        mNoRumble = true;
    } else {
        mNoRumble = false;
    }

    mJointCtrl->update();

    if (mDamageTimer > 0) {
        mDamageTimer--;

        if (mDamageTimer == 0) {
            fn_80051940();
        }
    }

    if (!isNerve(GET_NERVE(GliBird, GliBirdNrvGlide))) {
        MR::tryDeleteEffect(this, "GroundApproach");
        MR::tryDeleteEffect(this, "WaterApproach");
        return;
    }

    if (mVolumeDrawer != nullptr) {
        MR::makeMtxFrontNoSupportPos(&mVolumeMtx, mFront, mPosition);
        TVec3f volumeScale(10.0f, 10.0f, 50.0f);
        MR::preScaleMtx(mVolumeMtx, volumeScale);
    }

    TVec3f gravityPart = mGravity * (1.0f - 0.55f);
    TVec3f frontPart = mFront * 0.55f;
    TVec3f checkDir(frontPart);
    checkDir.add(gravityPart);
    MR::normalizeOrZero(&checkDir);

    bool isWater = false;
    TVec3f checkUp(0.0f, 1.0f, 0.0f);
    s32 isHit = fn_80038870(mPosition, &mGroundHitPos, checkDir, &checkUp, nullptr, &isWater, nullptr, 5000.0f);
    f32 dist = PSVECDistance(&mGroundHitPos, &mPosition);

    if (isHit && dist < 5000.0f) {
        if (!mIsNearGround || PSVECDistance(&mGroundSmoothPos, &mGroundHitPos) > 1000.0f) {
            mGroundSmoothPos.set(mGroundHitPos);
            mIsNearGround = true;
        }

        MR::vecBlend(mGroundSmoothPos, mGroundHitPos, &mGroundSmoothPos, 0.15f);
        mApproachMtx.identity();
        TVec3f offset = checkUp * 30.0f;
        TVec3f effectPos(mGroundSmoothPos);
        effectPos.add(offset);
        MR::makeMtxUpNoSupportPos(&mApproachMtx, checkUp, effectPos);

        f32 rate = MR::clamp((dist - 2500.0f) / (5000.0f - 2500.0f), 0.0f, 1.0f);
        s32 alpha = 255.0f * MR::getEaseInValue(1.0f - rate, 0.0f, 1.0f, 1.0f);

        // NOTE: symbols.txt names 0x800233C0 setEffectLocalScale(TVec3f&) but the target passes a u8 (alpha) in r5.
        if (isWater) {
            MR::tryEmitEffect(this, "WaterApproach");
            MR::tryDeleteEffect(this, "GroundApproach");
            MR::setEffectLocalScale(this, "WaterApproach", *reinterpret_cast< const TVec3f* >(static_cast< u32 >(static_cast< u8 >(alpha))));
        } else {
            MR::tryEmitEffect(this, "GroundApproach");
            MR::tryDeleteEffect(this, "WaterApproach");
            MR::setEffectLocalScale(this, "GroundApproach", *reinterpret_cast< const TVec3f* >(static_cast< u32 >(static_cast< u8 >(alpha))));
        }
    } else if (mIsNearGround) {
        MR::tryDeleteEffect(this, "GroundApproach");
        MR::tryDeleteEffect(this, "WaterApproach");
        mIsNearGround = false;
    }

    f32 speed = MR::calcVelocityLength(this);
    s32 volume = 100.0f * speed;
    MR::startActionSound(this, "FlyLowSpeed", volume, -1, -1);
    MR::startActionSound(this, "FlyRustle", volume, -1, -1);

    f32 speedRate = (speed - 10.0f) / (38.0f - 10.0f);
    if (!(speedRate >= 0.0f)) {
        speedRate = 0.0f;
    }
    f32 speedClamp = 1.0f;
    if (!(speedRate >= speedClamp)) {
        speedClamp = speedRate;
    }

    f32 turnRate = (MR::abs(mRollAngle) - 0.0f) / (25.0f - 0.0f);
    if (!(turnRate >= 0.0f)) {
        turnRate = 0.0f;
    }
    f32 turnClamp = 1.0f;
    if (!(turnRate >= turnClamp)) {
        turnClamp = turnRate;
    }

    MR::startActionSound(this, "FlyTurn", 100.0f * (turnClamp * speedClamp), -1, -1);
}

void GliBird::exeHide() {
    if (MR::isFirstStep(this)) {
        MR::invalidateHitSensors(this);
        MR::hideModel(this);
        mNpc->makeActorAppeared();
        mDirFront.x = 0.0f;
        mDirFront.y = 0.0f;
        mDirFront.z = 1.0f;
    }
}

void GliBird::exeReady() {
    if (MR::isFirstStep(this)) {
        showBird();
        mDirFront.set(mFront);
        mRollAngle = 0.0f;
        MR::getPlayerUpVec(&mDirUp);
        mUp.set(mDirUp);
        mPosition.add(mDirUp * 150.0f);
        MR::startAction(this, "GliBirdStartWait");
        MR::startBckPlayer("GliBirdStartWait", nullptr);

        TVec3f marioPos(0.0f);
        MR::copyJointPos(this, "MarioPos", &marioPos);
        marioPos.add(mDirUp * 150.0f);
        TPos3f playerMtx;
        MR::makeMtxFrontUpPos(&playerMtx, mDirFront, mDirUp, marioPos);
        MR::setPlayerBaseMtx(playerMtx);
        return;
    }

    TVec3f marioPos(0.0f);
    MR::copyJointPos(this, "MarioPos", &marioPos);
    TPos3f playerMtx;
    MR::makeMtxFrontUpPos(&playerMtx, mDirFront, mDirUp, marioPos);
    MR::setPlayerBaseMtx(playerMtx);
}

void GliBird::exeStart() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "GliBirdStart");
        MR::startBckPlayer("GliBirdStart", nullptr);
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(GliBird, GliBirdNrvTakeOff));
    }
}

void GliBird::exeTakeOff() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "GliBirdStartEnd");
        MR::startBckPlayer("GliBirdStartEnd", nullptr);

        if (!mNoRumble) {
            MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        }
    }

    if (MR::isEndGliBirdTutorialTalk()) {
        if (MR::isStep(this, 5)) {
            MR::startActionSound(this, "PlayerTakeOff", -1, -1, -1);
        }

        if (MR::isStep(this, 10)) {
            MR::startActionSound(this, "PlayerFlyStart", -1, -1, -1);
        }

        if (MR::isStep(this, 20)) {
            MR::startActionSound(this, "GlideStartUpVoice", -1, -1, -1);
        }

        MR::startActionSound(this, "GlideStartUpLoop", -1, -1, -1);
    }

    TVec3f accel = mFront * 0.4f;
    mVelocity.add(accel);
    MR::addVelocityJump(this, 0.8f);
    MR::addVelocityToGravity(this, 0.2f);

    if (MR::isGreaterStep(this, 45)) {
        mSpeed = mVelocity.dot(mFront);
        setNerve(GET_NERVE(GliBird, GliBirdNrvGlide));
    }
}

void GliBird::exeGlide() {
    MR::isFirstStep(this);

    if (mIsTutorial) {
        if (MR::tryStartActionNotPlaying(this, "GlideWait")) {
            MR::startBckPlayerJ("グライバード滑空Wait");
        }
    } else {
        if (mSpeed == 50.0f) {
            if (MR::tryStartActionNotPlaying(this, "GlideDownFast")) {
                MR::startBckPlayerJ("グライバード滑空Fast");
            }
        } else {
            if (mPitchAngle > 15.0f) {
                if (MR::tryStartActionNotPlaying(this, "GlideDown")) {
                    MR::startBckPlayerJ("グライバード滑空Down");
                }
            } else if (mPitchAngle < -0.0f || mPitchAccel < 0.0f) {
                if (MR::tryStartActionNotPlaying(this, "GlideUp")) {
                    MR::startBckPlayerJ("グライバード滑空Up");
                }
            } else {
                if (MR::tryStartActionNotPlaying(this, "GlideWait")) {
                    MR::startBckPlayerJ("グライバード滑空Wait");
                }
            }
        }

        updateGlide(0.995f, 0.2f);
    }
}

void GliBird::exeDamage() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "GliBirdGlideDamage");
        MR::startBckPlayer("GliBirdGlideDamage", nullptr);
        mPitchAccel = 0.0f;
        mYawAccel = 0.0f;
        mSideAccel = 0.0f;
    }

    mVelocity.add(mFront * 0.3f);
    MR::attenuateVelocity(this, 0.97f);
    mPitchAngle *= 0.95f;
    mYawAngle *= 0.95f;
    MR::rotateVecDegree(&mDirFront, mFront, mSide, mPitchAngle);
    MR::turnVecToVecDegree(&mUp, mUp, -mGravity, 1.0f, TVec3f(0, 1, 0));
    MR::rotateVecDegree(&mDirUp, mUp, mFront, mYawAngle);

    TVec3f upTmp(0.0f);
    MR::makeAxisFrontUp(&upTmp, &mDirUp, mDirFront, mDirUp);

    if (MR::isBinded(this)) {
        const TVec3f* pNormal = MR::getBindedPlaneNormal(this, 0);
        HitSensor* pSensor = MR::getBindedPlaneSensor(this, 0);
        reactionCollision(*pNormal, pSensor, false);

        if (tryDeadWall(*pNormal, pSensor, false)) {
            return;
        }
    }

    if (MR::isGreaterStep(this, 30)) {
        mSpeed = mVelocity.dot(mFront);
        setNerve(GET_NERVE(GliBird, GliBirdNrvGlide));
    }
}

void GliBird::exeDeadDamage() {
    exeDeadCommon(mParam->mDamageDeadBirdUpFront, mParam->mDamageDeadBirdSide, mParam->mDamageDeadPlayerUpFront, mParam->mDamageDeadPlayerSide);
}

void GliBird::exeDeadWall() {
    exeDeadCommon(mParam->mWallDeadBirdUpFront, mParam->mWallDeadBirdSide, mParam->mWallDeadPlayerUpFront, mParam->mWallDeadPlayerSide);
}

void GliBird::exeDeadWater() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "GliBirdGlideDeadLand");
        MtxPtr baseMtx = getBaseMtx();
        TVec3f front(baseMtx[0][2], baseMtx[1][2], baseMtx[2][2]);
        MR::makeMtxUpFrontPos(&mWaterMtx, mUp, front, mPosition);
        MR::setScale(mBirdBreakModel, 3.0f);
        mBirdBreakModel->appear();
    }

    if (MR::isStep(this, 90)) {
        fn_8004F030();
    }
}

void GliBird::calcAndSetBaseMtx() {
    mJointCtrl->setCallBackFunction();

    TPos3f mtx;
    mtx.identity();
    TVec3f dir(mDirFront);

    if (!isNerve(GET_NERVE(GliBird, GliBirdNrvDeadDamage)) && !isNerve(GET_NERVE(GliBird, GliBirdNrvDeadWall))) {
        if (mVelocity.dot(mFront) > 0.0f) {
            TVec3f velPart = mVelocity * mSpeed;
            dir.set(velPart);
            TVec3f sidePart = mSide * (mSideAccel * 0.1f);
            dir.add(sidePart);
        }
    }

    if (MR::isNearZero(dir, 0.001f)) {
        mtx.set(getBaseMtx());
        mtx.setTrans(mPosition);
    } else {
        MR::normalize(&dir);
        TVec3f up(mUp);
        MR::rotateVecDegree(&up, mUp, mFront, mRollAngle);
        MR::makeMtxFrontUpPos(&mtx, dir, up, mPosition);
    }

    MR::setBaseTRMtx(this, mtx);
}

void GliBird::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (mRider != nullptr && MR::isSensorRide(pSender)) {
        MR::tryGetItem(pSender, pReceiver);
        return;
    }

    if (mRider == nullptr && MR::isEqualSensor(pSender, this, "Push")) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool GliBird::receiveMsgEnemyAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (mRider == nullptr) {
        return false;
    }

    if (isNerve(GET_NERVE(GliBird, GliBirdNrvDeadDamage)) || isNerve(GET_NERVE(GliBird, GliBirdNrvDeadWall)) ||
        isNerve(GET_NERVE(GliBird, GliBirdNrvDeadWater)) || isNerve(GET_NERVE(GliBird, GliBirdNrvDeadLava))) {
        return false;
    }

    if (!isNerve(GET_NERVE(GliBird, GliBirdNrvDamage)) &&
        (MR::isEqualSensor(pReceiver, this, "Body") || MR::isEqualSensor(pReceiver, this, "Bind"))) {
        if (mDamageTimer <= 0) {
            MR::zeroVelocity(this);

            if (MR::isMsgEnemyAttackFire(msg) || MR::isMsgEnemyAttackFireStrong(msg)) {
                damage(true);
            } else {
                damage(false);
            }
        }

        return true;
    }

    return false;
}

bool GliBird::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgAutoRushBegin(msg)) {
        mRider = MR::getSensorHost(pSender);
        mDirFront.set(mFront);

        if (mVolumeDrawer != nullptr) {
            mVolumeDrawer->setScaleUpRate(60);
        }

        if (mIsTutorial) {
            setNerve(GET_NERVE(GliBird, GliBirdNrvGlide));
        } else {
            setNerve(GET_NERVE(GliBird, GliBirdNrvStart));
        }

        MR::setSupportTicoAttachRangeGliBird();
        return true;
    }

    if (MR::isMsgUpdateBaseMtx(msg)) {
        if (isNerve(GET_NERVE(GliBird, GliBirdNrvDeadDamage)) || isNerve(GET_NERVE(GliBird, GliBirdNrvDeadWall))) {
            MR::setBaseTRMtx(mRider, mPlayerDeadMtx);
        } else {
            MR::setBaseTRMtx(mRider, MR::getJointMtx(this, "MarioPos"));
        }

        return true;
    }

    if (MR::isMsgRushCancel(msg)) {
        restart();
        return true;
    }

    if (MR::isMsgRaceReady(msg)) {
        MR::validateHitSensor(this, "Bind");
        mDirFront.set(mFront);
        setNerve(GET_NERVE(GliBird, GliBirdNrvReady));

        if (MR::isValidSwitchA(this)) {
            MR::onSwitchA(this);
        }

        return true;
    }

    if (MR::isMsgRaceStart(msg)) {
        setNerve(GET_NERVE(GliBird, GliBirdNrvStart));
        return true;
    }

    if (MR::isMsgRaceReset(msg)) {
        MR::endBindAndPlayerWait(this);

        if (MR::isValidSwitchA(this)) {
            MR::offSwitchA(this);
        }

        if (MR::isDead(mNpc)) {
            mNpc->makeActorAppeared();
        }

        MR::resetSupportTicoAttachRangeGliBird();
        kill();
        return true;
    }

    return false;
}

void GliBird::damage(bool isFire) {
    if (isFire || MR::isBindedDamageFire(this)) {
        MR::emitEffect(this, "FireDamage");
        MR::tryHitReaction(this, "溶岩ダメージ");

        if (!mNoRumble) {
            MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        }
    } else {
        MR::tryHitReaction(this, "ダメージ");

        if (!mNoRumble) {
            MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        }
    }

    if (mDamageTimer <= 0) {
        if (MR::isPlayerLifeOne()) {
            setNerve(GET_NERVE(GliBird, GliBirdNrvDeadDamage));
        } else {
            fn_80051910();
            setNerve(GET_NERVE(GliBird, GliBirdNrvDamage));
        }

        fn_8004FE50();
        mDamageTimer = 180;
        return;
    }

    setNerve(GET_NERVE(GliBird, GliBirdNrvDamage));
}

void GliBird::deadLava() {
    MR::startAction(this, "GliBirdGlideDeadLand");
    MR::hideModel(this);
    MR::hidePlayer();
    MR::zeroVelocity(this);
    MR::tryDeleteEffect(this, "GroundApproach");
    MR::tryHitReaction(this, "溶岩死亡");

    if (!mNoRumble) {
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
    }

    MtxPtr baseMtx = getBaseMtx();
    TVec3f front(baseMtx[0][2], baseMtx[1][2], baseMtx[2][2]);
    MR::makeMtxUpFrontPos(&mLavaBigMtx, mUp, front, mPosition);
    MR::emitEffect(this, "GliBirdLavaInBig");
    setNerve(GET_NERVE(GliBird, GliBirdNrvDeadLava));
}

void GliBird::restart() {
    mRider = nullptr;
    MR::resetPosition(this, mInitPos);
    MR::zeroVelocity(this);
    mSpeed = 0.0f;
    mFriction = 0.0f;
    mSideAccel = 0.0f;
    mPitchAngle = 0.0f;
    mPitchAccel = 0.0f;
    mYawAngle = 0.0f;
    mYawAccel = 0.0f;
    mDirFront.set(mFront);
    mDirUp.set(mUp);
    setNerve(GET_NERVE(GliBird, GliBirdNrvHide));
}

bool GliBird::updateGlide(f32 frictionRate, f32 sideRate) {
    if (updateCollision()) {
        return true;
    }

    mSpeed *= 0.995f;
    f32 gravityRate = MR::clamp(mGravity.dot(mDirFront), 0.0f, 1.0f);
    mSpeed += gravityRate * 0.8f;
    mSpeed = MR::clamp(mSpeed, 15.0f, 50.0f);

    TVec3f yAxis(0, 1, 0);
    TVec3f negGravity = -mGravity;
    MR::turnVecToVecDegree(&mUp, mUp, negGravity, 1.0f, yAxis);

    TVec3f upTmp(mDirUp);
    MR::makeAxisFrontUp(&mSide, &upTmp, mFront, mUp);
    MR::vecKillElement(mVelocity, mDirFront, &mVelocity);
    MR::vecKillElement(mVelocity, mSide, &mVelocity);

    mFriction = mVelocity.dot(mUp) * frictionRate - sideRate * (1.0f - MR::abs(gravityRate));
    mFriction = MR::clamp(mFriction, -3.5f, 30.0f);

    TVec3f pad(0.0f, 0.0f, 0.0f);
    MR::getCorePadAcceleration(&pad, WPAD_CHAN0);
    pad.y = MR::clamp(pad.y, -0.75f, 0.6f) + 0.42f;

    mPitchAccel *= 0.95f;
    f32 pitchAdd;
    if (pad.y > 0.0f) {
        pitchAdd = 0.0f + pad.y * 1.0f;
    } else {
        pitchAdd = 0.0f + pad.y * 1.3f;
    }
    mPitchAccel += pitchAdd;
    mPitchAccel = MR::clamp(mPitchAccel, -30.0f, 30.0f);

    mPitchAngle = mPitchAngle * 0.7f + mPitchAccel;
    f32 speedRate = (mSpeed - 15.0f) / (50.0f - 15.0f);
    f32 ease = MR::getEaseOutValue(speedRate, 0.0f, 1.0f, 1.0f);
    ease = MR::getEaseOutValue(ease, 0.0f, 1.0f, 1.0f);
    mPitchAngle = MR::clamp(mPitchAngle, -(15.0f * ease), 60.0f);
    MR::rotateVecDegree(&mDirFront, mFront, mSide, mPitchAngle);

    mYawAccel *= 0.98f;
    if (pad.z > -0.7f || MR::abs(pad.x) > 0.15f) {
        f32 padX = pad.x;
        f32 padZ = MR::clamp(-pad.z, 0.0f, 1.0f);

        if (padX > 0.0f) {
            padX += padZ * 1.0f;
        } else if (padX < 0.0f) {
            padX -= padZ * 1.0f;
        }

        padX = MR::clamp(padX, -1.0f, 1.0f);
        mYawAccel += padX * 0.3f;
        mYawAccel = MR::clamp(mYawAccel, -30.0f, 30.0f);
    }

    if (MR::abs(mYawAccel) < 0.1f) {
        mYawAccel = 0.0f;
    }

    mYawAngle = mYawAngle * 0.7f + mYawAccel;
    mYawAngle = MR::clamp(mYawAngle, -45.0f, 45.0f);
    MR::rotateVecDegree(&mDirUp, mUp, mFront, mYawAngle);

    mSideAccel = 0.02f * (-mYawAngle * mSpeed);
    mSideAccel = MR::clamp(mSideAccel, -50.0f, 50.0f);

    TVec3f sideTmp(0.0f);
    MR::makeAxisFrontUp(&sideTmp, &mDirUp, mDirFront, mDirUp);

    TVec3f negUp = -mUp;
    mVelocity.set(negUp);
    MR::attenuateVelocity(this, -mFriction);
    TVec3f frontVel = mDirFront * mSpeed;
    MR::addVelocity(this, frontVel);
    TVec3f sideVel = mSide * mSideAccel;
    MR::addVelocity(this, sideVel);

    bool canRumble = !mNoRumble;
    if (MR::isEndGliBirdTutorialTalk()) {
        mRumbleCount = -1;
    } else if (mRumbleCount > 300) {
        canRumble = false;
    }

    f32 velLength = MR::calcVelocityLength(this);
    f32 velDelta = velLength - mLastSpeed;
    mLastSpeed = velLength;

    if (mSpeed >= 50.0f) {
        if (canRumble) {
            MR::tryRumblePadWeak(this, WPAD_CHAN0);
        }

        MR::startActionSound(this, "FlyHighSpeed", 100.0f * velLength, -1, -1);
        MR::startActionSound(this, "FlyHighSpeedVoice", -1, -1, 30);
        mRumbleCount++;
    } else if (velDelta > 0.4f) {
        if (canRumble) {
            MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        }

        mRumbleCount++;
    } else if (velDelta > 0.15f) {
        if (canRumble) {
            MR::tryRumblePadWeak(this, WPAD_CHAN0);
        }

        mRumbleCount++;
    } else {
        mRumbleCount = 0;
    }

    mRollAccel *= 0.98f;
    if (pad.z > -0.7f || MR::abs(pad.x) > 0.15f) {
        f32 padX = pad.x;
        f32 padZ = MR::clamp(-pad.z, 0.0f, 1.0f);

        if (padX > 0.0f) {
            padX += padZ * 1.0f;
        } else if (padX < 0.0f) {
            padX -= padZ * 1.0f;
        }

        padX = MR::clamp(padX, -1.0f, 1.0f);
        mRollAccel += padX * 0.3f;
        mRollAccel = MR::clamp(mRollAccel, -10.0f, 10.0f);
    }

    mRollAngle = mRollAngle * 0.8f + mRollAccel;
    mRollAngle = MR::clamp(mRollAngle, -50.0f, 50.0f);

    if (pad.z > -0.7f || MR::abs(pad.x) > 0.15f) {
        if (pad.x < 0.0f) {
            mBlend = MR::getInterpolateValue(0.12f, mBlend, MR::clamp(pad.x, -1.0f, 0.0f));
        } else if (pad.x > 0.0f) {
            mBlend = MR::getInterpolateValue(0.12f, mBlend, MR::clamp(pad.x, 0.0f, 1.0f));
        }
    } else {
        mBlend = MR::getInterpolateValue(0.12f, mBlend, 0.0f);
    }

    f32 blend = mBlend;
    if (blend > 0.0f) {
        fn_800095D0(this, 1.0f - blend, 0.0f, blend);
    } else {
        fn_800095D0(this, 1.0f + blend, -blend, 0.0f);
    }

    if (blend > 0.0f) {
        MR::setBckBlendWeight(1.0f - blend, 0.0f, blend);
    } else {
        MR::setBckBlendWeight(1.0f + blend, -blend, 0.0f);
    }

    return false;
}

bool GliBird::updateCollision() {
    if (MR::isBinded(this)) {
        if (MR::isBindedGroundWater(this)) {
            touchWater(MR::getBindedHitPos(this), 100.0f);
            mWaterCount++;

            if (mWaterCount >= 5) {
                MR::hideModel(this);
                MR::hidePlayer();
                MR::zeroVelocity(this);
                MR::deleteEffect(this, "NearWater");
                MR::tryDeleteEffect(this, "GroundApproach");
                MR::tryHitReaction(this, "水面死亡");

                if (!mNoRumble) {
                    MR::tryRumblePadMiddle(this, WPAD_CHAN0);
                }

                setNerve(GET_NERVE(GliBird, GliBirdNrvDeadWater));
                return true;
            }

            return false;
        }

        if (MR::isBindedGroundDamageFire(this)) {
            deadLava();
            return true;
        }

        if (mWaterCount > 0) {
            mWaterCount--;
        }

        const TVec3f* pNormal = MR::getBindedPlaneNormal(this, 0);
        reactionCollision(*pNormal, MR::getBindedPlaneSensor(this, 0), false);
        return true;
    }

    if (mWaterCount > 0) {
        mWaterCount--;
    }

    TPos3f jointMtx;
    MR::calcPlayerJointMtx(&jointMtx, "Center");
    TVec3f pos(jointMtx[0][3], jointMtx[1][3], jointMtx[2][3]);
    TVec3f up(jointMtx[0][1], jointMtx[1][1], jointMtx[2][1]);
    TVec3f front(jointMtx[0][2], jointMtx[1][2], jointMtx[2][2]);

    f32 radius = *mParam->mRadius;
    TVec3f upOffset = up * *mParam->mOffsetUp;
    TVec3f frontOffset = front * *mParam->mOffsetFront;
    TVec3f offset(frontOffset);
    offset.add(upOffset);
    pos.add(offset);

    TVec3f hitPos(0.0f);
    TVec3f hitNormal(0.0f);
    HitSensor* pHitSensor = nullptr;
    f32 depth = 0.0f;
    bool isWater = false;
    bool isFire = false;
    s32 isHit = fn_80038770(&hitPos, &hitNormal, &pHitSensor, pos, &isWater, &isFire, radius);

    if (isHit) {
        depth = hitPos.y - (pos.y - radius);
    }

    if (!isHit) {
        isHit = fn_80038870(mPosition, &hitPos, mGravity, &hitNormal, &pHitSensor, &isWater, &isFire, 100.0f);

        if (isHit) {
            depth = mPosition.y - hitPos.y;
        }
    }

    if (isHit) {
        if (isWater) {
            touchWater(hitPos, depth);
            return false;
        }

        reactionCollision(hitNormal, pHitSensor, isFire);
        return true;
    }

    if (mIsWaterEffect) {
        MR::deleteEffect(this, "NearWater");
        mIsWaterEffect = false;
    }

    return false;
}

void GliBird::reactionCollision(const TVec3f& rNormal, HitSensor* pSensor, bool isFire) {
    f32 frontDot = mGravity.dot(rNormal);
    f32 sideDot = mSide.dot(rNormal);
    mVelocity.dot(rNormal);

    if (!tryDeadWall(rNormal, pSensor, isFire)) {
        if (MR::abs(frontDot) > 0.01f) {
            if ((frontDot > 0.0f && mPitchAngle < 0.0f) || (frontDot < 0.0f && mPitchAngle > 0.0f)) {
                mPitchAngle *= -1.0f;
                mPitchAccel *= -1.0f;
            }
        } else if (MR::abs(sideDot) > 0.01f && ((sideDot > 0.0f && mYawAngle > 0.0f) || (sideDot < 0.0f && mYawAngle < 0.0f))) {
            mYawAngle *= -1.0f;
            mYawAccel *= -1.0f;
        }

        f32 velDot = mVelocity.dot(rNormal);
        if (velDot < 0.0f) {
            TVec3f negNormal = -rNormal;
            TVec3f a = negNormal * (1.0f + 1.0f);
            TVec3f b = a * velDot;
            TVec3f c = negNormal * (1.0f + 1.0f);
            TVec3f d = c * velDot;
            mVelocity.sub(b);
            mVelocity.sub(d);
        }

        TVec3f push = rNormal * 15.0f;
        mVelocity.add(push);

        if (mDamageTimer <= 0) {
            mSpeed *= 0.5f;

            if (frontDot < -0.08f) {
                TVec3f extra = rNormal * -30.0f;
                mVelocity.add(extra);
            }
        }

        damage(isFire);
    }
}

bool GliBird::tryDeadWall(const TVec3f& rNormal, HitSensor* pSensor, bool isFire) {
    f32 frontDot = mDirFront.dot(rNormal);

    if (!MR::sendArbitraryMsg(0x55, pSensor, getSensor("Body")) && (MR::abs(frontDot) > 0.8f || MR::abs(mFront.dot(rNormal)) > 0.8f)) {
        if (isFire) {
            deadLava();
            return true;
        }

        MR::tryHitReactionAtPos(this, "壁ヒット死亡", &mDeadBody->mPosition);

        if (!mNoRumble) {
            MR::tryRumblePadMiddle(this, WPAD_CHAN0);
        }

        setNerve(GET_NERVE(GliBird, GliBirdNrvDeadWall));
        return true;
    }

    return false;
}

void GliBird::touchWater(const TVec3f& rPos, f32 depth) {
    if (!mIsWaterEffect) {
        MR::emitEffect(this, "NearWater");
        mIsWaterEffect = true;
    }

    MR::tryHitReaction(this, "水面接触");

    if (!mNoRumble) {
        MR::tryRumblePadMiddle(this, WPAD_CHAN0);
    }

    f32 lift = 0.01f * depth;
    mPitchAccel -= 0.02f + lift;

    MtxPtr baseMtx = getBaseMtx();
    TVec3f front(baseMtx[0][2], baseMtx[1][2], baseMtx[2][2]);
    MR::makeMtxUpFrontPos(&mWaterMtx, mUp, front, rPos);
    mSpeed *= *mParam->mWaterFriction;
    MR::addVelocityToGravity(this, -(*mParam->mWaterLiftMin + *mParam->mWaterLiftRate * depth));
}

void GliBird::exeDeadCommon(const f32* pBirdUpFront, const f32* pBirdSide, const f32* pPlayerUpFront, const f32* pPlayerSide) {
    if (MR::isFirstStep(this)) {
        mDirFront.set(mFront);
        mRollAngle = 0.0f;
        mPlayerDeadMtx.set(MR::getJointMtx(this, "MarioPos"));
        mDeadBody->mPosition.set(mPlayerDeadMtx[0][3], mPlayerDeadMtx[1][3], mPlayerDeadMtx[2][3]);
        TVec3f front(mPlayerDeadMtx[0][2], mPlayerDeadMtx[1][2], mPlayerDeadMtx[2][2]);
        TVec3f negVel = -mVelocity;
        MR::makeMtxUpFront(&mPlayerDeadMtx, negVel, front);
        MR::startAction(this, "GliBirdGlideDead");
        MR::startBckPlayer("GliBirdGlideDead", nullptr);
        MR::tryDeleteEffect(this, "NearWater");
        fn_800126F0(this, &mFront, pBirdUpFront);
        TVec3f birdSide = mSide * *pBirdSide;
        MR::addVelocity(this, birdSide);
        mDeadBody->mVelocity.set(mVelocity);
        fn_800126F0(mDeadBody, &mFront, pPlayerUpFront);
        TVec3f playerSide = mFront * *pPlayerSide;
        MR::addVelocity(mDeadBody, playerSide);
        mCameraTarget->setMtx(mPlayerDeadMtx);
        MR::startMultiActorCameraTargetMtx(this, mCameraInfo, "死亡", mCameraTarget, -1);
    }

    if (!MR::isNoBind(this)) {
        MR::addVelocityToGravity(this, *mParam->mDeadGravity);

        if (MR::isGreaterStep(this, 5) && MR::isBindedGround(this)) {
            if (MR::isBindedGroundWater(this)) {
                MR::tryHitReaction(this, "水面死亡");

                if (!mNoRumble) {
                    MR::tryRumblePadMiddle(this, WPAD_CHAN0);
                }

                MR::hideModel(this);
                mWaterMtx.set(getBaseMtx());
                mBirdBreakModel->appear();
            } else if (MR::isBindedGroundDamageFire(this)) {
                mLavaSmallMtx.set(getBaseMtx());
                MR::emitEffect(this, "GliBirdLavaInSmall");
                MR::tryHitReaction(this, "溶岩死亡");

                if (!mNoRumble) {
                    MR::tryRumblePadMiddle(this, WPAD_CHAN0);
                }

                MR::hideModelAndOnCalcAnim(this);
            } else {
                MR::startAction(this, "GliBirdGlideDeadLand");
            }

            MR::offBind(this);
            MR::zeroVelocity(this);
        }
    }

    if (!MR::isDead(mDeadBody)) {
        mDeadBody->movement();
        mPlayerDeadMtx.setTrans(mDeadBody->mPosition);

        if (MR::isGreaterStep(this, 5) && MR::isBindedGround(mDeadBody)) {
            if (MR::isBindedGroundWater(mDeadBody)) {
                MR::tryHitReaction(this, "水面死亡");

                if (!mNoRumble) {
                    MR::tryRumblePadMiddle(this, WPAD_CHAN0);
                }

                MR::hidePlayer();
                mPlayerBreakModel->appear();
            } else if (MR::isBindedGroundDamageFire(mDeadBody)) {
                mLavaPlayerMtx.setTrans(mDeadBody->mPosition);
                MR::emitEffect(this, "GliBirdLavaInPlayer");
                MR::tryHitReaction(this, "溶岩死亡");

                if (!mNoRumble) {
                    MR::tryRumblePadMiddle(this, WPAD_CHAN0);
                }

                MR::hidePlayer();
            } else {
                MR::startBckPlayer("GliBirdGlideDeadLand", nullptr);
            }

            MR::tryHitReactionAtPos(this, "プレイヤー死亡着地", &mDeadBody->mPosition);

            if (!mNoRumble) {
                MR::tryRumblePadWeak(this, WPAD_CHAN0);
            }

            MR::zeroVelocity(mDeadBody);
            mDeadBody->kill();
        }
    }

    if (MR::isStep(this, 90)) {
        fn_8004F030();
    }
}

namespace {
    static TVec3f sFootOffsetL(-15.0f, 10.0f, 20.0f);
    static TVec3f sFootOffsetR(-15.0f, -10.0f, 20.0f);
};  // namespace

GliBirdFootJointCtrl::GliBirdFootJointCtrl(GliBird* pHost) : LiveActor("足ジョイント制御"), mHost(pHost) {
}

void GliBirdFootJointCtrl::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, -1, 0xB, -1, -1);
    MR::invalidateClipping(this);
    makeActorAppeared();
}

static void calcFootJoint(GliBird* pHost, const char* pJointName, const char* pPlayerJointName, const TVec3f& rOffset, f32 rate, f32 scale) {
    TPos3f playerMtx;
    playerMtx.identity();
    TPos3f jointMtx;
    jointMtx.identity();

    MtxPtr pJointMtx = MR::getJointMtx(pHost, pJointName);
    TVec3f playerPos(0.0f);
    TVec3f birdPos(0.0f);
    MR::calcPlayerJointMtx(&playerMtx, pPlayerJointName);
    playerMtx.getTrans(playerPos);
    playerMtx.mult(rOffset, playerPos);
    jointMtx.set(pJointMtx);
    jointMtx.getTrans(birdPos);

    TVec3f dir(birdPos);
    dir.sub(playerPos);
    MR::normalizeOrZero(&dir);
    TVec3f dirScaled = dir * scale;
    playerPos.add(dirScaled);

    TVec3f birdPart = birdPos * (1.0f - rate);
    TVec3f playerPart = playerPos * rate;
    TVec3f result(playerPart);
    result.add(birdPart);
    jointMtx[0][3] = result.x;
    jointMtx[1][3] = result.y;
    jointMtx[2][3] = result.z;
    PSMTXCopy(jointMtx, pJointMtx);
}

void GliBirdFootJointCtrl::calcAnim() {
    if (mHost->isEnableFootJointCtrl()) {
        calcFootJoint(mHost, "FingerL", "HandL0", sFootOffsetL, 0.5f, 5.0f);
        calcFootJoint(mHost, "FingerR", "HandR0", sFootOffsetR, 0.5f, 5.0f);
        calcFootJoint(mHost, "LegL3", "HandL0", sFootOffsetL, 0.5f, 5.0f);
        calcFootJoint(mHost, "LegR3", "HandR0", sFootOffsetR, 0.5f, 5.0f);
        calcFootJoint(mHost, "LegL2", "HandL0", sFootOffsetL, 0.1f, 5.0f);
        calcFootJoint(mHost, "LegR2", "HandR0", sFootOffsetR, 0.1f, 5.0f);
        calcFootJoint(mHost, "LegL1", "HandL0", sFootOffsetL, 0.05f, 5.0f);
        calcFootJoint(mHost, "LegR1", "HandR0", sFootOffsetR, 0.05f, 5.0f);
    }
}

namespace {
    static TVec3f sPichanOffset(-400.0f, 0.0f, -300.0f);
    static TVec3f sPichanRot(0.0f, 60.0f, 0.0f);
    static const char* sTutorialPosName = "チュートリアル位置";
};  // namespace

namespace NrvGliBirdTutorial {
    NEW_NERVE(GliBirdTutorialNrvTalkFirst, GliBirdTutorial, TalkFirst);
    NEW_NERVE(GliBirdTutorialNrvTalkStraight, GliBirdTutorial, TalkStraight);
    NEW_NERVE(GliBirdTutorialNrvStraightStart, GliBirdTutorial, StraightStart);
    NEW_NERVE(GliBirdTutorialNrvStraightHold, GliBirdTutorial, StraightHold);
    NEW_NERVE(GliBirdTutorialNrvStraightSuccess, GliBirdTutorial, StraightSuccess);
    NEW_NERVE(GliBirdTutorialNrvTalkTurnLeft, GliBirdTutorial, TalkTurnLeft);
    NEW_NERVE(GliBirdTutorialNrvTurnLeftStart, GliBirdTutorial, TurnLeftStart);
    NEW_NERVE(GliBirdTutorialNrvTurnLeftHold, GliBirdTutorial, TurnLeftHold);
    NEW_NERVE(GliBirdTutorialNrvTurnLeftSuccess, GliBirdTutorial, TurnLeftSuccess);
    NEW_NERVE(GliBirdTutorialNrvTalkTurnRight, GliBirdTutorial, TalkTurnRight);
    NEW_NERVE(GliBirdTutorialNrvTurnRightStart, GliBirdTutorial, TurnRightStart);
    NEW_NERVE(GliBirdTutorialNrvTurnRightHold, GliBirdTutorial, TurnRightHold);
    NEW_NERVE(GliBirdTutorialNrvTurnRightSuccess, GliBirdTutorial, TurnRightSuccess);
    NEW_NERVE(GliBirdTutorialNrvEnd, GliBirdTutorial, End);
};  // namespace NrvGliBirdTutorial

GliBirdTutorial::GliBirdTutorial()
    : LiveActor("グライバードチュートリアル"), mGliBird(nullptr), mPichan(nullptr), mPichanPos(0.0f, 0.0f, 0.0f), mPichanRot(0.0f, 0.0f, 0.0f),
      mGuidance(nullptr), mTalkCtrl(nullptr), mPadAccel(0.0f, 0.0f, 0.0f), mFrame(0), mTutorialPos(0.0f, 0.0f, 0.0f), mTimerCount(0) {
}

void GliBirdTutorial::init(const JMapInfoIter& rIter) {
    MR::connectToSceneMapObjMovement(this);
    mGuidance = new GliBirdGuidance("ガイダンス用レイアウト");
    mGuidance->initWithoutIter();
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTalkFirst), 0);
    TVec3f soundOffset(0.0f);
    initSound(4, "GliBirdTutorial", nullptr, soundOffset);
    MR::tryFindNamePos(::sTutorialPosName, &mTutorialPos, nullptr);
    makeActorDead();
}

void GliBirdTutorial::movement() {
    MR::getCorePadAcceleration(&mPadAccel, WPAD_CHAN0);
    f32 sinValue = JMASinDegree(mFrame * 3.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    TVec3f offset = up * sinValue;
    TVec3f offsetScaled = offset * 30.0f;
    TVec3f pos(mTutorialPos);
    pos.add(offsetScaled);
    mFrame++;
    MR::setPosition(mGliBird, pos);
    LiveActor::movement();
}

void GliBirdTutorial::appear() {
    LiveActor::appear();
    MR::requestSupportTicoHide();
    MR::emitEffect(mGliBird, "GliBirdTutorialDust");
    MR::onGameEventFlagGliBirdTutorialAtFirst();
}

void GliBirdTutorial::kill() {
    LiveActor::kill();
    MR::requestSupportTicoShow();
    MR::deleteEffect(mGliBird, "GliBirdTutorialDust");
    MR::onGameEventFlagGliBirdTutorialAtFirst();
}

void GliBirdTutorial::registerPichan(LiveActor* pPichan) {
    mPichan = pPichan;
    mTalkCtrl = MR::createTalkCtrlDirect(mPichan, "GliBirdTutorial", TVec3f(0.0f, 0.0f, 0.0f), nullptr);
}

void GliBirdTutorial::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::closeWipeFade(-1);
    }

    if (!MR::isWipeActive()) {
        MR::resetPosition(mPichan, mPichanPos);
        MR::setRotation(mPichan, mPichanRot);
        mGuidance->kill();
        kill();
    }
}

void GliBirdTutorial::exeTalkFirst() {
    if (MR::isFirstStep(this)) {
        MR::startGliderBGM();
        MR::resetAndForwardNode(mTalkCtrl, 0);
        mGliBird->mIsTutorial = true;
        mGliBird->showBird();
        mPichanPos.set(mPichan->mPosition);
        mPichanRot.set(mPichan->mRotation);
        TVec3f pos(0.0f, 0.0f, 0.0f);
        MR::findNamePos(::sTutorialPosName, &pos, nullptr);
        pos.add(::sPichanOffset);
        MR::resetPosition(mPichan, pos);
        MR::setRotation(mPichan, ::sPichanRot);
        MR::setPlayerPos(::sTutorialPosName);
        MR::startAction(mPichan, "Talk");
    }

    if (MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mTalkCtrl)) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTalkStraight));
        mGliBird->mIsTutorial = false;
    }
}

void GliBirdTutorial::exeTalkStraight() {
    if (MR::isFirstStep(this)) {
        MR::resetAndForwardNode(mTalkCtrl, 1);
        mGuidance->appear();
        MR::startAction(mPichan, "Talk");
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (MR::isStep(this, 120)) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvStraightStart));
    }
}

void GliBirdTutorial::exeStraightStart() {
    if (MR::isFirstStep(this)) {
        mGuidance->start(0);
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (isOkStraight()) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvStraightHold));
    }
}

void GliBirdTutorial::exeStraightHold() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OK", -1, -1, -1);
        mGuidance->hold(0);
    }

    startTimerSound(getNerveStep(), mTimerCount);

    if (MR::isStep(this, 120)) {
        MR::startActionSound(this, "GoNext", mTimerCount, -1, -1);
        mTimerCount++;
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvStraightSuccess));
        return;
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (!isOkStraight()) {
        MR::startActionSound(this, "NG", -1, -1, -1);
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvStraightStart));
    }
}

void GliBirdTutorial::exeStraightSuccess() {
    if (MR::isFirstStep(this)) {
        mGuidance->success(0);
        MR::startAction(mPichan, "TalkSmile");
    }

    if (!MR::isLessStep(this, 30)) {
        if (MR::isStep(this, 30)) {
            MR::resetAndForwardNode(mTalkCtrl, 2);
        }

        if (MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mTalkCtrl)) {
            setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTalkTurnLeft));
        }
    }
}

void GliBirdTutorial::exeTalkTurnLeft() {
    if (MR::isFirstStep(this)) {
        MR::resetAndForwardNode(mTalkCtrl, 3);
        MR::startAction(mPichan, "Talk");
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (MR::isStep(this, 120)) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnLeftStart));
    }
}

void GliBirdTutorial::exeTurnLeftStart() {
    if (MR::isFirstStep(this)) {
        mGuidance->start(1);
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (isOkTurnLeft()) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnLeftHold));
    }
}

void GliBirdTutorial::exeTurnLeftHold() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OK", -1, -1, -1);
        mGuidance->hold(1);
    }

    startTimerSound(getNerveStep(), mTimerCount);

    if (MR::isStep(this, 120)) {
        MR::startActionSound(this, "GoNext", mTimerCount, -1, -1);
        mTimerCount++;
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnLeftSuccess));
        return;
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (!isOkTurnLeft()) {
        MR::startActionSound(this, "NG", -1, -1, -1);
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnLeftStart));
    }
}

void GliBirdTutorial::exeTurnLeftSuccess() {
    if (MR::isFirstStep(this)) {
        mGuidance->success(1);
        MR::startAction(mPichan, "TalkSmile");
    }

    if (!MR::isLessStep(this, 30)) {
        if (MR::isStep(this, 30)) {
            MR::resetAndForwardNode(mTalkCtrl, 4);
        }

        if (MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mTalkCtrl)) {
            setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTalkTurnRight));
        }
    }
}

void GliBirdTutorial::exeTalkTurnRight() {
    if (MR::isFirstStep(this)) {
        MR::resetAndForwardNode(mTalkCtrl, 5);
        MR::startAction(mPichan, "Talk");
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (MR::isStep(this, 120)) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnRightStart));
    }
}

void GliBirdTutorial::exeTurnRightStart() {
    if (MR::isFirstStep(this)) {
        mGuidance->start(2);
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (isOkTurnRight()) {
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnRightHold));
    }
}

void GliBirdTutorial::exeTurnRightHold() {
    if (MR::isFirstStep(this)) {
        MR::startActionSound(this, "OK", -1, -1, -1);
        mGuidance->hold(2);
    }

    startTimerSound(getNerveStep(), mTimerCount);

    if (MR::isStep(this, 120)) {
        MR::startActionSound(this, "GoNext2", mTimerCount, -1, -1);
        mTimerCount++;
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnRightSuccess));
        return;
    }

    MR::tryTalkForceWithoutDemoMarioPuppetable(mTalkCtrl);

    if (!isOkTurnRight()) {
        MR::startActionSound(this, "NG", -1, -1, -1);
        setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvTurnRightStart));
    }
}

void GliBirdTutorial::exeTurnRightSuccess() {
    if (MR::isFirstStep(this)) {
        mGuidance->success(2);
        MR::startAction(mPichan, "TalkSmile");
    }

    if (!MR::isLessStep(this, 30)) {
        if (MR::isStep(this, 30)) {
            MR::resetAndForwardNode(mTalkCtrl, 6);
        }

        if (MR::tryTalkForceWithoutDemoMarioPuppetableAtEnd(mTalkCtrl)) {
            setNerve(GET_NERVE(GliBirdTutorial, GliBirdTutorialNrvEnd));
        }
    }
}

bool GliBirdTutorial::isOkStraight() const {
    bool result = false;
    if (MR::abs(mPadAccel.x) < 0.25f && MR::abs(mPadAccel.y) < 0.45f) {
        result = true;
    }

    return result;
}

bool GliBirdTutorial::isOkTurnLeft() const {
    bool result = false;
    if (mPadAccel.x < -0.65f && MR::abs(mPadAccel.y) < 0.5f) {
        result = true;
    }

    return result;
}

bool GliBirdTutorial::isOkTurnRight() const {
    bool result = false;
    if (MR::abs(mPadAccel.x) < 0.6f && 0.05f < mPadAccel.y) {
        result = true;
    }

    return result;
}

void GliBirdTutorial::startTimerSound(s32 step, s32 count) {
    if (step == 30) {
        MR::startActionSound(this, "TutorialTimer", count, -1, -1);
    }

    if (step == 60) {
        MR::startActionSound(this, "TutorialTimer2", count, -1, -1);
    }

    if (step == 90) {
        MR::startActionSound(this, "TutorialTimer3", count, -1, -1);
    }
}

namespace MR {
    void registerGliBirdForTutorial(GliBird* pGliBird) {
        MR::createSceneObj(SceneObj_GliBirdTutorial);
        MR::getSceneObj< GliBirdTutorial >(SceneObj_GliBirdTutorial)->mGliBird = pGliBird;
    }

    void registerPichanForTutorial(Pichan* pPichan) {
        MR::createSceneObj(SceneObj_GliBirdTutorial);
        MR::getSceneObj< GliBirdTutorial >(SceneObj_GliBirdTutorial)->registerPichan(pPichan);
    }

    void startGliBirdTutorialTalk() {
        MR::getSceneObj< GliBirdTutorial >(SceneObj_GliBirdTutorial)->appear();
    }

    bool isEndGliBirdTutorialTalk() {
        return MR::isDead(MR::getSceneObj< GliBirdTutorial >(SceneObj_GliBirdTutorial));
    }
};  // namespace MR
