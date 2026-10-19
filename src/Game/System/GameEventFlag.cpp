#include "Game/System/GameEventFlag.hpp"

GameEventFlagAccessor::GameEventFlagAccessor(const GameEventFlag* pFlag) : mFlag(pFlag) {}

const char* GameEventFlagAccessor::getName() const {
    return mFlag->mName;
}
