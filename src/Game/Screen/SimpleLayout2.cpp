#include "Game/Screen/SimpleLayout.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util.hpp"

SimpleFollowActorLayout::SimpleFollowActorLayout(const LiveActor* actor, const char* name, const char* layout, const TVec3f& offset, u32 count, int drawType)
    : SimpleLayout(name, layout, count, drawType), mFollowActor(actor), mFollowOffset(offset) {}

SimpleLayout::~SimpleLayout() {}

// NONMATCHING: local vector address register allocation and paired-single load scheduling.
void SimpleFollowActorLayout::control() {
    if (isFollowActorHidden()) {
        MR::hideLayout(this);
    } else {
        TVec3f position(mFollowActor->mPosition);
        position.add(mFollowOffset);
        TVec2f screen;
        MR::calcScreenPosition(&screen, position);
        setTrans(screen);
        MR::showLayout(this);
    }
}

bool SimpleFollowActorLayout::isFollowActorHidden() const {
    if (MR::isDead(mFollowActor)) return true;
    if (MR::isClipped(mFollowActor)) return true;
    return MR::isNoEntryDrawBuffer(mFollowActor);
}

SimpleFollowActorLayout::~SimpleFollowActorLayout() {}
