#pragma auto_inline off

#include "Game/MapObj/LeafBoard.hpp"
#include "Game/Util.hpp"

// Float constants (.sdata2) used by the LeafBoard constructor.
extern "C" f32 lbl_807DDAA8;
extern "C" f32 lbl_807DDAAC;

LeafBoard::LeafBoard(const char* pName) : LiveActor(pName) {
    mRot.x = lbl_807DDAAC;
    mRot.y = lbl_807DDAAC;
    mRot.z = lbl_807DDAAC;
    mRot.w = lbl_807DDAA8;
    mDir.x = lbl_807DDAAC;
    mDir.y = lbl_807DDAA8;
    mDir.z = lbl_807DDAAC;
    mVelocityDC.x = lbl_807DDAAC;
    mVelocityDC.y = lbl_807DDAAC;
    mVelocityDC.z = lbl_807DDAAC;
    mE8.x = lbl_807DDAAC;
    mE8.y = lbl_807DDAAC;
    mE8.z = lbl_807DDAAC;
    mF4.x = lbl_807DDAAC;
    mF4.y = lbl_807DDAA8;
    mF4.z = lbl_807DDAAC;
    mF100 = lbl_807DDAAC;
    mTimer104 = 0;
    mFlag108 = false;
    mMtx90.identity();
}

LeafBoard::~LeafBoard() {
}

void LeafBoard::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mRot);
}
