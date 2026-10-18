#include "Game/Enemy/Grapyon.hpp"
#include "Game/Util.hpp"

// Only the constructor and destructor are ported so far.
Grapyon::Grapyon(const char* pName) : LiveActor(pName), mUnk90(nullptr), mUnkC4(nullptr), mUnkC8(nullptr), mVec(0.0f, 0.0f, 0.0f), mUnkFC(false) {
    mMtx.identity();
}

Grapyon::~Grapyon() {}
