#include "Game/Enemy/KameckBeamHolder.hpp"
#include "Game/Enemy/Meramera.hpp"
#include "Game/Util.hpp"

KameckBeamMeramera::KameckBeamMeramera()
    : LiveActor("メラメラビーム"), KameckBeamProjectile(), mEnemy(nullptr), mStep(0) {
    MR::connectToSceneEnemyDecorationMovement(this);
    mEnemy = new Meramera("メラメラ[ビーム用]");
    mEnemy->initWithoutIter();
    mEnemy->makeActorDead();
    MR::invalidateClipping(this);
    initEffectKeeper(0, "BossKameckBeam", false);
    makeActorDead();
}

void KameckBeamMeramera::control() {
    if (mStep > 0) {
        --mStep;
        if (mStep == 0) {
            MR::emitEffectHit(this, mEnemy->mPosition, "BeamEnemyVanish");
            resetBeamProjectile();
        }
    }
}

void KameckBeamMeramera::appearDirection(const TVec3f& position, const TVec3f& direction) {
    appear();
    mEnemy->appearAtPos(position, direction);
    mStep = 600;
}

void KameckBeamMeramera::resetBeamProjectile() {
    kill();
    mEnemy->makeActorDead();
}

bool KameckBeamMeramera::isBeamProjectileDead() const {
    return MR::isDead(mEnemy);
}

KameckBeamMeramera::~KameckBeamMeramera() {}
