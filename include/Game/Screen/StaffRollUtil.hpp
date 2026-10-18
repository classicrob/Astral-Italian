#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class LayoutActor;
class Nerve;
class StaffRollPart;

namespace StaffRollUtil {
    StaffRollPart* createPart(const wchar_t*, u32*);
    void initPane(LayoutActor*, TVec2f*, const char*);
    bool updatePaneAppear(LayoutActor*, const char*, const Nerve*);
    bool updatePaneWait(LayoutActor*, const char*, s32, const Nerve*);
    bool updatePaneDisappear(LayoutActor*, const char*, const Nerve*);
    extern s32 sThankYouVoiceFrame;
}
