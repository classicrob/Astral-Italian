#include "Game/MapObj/StarPieceFlower.hpp"
#include "Game/Enemy/AnimScaleController.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#include "math_types.hpp"
#include <revolution/types.h>

namespace MR {
    bool getJMapInfoArg0NoInit(const JMapInfoIter&, bool*);
};  // namespace MR

extern "C" void fn_80318760(TVec3f*);
extern "C" void fn_8004DB60(LiveActor*, TVec3f*, TVec3f*, f32, s32, s32, s32, s32);
extern "C" void fn_800345E0(LiveActor*, const char*);

extern "C" const char lbl_806AF130[];
extern "C" const char lbl_806AF140[];
extern "C" const char lbl_806AF14C[];
extern "C" const char lbl_806AF154[];
extern "C" const char lbl_806AF164[];
extern "C" const char lbl_806AF178[];
extern "C" const char lbl_806AF180[];
extern "C" const char lbl_806AF188[];

namespace NrvStarPieceFlower {
    NEW_NERVE(StarPieceFlowerNrvCloseWait, StarPieceFlower, CloseWait);
    NEW_NERVE(StarPieceFlowerNrvOpen, StarPieceFlower, Open);

    class StarPieceFlowerNrvOpenWait : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static StarPieceFlowerNrvOpenWait sInstance;
    };

    StarPieceFlowerNrvOpenWait StarPieceFlowerNrvOpenWait::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(StarPieceFlowerNrvSpinReaction, StarPieceFlower, SpinReaction);
};  // namespace NrvStarPieceFlower

StarPieceFlower::StarPieceFlower(const char* pName)
    : LiveActor(pName), mUp(0.0f, 1.0f, 0.0f), mAngle(0.0f), mTargetPos(nullptr), mAnimScaleCtrl(nullptr) {}

StarPieceFlower::~StarPieceFlower() {}

void StarPieceFlower::init(const JMapInfoIter& rIter) {
    MR::initActor(this, rIter, lbl_806AF130, false);
    initNerve(&NrvStarPieceFlower::StarPieceFlowerNrvCloseWait::sInstance, 0);
    MR::declareStarPiece(this, 1);

    if (mTargetPos != nullptr) {
        MR::startAction(this, lbl_806AF140);
        MR::setAllAnimFrameAndStop(this, lbl_806AF140, 0.0f);
    }

    MR::createSceneObj(SceneObj_StarPieceFlowerController);
    mAnimScaleCtrl = new AnimScaleController(nullptr);

    if (mTargetPos == nullptr) {
        bool isArg0 = false;
        MR::getJMapInfoArg0NoInit(rIter, &isArg0);
        if (!isArg0) {
            MR::invalidateShadowAll(this);
        }
    }

    makeActorAppeared();
}

static inline f32 normalizeAngle(f32 value, f32 base) {
    return base + (f32)fmod(360.0f + (value - base), 360.0);
}

void StarPieceFlower::control() {
    mAnimScaleCtrl->updateNerve();

    if (mTargetPos != nullptr) {
        mPosition.x = mTargetPos->x;
        mPosition.y = mTargetPos->y;
        mPosition.z = mTargetPos->z;
    } else {
        TVec3f rot;
        fn_80318760(&rot);
        mAngle = rot.y;
        mAngle = normalizeAngle(rot.y, 0.0f);
    }
}

void StarPieceFlower::calcAndSetBaseMtx() {
    if (mTargetPos == nullptr) {
        TPos3f mtx;
        mtx.identity();
        MR::makeMtxTRS(mtx.toMtxPtr(), this);
        MR::rotateMtxLocalYDegree(mtx.toMtxPtr(), mAngle);
        MR::setBaseTRMtx(this, mtx);
        MR::updateBaseScale(this, mAnimScaleCtrl);
    } else {
        TPos3f mtx;
        MR::makeMtxUpNoSupportPos(&mtx, mUp, mPosition);
        MR::setBaseTRMtx(this, mtx);
    }
}

void StarPieceFlower::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isSensorPlayer(pReceiver)) {
        return;
    }
    if (!isNerve(&NrvStarPieceFlower::StarPieceFlowerNrvCloseWait::sInstance)) {
        return;
    }

    f32 kPush = 40.0f;
    TVec3f push = mGravity * -kPush;
    TVec3f playerVel = *MR::getPlayerVelocity() * 0.8f;
    push += playerVel;

    TVec3f killDir;
    killDir.negate(mGravity);
    f32 len = MR::vecKillElement(push, killDir, &push);

    f32 speed;
    if (len < 35.0f) {
        speed = 35.0f;
    } else if (len > 50.0f) {
        speed = 50.0f;
    } else {
        speed = len;
    }

    TVec3f back = mGravity * speed;
    push -= back;

    fn_8004DB60(this, &mPosition, &push, 0.0f, 1, 0, 0, 1);
    fn_800345E0(this, lbl_806AF164);
    MR::invalidateHitSensor(this, lbl_806AF178);
    setNerve(&NrvStarPieceFlower::StarPieceFlowerNrvOpen::sInstance);
}

bool StarPieceFlower::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceAttack(msg)) {
        mAnimScaleCtrl->startHitReaction();
        return true;
    }

    if (!MR::isEqualSensor(pReceiver, this, lbl_806AF180)) {
        return false;
    }

    if (!isNerve(&NrvStarPieceFlower::StarPieceFlowerNrvOpenWait::sInstance)) {
        return false;
    }

    if (MR::isMsgPlayerSpinAttackOrSupportTico(msg)) {
        fn_800345E0(this, lbl_806AF188);
        setNerve(&NrvStarPieceFlower::StarPieceFlowerNrvSpinReaction::sInstance);
        return false;
    }

    return false;
}

void StarPieceFlower::exeCloseWait() {
    if (mTargetPos != nullptr) {
        return;
    }

    if (MR::isFirstStep(this)) {
        MR::startAction(this, lbl_806AF140);
        MR::setAllAnimFrameAndStop(this, lbl_806AF140, 0.0f);
    }
}

void StarPieceFlower::exeOpen() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, lbl_806AF14C);
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvStarPieceFlower::StarPieceFlowerNrvOpenWait::sInstance);
    }
}

void StarPieceFlower::exeSpinReaction() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, lbl_806AF154);
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvStarPieceFlower::StarPieceFlowerNrvOpenWait::sInstance);
    }
}
