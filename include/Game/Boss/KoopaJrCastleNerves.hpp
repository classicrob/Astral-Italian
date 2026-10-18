#pragma once

#include "Game/Boss/KoopaJrCastle.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"

// Nerve class declarations shared by the KoopaJrCastle translation units.
// Each TU defines (and therefore constructs in its own __sinit) only the instances it owns.
#define NEW_NERVE_DECL(name, parent_class, executor_name)                                                                                           \
    class name : public Nerve {                                                                                                                      \
    public:                                                                                                                                          \
        virtual void execute(Spine* pSpine) const {                                                                                                  \
            parent_class* actor = reinterpret_cast< parent_class* >(pSpine->mExecutor);                                                              \
            actor->exe##executor_name();                                                                                                             \
        };                                                                                                                                           \
        static name sInstance;                                                                                                                       \
    }

extern const volatile s32 sKoopaJrCastleBattle3WaitStep;
namespace KoopaJrCastleFunction {
    bool fn_800D3540(const char*, const char*);
};
using KoopaJrCastleFunction::fn_800D3540;

namespace NrvKoopaJrCastle {
    class KoopaJrCastleNrvOpening : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KoopaJrCastleNrvOpening sInstance;
    };

    NEW_NERVE_DECL(KoopaJrCastleNrvBattle, KoopaJrCastle, Battle);

    inline void KoopaJrCastleNrvOpening::execute(Spine* pSpine) const {
        LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
        MR::updateActorStateAndNextNerve(actor, &KoopaJrCastleNrvBattle::sInstance);
    }
};  // namespace NrvKoopaJrCastle


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
            if (MR::isGreaterStep(a, sKoopaJrCastleBattle3WaitStep)) {
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
        virtual void execute(Spine* pSpine) const;
        static KoopaJrCastleStateBattle4NrvB sInstance;
    };
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
}  // namespace NrvKoopaJrCastleWindUp


namespace NrvKoopaJrCastleBody {
    NEW_NERVE_DECL(KoopaJrCastleBodyNrvWait, KoopaJrCastleBody, Wait);
    class KoopaJrCastleBodyNrvStop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleBody* a = reinterpret_cast< KoopaJrCastleBody* >(pSpine->mExecutor);
            if (MR::isFirstStep(a)) {
                a->_F0 = 0.0f;
            }
        }
        static KoopaJrCastleBodyNrvStop sInstance;
    };
    class KoopaJrCastleBodyNrvMove : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            LiveActor* actor = reinterpret_cast< LiveActor* >(pSpine->mExecutor);
            MR::updateActorStateAndNextNerve(actor, &KoopaJrCastleBodyNrvStop::sInstance);
        };
        static KoopaJrCastleBodyNrvMove sInstance;
    };
    class KoopaJrCastleBodyNrvDummy1 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static KoopaJrCastleBodyNrvDummy1 sInstance;
    };
    class KoopaJrCastleBodyNrvDummy2 : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static KoopaJrCastleBodyNrvDummy2 sInstance;
    };
    NEW_NERVE_DECL(KoopaJrCastleBodyNrvFall, KoopaJrCastleBody, Fall);
    NEW_NERVE_DECL(KoopaJrCastleBodyNrvDead, KoopaJrCastleBody, Dead);
};  // namespace NrvKoopaJrCastleBody
