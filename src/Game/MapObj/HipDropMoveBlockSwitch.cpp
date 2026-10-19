#include "Game/MapObj/HipDropMoveBlock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Map/CollisionParts.hpp"

namespace NrvHipDropMoveBlockSwitch {
    NEW_NERVE(HipDropMoveBlockSwitchNrvOff, HipDropMoveBlockSwitch, Off);
    NEW_NERVE(HipDropMoveBlockSwitchNrvSwitchDown, HipDropMoveBlockSwitch, SwitchDown);

    class HipDropMoveBlockSwitchNrvOn : public Nerve {
    public:
        virtual void execute(Spine*) const {
        }

        static HipDropMoveBlockSwitchNrvOn sInstance;
    };
    HipDropMoveBlockSwitchNrvOn HipDropMoveBlockSwitchNrvOn::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvHipDropMoveBlockSwitch

HipDropMoveBlockSwitch::HipDropMoveBlockSwitch(const char* pName, HipDropMoveBlock* pBlock, MtxPtr pMtx,
                                               const TVec3f& rOffset, const TVec3f& rRotate)
    : LiveActor(pName) {
    mMoveBlock = pBlock;
    mOffset = rOffset;
    mRotate = rRotate;
    mCollisionParts = nullptr;
    mBaseMtx.set(pMtx);
    MR::setMtxTrans(mBaseMtx, 0.0f, 0.0f, 0.0f);
}

void HipDropMoveBlockSwitch::init(const JMapInfoIter& rIter) {
    // pNames + N are offsets into one pooled literal block (target string layout); the substrings are not
    // separate literals, so they stay as offsets.
    const char* pNames = "HipDropMoveBlockSwitch";
    MR::initActor(this, rIter, pNames + 0x0, false);
    initHitSensor(2);
    MR::addHitSensorMapObj(this, pNames + 0x18, 16, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorMapObj(this, "Hit", 16, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initCollisionParts(this, pNames + 0x0, getSensor(pNames + 0x18), nullptr);
    mCollisionParts = MR::createCollisionPartsFromLiveActor(this, pNames + 0x20,
                                                            getSensor("Hit"),
                                                            MR::CollisionScaleType_Unk2);
    MR::validateCollisionParts(mCollisionParts);
    initNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvOff::sInstance, 0);
    makeActorAppeared();
}

void HipDropMoveBlockSwitch::calcAnim() {
    LiveActor::calcAnim();
    TPos3f mtx;
    MtxPtr jointMtx = MR::getJointMtx(this, "Move");
    mtx.set(jointMtx);
    mCollisionParts->setMtx(mtx);
}

// NONMATCHING: the vector copy constructor is inlined and mPosition uses scalar stores; target outlines the constructor and uses paired-single stores.
void HipDropMoveBlockSwitch::calcAndSetBaseMtx() {
    TVec3f pos(0.0f);
    mBaseMtx.mult(mOffset, pos);

    TPos3f mtx;
    TPos3f rot;
    rot.identity();
    MR::makeMtxRotate(rot, mRotate);
    PSMTXConcat(rot, mBaseMtx, mtx);

    mPosition.set(pos + mMoveBlock->mPosition);
    MR::setMtxTrans(mtx, mPosition.x, mPosition.y, mPosition.z);
    MR::setBaseTRMtx(this, mtx);
}

bool HipDropMoveBlockSwitch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvOff::sInstance)) {
        setNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvSwitchDown::sInstance);
        HitSensor* pThisSensor = getSensor("Body");
        HitSensor* pBlockSensor = mMoveBlock->getSensor("Body");
        MR::sendArbitraryMsg(msg, pBlockSensor, pThisSensor);
        return false;
    }

    return false;
}

void HipDropMoveBlockSwitch::exeOff() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, "Wait");
        MR::startBrk(this, "Off");
    }
}

void HipDropMoveBlockSwitch::exeSwitchDown() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startAction(this, "On");
        MR::startBrk(this, "On");
    }

    if (MR::isActionEnd(this)) {
        setNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvOn::sInstance);
    }
}

bool HipDropMoveBlockSwitch::tryOff() {
    if (isNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvOn::sInstance)) {
        setNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvOff::sInstance);
        return true;
    }

    return false;
}

HipDropMoveBlockSwitch::~HipDropMoveBlockSwitch() {
}
