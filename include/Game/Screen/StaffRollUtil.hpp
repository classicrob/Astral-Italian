#pragma once

#include <JSystem/JGeometry/TVec.hpp>

class LayoutActor;
class Nerve;
class StaffRollPart;
class StaffRollLine;
struct StaffRollPartInfo {
    s32 mType;
    TVec2f mPosition;

    StaffRollPartInfo(s32 type, const TVec2f& position) : mType(type), mPosition(position) {
    }
};

namespace StaffRollUtil {
    StaffRollLine* createLine(const wchar_t*, u32, s32, bool*);
    bool readPartInfo(StaffRollPartInfo*, const wchar_t*, u32);
    StaffRollPart* createPart(const wchar_t*, u32*);
    void initPane(LayoutActor*, TVec2f*, const char*);
    bool updatePaneAppear(LayoutActor*, const char*, const Nerve*);
    bool updatePaneWait(LayoutActor*, const char*, s32, const Nerve*);
    bool updatePaneDisappear(LayoutActor*, const char*, const Nerve*);
    extern s32 sThankYouVoiceFrame;
}
