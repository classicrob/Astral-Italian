#include "Game/Util.hpp"
#include "Game/Enemy/TakoboInkHolder.hpp"

bool TakoboInkHolder::spurtTakoboInk(const TVec3f& rPos, const TVec3f& rDir) {
    TakoboInk* pInk = getDeadMember();
    if (pInk == nullptr) {
        return false;
    }
    pInk->start(rPos, rDir, false);
    return true;
}
