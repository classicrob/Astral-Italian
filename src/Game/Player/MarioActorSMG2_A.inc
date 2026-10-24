#include "Game/Util/PlayerUtil.hpp"
// Retained helper and legacy definitions; SMG2 MarioActor methods are ordered in MarioActor.cpp.


void DLchanger::addDL(J3DModelX* m) {
    m->setDynamicDL(mBuffers[mCurrentBuffer].mDL, mBuffers[mCurrentBuffer].mSize);
}


bool MarioState::passRing(const HitSensor*) {
    return false;
}


void MultiEmitter::setGlobalScale(f32 scale, s32 n) {
    TVec3f v(scale, scale, scale);
    this->setGlobalScale(v, n);
}


void J3DModelX::swapDrawBuffer(u32 a) {
    if (this->_DC == a) {
        this->_DC = 0;
        return;
    }
    this->_DC = (u8)a;
}


void J3DModelX::setDynamicDL(u8* dl, u32 size) {
    if (dl == nullptr) {
        size = 0;
    }
    _1B8 = dl;
    _1BC = size;
}


