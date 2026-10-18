#include "Game/Util/SystemUtil.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemFontHolder.hpp"
#include "Game/System/RenderMode.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <nw4r/ut/ResFont.h>
#include <revolution/types.h>
#include "Game/NameObj/NameObjHolder.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/GameSystemFunction.hpp"

nw4r::ut::Font* MR::getFontOnCurrentLanguage() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->getMessageFont();
}

nw4r::ut::Font* MR::getPictureFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mPictureFont;
}

nw4r::ut::Font* MR::getMenuFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mMenuFont64;
}

nw4r::ut::Font* MR::getNumberFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mNumberFont;
}

nw4r::ut::Font* MR::getMenuFont96NW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mMenuFont96;
}

nw4r::ut::Font* MR::getStaffFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mStaffFont;
}

bool MR::hasPictureFontNW4R() {
    return SingletonHolder< GameSystem >::get()->mFontHolder->mPictureFont != nullptr;
}

bool MR::isScreen16Per9() {
    return MR::isAspectRatioFlag16Per9();
}

namespace {
NameObjHolder* getSceneNameObjHolder() NO_INLINE {
    return SingletonHolder< GameSystem >::get()->mSceneController->mObjHolder;
}
};  // namespace

namespace MR {
    u32 getHashCode(const char*);
    const char* getCurrentStageName();
ParticleResourceHolder* getParticleResourceHolder() {
    return getGameSystemObjHolder()->mParticleResHolder;
}

void requestChangeArchivePlayer(bool isPlayerMario) {
    GameSystemFunction::requestChangeArchivePlayer(isPlayerMario);
}

void waitEndChangeArchivePlayer() {
    while (!GameSystemFunction::isEndChangeArchivePlayer()) {
    }
}

void callMethodAllSceneNameObj(NameObjMethod pMethod) {
    ::getSceneNameObjHolder()->callMethodAllObj(pMethod);
}

void suspendAllSceneNameObj() {
    ::getSceneNameObjHolder()->suspendAllObj();
}

void resumeAllSceneNameObj() {
    ::getSceneNameObjHolder()->resumeAllObj();
}

void syncWithFlagsAllSceneNameObj() {
    ::getSceneNameObjHolder()->syncWithFlags();
}

void setRandomSeedFromStageName() {
    u32 seed = getHashCode(getCurrentStageName());

    getGameSystemObjHolder()->mRandom.mSeed = seed;
}

void clearFileLoaderRequestFileInfo(bool param1) {
    getGameSystemObjHolder()->clearRequestFileInfo(param1);
}
};  // namespace
