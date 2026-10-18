#include "Game/Enemy/KameckBeamHolder.hpp"
#include "Game/Enemy/KuriboMini.hpp"
#include "Game/Enemy/Meramera.hpp"
#include "Game/Util.hpp"

KameckBeamHolder::KameckBeamHolder(s32 kind, s32 count, s32 projectileCount)
    : DeriveActorGroup<KameckBeam>("カメックビーム管理", count) {
    for (s32 i = 0; i < count; ++i) {
        KameckBeam* beam = new KameckBeam("カメックビーム", kind, projectileCount);
        beam->initWithoutIter();
        registerActor(beam);
    }
}

KameckBeam* KameckBeamHolder::fn_801A7CF0(f32 scale, MtxPtr matrix, const TVec3f& position, KameckBeamEventListener* listener) {
    KameckBeam* beam = getDeadMember();
    if (beam == nullptr) {
        return nullptr;
    }
    if (!beam->requestFollowWand(matrix, scale)) {
        return nullptr;
    }
    beam->setWandLocalPosition(position);
    beam->setEventListener(listener);
    return beam;
}

KameckBeamHolder::~KameckBeamHolder() {}

KameckBeamProjectile::KameckBeamProjectile() : mEventListener(nullptr) {}

void KameckBeamProjectile::fn_801A7E30(KameckBeamEventListener* listener) {
    mEventListener = listener;
}

void KameckBeamProjectile::fn_801A7E40() {
    if (mEventListener != nullptr) {
        mEventListener->hitBeam();
    }
}

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
            fn_801A7180();
        }
    }
}

void KameckBeamKuriboMini::appearDirection(const TVec3f& position, const TVec3f& direction) {
    appear();
    mEnemy->fn_801BDB60(position, direction);
    mStep = 600;
}

void KameckBeamKuriboMini::fn_801A7180() {
    kill();
    mEnemy->makeActorDead();
}

bool KameckBeamKuriboMini::fn_801A78B0() const {
    return MR::isDead(mEnemy);
}

KameckBeamKuriboMini::~KameckBeamKuriboMini() {}

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
            fn_801A7180();
        }
    }
}

void KameckBeamMeramera::appearDirection(const TVec3f& position, const TVec3f& direction) {
    appear();
    mEnemy->appearAtPos(position, direction);
    mStep = 600;
}

void KameckBeamMeramera::fn_801A7180() {
    kill();
    mEnemy->makeActorDead();
}

bool KameckBeamMeramera::fn_801A78B0() const {
    return MR::isDead(mEnemy);
}

KameckBeamMeramera::~KameckBeamMeramera() {}
