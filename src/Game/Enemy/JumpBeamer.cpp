#include "Game/Enemy/JumpBeamer.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/Enemy/JumpGuarder.hpp"
#include "Game/Enemy/RingBeam.hpp"
#include "Game/Enemy/WalkerStateBindStarPointer.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

// SMG2 step-tracking state, stored at JumpEmitter+0xC8 (not declared in our headers yet).
class ActorStateParamScale {
public:
    bool isFirstStep();
    bool isGreaterEqualStep(s32);
    bool isStep(s32);
    bool isIntervalStep(s32, s32);
    f32 getNerveStepRate();
};

static ActorStateParamScale* getParamScale(const JumpBeamer* pBeamer) {
    return *reinterpret_cast< ActorStateParamScale* const* >(reinterpret_cast< const char* >(pBeamer) + 0xC8);
}

// Unnamed SMG2 bind-star-pointer exe (called from the nerve execute).

namespace NrvJumpBeamer {
    NEW_NERVE(JumpBeamerNrvHide, JumpBeamer, Hide);
    NEW_NERVE(JumpBeamerNrvUp, JumpBeamer, Up);
    NEW_NERVE(JumpBeamerNrvWait, JumpBeamer, Wait);
    NEW_NERVE(JumpBeamerNrvDown, JumpBeamer, Down);
    NEW_NERVE(JumpBeamerNrvHopStart, JumpBeamer, HopStart);
    NEW_NERVE(JumpBeamerNrvHopWait, JumpBeamer, HopWait);
    NEW_NERVE(JumpBeamerNrvHopJump, JumpBeamer, HopJump);
    NEW_NERVE(JumpBeamerNrvHopEnd, JumpBeamer, HopEnd);
    NEW_NERVE(JumpBeamerNrvPreOpen, JumpBeamer, PreOpen);
    NEW_NERVE(JumpBeamerNrvOpen, JumpBeamer, Open);
    NEW_NERVE(JumpBeamerNrvClose, JumpBeamer, Close);
    NEW_NERVE(JumpBeamerNrvInter, JumpBeamer, Inter);

    // NONMATCHING: execute bodies are inlined here; SMG2 has them as unnamed fn_801A0C00 / fn_801A0C10
    class JumpBeamerNrvBindStarPointer : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            reinterpret_cast< JumpBeamer* >(pSpine->mExecutor)->fn_801A0860();
        };
        static JumpBeamerNrvBindStarPointer sInstance;
    };
    JumpBeamerNrvBindStarPointer JumpBeamerNrvBindStarPointer::sInstance ATTRIBUTE_WEAK;

    class JumpBeamerNrvBindStarPointerHop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            JumpBeamer* actor = reinterpret_cast< JumpBeamer* >(pSpine->mExecutor);
            if (MR::updateActorState(actor)) {
                actor->setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvWait));
            }
        };
        static JumpBeamerNrvBindStarPointerHop sInstance;
    };
    JumpBeamerNrvBindStarPointerHop JumpBeamerNrvBindStarPointerHop::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvJumpBeamer

JumpBeamer::JumpBeamer(const char* pName) : JumpEmitter(pName), mAnimScaleController(nullptr), mBindStarPointer(nullptr) {
}

void JumpBeamer::control() {
    mAnimScaleController->updateNerve();
    TMtx34f mtx;
    TVec3f rotation = mHeadModel->mRotation;
    TVec3f scale = mHeadModel->mScale;
    mtx.identity();
    scale.x *= mScale.x;
    scale.y *= mScale.y;
    scale.z *= mScale.z;
    MR::makeMtxTRS(mtx, TVec3f(0.0f, 0.0f, 0.0f), rotation, scale);
    _90.setInline(mBodyJointMtx);
    TMtx34f v5;
    v5.concat(_90, mtx);
    _90.setInline(v5);
    updateEventCamera();
    JumpEmitter::control();

    if (isStarPointerBindable() && mBindStarPointer->tryStartPointBind()) {
        if (isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopWait))) {
            if (MR::isGreaterEqualStep(this, 30)) {
                setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvBindStarPointerHop));
            }
        } else {
            setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvBindStarPointer));
        }
    }
}

void JumpBeamer::calcAndSetBaseMtx() {
    if (!isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvBindStarPointerHop))) {
        MR::updateBaseScale(this, mAnimScaleController);
    }

    LiveActor::calcAndSetBaseMtx();
}

bool JumpBeamer::isStarPointerBindable() const {
    bool ret = false;
    if (!isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvWait)) && !isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvPreOpen)) &&
        !isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvOpen)) && !isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvClose)) &&
        !isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopWait)) && isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvInter))) {
        ret = true;
    }

    return ret;
}

void JumpBeamer::init(const JMapInfoIter& rIter) {
    JumpEmitter::init(rIter);
    MR::initActor(this, rIter, "JumpBeamerBody", false);
    mHeadModel = MR::createPartsModelMapObjStrongLight(this, "ジャンプビーマー頭", "JumpBeamerHead", _90);
    MR::initLightCtrl(mHeadModel);
    MR::initDefaultPos(this, rIter);
    MR::connectToSceneEnemy(this);
    MR::initLightCtrl(this);
    initHitSensor(2);
    MR::addHitSensorMtx(this, "Jump", 0x23, 8, 145.0f, MR::getJointMtx(mHeadModel, "SpringJoint3"), TVec3f(0.0f, -100.0f, 0.0f));
    MR::addHitSensorMtxEnemy(this, "Body", 8, 145.0f, MR::getJointMtx(this, "Body"), TVec3f(0.0f, 35.0f, 0.0f));
    MR::setSensorType(this, "Body", 0x21);
    MR::validateHitSensor(this, "Body");
    MR::invalidateHitSensor(this, "Jump");
    MR::initShadowVolumeSphere(this, 140.0f);
    MR::invalidateShadow(this, nullptr);
    initEffectKeeper(1, nullptr, false);
    initSound(8, "JumpBeamer", nullptr, TVec3f(0.0f));
    MR::invalidateClipping(this);
    initNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHide), 2);
    MR::initStarPointerTarget(this, 150.0f, TVec3f(0.0f, 50.0f, 0.0f));
    mAnimScaleController = new AnimScaleController(nullptr);
    mBindStarPointer = new WalkerStateBindStarPointer(this, mAnimScaleController);
    MR::initActorState(this, mBindStarPointer, GET_NERVE(JumpBeamer, JumpBeamerNrvBindStarPointer), "SupportFreeze");
    MR::initActorState(this, mBindStarPointer, GET_NERVE(JumpBeamer, JumpBeamerNrvBindStarPointerHop), "SupportFreezeHop");
    MR::startBckWithInterpole(this, "Down", 0);
    MR::setBckFrame(this, MR::getBckCtrl(this)->getEnd() - 1);
    MR::calcAnimDirect(this);
    mBodyJointMtx = MR::getJointMtx(this, "Top");
    MR::initUseStageSwitchReadA(this, rIter);

    if (MR::initUseStageSwitchReadB(this, rIter)) {
        MR::listenStageSwitchOffB(this, MR::FunctorV0M< JumpBeamer*, void (JumpBeamer::*)() >(this, &JumpBeamer::syncSwitchOffB));
    }

    MR::joinToGroupArray(this, rIter, getName(), 32);
    makeActorAppeared();
    f32 arg1 = 20.0f;
    s32 arg0 = 100;
    MR::getJMapInfoArg0NoInit(rIter, &arg1);
    MR::getJMapInfoArg1NoInit(rIter, &arg0);
    mBeams = new RingBeam*[5];

    for (s32 i = 0; i < 5; i++) {
        mBeams[i] = nullptr;
    }

    for (s32 i = 0; i < 3; i++) {
        mBeams[i] = new RingBeam("リングビーム", this, false, false, false, false, nullptr);
        mBeams[i]->init(rIter);
        mBeams[i]->setSpeed(arg1);
        mBeams[i]->setLife(arg0);
    }
}

void JumpBeamer::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isSensorPlayerOrYoshi(pReceiver)) {
        MR::sendMsgPush(pReceiver, pSender);
    }
}

bool JumpBeamer::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerTrample(msg)) {
        if (MR::isSensorType(pReceiver, 0x23)) {
            TVec3f up;
            MR::calcUpVec(&up, this);
            MR::setPlayerJumpVec(up);
            startEventCamera();
            setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopJump));
        } else {
            setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopStart));
        }

        return true;
    } else if (MR::isMsgPlayerHipDrop(msg)) {
        MR::forceJumpPlayer(-mGravity);
        return true;
    } else if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        if (!isNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopStart))) {
            setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopStart));
            return true;
        }

        return false;
    }

    return MR::isMsgStarPieceReflect(msg);
}

void JumpBeamer::syncSwitchOffB() {
    setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvInter));

    for (s32 i = 0; i < 3; i++) {
        mBeams[i]->kill();
    }
}

bool JumpBeamer::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0x7C) {
        MR::invalidateClipping(this);
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvUp));
        return true;
    }

    if (msg == 0x7E) {
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvDown));
        return true;
    }

    return false;
}

void JumpBeamer::exeHide() {
    bool v2 = MR::isValidSwitchB(this) && !MR::isOnSwitchB(this);

    if (!v2) {
        updateRotate();
    }

    if (getParamScale(this)->isFirstStep()) {
        MR::startAction(mHeadModel, "Wait");
        MR::startBrk(mHeadModel, "Green");
        MR::validateClipping(this);
        MR::setShadowVolumeSphereRadius(this, nullptr, 110.0f);
    }

    if (MR::enableGroupAttack(this, 3000.0f, 500.0f)) {
        MR::sendMsgToGroupMember(0x7C, this, getSensor("Body"), "Body");
    }
}

void JumpBeamer::exeUp() {
    if (getParamScale(this)->isFirstStep()) {
        MR::startAction(this, "Up");
        MR::startActionSound(this, "EmJguarderAppear");
        MR::validateShadow(this, nullptr);
    }

    s16 end = MR::getBckCtrl(this)->getEnd();
    f32 frame = MR::getBckFrame(this);
    MR::setShadowVolumeSphereRadius(this, nullptr, (110.0f + (30.0f * (frame / (end)))));

    if (MR::isActionEnd(this)) {
        MR::invalidateShadow(this, nullptr);
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvWait));
    }
}

void JumpBeamer::exeWait() {
    updateRotate();

    if (!MR::enableGroupAttack(this, 3200.0f, 500.0f)) {
        MR::sendMsgToGroupMember(0x7E, this, getSensor("Body"), "Body");
    } else {
        bool v3 = MR::isValidSwitchB(this) && !MR::isOnSwitchB(this);

        if (!v3) {
            setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvPreOpen));
        }
    }
}

void JumpBeamer::exeDown() {
    if (getParamScale(this)->isFirstStep()) {
        MR::startAction(this, "Down");
        MR::startActionSound(this, "EmJguarderHide");
    }

    s16 end = MR::getBckCtrl(this)->getEnd();
    f32 frame = MR::getBckFrame(this);
    MR::setShadowVolumeSphereRadius(this, nullptr, (110.0f + (30.0f * (1.0f - frame / (end)))));
    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHide));
    }
}

void JumpBeamer::exeHopStart() {
    if (getParamScale(this)->isFirstStep()) {
        MR::startAction(mHeadModel, "HopStart");
        MR::startBrk(mHeadModel, "OnAndOff");
        MR::startAction(this, "Damage");
        MR::deleteEffect(this, "Charge");
        MR::startActionSound(this, "EmJguarderHit");
        MR::setSensorRadius(this, "Body", 120.0f);
    }

    MR::startActionSound(this, "EmLvJguarderShake");

    if (MR::isActionEnd(mHeadModel)) {
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopWait));
    }
}

void JumpBeamer::exeHopWait() {
    if (getParamScale(this)->isFirstStep()) {
        MR::stopBck(this);
        MR::startAction(mHeadModel, "HopWait");
        MR::validateHitSensor(this, "Jump");
    }

    MR::startActionSound(this, "EmLvJguarderShake");

    if (getParamScale(this)->isStep(300)) {
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopEnd));
    }
}

void JumpBeamer::exeHopJump() {
    if (getParamScale(this)->isFirstStep()) {
        MR::startAction(mHeadModel, "HopJump");
        MR::startActionSound(this, "EmJguarderTrample");
    }

    if (MR::isActionEnd(mHeadModel)) {
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopWait));
    }
}

void JumpBeamer::exeHopEnd() {
    if (getParamScale(this)->isFirstStep()) {
        MR::startAction(this, "HopEnd");
        MR::startAction(mHeadModel, "HopEnd");
        MR::startBrk(mHeadModel, "Green");
        MR::startActionSound(this, "EmJguarderCloseSpring");
    }

    if (MR::isActionEnd(this)) {
        if (MR::isActionEnd(mHeadModel)) {
            MR::setSensorRadius(this, "Body", 145.0f);
            MR::invalidateHitSensor(this, "Jump");
            setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvWait));
        }
    }
}

void JumpBeamer::exePreOpen() {
    updateRotate();

    if (!MR::enableGroupAttack(this, 3200.0f, 500.0f)) {
        MR::sendMsgToGroupMember(0x7E, this, getSensor("Body"), "Body");

    } else {
        if (getParamScale(this)->isStep(0)) {
            setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvOpen));
        }
    }
}

void JumpBeamer::exeOpen() {
    if (getParamScale(this)->isFirstStep()) {
        MR::startAction(this, "Open");
    }

    if (getParamScale(this)->isGreaterEqualStep(240)) {
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvClose));
    } else {
        if (getParamScale(this)->isIntervalStep(80, 0)) {
            MR::emitEffect(this, "Charge");
        }

        if (getParamScale(this)->isIntervalStep(80, 79)) {
            MR::deleteEffect(this, "Charge");

            if (mBeams[(s32)getParamScale(this)->getNerveStepRate() / 80] != nullptr) {
                mBeams[(s32)getParamScale(this)->getNerveStepRate() / 80]->appear();
            }
        }
    }
}

void JumpBeamer::exeClose() {
    if (getParamScale(this)->isFirstStep()) {
    }

    MR::startActionSound(this, "EmLvJguarderShutterClose");

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvInter));
    }
}

void JumpBeamer::exeInter() {
    bool v2 = MR::isValidSwitchB(this) && !MR::isOnSwitchB(this);

    if (!v2) {
        updateRotate();
    }

    if (!MR::enableGroupAttack(this, 3200.0f, 500.0f)) {
        MR::sendMsgToGroupMember(0x7E, this, getSensor("Body"), "Body");
    } else if (getParamScale(this)->isGreaterEqualStep(80)) {
        for (s32 i = 0; i < 3; i++) {
            if (!MR::isDead(mBeams[i])) {
                return;
            }
        }

        setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvWait));
    }
}

// SMG2 unnamed bind-star-pointer exe (fn_801A0860).
void JumpBeamer::fn_801A0860() {
    if (getParamScale(this)->isFirstStep()) {
        MR::fn_8000A320(this->mHeadModel);
        MR::fn_8000A320(this);
    }

    if (MR::updateActorState(this)) {
        MR::fn_8000A400(this->mHeadModel);
        MR::fn_8000A400(this);
        this->setNerve(GET_NERVE(JumpBeamer, JumpBeamerNrvHopWait));
    }
}
