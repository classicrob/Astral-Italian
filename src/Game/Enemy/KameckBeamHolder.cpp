#include "Game/Enemy/KameckBeamHolder.hpp"
#include "Game/Util.hpp"

KameckBeamHolder::KameckBeamHolder(s32 kind, s32 count, s32 projectileCount)
    : DeriveActorGroup<KameckBeam>("カメックビーム管理", count) {
    for (s32 i = 0; i < count; ++i) {
        KameckBeam* beam = new KameckBeam("カメックビーム", kind, projectileCount);
        beam->initWithoutIter();
        registerActor(beam);
    }
}

KameckBeam* KameckBeamHolder::requestBeamFromWand(f32 scale, MtxPtr matrix, const TVec3f& position, KameckBeamEventListener* listener) {
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

void KameckBeamProjectile::setProjectileEventListener(KameckBeamEventListener* listener) {
    mEventListener = listener;
}

void KameckBeamProjectile::notifyBeamHit() {
    if (mEventListener != nullptr) {
        mEventListener->hitBeam();
    }
}

