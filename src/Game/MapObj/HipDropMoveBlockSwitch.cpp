#include "Game/MapObj/HipDropMoveBlock.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/ActionSoundUtil.hpp"
#include "Game/Map/CollisionParts.hpp"

extern "C" char lbl_806A23D0[];
extern "C" char lbl_806A23E8[];
extern "C" char lbl_806A23F0[];
extern "C" char lbl_806A23F8[];
extern "C" u32 lbl_807CFC68;
extern "C" u32 lbl_807CFC6C;
extern "C" u32 lbl_807CFC70;

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
    const char* pNames = lbl_806A23D0;
    MR::initActor(this, rIter, pNames + 0x0, false);
    initHitSensor(2);
    MR::addHitSensorMapObj(this, pNames + 0x18, 16, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::addHitSensorMapObj(this, reinterpret_cast<const char*>(&lbl_807CFC68), 16, 0.0f, TVec3f(0.0f, 0.0f, 0.0f));
    MR::initCollisionParts(this, pNames + 0x0, getSensor(pNames + 0x18), nullptr);
    mCollisionParts = MR::createCollisionPartsFromLiveActor(this, pNames + 0x20,
                                                            getSensor(reinterpret_cast<const char*>(&lbl_807CFC68)),
                                                            MR::CollisionScaleType_Unk2);
    MR::validateCollisionParts(mCollisionParts);
    initNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvOff::sInstance, 0);
    makeActorAppeared();
}

void HipDropMoveBlockSwitch::calcAnim() {
    LiveActor::calcAnim();
    TPos3f mtx;
    MtxPtr jointMtx = MR::getJointMtx(this, lbl_806A23F0);
    mtx.set(jointMtx);
    mCollisionParts->setMtx(mtx);
}

// NONMATCHING: the TVec3 copy (trans(pos)) is inlined here (target calls out-of-line copy ctor) and the
// position adds are scheduled differently, so the body only approximates the target.
void HipDropMoveBlockSwitch::calcAndSetBaseMtx() {
    TVec3f pos(0.0f);
    mBaseMtx.mult(mOffset, pos);

    TPos3f mtx;
    TPos3f rot;
    rot.identity();
    MR::makeMtxRotate(rot, mRotate);
    PSMTXConcat(rot, mBaseMtx, mtx);

    TVec3f trans(pos);
    HipDropMoveBlock* pBlock = mMoveBlock;
    trans.x += pBlock->mPosition.x;
    trans.y += pBlock->mPosition.y;
    trans.z += pBlock->mPosition.z;
    mPosition = trans;

    MR::setMtxTrans(mtx, mPosition.x, mPosition.y, trans.z);
    MR::setBaseTRMtx(this, mtx);
}

bool HipDropMoveBlockSwitch::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgPlayerHipDropFloor(msg) && isNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvOff::sInstance)) {
        setNerve(&NrvHipDropMoveBlockSwitch::HipDropMoveBlockSwitchNrvSwitchDown::sInstance);
        HitSensor* pThisSensor = getSensor(lbl_806A23E8);
        HitSensor* pBlockSensor = mMoveBlock->getSensor(lbl_806A23E8);
        MR::sendArbitraryMsg(msg, pBlockSensor, pThisSensor);
        return false;
    }

    return false;
}

void HipDropMoveBlockSwitch::exeOff() {
    if (MR::isFirstStep(this)) {
        MR::validateClipping(this);
        MR::startAction(this, lbl_806A23F8);
        MR::startBrk(this, reinterpret_cast<const char*>(&lbl_807CFC6C));
    }
}

void HipDropMoveBlockSwitch::exeSwitchDown() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startAction(this, reinterpret_cast<const char*>(&lbl_807CFC70));
        MR::startBrk(this, reinterpret_cast<const char*>(&lbl_807CFC70));
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
