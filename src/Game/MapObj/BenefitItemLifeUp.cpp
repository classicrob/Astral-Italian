#include "Game/MapObj/BenefitItemLifeUp.hpp"
#include "Game/NPC/ScoreAttackCounter.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

BenefitItemLifeUp::BenefitItemLifeUp(const char* pName) : BenefitItemObj(pName, "KinokoLifeUp") {
    if (MR::isGalaxyDarkCometAppearInCurrentStage()) {
        _DC = 1;
    }
}

void BenefitItemLifeUp::exeCatch() {
    MR::changePlayerItemLifeUp();
    ScoreAttackFunction::score1000Points(this);
}

