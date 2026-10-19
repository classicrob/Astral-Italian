#pragma once

class MameMuimui;

// Secondary base of MameMuimuiScorer (SMG2-only). The scorer stores this sub-object at its 0x90;
// every MameMuimui keeps a pointer to it (MameMuimui::setMuimuiListener). Virtual slots are by address.
class MameMuimuiListener {
public:
    virtual void scoreHit(s32 points, MameMuimui* pActor);  // slot +0x08 (thunk to the scorer)
    virtual void breakCombo(MameMuimui* pActor);              // slot +0x0C (thunk to the scorer)
    virtual void onActorEventEmpty(MameMuimui* pActor);              // slot +0x10 (empty in SMG2)
    void onListenerEmpty();                                        // slot +0x24 (empty)
    void onUpdateEmpty();                                        // slot +0x28 (empty)
};
