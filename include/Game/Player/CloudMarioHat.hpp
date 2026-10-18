#pragma once

#include "Game/LiveActor/LiveActor.hpp"

/// @brief The cloud mario cap actor (allocated with size 0xD4 by MarioActor::initCloudMario).
class CloudMarioHat : public LiveActor {
public:
    CloudMarioHat(const char*);

    /* 0x90 */ u8 _90;
    /* 0x91 */ u8 _91;
    /* 0x92 */ u8 _92[0x42];
};
