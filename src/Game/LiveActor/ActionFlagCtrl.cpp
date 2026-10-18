#include "Game/LiveActor/ActionFlagCtrl.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util/LiveActorUtil.hpp"

extern char lbl_80694810[];
extern const char* lbl_807CFA18;

ActionFlagCtrl::ActionFlagCtrl(LiveActor* pHost) {
    mHost = pHost;
    mHitSensorKeeper = nullptr;
    mJointCtrl = nullptr;
    mNumEntries = 0;
    mEntries = nullptr;
    mFlagData = nullptr;
    mIsActive = false;
}

ActionFlagCtrl* ActionFlagCtrl::tryCreate(LiveActor* pHost) {
    if (MR::isExistFileInArc(MR::getResourceHolder(pHost), lbl_80694810, lbl_807CFA18)) {
        ActionFlagCtrl* pCtrl = new ActionFlagCtrl(pHost);
        if (pCtrl != nullptr) {
            return pCtrl;
        }
        return pCtrl;
    }
    return nullptr;
}
