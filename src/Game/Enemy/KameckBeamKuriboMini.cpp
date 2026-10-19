#include "Game/Enemy/KameckBeamHolder.hpp"
#include "Game/Enemy/KuriboMini.hpp"
#include "Game/Util.hpp"

KameckBeamKuriboMini::KameckBeamKuriboMini()
    : LiveActor("マメクリビーム"), KameckBeamProjectile(), mEnemy(nullptr), mStep(0) {
    MR::connectToSceneEnemyDecorationMovement(this);
    mEnemy = new KuriboMini("マメクリ[ビーム用]");
    mEnemy->initWithoutIter();
    mEnemy->makeActorDead();
    MR::invalidateClipping(this);
    initEffectKeeper(0, "BossKameckBeam", false);
    makeActorDead();
}

void KameckBeamKuriboMini::control() {
    if (mStep > 0) {
        --mStep;
        if (mStep == 0) {
            MR::emitEffectHit(this, mEnemy->mPosition, "BeamEnemyVanish");
            resetBeamProjectile();
        }
    }
}

void KameckBeamKuriboMini::appearDirection(const TVec3f& position, const TVec3f& direction) {
    appear();
    mEnemy->resetAndAppearAt(position, direction);
    mStep = 600;
}

void KameckBeamKuriboMini::resetBeamProjectile() {
    kill();
    mEnemy->makeActorDead();
}

bool KameckBeamKuriboMini::isBeamProjectileDead() const {
    return MR::isDead(mEnemy);
}

KameckBeamKuriboMini::~KameckBeamKuriboMini() {}

