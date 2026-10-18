#include "Game/Enemy/Jugem.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

// Nerve instances in the order the SB4E01 sinit initialises them (sbss 0x807D26B8 .. 0x807D2710).
// Exe names are provisional; the nerve vtables only carry the exe thunks (`lwz r3,0(r4); b exeX`).
namespace NrvJugem {
    NEW_NERVE(JugemNrvWait, Jugem, Wait);
    NEW_NERVE(JugemNrvReady, Jugem, Ready);
    NEW_NERVE(JugemNrvAim, Jugem, Aim);
    NEW_NERVE(JugemNrvApproach, Jugem, Approach);
    NEW_NERVE(JugemNrvMove, Jugem, Move);
    NEW_NERVE(JugemNrvMoveUp, Jugem, MoveUp);
    NEW_NERVE(JugemNrvThrowSign, Jugem, ThrowSign);
    NEW_NERVE(JugemNrvThrow, Jugem, Throw);
    NEW_NERVE(JugemNrvChase, Jugem, Chase);
    NEW_NERVE(JugemNrvHitReaction, Jugem, HitReaction);
    NEW_NERVE(JugemNrvFall, Jugem, Fall);
    NEW_NERVE(JugemNrvDrop, Jugem, Drop);
    NEW_NERVE(JugemNrvLand, Jugem, Land);
    NEW_NERVE(JugemNrvHide, Jugem, Hide);
    NEW_NERVE(JugemNrvReset, Jugem, Reset);

    // Nerve with an empty execute (vtable thunk is a bare blr in SB4E01).
    class JugemNrvNone : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static JugemNrvNone sInstance;
    };
    JugemNrvNone JugemNrvNone::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(JugemNrvBlowDown, Jugem, BlowDown);
    NEW_NERVE(JugemNrvFallDown, Jugem, FallDown);
    NEW_NERVE(JugemNrvEmerge, Jugem, Emerge);
    NEW_NERVE(JugemNrvDie, Jugem, Die);
    NEW_NERVE(JugemNrvFallLand, Jugem, FallLand);
    NEW_NERVE(JugemNrvFallRecovery, Jugem, FallRecovery);
    NEW_NERVE(JugemNrvSupportFreeze, Jugem, SupportFreeze);
};  // namespace NrvJugem

// Constant TVec3f globals read by Jugem code (SB4E01 sinit writes them at 0x80729010/1C/34/40).
TVec3f gJugemVecA(0.0f, 180.0f, 110.0f);
TVec3f gJugemVecB(0.0f, 0.0f, 0.0f);
TVec3f gJugemVecC(0.0f, 100.0f, 0.0f);
TVec3f gJugemVecD(70.0f, -70.0f, 0.0f);
TVec3f gJugemVecE(0.0f, 0.0f, 0.0f);

Jugem::~Jugem() {}

JugemCloud::~JugemCloud() {}
