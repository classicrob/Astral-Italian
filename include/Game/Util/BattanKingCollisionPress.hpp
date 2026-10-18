#pragma once
#include "Game/Enemy/BattanKing.hpp"

namespace BattanKingCollisionUtil {
    bool isPointInBox(const TPos3f*, const TVec3f*, const TVec3f*);
    void calcDemoBounds(const NameObj*, TVec3f*, TVec3f*, TVec3f*);
    bool isPointInDemoBounds(const NameObj*, const TVec3f*);
    bool isFacingOutsideDemoBounds(const BattanKing*);
}
namespace MR {
    bool isPlayerInCollisionCylinder(MtxPtr, const TVec3f*, f32, f32);
    bool isPlayerInCollisionBox(const TPos3f*, const TVec3f*);
}
namespace NrvBattanKingTalkState {
    BATTAN_KING_NERVE_DECL(TryStart);
    BATTAN_KING_NERVE_DECL(Talk);
}

namespace BattanKingTalkVec {
    extern TVec3f sTalkOffset;
    extern TVec3f sBalloonOffset;
}
