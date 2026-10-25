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
            a->exeSelectMove();
        }
        static KoopaJrCastleStateBodyMoveNrvSel sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvTurnA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exeTurnToPlayer();
        }
        static KoopaJrCastleStateBodyMoveNrvTurnA sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvTurnB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exeMoveStart();
        }
        static KoopaJrCastleStateBodyMoveNrvTurnB sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvMoveA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exeMoveStraight();
        }
        static KoopaJrCastleStateBodyMoveNrvMoveA sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvRotA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exeTurnAroundCenter();
        }
        static KoopaJrCastleStateBodyMoveNrvRotA sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvRotB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exeMoveAroundCenter();
        }
        static KoopaJrCastleStateBodyMoveNrvRotB sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvRotC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exeMoveBackAroundCenter();
        }
        static KoopaJrCastleStateBodyMoveNrvRotC sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvMoveB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exeMoveChase();
        }
        static KoopaJrCastleStateBodyMoveNrvMoveB sInstance;
    };
    class KoopaJrCastleStateBodyMoveNrvStop : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBodyMove* a = reinterpret_cast< KoopaJrCastleStateBodyMove* >(pSpine->mExecutor);
            a->exeStop();
        }
        static KoopaJrCastleStateBodyMoveNrvStop sInstance;
    };
}  // namespace NrvKoopaJrCastleStateBodyMove

namespace NrvKoopaJrCastleCannon {
    class KoopaJrCastleCannonNrvPre : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const;
        static KoopaJrCastleCannonNrvPre sInstance;
    };
    class KoopaJrCastleCannonNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exeAim();
        }
        static KoopaJrCastleCannonNrvA sInstance;
    };
    inline void KoopaJrCastleCannonNrvPre::execute(Spine* pSpine) const {
        KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
        if (MR::isGreaterStep(a, 60)) {
            a->setNerve(&KoopaJrCastleCannonNrvA::sInstance);
        }
    }
    class KoopaJrCastleCannonNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exeAimContinuous();
        }
        static KoopaJrCastleCannonNrvB sInstance;
    };
    class KoopaJrCastleCannonNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exeShoot();
        }
        static KoopaJrCastleCannonNrvC sInstance;
    };
    class KoopaJrCastleCannonNrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exeWaitAfterShoot();
        }
        static KoopaJrCastleCannonNrvD sInstance;
    };
    class KoopaJrCastleCannonNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannon* a = reinterpret_cast< KoopaJrCastleCannon* >(pSpine->mExecutor);
            a->exeShootShock();
        }
        static KoopaJrCastleCannonNrvE sInstance;
    };
}  // namespace NrvKoopaJrCastleCannon

namespace NrvKoopaJrCastleCannonShock {
    class KoopaJrCastleCannonShockNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannonShock* a = reinterpret_cast< KoopaJrCastleCannonShock* >(pSpine->mExecutor);
            a->exeGrow();
        }
        static KoopaJrCastleCannonShockNrvA sInstance;
    };
    class KoopaJrCastleCannonShockNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannonShock* a = reinterpret_cast< KoopaJrCastleCannonShock* >(pSpine->mExecutor);
            a->exeFly();
        }
        static KoopaJrCastleCannonShockNrvB sInstance;
    };
    class KoopaJrCastleCannonShockNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannonShock* a = reinterpret_cast< KoopaJrCastleCannonShock* >(pSpine->mExecutor);
            a->exeShrink();
        }
        static KoopaJrCastleCannonShockNrvC sInstance;
    };
}  // namespace NrvKoopaJrCastleCannonShock

namespace NrvKoopaJrCastleCannonBall {
    class KoopaJrCastleCannonBallNrvMove : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleCannonBall* a = reinterpret_cast< KoopaJrCastleCannonBall* >(pSpine->mExecutor);
            a->exeMove();
        }
        static KoopaJrCastleCannonBallNrvMove sInstance;
    };
}  // namespace NrvKoopaJrCastleCannonBall

namespace NrvKoopaJrCastleElectric {
    class KoopaJrCastleElectricNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleElectric* a = reinterpret_cast< KoopaJrCastleElectric* >(pSpine->mExecutor);
            a->exeDischargeSign();
        }
        static KoopaJrCastleElectricNrvA sInstance;
    };
    class KoopaJrCastleElectricNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleElectric* a = reinterpret_cast< KoopaJrCastleElectric* >(pSpine->mExecutor);
            a->exeDischarge();
        }
        static KoopaJrCastleElectricNrvB sInstance;
    };
}  // namespace NrvKoopaJrCastleElectric

namespace NrvKoopaJrCastleFireBar {
    class KoopaJrCastleFireBarNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleFireBar* a = reinterpret_cast< KoopaJrCastleFireBar* >(pSpine->mExecutor);
            a->exeExtend();
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
            a->exeScaleUp();
        }
        static KoopaJrCastleFireBarNrvC sInstance;
    };
}  // namespace NrvKoopaJrCastleFireBar

namespace NrvKoopaJrCastleHead {
    class KoopaJrCastleHeadNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exeWait();
        }
        static KoopaJrCastleHeadNrvA sInstance;
    };
    class KoopaJrCastleHeadNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exeFirstDamage();
        }
        static KoopaJrCastleHeadNrvB sInstance;
    };
    class KoopaJrCastleHeadNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exeSecondDamage();
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
            a->exePanic();
        }
        static KoopaJrCastleHeadNrvE sInstance;
    };
    class KoopaJrCastleHeadNrvF : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exeSpinSign();
        }
        static KoopaJrCastleHeadNrvF sInstance;
    };
    class KoopaJrCastleHeadNrvG : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exeSpinSign();
        }
        static KoopaJrCastleHeadNrvG sInstance;
    };
    class KoopaJrCastleHeadNrvH : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleHead* a = reinterpret_cast< KoopaJrCastleHead* >(pSpine->mExecutor);
            a->exeSpin();
        }
        static KoopaJrCastleHeadNrvH sInstance;
    };
}  // namespace NrvKoopaJrCastleHead

namespace NrvKoopaJrCastleStateBattle {
    class KoopaJrCastleStateBattleNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exeBattle1();
        }
        static KoopaJrCastleStateBattleNrvA sInstance;
    };
    class KoopaJrCastleStateBattleNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exeBattle2();
        }
        static KoopaJrCastleStateBattleNrvB sInstance;
    };
    class KoopaJrCastleStateBattleNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exeBattle3();
        }
        static KoopaJrCastleStateBattleNrvC sInstance;
    };
    class KoopaJrCastleStateBattleNrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exeBattle4();
        }
        static KoopaJrCastleStateBattleNrvD sInstance;
    };
    class KoopaJrCastleStateBattleNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle* a = reinterpret_cast< KoopaJrCastleStateBattle* >(pSpine->mExecutor);
            a->exePowerUpDemo();
        }
        static KoopaJrCastleStateBattleNrvE sInstance;
    };
}  // namespace NrvKoopaJrCastleStateBattle

namespace NrvKoopaJrCastleStateOpening {
    class KoopaJrCastleStateOpeningNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateOpening* a = reinterpret_cast< KoopaJrCastleStateOpening* >(pSpine->mExecutor);
            a->exeWaitSwitch();
        }
        static KoopaJrCastleStateOpeningNrvA sInstance;
    };
    class KoopaJrCastleStateOpeningNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateOpening* a = reinterpret_cast< KoopaJrCastleStateOpening* >(pSpine->mExecutor);
            a->exeStartOpeningDemo();
        }
        static KoopaJrCastleStateOpeningNrvB sInstance;
    };
    class KoopaJrCastleStateOpeningNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateOpening* a = reinterpret_cast< KoopaJrCastleStateOpening* >(pSpine->mExecutor);
            a->exeOpeningDemo();
        }
        static KoopaJrCastleStateOpeningNrvC sInstance;
    };
}  // namespace NrvKoopaJrCastleStateOpening

namespace NrvKoopaJrCastleDominate {
    class KoopaJrCastleDominateNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleDominate* a = reinterpret_cast< KoopaJrCastleDominate* >(pSpine->mExecutor);
            a->exeBindPlayerFall();
        }
        static KoopaJrCastleDominateNrvA sInstance;
    };
}  // namespace NrvKoopaJrCastleDominate

namespace NrvKoopaJrCastleRingBeam {
    class KoopaJrCastleRingBeamNrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleRingBeam* a = reinterpret_cast< KoopaJrCastleRingBeam* >(pSpine->mExecutor);
            a->exeShoot();
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
            a->exeCannonVolley();
        }
        static KoopaJrCastleStateBattle1NrvA sInstance;
    };
    class KoopaJrCastleStateBattle1NrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle1* a = reinterpret_cast< KoopaJrCastleStateBattle1* >(pSpine->mExecutor);
            a->exeCannonBallVolley();
        }
        static KoopaJrCastleStateBattle1NrvB sInstance;
    };
    class KoopaJrCastleStateBattle1NrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle1* a = reinterpret_cast< KoopaJrCastleStateBattle1* >(pSpine->mExecutor);
            a->exeHeadPanic();
        }
        static KoopaJrCastleStateBattle1NrvC sInstance;
    };
}  // namespace NrvKoopaJrCastleStateBattle1

namespace NrvKoopaJrCastleStateBattle2 {
    class KoopaJrCastleStateBattle2NrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exeCannonVolley();
        }
        static KoopaJrCastleStateBattle2NrvA sInstance;
    };
    class KoopaJrCastleStateBattle2NrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exeCannonBallVolley();
        }
        static KoopaJrCastleStateBattle2NrvB sInstance;
    };
    class KoopaJrCastleStateBattle2NrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exeWindAndElectric();
        }
        static KoopaJrCastleStateBattle2NrvC sInstance;
    };
    class KoopaJrCastleStateBattle2NrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exeWait();
        }
        static KoopaJrCastleStateBattle2NrvD sInstance;
    };
    class KoopaJrCastleStateBattle2NrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exeHeadPanic();
        }
        static KoopaJrCastleStateBattle2NrvE sInstance;
    };
    class KoopaJrCastleStateBattle2NrvF : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle2* a = reinterpret_cast< KoopaJrCastleStateBattle2* >(pSpine->mExecutor);
            a->exeHeadSpinSign();
        }
        static KoopaJrCastleStateBattle2NrvF sInstance;
    };
}  // namespace NrvKoopaJrCastleStateBattle2

namespace NrvKoopaJrCastleStateBattle3 {
    class KoopaJrCastleStateBattle3NrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeCannonBallVolleyBeforeMove();
        }
        static KoopaJrCastleStateBattle3NrvA sInstance;
    };
    class KoopaJrCastleStateBattle3NrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeCannonBallVolleyBeforeWind();
        }
        static KoopaJrCastleStateBattle3NrvB sInstance;
    };
    class KoopaJrCastleStateBattle3NrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeCannonBallVolleyBeforeChase();
        }
        static KoopaJrCastleStateBattle3NrvC sInstance;
    };
    class KoopaJrCastleStateBattle3NrvD : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeMoveStraight();
        }
        static KoopaJrCastleStateBattle3NrvD sInstance;
    };
    class KoopaJrCastleStateBattle3NrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeMoveChase();
        }
        static KoopaJrCastleStateBattle3NrvE sInstance;
    };
    class KoopaJrCastleStateBattle3NrvF : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeMoveAroundCenter();
        }
        static KoopaJrCastleStateBattle3NrvF sInstance;
    };
    class KoopaJrCastleStateBattle3NrvG : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeWindAndElectric();
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
            a->exeCannonVolley();
        }
        static KoopaJrCastleStateBattle3NrvI sInstance;
    };
    class KoopaJrCastleStateBattle3NrvJ : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeShockSign();
        }
        static KoopaJrCastleStateBattle3NrvJ sInstance;
    };
    class KoopaJrCastleStateBattle3NrvK : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeShockVolley();
        }
        static KoopaJrCastleStateBattle3NrvK sInstance;
    };
    class KoopaJrCastleStateBattle3NrvL : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeSelectAttack();
        }
        static KoopaJrCastleStateBattle3NrvL sInstance;
    };
    class KoopaJrCastleStateBattle3NrvM : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeHeadPanic();
        }
        static KoopaJrCastleStateBattle3NrvM sInstance;
    };
    class KoopaJrCastleStateBattle3NrvN : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle3* a = reinterpret_cast< KoopaJrCastleStateBattle3* >(pSpine->mExecutor);
            a->exeHeadSpinSign();
        }
        static KoopaJrCastleStateBattle3NrvN sInstance;
    };
}  // namespace NrvKoopaJrCastleStateBattle3

namespace NrvKoopaJrCastleStateBattle4 {
    class KoopaJrCastleStateBattle4NrvA : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle4* a = reinterpret_cast< KoopaJrCastleStateBattle4* >(pSpine->mExecutor);
            a->exePushPlayerOff();
        }
        static KoopaJrCastleStateBattle4NrvA sInstance;
    };
    class KoopaJrCastleStateBattle4NrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleStateBattle4* a = reinterpret_cast< KoopaJrCastleStateBattle4* >(pSpine->mExecutor);
            a->exeDownDemo();
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
            a->exeBlowStart();
        }
        static KoopaJrCastleWindNrvA sInstance;
    };
    class KoopaJrCastleWindNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWind* a = reinterpret_cast< KoopaJrCastleWind* >(pSpine->mExecutor);
            a->exeVacuum();
        }
        static KoopaJrCastleWindNrvB sInstance;
    };
    class KoopaJrCastleWindNrvC : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWind* a = reinterpret_cast< KoopaJrCastleWind* >(pSpine->mExecutor);
            a->exeBlowEnd();
        }
        static KoopaJrCastleWindNrvC sInstance;
    };
    class KoopaJrCastleWindNrvE : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWind* a = reinterpret_cast< KoopaJrCastleWind* >(pSpine->mExecutor);
            a->exeBlow();
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
            a->exeBlow();
        }
        static KoopaJrCastleWindUpNrvA sInstance;
    };
    class KoopaJrCastleWindUpNrvB : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {
            KoopaJrCastleWindUp* a = reinterpret_cast< KoopaJrCastleWindUp* >(pSpine->mExecutor);
            a->exeStop();
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
                a->mVerticalOffset = 0.0f;
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
