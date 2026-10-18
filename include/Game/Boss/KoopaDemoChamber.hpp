#pragma once

#include "Game/LiveActor/LiveActor.hpp"

// SMG2-only parts of the final Bowser battle (units outside the Koopa state family); only the interface used by
// the battle helpers is declared here.
class KoopaDemoChamber : public LiveActor {
public:
    void startBreak();
    void getDemoEndPlayerPos(MtxPtr) const;
};

class KoopaBattlePlanet : public LiveActor {
public:
};

class KoopaFinalStarPlanet : public LiveActor {
public:
};
