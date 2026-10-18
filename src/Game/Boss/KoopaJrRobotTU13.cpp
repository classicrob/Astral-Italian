#pragma auto_inline off
#include "Game/Boss/KoopaJrRobot.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/LiveActor/BreakModel.hpp"

// PLACEHOLDER(objects): unnamed target symbol fn_800DA9B0, declared extern "C" so the reloc name matches; resolve when fn_800DA9B0 gets its real name in symbols.txt
extern "C" s32 fn_800DA9B0();

static KoopaJrRobotPart::PartInfo sPartTable[4] = {
    {0, "\x8e\xe3\x93_:\x8d\xb6", "WeakPointFL", "WeakPointLeft", "JrRobotWeakPointL"},
    {1, "\x8e\xe3\x93_:\x89""E", "WeakPointFR", "WeakPointRight", "JrRobotWeakPointR"},
    {2, "\x8e\xe3\x93_:\x8c\xe3", "WeakPointB", "WeakPointBack", "JrRobotWeakPointB"},
    {3, "\x8e\xe3\x93_:\x93\xaa", "Head", "WeakPointHead", nullptr},
};

namespace NrvKoopaJrRobotPart {
    NEW_NERVE(KoopaJrRobotPartNrvNormal, KoopaJrRobotPart, Normal);
    NEW_NERVE(KoopaJrRobotPartNrvDamage1, KoopaJrRobotPart, Damage1);
    NEW_NERVE(KoopaJrRobotPartNrvDamage2, KoopaJrRobotPart, Damage2);
    class KoopaJrRobotPartNrvDead : public Nerve {
    public:
        virtual void execute(Spine*) const {}
        static KoopaJrRobotPartNrvDead sInstance;
    };
    KoopaJrRobotPartNrvDead KoopaJrRobotPartNrvDead::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvKoopaJrRobotPart

const KoopaJrRobotPart::PartInfo* KoopaJrRobotPart::getInfo(s32 id) {
    for (s32 i = 0; i < 4; i++) {
        if (id == sPartTable[i].mId) {
            return &sPartTable[i];
        }
    }
    return &sPartTable[0];
}

KoopaJrRobotPart* KoopaJrRobotPart::create(KoopaJrRobot* pHost, s32 id) {
    const PartInfo* pInfo = getInfo(id);
    KoopaJrRobotPart* pPart = new KoopaJrRobotPart(pHost, pInfo->mName, pInfo->mJoint, pInfo->mSensor, pInfo->mAction);
    pPart->init();
    if (MR::isEqualString(getInfo(3)->mJoint, pInfo->mJoint)) {
        pPart->kill();
        MR::invalidateHitSensor(pHost, pInfo->mSensor);
    } else {
        pPart->appear();
    }
    return pPart;
}

const char* KoopaJrRobotPart::getSensorName(s32 id) {
    return getInfo(id)->mSensor;
}

KoopaJrRobotPart::KoopaJrRobotPart(KoopaJrRobot* pHost, const char* pName, const char* pJoint, const char* pSensor, const char* pAction)
    : NerveExecutor(pName), mHost(pHost), mModel(nullptr), mBreakModel(nullptr), mJoint(pJoint), mSensor(pSensor), mAction(pAction) {
    if (!MR::isEqualString(getInfo(3)->mJoint, mJoint)) {
        mModel = MR::createPartsModelEnemy(mHost, pName, "JrRobotWeakPoint", MR::getJointMtx(mHost, mJoint));
        mBreakModel = MR::createBreakModel(mModel, nullptr);
    }
}

void KoopaJrRobotPart::init() {
    initNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvNormal::sInstance, 0);
}

void KoopaJrRobotPart::appear() {
    if (mModel != nullptr) {
        mModel->appear();
        if (mAction != nullptr) {
            MR::startAction(mModel, mAction);
        }
    }
    setNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvNormal::sInstance);
}

void KoopaJrRobotPart::kill() {
    if (mModel != nullptr) {
        mModel->kill();
        mBreakModel->kill();
    }
}

bool KoopaJrRobotPart::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    bool b = false;
    if (isNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDamage2::sInstance) || isNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDead::sInstance)) {
        b = true;
    }
    if (b) {
        return false;
    }
    return MR::isMsgStarPieceReflect(msg);
}

bool KoopaJrRobotPart::receiveMsgExplosion(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (!MR::isMsgExplosionAttack(msg)) {
        return false;
    }
    if (!mHost->fn_800DB9B0(pSender)) {
        return false;
    }
    if (!MR::isEqualSensor(pReceiver, mHost, mSensor)) {
        return false;
    }
    if (isNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvNormal::sInstance)) {
        setNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDamage1::sInstance);
        return true;
    }
    if (isNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDamage1::sInstance)) {
        MR::invalidateHitSensor(pReceiver);
        setNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDamage2::sInstance);
        return true;
    }
    return false;
}

// NONMATCHING: regalloc (r4/r5, f0-f4) only
void KoopaJrRobotPart::appearStarPiece() {
    PartsModel* pModel = mModel;
    if (pModel != nullptr) {
        TVec3f pos(pModel->mPosition);
        TVec3f up;
        MR::calcUpVec(&up, pModel);
        up += -mHost->mGravity;
        if (MR::normalizeOrZero(&up)) {
            MR::calcUpVec(&up, mModel);
        }
        MR::appearStarPieceToDirection(mHost, pos, up, fn_800DA9B0(), 45.0f, 50.0f, false);
    }
}

bool KoopaJrRobotPart::fn_800E25F0() {
    bool b = false;
    if (isNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDamage2::sInstance) || isNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDead::sInstance)) {
        b = true;
    }
    return b;
}

void KoopaJrRobotPart::exeNormal() {
    if (mModel != nullptr && MR::isFirstStep(this)) {
        MR::startAction(mModel, "Damage0");
    }
}

void KoopaJrRobotPart::exeDamage1() {
    if (MR::isFirstStep(this)) {
        if (mModel != nullptr) {
            MR::startAction(mModel, "Damage1");
            MR::tryHitReaction(mModel, "\x83_\x83\x81\x81[\x83W");
        }
    }
}

void KoopaJrRobotPart::exeDamage2() {
    if (mModel == nullptr) {
        setNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDead::sInstance);
        return;
    }
    if (MR::isFirstStep(this)) {
        MR::startAction(mModel, "Damage2");
        mBreakModel->appear();
        MR::tryHitReactionBreak(mModel);
        appearStarPiece();
    }
    if (MR::isBvaStopped(mModel)) {
        setNerve(&NrvKoopaJrRobotPart::KoopaJrRobotPartNrvDead::sInstance);
    }
}

KoopaJrRobotPart::~KoopaJrRobotPart() {}
