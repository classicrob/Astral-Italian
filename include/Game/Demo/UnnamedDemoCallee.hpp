#pragma once

// Unnamed callees used by the core-lane demo units (SB4E01 symbols are still fn_XXXXXXXX).
// Each one is declared as a namespace function under its target address; the lead renames
// them with namefn/symbols.txt once the owning code is identified.

#include "Game/LiveActor/LiveActor.hpp"

class StaffRoll;
class NameObj;
class LayoutActor;
class ActorCameraInfo;

namespace UnnamedDemoCallee {
    void fn_8001BFE0();
    void fn_8001F5B0(LiveActor*, void*, const char*, s32);
    void fn_80011F80(f32, f32);
    void fn_800085C0(LayoutActor*, const char*, s32, s32);
    void fn_80020110(LiveActor*, const char*, s32);
    void fn_800209B0(LiveActor*, const JMapInfoIter&, const char*, const char*);
    void fn_804D64F0();
    s32 fn_80147640();
    s32 fn_801476E0();
    s32 fn_80147730();
    s32 fn_80147780();
    void fn_801477D0();
    void fn_80147800();
    void fn_80147830();
    void fn_80147860();
    void fn_80147930();
    void fn_80147990();
    void fn_801478C0();
    LiveActor* fn_8013FCC0(LiveActor*, const char*, const char*, MtxPtr);
    LiveActor* fn_8013FD20(LiveActor*, const char*, const char*, const char*, MtxPtr);
    LiveActor* fn_8013FD90(LiveActor*, const char*, const char*, const char*, s32);
    LiveActor* fn_8013FE00(LiveActor*, const char*, const char*);
    void fn_8013FE60(s32);
    void* fn_80051080();
    s32 fn_8004E770(const char*, const char*);
    s32 fn_80492640(NameObj*);
    s32 fn_804926A0(NameObj*);
    s32 fn_80492730(NameObj*);
    s32 fn_80492760(NameObj*);
    void fn_80422740();
    StaffRoll* fn_801440F0(LiveActor*, ActorCameraInfo*, s32);
    void fn_80144280(StaffRoll*);
    void fn_801442A0(StaffRoll*, s32);
    void fn_801442D0(StaffRoll*);
};  // namespace UnnamedDemoCallee
