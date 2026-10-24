#include "Game/Map/SwitchWatcher.hpp"
#include "Game/Map/StageSwitch.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util/ObjUtil.hpp"
#include <algorithm>
#ifdef GALAXY_NATIVE
#include <petari/game_compat.hpp>
#endif

#define Listener_A 1
#define Listener_B 2
#define Listener_Appear 4

SwitchWatcher::SwitchWatcher(const StageSwitchCtrl* pSwitchCtrl) : NameObj("スイッチ監視") {
    mCheck = 0;
    mSwitchCtrl = pSwitchCtrl;
    mListener_A = nullptr;
    mListener_B = nullptr;
    mListener_Appear = nullptr;
}

void SwitchWatcher::movement() {
    if (mListener_A != nullptr) {
        checkSwitch(mListener_A, Listener_A, mSwitchCtrl->isOnSwitchA());
    }

    if (mListener_B != nullptr) {
        checkSwitch(mListener_B, Listener_B, mSwitchCtrl->isOnSwitchB());
    }

    if (mListener_Appear != nullptr) {
        checkSwitch(mListener_Appear, Listener_Appear, mSwitchCtrl->isOnSwitchAppear());
    }
}

void SwitchWatcher::checkSwitch(SwitchEventListener* pListener, u32 check, bool isOnEvent) {
    if (isOnEvent) {
        if ((mCheck & check) == 0) {
            pListener->listenSwitchOnEvent();
        }

        mCheck |= check;
    } else {
        if ((mCheck & check) != 0) {
            pListener->listenSwitchOffEvent();
        }

        mCheck &= ~check;
    }
}

bool SwitchWatcher::isSameSwitch(const StageSwitchCtrl* pCtrl) const {
    return mSwitchCtrl == pCtrl;
}

void SwitchWatcher::addSwitchListener(SwitchEventListener* pListener, u32 type) {
    switch (type) {
    case Listener_A:
        mListener_A = pListener;
        break;
    case Listener_B:
        mListener_B = pListener;
        break;
    case Listener_Appear:
        mListener_Appear = pListener;
        break;
    }
}

SwitchWatcher::~SwitchWatcher() {
}

SwitchWatcherHolder::SwitchWatcherHolder() : NameObj("SwitchWatcherHolder") {
    mWatcherCount = 0;
    MR::connectToScene(this, 0x19, -1, -1, -1);
}

// https://decomp.me/scratch/BNgz7
// NONMATCHING: member-pointer stack copies and register allocation differ; listener range and calls match.
void SwitchWatcherHolder::movement() {
    std::for_each(&mWatchers[0], &mWatchers[mWatcherCount], std::mem_func(&SwitchWatcher::movement));
}

void SwitchWatcherHolder::joinSwitchEventListenerA(const StageSwitchCtrl* pCtrl, SwitchEventListener* pListener) {
    joinSwitchEventListener(pCtrl, Listener_A, pListener);
}

void SwitchWatcherHolder::joinSwitchEventListenerB(const StageSwitchCtrl* pCtrl, SwitchEventListener* pListener) {
    joinSwitchEventListener(pCtrl, Listener_B, pListener);
}

void SwitchWatcherHolder::joinSwitchEventListenerAppear(const StageSwitchCtrl* pCtrl, SwitchEventListener* pListener) {
    joinSwitchEventListener(pCtrl, Listener_Appear, pListener);
}

SwitchWatcher* SwitchWatcherHolder::findSwitchWatcher(const StageSwitchCtrl* pCtrl) {
    for (SwitchWatcher** cur = &mWatchers[0]; cur != &mWatchers[mWatcherCount]; cur++) {
        if ((*cur)->isSameSwitch(pCtrl)) {
            return *cur;
        }
    }

    return nullptr;
}

void SwitchWatcherHolder::joinSwitchEventListener(const StageSwitchCtrl* pCtrl, u32 type, SwitchEventListener* pEventListener) {
    SwitchWatcher* sw = findSwitchWatcher(pCtrl);

    if (sw == nullptr) {
        sw = new SwitchWatcher(pCtrl);
        addSwitchWatcher(sw);
    }

    sw->addSwitchListener(pEventListener, type);
}

void SwitchWatcherHolder::addSwitchWatcher(SwitchWatcher* pWatcher) {
    s32 count = mWatcherCount++;
    mWatchers[count] = pWatcher;
}

namespace MR {
    SwitchWatcherHolder* getSwitchWatcherHolder() {
        return MR::getSceneObj< SwitchWatcherHolder* >(SceneObj_SwitchWatcherHolder);
    }

    void requestMovementOnSwitchWatcher() {
        MR::requestMovementOn(getSwitchWatcherHolder());
    }
};  // namespace MR

SwitchWatcherHolder::~SwitchWatcherHolder() {
}
