#include "Game/LiveActor/ActorReactionHelper.hpp"

bool tryRumblePadByName(const void* pHost, const char* pName) {
    if (pName == NULL) {
        return false;
    }

    MR::tryRumblePad(pHost, pName, 0);
    return true;
}

bool tryShakeCameraByName(const char* pName) {
    if (pName == NULL) {
        return false;
    }

    if (MR::isEqualStringCase(pName, "最強")) {
        MR::shakeCameraVeryStrong();
    } else if (MR::isEqualStringCase(pName, "強")) {
        MR::shakeCameraStrong();
    } else if (MR::isEqualStringCase(pName, "中強")) {
        MR::shakeCameraNormalStrong();
    } else if (MR::isEqualStringCase(pName, "中")) {
        MR::shakeCameraNormal();
    } else if (MR::isEqualStringCase(pName, "中弱")) {
        MR::shakeCameraNormalWeak();
    } else if (MR::isEqualStringCase(pName, "弱")) {
        MR::shakeCameraWeak();
    } else if (MR::isEqualStringCase(pName, "最弱")) {
        MR::shakeCameraVeryWeak();
    }

    return true;
}
