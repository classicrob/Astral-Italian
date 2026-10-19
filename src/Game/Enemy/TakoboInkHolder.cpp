#include "Game/Util.hpp"
#include "Game/Enemy/TakoboInkHolder.hpp"

TakoboInkHolder::TakoboInkHolder(s32 numMax) : DeriveActorGroup< TakoboInk >("タコボー墨管理", numMax) {
    for (s32 i = 0; i < numMax; i++) {
        TakoboInk* pInk = new TakoboInk("タコヘイ墨");
        pInk->initWithoutIter();
        registerActor(pInk);
    }
}

bool TakoboInkHolder::spurtTakoboInk(const TVec3f& rPos, const TVec3f& rDir) {
    TakoboInk* pInk = getDeadMember();
    if (pInk == nullptr) {
        return false;
    }
    pInk->start(rPos, rDir, false);
    return true;
}
