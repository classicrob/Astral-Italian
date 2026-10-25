#include "Game/Util/Functor.hpp"
#pragma auto_inline off
#include "Game/Boss/KoopaBattle.hpp"
#include "Game/Boss/KoopaBattleAccessor.hpp"
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Boss/KoopaStates.hpp"
#include "Game/LiveActor/ActorJointCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/LiveActor/VolumeDrawer.hpp"
#include "Game/Util.hpp"
#include "Game/Util/BaseMatrixFollowTargetHolder.hpp"

namespace {
    // scale of the attack range volume (cylinder) drawn around Bowser
    TVec3f sVolumeScale(1.4f, 8.0f, 1.4f);
};  // namespace

namespace NrvKoopaBattle {
    NEW_NERVE(KoopaBattleNrvBattle, KoopaBattle, Battle);
};  // namespace NrvKoopaBattle

KoopaBattle::KoopaBattle(const char* pName)
    : LiveActor(pName), mSequence(nullptr), mJointCtrl(nullptr), mCameraInfo(nullptr), mShell(nullptr), mDefenseForm(nullptr),
      mDefenseCollision(nullptr), mVolumeDrawer(nullptr), mPoseQuat(0.0f, 0.0f, 0.0f, 1.0f), mBattleQuat(0.0f, 0.0f, 0.0f, 1.0f), mBattleOrigin(0.0f, 0.0f, 0.0f),
      mBattleNo(-1), mIsDefenseForm(false) {
    MR::createKoopaBattleAccessor();
    MR::getKoopaBattleAccessor()->registerKoopaBattle(this);
    mBaseMtx.identity();
}

// NONMATCHING: saved registers and vector temporary stack allocation.
void KoopaBattle::init(const JMapInfoIter& rIter) {
    s32 level = 1;
    const char* pName = "KoopaVs1";
    if (MR::isObjectName(rIter, "KoopaLv2")) {
        level = 2;
        pName = "KoopaVs2";
    }
    if (MR::isObjectName(rIter, "KoopaLv3")) {
        level = 3;
        pName = "KoopaVs3";
    }
    MR::initActor(this, rIter, pName, false);
    MR::makeQuatFromRotate(&mPoseQuat, this);
    mBattleQuat.set< f32 >(mPoseQuat);
    mBattleOrigin.set< f32 >(mPosition);
    MR::getKoopaBattleAccessor()->setPoseA(this);
    {
        TVec3f front;
        MR::calcFrontVec(&front, this);
        mPosition.set< f32 >(mPosition - front.scaleInline(4000.0f));
    }
    mJointCtrl = new ActorJointCtrl(this);
    mJointCtrl->endFaceCtrl(0);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initActorCameraProgrammable(this);
    switch (level) {
    default:
        mSequence = new KoopaStateLv1Sequence(this, rIter);
        break;
    case 2:
        mSequence = new KoopaStateLv2Sequence(this, rIter);
        break;
    case 3:
        mSequence = new KoopaStateLv3Sequence(this, rIter);
        break;
    }
    initNerve(&NrvKoopaBattle::KoopaBattleNrvBattle::sInstance, 1);
    MR::initActorState(this, mSequence, &NrvKoopaBattle::KoopaBattleNrvBattle::sInstance, "PhaseSequence");
    BaseMatrixFollowValidater* pValidater =
        MR::createBaseMatrixFollowValidateDelegator(this, &KoopaBattle::isValidFollowId);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, pValidater);
    mShell = MR::createPartsModelEnemy(this, "クッパ殻モデル", "KoopaShell", MR::getJointMtx(this, "Armor"));
    mShell->mScale.set(1.0f, 1.0f, 1.0f);
    MR::registerDemoSimpleCastAll(mShell);
    MR::startBtk(mShell, "Scale");
    mVolumeDrawer = MR::createVolumeDrawer("攻撃範囲ボリューム描画", "VolumeCylinder", mBaseMtx.toMtxPtr());
    mVolumeDrawer->setColor(Color8(0xF0, 0x78, 0x50, 0x78));
    mVolumeDrawer->setDrawPasses(5);
    mVolumeDrawer->setScale(TVec3f(0.05f, 0.0f, 0.05f));
    mVolumeDrawer->setScaleDownRate(-1);
    MR::setScale(mVolumeDrawer, sVolumeScale);
    MR::declarePowerStar(this);
    MR::declareCoin(this, 0x20);
    makeActorDead();
    MR::invalidateShadowAll(this);
    if (MR::initUseStageSwitchReadA(this, rIter)) {
        MR::listenStageSwitchOnA(this, MR::FunctorV0M< KoopaBattle*, void (KoopaBattle::*)() >(this, &KoopaBattle::startBattle));
    } else {
        setNerve(&NrvKoopaBattle::KoopaBattleNrvBattle::sInstance);
        makeActorAppeared();
    }
    MR::hideModelAndOnCalcAnim(this);
}

void KoopaBattle::appearDefenseForm() {
    if (mDefenseForm == nullptr) {
        PartsModel* pForm = MR::createPartsModelEnemy(this, "クッパ防御姿勢", "KoopaDefenseForm", getBaseMtx());
        pForm->mCheckHiddenHost = true;
        mDefenseForm = pForm;
        mDefenseForm->mScale.set< f32 >(mScale);
        mDefenseForm->kill();
        MR::startAction(mDefenseForm, "Wait");
        MR::invalidateClipping(mDefenseForm);
        MR::registerDemoSimpleCastAll(mDefenseForm);
    }
    if (mDefenseCollision == nullptr) {
        mDefenseCollision = MR::createCollisionObj(this, "DefenseCollision", getSensor("Shell2"), nullptr);
        mDefenseCollision->mScale.set< f32 >(mScale);
        mDefenseCollision->kill();
    }
}

PartsModel* KoopaBattle::getDefenseForm() const {
    return mDefenseForm;
}

CollisionObj* KoopaBattle::getDefenseCollision() const {
    return mDefenseCollision;
}

void KoopaBattle::setPose(const TQuat4f& rQuat, const TVec3f& rPos) {
    mBattleQuat.set< f32 >(rQuat);
    mPoseQuat.set< f32 >(rQuat);
    mBattleOrigin.set< f32 >(rPos);
}

void KoopaBattle::makeActorAppeared() {
    LiveActor::makeActorAppeared();
    mVolumeDrawer->makeActorAppeared();
    mVolumeDrawer->setScaleDownRate(-1);
}

void KoopaBattle::makeActorDead() {
    LiveActor::makeActorDead();
    mVolumeDrawer->makeActorDead();
}

void KoopaBattle::control() {
    mJointCtrl->update();
    mBaseMtx.setInline(getBaseMtx());
    MR::rotateMtxLocalXDegree(mBaseMtx.toMtxPtr(), 90.0f);
    TVec3f scale;
    scale.x = 1.0f;
    scale.y = 1.0f;
    scale.z = 1.0f;
    MR::copyJointScale(this, "Root", &scale);
    f32 frame;
    if (scale.x <= 0.422f) {
        frame = MR::getLinerValueFromMinMax(scale.x, 0.1666f, 0.422f, 2.0f, 1.0f);
    } else {
        frame = MR::getLinerValueFromMinMax(scale.x, 0.422f, 1.0f, 1.0f, 0.0f);
    }
    MR::setBtkFrameAndStop(mShell, frame);
}

// Unrestricted followers use a negative ID; numbered followers must match the current battle.
bool KoopaBattle::isValidFollowId(s32 n) const {
    if (n < 0) {
        return true;
    }
    return mBattleNo == n;
}

void KoopaBattle::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mPoseQuat);
    mJointCtrl->setCallBackFunction();
}

void KoopaBattle::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaBattle::KoopaBattleNrvBattle::sInstance)) {
        mSequence->attackSensor(pSender, pReceiver);
    }
}

bool KoopaBattle::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaBattle::KoopaBattleNrvBattle::sInstance)) {
        return mSequence->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool KoopaBattle::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaBattle::KoopaBattleNrvBattle::sInstance)) {
        return mSequence->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

void KoopaBattle::startBattle() {
    setNerve(&NrvKoopaBattle::KoopaBattleNrvBattle::sInstance);
    makeActorAppeared();
}

void KoopaBattle::exeBattle() {
    if (MR::updateActorState(this)) {
        kill();
    }
}
