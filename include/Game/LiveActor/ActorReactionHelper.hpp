#pragma once

#include "Inline.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"

// Shared by ActorHitReactionCtrl and ActorPadAndCameraCtrl. The target keeps one out-of-line weak copy of each
// (fn_8022AF90 / fn_8022AFD0), so they are NO_INLINE.

// Rumbles the pad with the named pattern. Returns false if there is no name.
inline bool tryRumblePadByName(const void* pHost, const char* pName) NO_INLINE;
inline bool tryRumblePadByName(const void* pHost, const char* pName) {
    if (pName == NULL) {
        return false;
    }

    MR::tryRumblePad(pHost, pName, 0);
    return true;
}

// Shakes the camera with the strength named by pName (Shift-JIS names). Returns false if there is no name.
inline bool tryShakeCameraByName(const char* pName) NO_INLINE;
inline bool tryShakeCameraByName(const char* pName) {
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
