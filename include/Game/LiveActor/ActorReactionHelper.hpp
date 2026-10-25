#pragma once

#include "Inline.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/StringUtil.hpp"

// Shared camera and rumble reactions implemented in ActorReactionHelper.cpp.

// Rumbles the pad with the named pattern. Returns false if there is no name.
bool tryRumblePadByName(const void* pHost, const char* pName);

// Shakes the camera with the strength named by pName (Shift-JIS names). Returns false if there is no name.
bool tryShakeCameraByName(const char* pName);
