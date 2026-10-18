#pragma auto_inline off
#include "Game/Boss/KoopaJrCastle.hpp"
#include "Game/Util.hpp"
#include <cmath>
#include "Game/LiveActor/ActorCameraInfo.hpp"
#include "Game/LiveActor/BreakModel.hpp"
#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModelLodCtrl.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/Util/JointController.hpp"
#include "Game/Ride/FluffWind.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/NPC/TalkMessageCtrl.hpp"

extern "C" void fn_8000E570(TVec3f*, LiveActor*, int);
extern "C" void fn_800209B0(LiveActor*, const JMapInfoIter&, const char*, const char*);
extern "C" void fn_801392A0(LiveActor*, TalkMessageCtrl*, const char*);

namespace NrvKoopaJrCastle {
    class KoopaJrCastleNrvOpening : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KoopaJrCastleNrvOpening sInstance;
    };
    KoopaJrCastleNrvOpening KoopaJrCastleNrvOpening::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(KoopaJrCastleNrvBattle, KoopaJrCastle, Battle);

    inline void KoopaJrCastleNrvOpening::execute(Spine* pSpine) const {
        LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
        MR::updateActorStateAndNextNerve(actor, &KoopaJrCastleNrvBattle::sInstance);
    }
};  // namespace NrvKoopaJrCastle

extern s32 lbl_807E3008;
namespace KoopaJrCastleFunction {
    bool fn_800D3540(const char*, const char*);
};
using KoopaJrCastleFunction::fn_800D3540;

namespace NrvKoopaJrCastleStateBodyMove {
    class KoopaJrCastleStateBodyMoveNrvSel : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CF2A0();
        }
        static KoopaJrCastleStateBodyMoveNrvSel sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvTurnA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CF3B0();
        }
        static KoopaJrCastleStateBodyMoveNrvTurnA sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvTurnB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CF530();
        }
        static KoopaJrCastleStateBodyMoveNrvTurnB sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvMoveA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CF5E0();
        }
        static KoopaJrCastleStateBodyMoveNrvMoveA sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvRotA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CF8D0();
        }
        static KoopaJrCastleStateBodyMoveNrvRotA sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvRotB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CFA70();
        }
        static KoopaJrCastleStateBodyMoveNrvRotB sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvRotC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CFC40();
        }
        static KoopaJrCastleStateBodyMoveNrvRotC sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvMoveB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CF700();
        }
        static KoopaJrCastleStateBodyMoveNrvMoveB sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvStop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exe800CFDD0();
        }
        static KoopaJrCastleStateBodyMoveNrvStop sInstance;
    };
    KoopaJrCastleStateBodyMoveNrvSel KoopaJrCastleStateBodyMoveNrvSel::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBodyMoveNrvTurnA KoopaJrCastleStateBodyMoveNrvTurnA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBodyMoveNrvTurnB KoopaJrCastleStateBodyMoveNrvTurnB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBodyMoveNrvMoveA KoopaJrCastleStateBodyMoveNrvMoveA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBodyMoveNrvRotA KoopaJrCastleStateBodyMoveNrvRotA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBodyMoveNrvRotB KoopaJrCastleStateBodyMoveNrvRotB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBodyMoveNrvRotC KoopaJrCastleStateBodyMoveNrvRotC::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBodyMoveNrvMoveB KoopaJrCastleStateBodyMoveNrvMoveB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBodyMoveNrvStop KoopaJrCastleStateBodyMoveNrvStop::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleStateBodyMove

namespace NrvKoopaJrCastleCannon {
    class KoopaJrCastleCannonNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exe800D0640();
        }
        static KoopaJrCastleCannonNrvA sInstance;
    };
    class KoopaJrCastleCannonNrvPre : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            if (MR::isGreaterStep(a, 60)) {
                a->setNerve(&KoopaJrCastleCannonNrvA::sInstance);
            }
        }
        static KoopaJrCastleCannonNrvPre sInstance;
    };
    class KoopaJrCastleCannonNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exe800D0720();
        }
        static KoopaJrCastleCannonNrvB sInstance;
    };
    class KoopaJrCastleCannonNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exe800D07E0();
        }
        static KoopaJrCastleCannonNrvC sInstance;
    };
    class KoopaJrCastleCannonNrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exe800D09B0();
        }
        static KoopaJrCastleCannonNrvD sInstance;
    };
    class KoopaJrCastleCannonNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exe800D0A10();
        }
        static KoopaJrCastleCannonNrvE sInstance;
    };
    KoopaJrCastleCannonNrvPre KoopaJrCastleCannonNrvPre::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleCannonNrvA KoopaJrCastleCannonNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleCannonNrvB KoopaJrCastleCannonNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleCannonNrvC KoopaJrCastleCannonNrvC::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleCannonNrvD KoopaJrCastleCannonNrvD::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleCannonNrvE KoopaJrCastleCannonNrvE::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleCannon

namespace NrvKoopaJrCastleCannonShock {
    class KoopaJrCastleCannonShockNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannonShock* a = reinterpret_cast< KoopaJrCastleCannonShock* >(pSpine->mExecutor);
            a->exe800D0FA0();
        }
        static KoopaJrCastleCannonShockNrvA sInstance;
    };
    class KoopaJrCastleCannonShockNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannonShock* a = reinterpret_cast< KoopaJrCastleCannonShock* >(pSpine->mExecutor);
            a->exe800D10A0();
        }
        static KoopaJrCastleCannonShockNrvB sInstance;
    };
    class KoopaJrCastleCannonShockNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannonShock* a = reinterpret_cast< KoopaJrCastleCannonShock* >(pSpine->mExecutor);
            a->exe800D1220();
        }
        static KoopaJrCastleCannonShockNrvC sInstance;
    };
    KoopaJrCastleCannonShockNrvA KoopaJrCastleCannonShockNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleCannonShockNrvB KoopaJrCastleCannonShockNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleCannonShockNrvC KoopaJrCastleCannonShockNrvC::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleCannonShock

namespace NrvKoopaJrCastleCannonBall {
    class KoopaJrCastleCannonBallNrvMove : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannonBall* a = reinterpret_cast< KoopaJrCastleCannonBall* >(pSpine->mExecutor);
            a->exe800D1830();
        }
        static KoopaJrCastleCannonBallNrvMove sInstance;
    };
    KoopaJrCastleCannonBallNrvMove KoopaJrCastleCannonBallNrvMove::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleCannonBall

namespace NrvKoopaJrCastleElectric {
    class KoopaJrCastleElectricNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleElectric* a = reinterpret_cast< KoopaJrCastleElectric* >(pSpine->mExecutor);
            a->exe800D20F0();
        }
        static KoopaJrCastleElectricNrvA sInstance;
    };
    class KoopaJrCastleElectricNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleElectric* a = reinterpret_cast< KoopaJrCastleElectric* >(pSpine->mExecutor);
            a->exe800D21A0();
        }
        static KoopaJrCastleElectricNrvB sInstance;
    };
    KoopaJrCastleElectricNrvA KoopaJrCastleElectricNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleElectricNrvB KoopaJrCastleElectricNrvB::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleElectric

namespace NrvKoopaJrCastleFireBar {
    class KoopaJrCastleFireBarNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleFireBar* a = reinterpret_cast< KoopaJrCastleFireBar* >(pSpine->mExecutor);
            a->exe800D2B50();
        }
        static KoopaJrCastleFireBarNrvA sInstance;
    };
    class KoopaJrCastleFireBarNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleFireBar* a = reinterpret_cast< KoopaJrCastleFireBar* >(pSpine->mExecutor);
            MR::isFirstStep(a);
            MR::startActionSound(a, "OjLvFireBarCenter", -1, -1, -1);
        }
        static KoopaJrCastleFireBarNrvB sInstance;
    };
    class KoopaJrCastleFireBarNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleFireBar* a = reinterpret_cast< KoopaJrCastleFireBar* >(pSpine->mExecutor);
            a->exe800D2BF0();
        }
        static KoopaJrCastleFireBarNrvC sInstance;
    };
    KoopaJrCastleFireBarNrvA KoopaJrCastleFireBarNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleFireBarNrvB KoopaJrCastleFireBarNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleFireBarNrvC KoopaJrCastleFireBarNrvC::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleFireBar

namespace NrvKoopaJrCastleHead {
    class KoopaJrCastleHeadNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exe800D5960();
        }
        static KoopaJrCastleHeadNrvA sInstance;
    };
    class KoopaJrCastleHeadNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exe800D59F0();
        }
        static KoopaJrCastleHeadNrvB sInstance;
    };
    class KoopaJrCastleHeadNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exe800D5A80();
        }
        static KoopaJrCastleHeadNrvC sInstance;
    };
    class KoopaJrCastleHeadNrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            
        }
        static KoopaJrCastleHeadNrvD sInstance;
    };
    class KoopaJrCastleHeadNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exe800D5B10();
        }
        static KoopaJrCastleHeadNrvE sInstance;
    };
    class KoopaJrCastleHeadNrvF : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exe800D5BF0();
        }
        static KoopaJrCastleHeadNrvF sInstance;
    };
    class KoopaJrCastleHeadNrvG : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exe800D5BF0();
        }
        static KoopaJrCastleHeadNrvG sInstance;
    };
    class KoopaJrCastleHeadNrvH : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exe800D5D20();
        }
        static KoopaJrCastleHeadNrvH sInstance;
    };
    KoopaJrCastleHeadNrvA KoopaJrCastleHeadNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleHeadNrvB KoopaJrCastleHeadNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleHeadNrvC KoopaJrCastleHeadNrvC::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleHeadNrvD KoopaJrCastleHeadNrvD::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleHeadNrvE KoopaJrCastleHeadNrvE::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleHeadNrvF KoopaJrCastleHeadNrvF::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleHeadNrvG KoopaJrCastleHeadNrvG::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleHeadNrvH KoopaJrCastleHeadNrvH::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleHead

namespace NrvKoopaJrCastleStateBattle {
    class KoopaJrCastleStateBattleNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exe800D68C0();
        }
        static KoopaJrCastleStateBattleNrvA sInstance;
    };
    class KoopaJrCastleStateBattleNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exe800D6910();
        }
        static KoopaJrCastleStateBattleNrvB sInstance;
    };
    class KoopaJrCastleStateBattleNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exe800D6970();
        }
        static KoopaJrCastleStateBattleNrvC sInstance;
    };
    class KoopaJrCastleStateBattleNrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exe800D69D0();
        }
        static KoopaJrCastleStateBattleNrvD sInstance;
    };
    class KoopaJrCastleStateBattleNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exe800D6AD0();
        }
        static KoopaJrCastleStateBattleNrvE sInstance;
    };
    KoopaJrCastleStateBattleNrvA KoopaJrCastleStateBattleNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattleNrvB KoopaJrCastleStateBattleNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattleNrvC KoopaJrCastleStateBattleNrvC::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattleNrvD KoopaJrCastleStateBattleNrvD::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattleNrvE KoopaJrCastleStateBattleNrvE::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleStateBattle

namespace NrvKoopaJrCastleStateOpening {
    class KoopaJrCastleStateOpeningNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateOpening* a = reinterpret_cast< KoopaJrCastleStateOpening* >(pSpine->mExecutor);
            a->exe800D6D80();
        }
        static KoopaJrCastleStateOpeningNrvA sInstance;
    };
    class KoopaJrCastleStateOpeningNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateOpening* a = reinterpret_cast< KoopaJrCastleStateOpening* >(pSpine->mExecutor);
            a->exe800D6E00();
        }
        static KoopaJrCastleStateOpeningNrvB sInstance;
    };
    class KoopaJrCastleStateOpeningNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateOpening* a = reinterpret_cast< KoopaJrCastleStateOpening* >(pSpine->mExecutor);
            a->exe800D6E90();
        }
        static KoopaJrCastleStateOpeningNrvC sInstance;
    };
    KoopaJrCastleStateOpeningNrvA KoopaJrCastleStateOpeningNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateOpeningNrvB KoopaJrCastleStateOpeningNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateOpeningNrvC KoopaJrCastleStateOpeningNrvC::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleStateOpening

namespace NrvKoopaJrCastleDominate {
    class KoopaJrCastleDominateNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleDominate* a = reinterpret_cast< KoopaJrCastleDominate* >(pSpine->mExecutor);
            a->exe800D7570();
        }
        static KoopaJrCastleDominateNrvA sInstance;
    };
    KoopaJrCastleDominateNrvA KoopaJrCastleDominateNrvA::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleDominate

namespace NrvKoopaJrCastleRingBeam {
    class KoopaJrCastleRingBeamNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleRingBeam* a = reinterpret_cast< KoopaJrCastleRingBeam* >(pSpine->mExecutor);
            a->exe800D7C00();
        }
        static KoopaJrCastleRingBeamNrvA sInstance;
    };
    class KoopaJrCastleRingBeamNrvPre : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleRingBeam* a = reinterpret_cast< KoopaJrCastleRingBeam* >(pSpine->mExecutor);
            if (MR::isGreaterStep(a, 90)) {
                a->setNerve(&KoopaJrCastleRingBeamNrvA::sInstance);
            }
        }
        static KoopaJrCastleRingBeamNrvPre sInstance;
    };
    KoopaJrCastleRingBeamNrvPre KoopaJrCastleRingBeamNrvPre::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleRingBeamNrvA KoopaJrCastleRingBeamNrvA::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleRingBeam

namespace NrvKoopaJrCastleStateBattle1 {
    class KoopaJrCastleStateBattle1NrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle1* a = reinterpret_cast< KoopaJrCastleStateBattle1* >(pSpine->mExecutor);
            a->exe800D7F60();
        }
        static KoopaJrCastleStateBattle1NrvA sInstance;
    };
    class KoopaJrCastleStateBattle1NrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle1* a = reinterpret_cast< KoopaJrCastleStateBattle1* >(pSpine->mExecutor);
            a->exe800D7FE0();
        }
        static KoopaJrCastleStateBattle1NrvB sInstance;
    };
    class KoopaJrCastleStateBattle1NrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle1* a = reinterpret_cast< KoopaJrCastleStateBattle1* >(pSpine->mExecutor);
            a->exe800D8060();
        }
        static KoopaJrCastleStateBattle1NrvC sInstance;
    };
    KoopaJrCastleStateBattle1NrvA KoopaJrCastleStateBattle1NrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle1NrvB KoopaJrCastleStateBattle1NrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle1NrvC KoopaJrCastleStateBattle1NrvC::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleStateBattle1

namespace NrvKoopaJrCastleStateBattle2 {
    class KoopaJrCastleStateBattle2NrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exe800D82B0();
        }
        static KoopaJrCastleStateBattle2NrvA sInstance;
    };
    class KoopaJrCastleStateBattle2NrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exe800D8330();
        }
        static KoopaJrCastleStateBattle2NrvB sInstance;
    };
    class KoopaJrCastleStateBattle2NrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exe800D83B0();
        }
        static KoopaJrCastleStateBattle2NrvC sInstance;
    };
    class KoopaJrCastleStateBattle2NrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exe800D8490();
        }
        static KoopaJrCastleStateBattle2NrvD sInstance;
    };
    class KoopaJrCastleStateBattle2NrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exe800D8500();
        }
        static KoopaJrCastleStateBattle2NrvE sInstance;
    };
    class KoopaJrCastleStateBattle2NrvF : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exe800D8550();
        }
        static KoopaJrCastleStateBattle2NrvF sInstance;
    };
    KoopaJrCastleStateBattle2NrvA KoopaJrCastleStateBattle2NrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle2NrvB KoopaJrCastleStateBattle2NrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle2NrvC KoopaJrCastleStateBattle2NrvC::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle2NrvD KoopaJrCastleStateBattle2NrvD::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle2NrvE KoopaJrCastleStateBattle2NrvE::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle2NrvF KoopaJrCastleStateBattle2NrvF::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleStateBattle2

namespace NrvKoopaJrCastleStateBattle3 {
    class KoopaJrCastleStateBattle3NrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8840();
        }
        static KoopaJrCastleStateBattle3NrvA sInstance;
    };
    class KoopaJrCastleStateBattle3NrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D88B0();
        }
        static KoopaJrCastleStateBattle3NrvB sInstance;
    };
    class KoopaJrCastleStateBattle3NrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8920();
        }
        static KoopaJrCastleStateBattle3NrvC sInstance;
    };
    class KoopaJrCastleStateBattle3NrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8990();
        }
        static KoopaJrCastleStateBattle3NrvD sInstance;
    };
    class KoopaJrCastleStateBattle3NrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8A00();
        }
        static KoopaJrCastleStateBattle3NrvE sInstance;
    };
    class KoopaJrCastleStateBattle3NrvF : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8AC0();
        }
        static KoopaJrCastleStateBattle3NrvF sInstance;
    };
    class KoopaJrCastleStateBattle3NrvG : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8BA0();
        }
        static KoopaJrCastleStateBattle3NrvG sInstance;
    };
    class KoopaJrCastleStateBattle3NrvH : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            MR::isFirstStep(a);
            if (MR::isGreaterStep(a, lbl_807E3008)) {
                a->setNerve(&KoopaJrCastleStateBattle3NrvF::sInstance);
            }
        }
        static KoopaJrCastleStateBattle3NrvH sInstance;
    };
    class KoopaJrCastleStateBattle3NrvI : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8C90();
        }
        static KoopaJrCastleStateBattle3NrvI sInstance;
    };
    class KoopaJrCastleStateBattle3NrvJ : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8CF0();
        }
        static KoopaJrCastleStateBattle3NrvJ sInstance;
    };
    class KoopaJrCastleStateBattle3NrvK : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8D50();
        }
        static KoopaJrCastleStateBattle3NrvK sInstance;
    };
    class KoopaJrCastleStateBattle3NrvL : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8DA0();
        }
        static KoopaJrCastleStateBattle3NrvL sInstance;
    };
    class KoopaJrCastleStateBattle3NrvM : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8E30();
        }
        static KoopaJrCastleStateBattle3NrvM sInstance;
    };
    class KoopaJrCastleStateBattle3NrvN : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exe800D8E80();
        }
        static KoopaJrCastleStateBattle3NrvN sInstance;
    };
    KoopaJrCastleStateBattle3NrvA KoopaJrCastleStateBattle3NrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvB KoopaJrCastleStateBattle3NrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvC KoopaJrCastleStateBattle3NrvC::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvD KoopaJrCastleStateBattle3NrvD::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvE KoopaJrCastleStateBattle3NrvE::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvF KoopaJrCastleStateBattle3NrvF::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvG KoopaJrCastleStateBattle3NrvG::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvH KoopaJrCastleStateBattle3NrvH::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvI KoopaJrCastleStateBattle3NrvI::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvJ KoopaJrCastleStateBattle3NrvJ::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvK KoopaJrCastleStateBattle3NrvK::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvL KoopaJrCastleStateBattle3NrvL::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvM KoopaJrCastleStateBattle3NrvM::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle3NrvN KoopaJrCastleStateBattle3NrvN::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleStateBattle3

namespace NrvKoopaJrCastleStateBattle4 {
    class KoopaJrCastleStateBattle4NrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle4* a = reinterpret_cast< KoopaJrCastleStateBattle4* >(pSpine->mExecutor);
            a->exe800D93F0();
        }
        static KoopaJrCastleStateBattle4NrvA sInstance;
    };
    class KoopaJrCastleStateBattle4NrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle4* a = reinterpret_cast< KoopaJrCastleStateBattle4* >(pSpine->mExecutor);
            a->exe800D9450();
        }
        static KoopaJrCastleStateBattle4NrvC sInstance;
    };
    class KoopaJrCastleStateBattle4NrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle4* a = reinterpret_cast< KoopaJrCastleStateBattle4* >(pSpine->mExecutor);
            if (fn_800D3540("DemoDownKoopaJrCastle", nullptr)) {
                a->setNerve(&KoopaJrCastleStateBattle4NrvC::sInstance);
            }
        }
        static KoopaJrCastleStateBattle4NrvB sInstance;
    };
    KoopaJrCastleStateBattle4NrvA KoopaJrCastleStateBattle4NrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle4NrvB KoopaJrCastleStateBattle4NrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleStateBattle4NrvC KoopaJrCastleStateBattle4NrvC::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleStateBattle4

namespace NrvKoopaJrCastleWind {
    class KoopaJrCastleWindNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWind* a = reinterpret_cast< KoopaJrCastleWind* >(pSpine->mExecutor);
            a->exe800D9CF0();
        }
        static KoopaJrCastleWindNrvA sInstance;
    };
    class KoopaJrCastleWindNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWind* a = reinterpret_cast< KoopaJrCastleWind* >(pSpine->mExecutor);
            a->exe800D9D60();
        }
        static KoopaJrCastleWindNrvB sInstance;
    };
    class KoopaJrCastleWindNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWind* a = reinterpret_cast< KoopaJrCastleWind* >(pSpine->mExecutor);
            a->exe800D9E20();
        }
        static KoopaJrCastleWindNrvC sInstance;
    };
    class KoopaJrCastleWindNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWind* a = reinterpret_cast< KoopaJrCastleWind* >(pSpine->mExecutor);
            a->exe800D9EC0();
        }
        static KoopaJrCastleWindNrvE sInstance;
    };
    class KoopaJrCastleWindNrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWind* a = reinterpret_cast< KoopaJrCastleWind* >(pSpine->mExecutor);
            if (MR::isGreaterStep(a, 73)) {
                a->setNerve(&KoopaJrCastleWindNrvE::sInstance);
            }
        }
        static KoopaJrCastleWindNrvD sInstance;
    };
    KoopaJrCastleWindNrvA KoopaJrCastleWindNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleWindNrvB KoopaJrCastleWindNrvB::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleWindNrvC KoopaJrCastleWindNrvC::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleWindNrvD KoopaJrCastleWindNrvD::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleWindNrvE KoopaJrCastleWindNrvE::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleWind

namespace NrvKoopaJrCastleWindUp {
    class KoopaJrCastleWindUpNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWindUp* a = reinterpret_cast< KoopaJrCastleWindUp* >(pSpine->mExecutor);
            a->exe800DA640();
        }
        static KoopaJrCastleWindUpNrvA sInstance;
    };
    class KoopaJrCastleWindUpNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWindUp* a = reinterpret_cast< KoopaJrCastleWindUp* >(pSpine->mExecutor);
            a->exe800DA6F0();
        }
        static KoopaJrCastleWindUpNrvB sInstance;
    };
    KoopaJrCastleWindUpNrvA KoopaJrCastleWindUpNrvA::sInstance ATTRIBUTE_WEAK;
    KoopaJrCastleWindUpNrvB KoopaJrCastleWindUpNrvB::sInstance ATTRIBUTE_WEAK;
}  // namespace NrvKoopaJrCastleWindUp

KoopaJrCastle::KoopaJrCastle(const char* pName)
    : LiveActor(pName), mWind(nullptr), mCannon(nullptr), mFireBar(nullptr), mRingBeam(nullptr), mElectric(nullptr), mDominate(nullptr),
      mBody(nullptr), mHead(nullptr), mCapsule(nullptr), mKoopaJr(nullptr), mBreakModel(nullptr), mStateOpening(nullptr),
      mStateBattle(nullptr), mCameraInfo(nullptr), mFrontVec(0.0f, 0.0f, 0.0f), mStartPos(0.0f, 0.0f, 0.0f), _110(0.0f, 0.0f, 0.0f) {
    PSMTXIdentity(mBaseMtx.toMtxPtr());
}

void KoopaJrCastle::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    MR::initDefaultPos(this, rIter);
    MR::invalidateClipping(this);
    mStartPos = mPosition;
    MR::makeMtxTR(mBaseMtx.toMtxPtr(), this);
    MR::calcFrontVec(&mFrontVec, this);
    MR::initUseStageSwitchReadA(this, rIter);
    MR::initUseStageSwitchReadB(this, rIter);
    MR::useStageSwitchWriteAutoDead(this, rIter);
    MR::setClippingFarMax(this);
    MR::addBaseMatrixFollowTarget(this, rIter, nullptr, nullptr);
    MR::getJMapInfoArg7WithInit(rIter, &mCameraRegId);
    if (mCameraRegId != -1) {
        MR::declareCameraRegisterVec(this, mCameraRegId, &mCameraPos);
    }
    MR::createSceneObj(0x8B);
    reinterpret_cast< KoopaJrCastleAccessor* >(MR::getSceneObjHolder()->getObj(0x8B))->mCastle = this;

    mBody = new KoopaJrCastleBody(this, "\x93y\x91\xe4(Body)");
    mBody->initWithoutIter();
    mHead = new KoopaJrCastleHead(mBody, "\x96""C\x91\xe4(Head)");
    MR::invalidateClipping(mHead);
    mHead->initFixedPosition(MR::getJointMtx(mBody, "HeadPos"), TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mHead->initWithoutIter();
    mCapsule = new KoopaJrCastleCapsule(mHead, "\x83J\x83v\x83Z\x83\x8b");
    mCapsule->initFixedPosition(MR::getJointMtx(mHead, "Cockpit"), TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mCapsule->initWithoutIter();
    MR::invalidateClipping(mCapsule);
    mKoopaJr = new PartsModelLodCtrl(mHead, "\x83N\x83""b\x83pJr", "KoopaJr", 0, 0x13, false);
    MR::initShadowFromCSVWithInitShadowVolumeSphere(mKoopaJr, "ShadowKoopaJrCastle");
    MR::validateClipping(mKoopaJr);
    mKoopaJr->initFixedPosition(MR::getJointMtx(mHead, "Apron"), TVec3f(0.0f, 130.0f, 100.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mKoopaJr->initWithoutIter();
    mKoopaJr->initLightCtrl();
    mKoopaJr->mLinkInfo.mZoneID = mLinkInfo.mZoneID;
    mKoopaJr->setLodCtrl(nullptr)->setDistanceToMiddle(0.0f);
    MR::startBva(mKoopaJr->setLodCtrl(nullptr)->mMiddleModel, "Wait");
    MR::startBva(mKoopaJr->setLodCtrl(nullptr)->mLowModel, "Wait");
    MR::startAction(mKoopaJr, "Wait");
    mBreakModel = new BreakModel(this, "\x83L\x83\x83\x83""b\x83X\x83\x8b\x89\xf3\x82\xea\x83\x82\x83""f\x83\x8b", "KoopaJrCastleBreak", 0, "Break");
    mBreakModel->initWithoutIter();
    mCameraInfo = new ActorCameraInfo(rIter);
    mWind = new KoopaJrCastleWind(this, "\x95\x97\x81i\x89~\x81j");
    MR::setPosition(mWind, mPosition);
    mWind->initWithoutIter();
    mCannon = new KoopaJrCastleCannon(this, "\x96""C\x8c\x82");
    mCannon->initWithoutIter();
    mFireBar = new KoopaJrCastleFireBar("\x83t\x83@\x83""C\x83""A\x83o\x81[");
    mFireBar->setup(mBody, TVec3f(0.0f, 1635.0f, 0.0f), TVec3f(0.0f), "KoopaJrCastleBody");
    mFireBar->initWithoutIter();
    mRingBeam = new KoopaJrCastleRingBeam("\x83\x8a\x83\x93\x83O\x83r\x81[\x83\x80\x8dU\x8c\x82");
    mRingBeam->setup(mBody, TVec3f(0.0f, 1400.0f, 0.0f), TVec3f(0.0f), "KoopaJrCastleBody");
    mRingBeam->initWithoutIter();
    mElectric = new KoopaJrCastleElectric("\x93""d\x8b""C");
    mElectric->initWithoutIter();
    mDominate = new KoopaJrCastleDominate("\x83v\x83\x8c\x83""C\x83\x84\x81[\x8ex\x94z");
    mDominate->initWithoutIter();
    initNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvOpening::sInstance, 2);
    mStateOpening = new KoopaJrCastleStateOpening(this);
    MR::initActorState(this, mStateOpening, &NrvKoopaJrCastle::KoopaJrCastleNrvOpening::sInstance, "PhaseOpening");
    mStateBattle = new KoopaJrCastleStateBattle(this);
    MR::initActorState(this, mStateBattle, &NrvKoopaJrCastle::KoopaJrCastleNrvBattle::sInstance, "PhaseBattle");
    initBinder(375.0f, 375.0f, 0);
    MR::offBind(this);
    MR::declarePowerStar(this);
    MR::declareStarPiece(this, 30);
    fn_800209B0(mBody, rIter, "\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x93o\x8f\xea", "DemoOpeningKoopaJrCastle");
    MtxPtr rootMtx = MR::getJointMtx(mKoopaJr, "Root");
    fn_801392A0(mKoopaJr, MR::createTalkCtrlDirect(mKoopaJr, rIter, "DemoOpeningKoopaJrCastle000", TVec3f(0.0f), rootMtx), "\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x93o\x8f\xea");
    MR::initTalkAnimDemoCast(mKoopaJr, rIter, "\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x93o\x8f\xea", "DemoOpeningKoopaJrCastle");
    MR::initTalkAnimDemoCast(mHead, rIter, "\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x93o\x8f\xea", "DemoOpeningKoopaJrCastle");
    makeActorAppeared();
}

void KoopaJrCastle::makeActorAppeared() {
    MR::invalidateClipping(this);
    setNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvOpening::sInstance);
    LiveActor::makeActorAppeared();
}

void KoopaJrCastle::control() {
    mCameraPos = mCapsule->mPosition;
    TVec3f grav(0.0f);
    fn_8000E570(&grav, mCapsule, 0);
    if (!MR::isNearZero(grav, 0.001f)) {
        MR::normalize(&grav);
        TPos3f mtx;
        PSMTXIdentity(mtx.toMtxPtr());
        MR::makeMtxFrontUp(&mtx, mGravity, grav);
        MR::extractMtxXDir(mtx.toMtxPtr(), &_110);
        TVec3f toPlayer(0.0f);
        MR::calcVecToPlayer(&toPlayer, mCapsule);
        f32 d = toPlayer.dot(mGravity) / 2000.0f;
        f32 v;
        if (d < 0.0f) {
            v = 0.0f;
        } else if (d > 1.0f) {
            v = 1.0f;
        } else {
            v = d;
        }
        _110 = (grav * -0.0f) * v;
    }
    mCameraPos += _110;
    if (mStateBattle->isDemo()) {
        mCameraPos += mGravity * 1200.0f;
    } else {
        mCameraPos += mGravity * -100.0f;
    }
    MR::calcMtxFromGravityAndZAxis(&mBaseMtx, this, mFrontVec);
}

void KoopaJrCastle::exeBattle() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(this);
        MR::startStageBGM("MBGM_SMG2_BOSS_06A", false);
    }
    if (MR::updateActorState(this)) {
        TVec3f pos(mPosition);
        pos -= mGravity * 1600.0f;
        MR::requestAppearPowerStar(this, pos);
        TVec3f cam(mStartPos);
        cam += mGravity * -100.0f;
        mCameraPos = cam;
        mCapsule->kill();
        mHead->kill();
        mBody->kill();
        kill();
    }
}

void KoopaJrCastle::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvBattle::sInstance)) {
        mStateBattle->attackSensor(pSender, pReceiver);
    }
}

bool KoopaJrCastle::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvBattle::sInstance)) {
        return mStateBattle->receiveMsgPlayerAttack(msg, pSender, pReceiver);
    }
    return false;
}

bool KoopaJrCastle::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastle::KoopaJrCastleNrvBattle::sInstance)) {
        return mStateBattle->receiveOtherMsg(msg, pSender, pReceiver);
    }
    return false;
}

KoopaJrCastleAccessor::KoopaJrCastleAccessor() : NameObj("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x83""A\x83N\x83Z\x83T"), mCastle(nullptr) {}

MtxPtr KoopaJrCastle::getBaseMtx() const {
    return const_cast< TPos3f& >(mBaseMtx).toMtxPtr();
}

KoopaJrCastle::~KoopaJrCastle() {}

KoopaJrCastleAccessor::~KoopaJrCastleAccessor() {}

void KoopaJrCastleBody::exeStop() {
    if (MR::isFirstStep(this)) {
        _F0 = 0.0f;
    }
}

namespace NrvKoopaJrCastleBody {
    NEW_NERVE(KoopaJrCastleBodyNrvWait, KoopaJrCastleBody, Wait);
    NEW_NERVE(KoopaJrCastleBodyNrvStop, KoopaJrCastleBody, Stop);
    class KoopaJrCastleBodyNrvMove : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, &KoopaJrCastleBodyNrvStop::sInstance);
        };
        static KoopaJrCastleBodyNrvMove sInstance;
    };
    KoopaJrCastleBodyNrvMove KoopaJrCastleBodyNrvMove::sInstance ATTRIBUTE_WEAK;
    class KoopaJrCastleBodyNrvDummy1 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static KoopaJrCastleBodyNrvDummy1 sInstance;
    };
    KoopaJrCastleBodyNrvDummy1 KoopaJrCastleBodyNrvDummy1::sInstance ATTRIBUTE_WEAK;
    class KoopaJrCastleBodyNrvDummy2 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static KoopaJrCastleBodyNrvDummy2 sInstance;
    };
    KoopaJrCastleBodyNrvDummy2 KoopaJrCastleBodyNrvDummy2::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(KoopaJrCastleBodyNrvFall, KoopaJrCastleBody, Fall);
    NEW_NERVE(KoopaJrCastleBodyNrvDead, KoopaJrCastleBody, Dead);
};  // namespace NrvKoopaJrCastleBody

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleBody (PartsModel, 0x110)
// ---------------------------------------------------------------------------------------------
namespace {
    struct GearInfo {
        const char* mName;
        int mType;
    };

    static const GearInfo sGearTable[28] = {
        {"GearL1", 1},  {"GearL2", -2}, {"GearL3", 2},  {"GearL4", -2}, {"GearL5", 2},  {"GearL6", 2},  {"GearL7", 2},
        {"GearL8", 2},  {"GearL9", 2},  {"GearL10", 2}, {"GearL11", 2}, {"GearL12", 2}, {"GearL13", 2}, {"GearL14", 1},
        {"GearR1", 1},  {"GearR2", -2}, {"GearR3", 2},  {"GearR4", -2}, {"GearR5", 2},  {"GearR6", 2},  {"GearR7", 2},
        {"GearR8", 2},  {"GearR9", 2},  {"GearR10", 2}, {"GearR11", 2}, {"GearR12", 2}, {"GearR13", 2}, {"GearR14", 1},
    };

    static const char* sGearNames[11] = {"BodyGear1", "BodyGear2", "BodyGear3", "Propeller1", "Propeller2", "Propeller3",
                                         "Propeller4", "Windmill1", "Windmill2", "Windmill3", "Windmill4"};

    TVec3f sAxisY(0.0f, 1.0f, 0.0f);
    TVec3f sAxisBack(0.0f, 0.0f, -1.0f);
    TVec3f sAxisFront(0.0f, 0.0f, 1.0f);

    typedef JointControlDelegator< KoopaJrCastleBody > BodyDelegator;

    static JointController* createBodyJoint(KoopaJrCastleBody* pHost, const char* pName, bool (KoopaJrCastleBody::*pFunc)(TPos3f*, const JointControllerInfo&)) {
        BodyDelegator* pCtrl = new BodyDelegator(pHost, pFunc, nullptr);
        MR::setJointControllerParam(pCtrl, pHost, pName);
        return pCtrl;
    }
};  // namespace

KoopaJrCastleBody::KoopaJrCastleBody(LiveActor* pHost, const char* pName)
    : PartsModel(pHost, pName, "KoopaJrCastleBody", mMtx, 0x13, false), mLowModel(nullptr), mGearCtrl(nullptr), mGearCtrlLR(nullptr),
      mCollision(nullptr), mState(nullptr), _B4(true), mGearMtx(nullptr), mJointIdx(nullptr), _F4(false), _F5(false), _F6(false),
      _F8(0.0f), _FC(0.0f), mRate(0.0f), mGearStep(0), mPropStep(0), mWindStep(0) {
    PSMTXCopy(pHost->getBaseMtx(), mMtx);
}

KoopaJrCastleBody::~KoopaJrCastleBody() {}

void KoopaJrCastleBody::makeActorAppeared() {
    PartsModel::makeActorAppeared();
    MR::tryStartActionNotPlaying(this, "Wait");
    mGearStep = 0;
    mPropStep = 0;
    mWindStep = 0;
    _F8 = 0.0f;
    _FC = 0.0f;
    mRate = 0.0f;
}

void KoopaJrCastleBody::calcAnim() {
    PartsModel::calcAnim();
    if (!MR::isDead(this) && !MR::isNoCalcAnim(this)) {
        if (!MR::isClipped(this)) {
            if (mCollision != nullptr) {
                for (int i = 0; i < 2; i++) {
                    mCollision[i]->setMtx();
                }
            }
        }
    }
}

void KoopaJrCastleBody::control() {
    if (_F4) {
        if (!MR::isDead(mLowModel)) {
            mLowModel->kill();
        }
        MR::showModelIfHidden(this);
    } else {
        if (MR::isDead(mLowModel)) {
            mLowModel->appear();
        }
        MR::hideModelAndOnCalcAnimIfShown(this);
    }
    PSMTXCopy(mHost->getBaseMtx(), mMtx);
    if (!_F5) {
        MR::addTransMtxLocalY(mMtx, _F0);
    }
    if (MR::isBindedWall(this)) {
        HitSensor* pSensor = getSensor("Collision");
        MR::sendArbitraryMsg(0xF4, MR::getWallSensor(this), pSensor);
    }
    const char* pBck = MR::getPlayingBckName(this);
    if (MR::isEqualString(pBck, "MoveR")) {
        _F8 -= mRate;
        _FC += mRate;
        MR::repeat(_F8, 0.0f, MR::getBckFrameMax(this));
        MR::repeat(_FC, 0.0f, MR::getBckFrameMax(this));
    } else if (MR::isEqualString(pBck, "MoveL")) {
        _F8 += mRate;
        _FC -= mRate;
        MR::repeat(_F8, 0.0f, MR::getBckFrameMax(this));
        MR::repeat(_FC, 0.0f, MR::getBckFrameMax(this));
    } else if (MR::isEqualString(pBck, "MoveLR")) {
        _F8 += mRate;
        _FC += mRate;
        MR::repeat(_F8, 0.0f, MR::getBckFrameMax(this));
        MR::repeat(_FC, 0.0f, MR::getBckFrameMax(this));
    } else if (MR::isEqualString(pBck, "MoveBack")) {
        _F8 -= mRate;
        _FC -= mRate;
        MR::repeat(_F8, 0.0f, MR::getBckFrameMax(this));
        MR::repeat(_FC, 0.0f, MR::getBckFrameMax(this));
    }
    if (_F6) {
        if (++mGearStep > 500) {
            mGearStep = 0;
        }
        if (++mPropStep > 20) {
            mPropStep = 0;
        }
        if (++mWindStep > 20) {
            mWindStep = 0;
        }
        MR::startActionSound(this, "BodyGear", -1, -1, -1);
    }
}

void KoopaJrCastleBody::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::offBind(mHost);
        _F0 = -1600.0f;
    }
}

void KoopaJrCastleBody::exeFall() {
    MR::offBind(mHost);
    _F0 = MR::calcNerveValue(this, 120, -1600.0f, 0.0f);
}

void KoopaJrCastleBody::exeDead() {
    if (MR::isFirstStep(this)) {
        for (int i = 0; i < 2; i++) {
            MR::invalidateCollisionParts(mCollision[i]);
        }
    }
}

void KoopaJrCastleBody::setNerveDummy1() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDummy1::sInstance);
}

void KoopaJrCastleBody::setNerveDummy2() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDummy2::sInstance);
}

void KoopaJrCastleBody::setNerveFall() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvFall::sInstance);
}

void KoopaJrCastleBody::setNerveDead() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDead::sInstance);
}

void KoopaJrCastleBody::setNerveWait() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvWait::sInstance);
}

void KoopaJrCastleBody::setNerveStop() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvStop::sInstance);
}

void KoopaJrCastleBody::startMove0() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance);
    mState->setMode0();
}

void KoopaJrCastleBody::startMove1() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance);
    mState->setMode1();
}

void KoopaJrCastleBody::startMove2() {
    setNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance);
    mState->setMode2();
}

void KoopaJrCastleBody::attackSensor(HitSensor* pSender, HitSensor* pReceiver) {
    if (!isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDead::sInstance)) {
        if (MR::isSensorType(pSender, 0x36)) {
            if (MR::isSensorPlayerOrRide(pReceiver)) {
                MR::sendMsgPush(pReceiver, pSender);
            }
        } else {
            mHost->attackSensor(pSender, pReceiver);
        }
    }
}

bool KoopaJrCastleBody::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDead::sInstance)) {
        return false;
    }
    return mHost->receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

bool KoopaJrCastleBody::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvDead::sInstance)) {
        return false;
    }
    return mHost->receiveOtherMsg(msg, pSender, pReceiver);
}

void KoopaJrCastleBody::validateCollisionA() {
    if (!MR::isOnPlayer(getSensor("Collision"))) {
        MR::invalidateCollisionParts(mCollision[0]);
        MR::validateCollisionParts(mCollision[1]);
        mCollision[0]->resetAllMtx();
        mCollision[1]->resetAllMtx();
    }
}

void KoopaJrCastleBody::validateCollisionB() {
    if (!MR::isOnPlayer(getSensor("Collision"))) {
        MR::validateCollisionParts(mCollision[0]);
        MR::invalidateCollisionParts(mCollision[1]);
        mCollision[0]->resetAllMtx();
        mCollision[1]->resetAllMtx();
    }
}

void KoopaJrCastleBody::onFlagF4() {
    _F4 = true;
}

void KoopaJrCastleBody::offFlagF4() {
    _F4 = false;
}

void KoopaJrCastleBody::onFlagF5() {
    _F5 = true;
    MR::setBckRate(this, 1.0f);
}

void KoopaJrCastleBody::offFlagF5() {
    _F5 = false;
    MR::startAction(this, "Wait");
}

void KoopaJrCastleBody::onFlagF6() {
    _F6 = true;
}

void KoopaJrCastleBody::resetF0() {
    _F0 = 0.0f;
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleFunction (public accessors on the castle sub-actors)
// ---------------------------------------------------------------------------------------------
namespace KoopaJrCastleFunction {
#define CASTLE() (MR::getSceneObj< KoopaJrCastleAccessor* >(0x8B)->mCastle)

    KoopaJrCastle* getCastle() {
        return CASTLE();
    }

    s32 getBattlePhase() {
        return CASTLE()->mStateBattle->mPhase;
    }

    const TVec3f& getStartPos() {
        return CASTLE()->mStartPos;
    }

    bool isCapsuleSensorHost(const KoopaJrCastle* pCastle, const HitSensor* pSensor) {
        LiveActor* pCapsule = pCastle->mCapsule;
        return pCapsule == MR::getSensorHost(pSensor);
    }

    bool isPlayerOnCapsule() {
        return MR::isOnPlayer(CASTLE()->mCapsule);
    }

    bool isPlayerOnHead() {
        return MR::isOnPlayer(CASTLE()->mHead);
    }

    bool isPlayerOnCastle() {
        return isPlayerOnCapsule() || isPlayerOnHead();
    }

    void windFn_800D4290() {
        CASTLE()->mWind->fn_800D9B40();
    }

    bool windIsDead() {
        return MR::isDead(CASTLE()->mWind);
    }

    bool windFn_800D4310() {
        return CASTLE()->mWind->fn_800D9C40();
    }

    void bodyStartMove0() {
        CASTLE()->mBody->startMove0();
    }

    void bodyStartMove1() {
        CASTLE()->mBody->startMove1();
    }

    void bodyStartMove2() {
        CASTLE()->mBody->startMove2();
    }

    bool bodyIsNotMove() {
        return !CASTLE()->mBody->isMove();
    }

    void bodyTryStop() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!bodyIsNotMove()) {
            pCastle->mBody->setNerveStop();
        }
    }

    bool bodyIsMoveState() {
        return CASTLE()->mBody->isMoveState();
    }

    u8 bodyGetStateFlag() {
        return CASTLE()->mBody->mState->getFlag();
    }

    void cannonFn_800D4590() {
        CASTLE()->mCannon->fn_800D0540();
    }

    void cannonFn_800D45D0() {
        CASTLE()->mCannon->fn_800D05C0();
    }

    void cannonFn_800D4610() {
        CASTLE()->mCannon->fn_800D0580();
    }

    void killCannon() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mCannon)) {
            pCastle->mCannon->kill();
        }
    }

    bool isCannonDead() {
        return MR::isDead(CASTLE()->mCannon);
    }

    bool isHeadFn_800D4790() {
        return !CASTLE()->mHead->fn_800D58D0();
    }

    void appearFireBar() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->appear();
        }
    }

    void killFireBar() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->kill();
        }
    }

    void fireBarOff() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->_CC = 0;
        }
    }

    void fireBarOn() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->_CC = 1;
        }
    }

    void appearFireBarAndStart() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mFireBar)) {
            pCastle->mFireBar->appear();
        }
        pCastle->mFireBar->fn_800D2C50();
    }

    f32 getFireBarB0() {
        KoopaJrCastleFireBar* pFireBar = CASTLE()->mFireBar;
        if (pFireBar != nullptr) {
            return pFireBar->_B0;
        }
        return 0.0f;
    }

    void killRingBeam() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mRingBeam)) {
            pCastle->mRingBeam->kill();
        }
    }

    void appearElectric() {
        KoopaJrCastle* pCastle = CASTLE();
        if (MR::isDead(pCastle->mElectric)) {
            pCastle->mElectric->appear();
        }
    }

    void killElectric() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mElectric)) {
            pCastle->mElectric->kill();
        }
    }

    bool isElectricDead() {
        return MR::isDead(CASTLE()->mElectric);
    }

    bool isElectricActive() {
        KoopaJrCastle* pCastle = CASTLE();
        if (!MR::isDead(pCastle->mElectric) && pCastle->mElectric->fn_800D2250()) {
            return true;
        }
        return false;
    }
};  // namespace KoopaJrCastleFunction

bool KoopaJrCastleBody::isMove() const {
    return isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance);
}

bool KoopaJrCastleBody::isMoveState() const {
    bool ret = false;
    if (isNerve(&NrvKoopaJrCastleBody::KoopaJrCastleBodyNrvMove::sInstance) && mState->isMoving()) {
        ret = true;
    }
    return ret;
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleStateBodyMove (ActorStateBase<KoopaJrCastleBody>, 0x30)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMove(KoopaJrCastleBody* pHost) : ActorStateBase< KoopaJrCastleBody >("\x88\xda\x93\xae", pHost) {
    mDirection.x = 0.0f;
    mDirection.y = 0.0f;
    mDirection.z = 0.0f;
    _1C = 0.0f;
    mMode = 0;
    _24 = 1.0f;
    _2C = 0;
}

void KoopaJrCastleStateBodyMove::init() {
    initNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvSel::sInstance, 0);
    kill();
    mDirection = mHost->mPosition;
    mMode = 2;
}

void KoopaJrCastleStateBodyMove::appear() {
    mIsDead = false;
    setNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvSel::sInstance);
    mHost->validateCollisionA();
    KoopaJrCastleFunction::fn_800D4B40();
    _24 = 1.0f;
    mMoveStep = 0;
    _2C = 0;
}

void KoopaJrCastleStateBodyMove::setMode0() {
    mMode = 0;
}

void KoopaJrCastleStateBodyMove::setMode1() {
    mMode = 1;
}

void KoopaJrCastleStateBodyMove::setMode2() {
    mMode = 2;
}

void KoopaJrCastleStateBodyMove::kill() {
    MR::zeroVelocity(KoopaJrCastleFunction::getCastle());
    mIsDead = true;
    mHost->validateCollisionB();
    MR::setBckRate(mHost, 1.0f);
    MR::setBtkRate(mHost, 0.0f);
}

bool KoopaJrCastleStateBodyMove::isMoving() const {
    bool ret = false;
    if (isNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvSel::sInstance) ||
        isNerve(&NrvKoopaJrCastleStateBodyMove::KoopaJrCastleStateBodyMoveNrvTurnA::sInstance)) {
        ret = true;
    }
    return ret;
}

u8 KoopaJrCastleStateBodyMove::getFlag() const {
    return _2C;
}

void KoopaJrCastleStateBodyMove::setMoveRate(f32 rate) {
    mHost->mRate = rate;
    MR::setBckRate(mHost, rate);
    MR::setBtkRate(mHost, rate);
}

KoopaJrCastleStateBodyMove::~KoopaJrCastleStateBodyMove() {}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleCannon (LiveActor, 0xDC)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleCannon::KoopaJrCastleCannon(KoopaJrCastle* pHost, const char* pName)
    : LiveActor(pName), mHost(pHost), mShock(nullptr), _C8(1), _CC(0), _D0(0), _D4(false), _D8(0.0f) {
    for (int i = 0; i < 12; i++) {
        mBall[i] = nullptr;
    }
    MR::setPosition(this, pHost->mPosition);
}

void KoopaJrCastleCannon::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    mShock = new KoopaJrCastleCannonShock("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x92""e");
    mShock->initWithoutIter();
    for (int i = 0; i < 12; i++) {
        mBall[i] = new KoopaJrCastleCannonBall("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x92""e");
        MR::setScale(mBall[i], 1.5f);
        mBall[i]->initWithoutIter();
    }
    initNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvA::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void KoopaJrCastleCannon::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvA::sInstance);
    _CC = 0;
    _D4 = false;
    _D8 = 0.0f;
}

void KoopaJrCastleCannon::fn_800D0540() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvB::sInstance);
}

void KoopaJrCastleCannon::fn_800D0580() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvB::sInstance);
    _D4 = true;
}

void KoopaJrCastleCannon::fn_800D05C0() {
    KoopaJrCastleCannon::appear();
    setNerve(&NrvKoopaJrCastleCannon::KoopaJrCastleCannonNrvC::sInstance);
    _D4 = true;
}

void KoopaJrCastleCannon::makeActorDead() {
    LiveActor::makeActorDead();
    mHost->mHead->fn_800D57D0();
}

void KoopaJrCastleCannon::killAllBalls() {
    for (int i = 0; i < 12; i++) {
        if (!MR::isDead(mBall[i])) {
            mBall[i]->makeActorDead();
        }
    }
}

KoopaJrCastleCannonBall* KoopaJrCastleCannon::findDeadBall() {
    for (int i = 0; i < 12; i++) {
        if (MR::isDead(mBall[i])) {
            return mBall[i];
        }
    }
    return nullptr;
}

// NONMATCHING: MR::clamp is inlined here (target calls the out-of-line MR::clamp)
void KoopaJrCastleCannon::updateGauge() {
    _D8 = _D8 + 0.02f;
    _D8 = MR::clamp(_D8, 0.0f, 0.55f);
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleCannonShock (LiveActor, 0xA4)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleCannonShock::KoopaJrCastleCannonShock(const char* pName) : LiveActor(pName) {
    mDir.x = 0.0f;
    mDir.y = 0.0f;
    mDir.z = 0.0f;
    _9C = 0.0f;
    _A0 = 1.0f;
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleCannonBall (LiveActor, 0xA0)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleCannonBall::KoopaJrCastleCannonBall(const char* pName) : LiveActor(pName) {
    mDir.x = 0.0f;
    mDir.y = 0.0f;
    mDir.z = 0.0f;
    _9C = 0.0f;
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleCapsule (PartsModel, 0xA0)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleCapsule::KoopaJrCastleCapsule(LiveActor* pHost, const char* pName) : PartsModel(pHost, pName, "KoopaJrCastleCapsule", 0, 0x13, false) {}

bool KoopaJrCastleCapsule::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return mHost->receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

bool KoopaJrCastleCapsule::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    return mHost->receiveOtherMsg(msg, pSender, pReceiver);
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleElectric (LiveActor, 0x90)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleElectric::KoopaJrCastleElectric(const char* pName) : LiveActor(pName) {}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleFireBar (LiveActor, 0xD0)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleFireBar::KoopaJrCastleFireBar(const char* pName) : LiveActor(pName) {
    mBody = nullptr;
    _98.x = 0.0f;
    _98.y = 0.0f;
    _98.z = 0.0f;
    _A4 = 0.0f;
    _A8 = 0.0f;
    _AC = 1.0f;
    _B0 = 0.0f;
    _B4 = -0.5f;
    _B8 = 3;
    _BC = 1.0f;
    _C0 = 650.0f;
    _C4 = 0.0f;
    _C8 = 3000.0f;
    _CC = true;
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleHead (PartsModel, 0xD8)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleHead::KoopaJrCastleHead(LiveActor* pHost, const char* pName) : PartsModel(pHost, pName, "KoopaJrCastleHead", 0, 0x13, false) {
    _A0 = nullptr;
    _D4 = true;
    PSMTXIdentity(_A4);
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleDominate (LiveActor, 0xA8)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleDominate::KoopaJrCastleDominate(const char* pName) : LiveActor(pName) {
    _90 = nullptr;
    _94 = 0.0f;
    _98 = NAN;
    _9C = 0.0f;
    _A0 = 0.0f;
    _A4 = 0.0f;
}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleRingBeam (LiveActor, 0x98)
// ---------------------------------------------------------------------------------------------
KoopaJrCastleRingBeam::KoopaJrCastleRingBeam(const char* pName) : LiveActor(pName) {
    _90 = nullptr;
    _94 = 0.0f;
}

// ---------------------------------------------------------------------------------------------
// States
// ---------------------------------------------------------------------------------------------
KoopaJrCastleStateOpening::KoopaJrCastleStateOpening(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("Phase[\x83I\x81[\x83v\x83j\x83\x93\x83O]", pHost) {}

KoopaJrCastleStateBattle::KoopaJrCastleStateBattle(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("Phase[\x90\xed\x93\xac]", pHost) {
    mPhaseState[0] = nullptr;
    mPhaseState[1] = nullptr;
    mPhaseState[2] = nullptr;
    mPhaseState[3] = nullptr;
    mPhase = 3;
}

KoopaJrCastleStateBattle1::KoopaJrCastleStateBattle1(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82P\x92i\x8aK]", pHost) {}

KoopaJrCastleStateBattle2::KoopaJrCastleStateBattle2(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82Q\x92i\x8aK]", pHost) {}

KoopaJrCastleStateBattle3::KoopaJrCastleStateBattle3(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x8dU\x8c\x82:\x91\xe6\x82R\x92i\x8aK]", pHost) {
    _pad[0] = 0;
}

KoopaJrCastleStateBattle4::KoopaJrCastleStateBattle4(KoopaJrCastle* pHost) : ActorStateBase< KoopaJrCastle >("State[\x83_\x83\x81\x81[\x83W]", pHost) {}

KoopaJrCastleWind::KoopaJrCastleWind(KoopaJrCastle* pHost, const char* pName) : LiveActor(pName) {
    _pad[0] = 0;
}

// destructors (all identical)
KoopaJrCastleCannon::~KoopaJrCastleCannon() {}
KoopaJrCastleCannonShock::~KoopaJrCastleCannonShock() {}
KoopaJrCastleCannonBall::~KoopaJrCastleCannonBall() {}
KoopaJrCastleCapsule::~KoopaJrCastleCapsule() {}
KoopaJrCastleElectric::~KoopaJrCastleElectric() {}
KoopaJrCastleFireBar::~KoopaJrCastleFireBar() {}
KoopaJrCastleHead::~KoopaJrCastleHead() {}
KoopaJrCastleDominate::~KoopaJrCastleDominate() {}
KoopaJrCastleRingBeam::~KoopaJrCastleRingBeam() {}
KoopaJrCastleWind::~KoopaJrCastleWind() {}
KoopaJrCastleStateOpening::~KoopaJrCastleStateOpening() {}
KoopaJrCastleStateBattle::~KoopaJrCastleStateBattle() {}
KoopaJrCastleStateBattle1::~KoopaJrCastleStateBattle1() {}
KoopaJrCastleStateBattle2::~KoopaJrCastleStateBattle2() {}
KoopaJrCastleStateBattle3::~KoopaJrCastleStateBattle3() {}
KoopaJrCastleStateBattle4::~KoopaJrCastleStateBattle4() {}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleWindHolder
// ---------------------------------------------------------------------------------------------
KoopaJrCastleWindHolder::KoopaJrCastleWindHolder() : LiveActorGroup("\x83N\x83""b\x83pJr\x83L\x83\x83\x83""b\x83X\x83\x8b\x95\x97\x83z\x83\x8b\x83_\x81[", 0x20) {}

KoopaJrCastleWindHolder::~KoopaJrCastleWindHolder() {}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleWindUp
// ---------------------------------------------------------------------------------------------
KoopaJrCastleWindUp::KoopaJrCastleWindUp(const char* pName) : LiveActor(pName), mModel(nullptr), mLength(0.0f) {
    PSMTXIdentity(mMtx);
}

void KoopaJrCastleWindUp::makeActorAppeared() {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffect[i]->makeActorAppeared();
    }
    LiveActor::makeActorAppeared();
}

void KoopaJrCastleWindUp::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvA::sInstance);
}

void KoopaJrCastleWindUp::makeActorDead() {
    for (s32 i = 0; i < mEffectNum; i++) {
        mEffect[i]->makeActorDead();
    }
    LiveActor::makeActorDead();
}

bool KoopaJrCastleWindUp::receiveOtherMsg(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (msg == 0xF4) {
        setNerve(&NrvKoopaJrCastleWindUp::KoopaJrCastleWindUpNrvB::sInstance);
        return true;
    }
    return false;
}

KoopaJrCastleWindUp::~KoopaJrCastleWindUp() {}

// ---------------------------------------------------------------------------------------------
// KoopaJrCastleCapsule / Electric / Head small members
// ---------------------------------------------------------------------------------------------
void KoopaJrCastleCapsule::init(const JMapInfoIter& rIter) {
    MR::startBrk(this, "Wait");
    MR::startBva(this, "Normal");
    makeActorAppeared();
}

void KoopaJrCastleElectric::init(const JMapInfoIter& rIter) {
    MR::connectToScene(this, 0x29, -1, -1, -1);
    initNerve(&NrvKoopaJrCastleElectric::KoopaJrCastleElectricNrvB::sInstance, 0);
    MR::invalidateClipping(this);
    makeActorDead();
}

void KoopaJrCastleElectric::appear() {
    LiveActor::appear();
    setNerve(&NrvKoopaJrCastleElectric::KoopaJrCastleElectricNrvA::sInstance);
    KoopaJrCastleFunction::getCastle();
    KoopaJrCastleFunction::getCastle();
    KoopaJrCastleFunction::fn_800D4B40();
}

void KoopaJrCastleElectric::makeActorDead() {
    KoopaJrCastleFunction::getCastle();
    LiveActor* pHead = KoopaJrCastleFunction::getCastle()->mHead;
    MR::deleteEffect(pHead, "LeakSign");
    MR::deleteEffect(pHead, "Leak");
    LiveActor::makeActorDead();
}

bool KoopaJrCastleElectric::fn_800D2250() {
    if (!MR::isDead(this) && isNerve(&NrvKoopaJrCastleElectric::KoopaJrCastleElectricNrvB::sInstance)) {
        return true;
    }
    return false;
}

void KoopaJrCastleHead::setNerveA() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
}

void KoopaJrCastleHead::setNerveB() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance);
}

void KoopaJrCastleHead::setNerveC() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance);
}

void KoopaJrCastleHead::setNerveD() {
    setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvD::sInstance);
}

void KoopaJrCastleHead::fn_800D57D0() {
    if (!isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvB::sInstance) && !isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvC::sInstance)) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvA::sInstance);
    }
}

bool KoopaJrCastleHead::fn_800D5830() {
    if (fn_800D5E90()) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvE::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrCastleHead::fn_800D5880() {
    if (fn_800D5E90()) {
        setNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance);
        return true;
    }
    return false;
}

bool KoopaJrCastleHead::fn_800D58D0() const {
    bool ret = false;
    if (isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvE::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvF::sInstance) ||
        isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvG::sInstance) || isNerve(&NrvKoopaJrCastleHead::KoopaJrCastleHeadNrvH::sInstance)) {
        ret = true;
    }
    return ret;
}
