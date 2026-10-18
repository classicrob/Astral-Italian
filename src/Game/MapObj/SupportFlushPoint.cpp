#include "Game/MapObj/SupportFlushPoint.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
namespace NrvSupportFlushPoint {
NEW_NERVE(SupportFlushPointNrvAppear, SupportFlushPoint, Appear);
NEW_NERVE(SupportFlushPointNrvHide, SupportFlushPoint, Hide);
NEW_NERVE(SupportFlushPointNrvWait, SupportFlushPoint, Wait);
NEW_NERVE(SupportFlushPointNrvGenerateItem, SupportFlushPoint, GenerateItem);
class SupportFlushPointNrvDisappear : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static SupportFlushPointNrvDisappear sInstance;
};
SupportFlushPointNrvDisappear SupportFlushPointNrvDisappear::sInstance;
}
SupportFlushPoint::SupportFlushPoint(const char* name) : LiveActor(name), mItemType(-1) { MR::joinSupportPlayGroup(this); }
void SupportFlushPoint::init(const JMapInfoIter& iter) {
    MR::initActor(this, iter, false);
    initNerve(&NrvSupportFlushPoint::SupportFlushPointNrvWait::sInstance);
    MR::getJMapInfoArg0WithInit(iter, &mItemType);
    switch (mItemType) { case -1: MR::declareStarPiece(this, 3); break; case 0: MR::declareCoin(this, 1); break; }
    makeActorDead();
}
void SupportFlushPoint::control() {}
bool SupportFlushPoint::receiveMsgPlayerAttack(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgSupportTicoSpin(msg) && isNerve(&NrvSupportFlushPoint::SupportFlushPointNrvWait::sInstance)) { setNerve(&NrvSupportFlushPoint::SupportFlushPointNrvGenerateItem::sInstance); return true; }
    return false;
}
bool SupportFlushPoint::receiveOtherMsg(u32 msg, HitSensor*, HitSensor*) {
    if (MR::isMsgSupportPlayGroupStart(msg) && !isNerve(&NrvSupportFlushPoint::SupportFlushPointNrvDisappear::sInstance)) { setNerve(&NrvSupportFlushPoint::SupportFlushPointNrvAppear::sInstance); appear(); return true; }
    if (MR::isMsgSupportPlayGroupStop(msg) && !isNerve(&NrvSupportFlushPoint::SupportFlushPointNrvDisappear::sInstance)) { setNerve(&NrvSupportFlushPoint::SupportFlushPointNrvHide::sInstance); return true; }
    return false;
}
void SupportFlushPoint::exeAppear() { if (MR::isFirstStep(this)) MR::startAction(this, "Appear"); if (MR::isActionEnd(this)) setNerve(&NrvSupportFlushPoint::SupportFlushPointNrvWait::sInstance); }
void SupportFlushPoint::exeHide() { if (MR::isFirstStep(this)) MR::startAction(this, "Hide"); if (MR::isActionEnd(this)) kill(); }
void SupportFlushPoint::exeWait() { if (MR::isFirstStep(this)) MR::startAction(this, "Wait"); MR::attachSupportTicoToTarget(this); }
void SupportFlushPoint::exeGenerateItem() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "GenerateItem");
        switch (mItemType) {
        case -1: MR::appearStarPiece(this, mPosition, 3, 10.0f, 40.0f, false); break;
        case 0: MR::appearCoinPop(this, mPosition, 1); break;
        }
    }
    if (MR::isActionEnd(this)) { setNerve(&NrvSupportFlushPoint::SupportFlushPointNrvDisappear::sInstance); kill(); }
}
SupportFlushPoint::~SupportFlushPoint() {}
