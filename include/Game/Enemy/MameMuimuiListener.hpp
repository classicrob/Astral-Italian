#pragma once

class MameMuimui;

// Secondary base of MameMuimuiScorer (SMG2-only). The scorer stores this sub-object at its 0x90;
// every MameMuimui keeps a pointer to it (MameMuimui::fn_801BF570). Virtual slots are by address.
class MameMuimuiListener {
public:
    virtual void fn_801C19C0(s32 points, MameMuimui* pActor);  // slot +0x08 (thunk to the scorer)
    virtual void fn_801C19B0(MameMuimui* pActor);              // slot +0x0C (thunk to the scorer)
    virtual void fn_801BF5F0(MameMuimui* pActor);              // slot +0x10 (empty in SMG2)
    void fn_801BF600();                                        // slot +0x24 (empty)
    void fn_801BFB00();                                        // slot +0x28 (empty)
};
