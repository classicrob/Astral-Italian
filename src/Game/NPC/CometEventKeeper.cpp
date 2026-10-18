#include "Game/NPC/CometEventKeeper.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

CometEventKeeper::CometEventKeeper() : mExecutorTimeLimit(nullptr), mScreenFilter(nullptr), mCometName(nullptr) {
}

bool CometEventKeeper::isStartEvent(const char* pParam1) const {
    if (mCometName == nullptr) {
        return false;
    }

    return MR::isEqualString(mCometName, pParam1);
}
