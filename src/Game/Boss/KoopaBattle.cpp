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
#include "Game/Util/TriangleFilter.hpp"

namespace {
    // scale of the attack range volume (cylinder) drawn around Bowser
    TVec3f sVolumeScale(1.4f, 8.0f, 1.4f);
};  // namespace

namespace NrvKoopaBattle {
    NEW_NERVE(KoopaBattleNrvBattle, KoopaBattle, Battle);
};  // namespace NrvKoopaBattle

KoopaBattle::KoopaBattle(const char* pName)
    : LiveActor(pName), mSequence(nullptr), mJointCtrl(nullptr), mCameraInfo(nullptr), mShell(nullptr), mDefenseForm(nullptr),
      mDefenseCollision(nullptr), mVolumeDrawer(nullptr), mQuatA(0.0f, 0.0f, 0.0f, 1.0f), mQuatB(0.0f, 0.0f, 0.0f, 1.0f), mPos(0.0f, 0.0f, 0.0f),
      mBattleNo(-1), _10C(false) {
    MR::createKoopaBattleAccessor();
    MR::getKoopaBattleAccessor()->registerKoopaBattle(this);
    mBaseMtx.identity();
}

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
    MR::makeQuatFromRotate(&mQuatA, this);
    mQuatB.set< f32 >(mQuatA);
    mPos.set< f32 >(mPosition);
    MR::getKoopaBattleAccessor()->setPoseA(this);
    {
        TVec3f front;
        MR::calcFrontVec(&front, this);
        TVec3f offset(front);
        offset.scale(4000.0f);
        TVec3f newPos(mPosition);
        newPos.sub(offset);
        mPosition.set< f32 >(newPos);
    }
    mJointCtrl = new ActorJointCtrl(this);
    mJointCtrl->endFaceCtrl(0);
    mCameraInfo = MR::createActorCameraInfo(rIter);
    MR::initActorCameraProgrammable(this);
    if (level != 2) {
        if (level != 3) {
            mSequence = new KoopaStateLv1Sequence(this, rIter);
        } else {
            mSequence = new KoopaStateLv3Sequence(this, rIter);
        }
    } else {
        mSequence = new KoopaStateLv2Sequence(this, rIter);
    }
    initNerve(&NrvKoopaBattle::KoopaBattleNrvBattle::sInstance, 1);
    MR::initActorState(this, mSequence, &NrvKoopaBattle::KoopaBattleNrvBattle::sInstance, "PhaseSequence");
    TriangleFilterDelegator< KoopaBattle >* pFilter = new TriangleFilterDelegator< KoopaBattle >(this, &KoopaBattle::isInvalidTriangle);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, reinterpret_cast< BaseMatrixFollowValidater* >(pFilter));
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
        mDefenseForm = MR::createPartsModelEnemy(this, "クッパ防御姿勢", "KoopaDefenseForm", getBaseMtx());
        mDefenseForm->mCheckHiddenHost = true;
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
    mQuatB.set< f32 >(rQuat);
    mQuatA.set< f32 >(rQuat);
    mPos.set< f32 >(rPos);
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

// all triangles are rejected when the pointer is "negative", otherwise only the battle number match
bool KoopaBattle::isInvalidTriangle(const Triangle* pTriangle) {
    s32 n = reinterpret_cast< s32 >(pTriangle);
    if (n < 0) {
        return true;
    }
    return mBattleNo == n;
}

void KoopaBattle::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuatA);
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
