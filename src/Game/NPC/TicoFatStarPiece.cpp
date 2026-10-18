#pragma auto_inline off
#include "Game/NPC/TicoFatStarPiece.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
extern "C" {
extern s32 lbl_807D4C58;
extern char lbl_806BE848[], lbl_806BE858[];
extern const char* lbl_807CFF50;
}
TicoFatStarPiece::TicoFatStarPiece(const char* name) : NPCActor(name), _164(0), _168(0), mStarPieceCount(0), mEatenStarPieceCount(0), _174(0), _178(0), _17C(0), mEventRequested(false), _184(-1), _188(0), mEatSoundTimer(0), _190(0) {}
TicoFatStarPiece::~TicoFatStarPiece() {}
bool TicoFatStarPiece::eventFunc(u32) {
    mEventRequested = true;
    setNerve(reinterpret_cast<Nerve*>(&lbl_807D4C58));
    return true;
}
s32 TicoFatStarPiece::calcEatSoundPitch() {
    s32 divisor = 50;
    s32 pitch = (mStarPieceCount - mEatenStarPieceCount - 1) / divisor;
    if (pitch < 0) pitch = 0;
    if (pitch > 32) pitch = 32;
    return pitch;
}
void TicoFatStarPiece::startEatSound() {
    if (mEatSoundTimer <= 0) {
        MR::startActionSound(this, lbl_806BE848, -1, -1, -1);
        mEatSoundTimer = 5;
    }
}
void TicoFatStarPiece::startEatCompleteSound() {
    s32 pitch = calcEatSoundPitch();
    MR::startActionSound(this, lbl_806BE858, pitch, -1, -1);
}
bool TicoFatStarPiece::tryEndEventCamera() {
    if (MR::isGlobalEventCameraActive(lbl_807CFF50)) {
        MR::endGlobalEventCamera(lbl_807CFF50, 120, true);
        return true;
    }
    return false;
}
