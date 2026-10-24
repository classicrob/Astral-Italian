#include "Game/Player/MarioHang.hpp"
#include "Game/Util.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include <cstring>

// native: defined in Mario.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
void MarioHang_FORCE_MATCH_SDATA2() {
    (void)-0.8f;
    (void)50.0f;
    (void)5.0f;
    (void)60.0f;
    (void)160.0f;
    (void)0.1f;
    (void)0.08726647f;
    (void)100.0f;
    (void)150.0f;
    (void)0.2f;
    (void)30.0f;
    (void)-0.707f;
    (void)84.0f;
    (void)-20.0f;
    (void)-100.0f;
    (void)0.707f;
    (void)45.0f;
    (void)80.0f;
    (void)6.0f;
    (void)0.95f;
    (void)20.0f;
    (void)15.0f;
    (void)55.0f;
    (void)-55.0f;
    (void)-15.0f;
    (void)-150.0f;
    (void)0.001f;
    (void)0.01f;
}
#endif

// native: defined in Mario.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
bool Mario::isHanging() const {
    return getCurrentStatus() == MarioStatus_Hang;
}
#endif

void MarioHang::recordWallPolygon(const Triangle* pTriangle) {
    Triangle* triangle = _20;
    triangle->mParts = pTriangle->mParts;
    triangle->mIdx = pTriangle->mIdx;
    triangle->mSensor = pTriangle->mSensor;
    triangle->mNormals[0] = pTriangle->mNormals[0];
    triangle->mNormals[1] = pTriangle->mNormals[1];
    triangle->mNormals[2] = pTriangle->mNormals[2];
    triangle->mNormals[3] = pTriangle->mNormals[3];
    triangle->mPos[0] = pTriangle->mPos[0];
    triangle->mPos[1] = pTriangle->mPos[1];
    triangle->mPos[2] = pTriangle->mPos[2];
    mWallSensor = pTriangle->mSensor;
    TPos3f matrix;
    MR::makeMtxWithoutScale(&matrix, *_20->getBaseInvMtx());
    PSMTXMultVecSR(matrix.toMtxPtr(), _20->getNormal(0), &_28);
    _34 = *_20->getNormal(0);
}

void MarioHang::recordHangNorm(const TVec3f& rNormal) {
    _34 = rNormal;
    TPos3f matrix;
    MR::makeMtxWithoutScale(&matrix, *_20->getBaseInvMtx());
    PSMTXMultVecSR(matrix.toMtxPtr(), &rNormal, &_28);
}

void MarioHang::forceDrop() {
    _1C = 1;
}

MarioHang::MarioHang(MarioActor* pActor) : MarioState(pActor, MarioStatus_Hang) {
    _12 = 0;
    _14 = 0;
    _16 = 0;
    mHangTimer = 0;
    _1A = 0;
    _1B = 0;
    _1C = 0;
    _1D = 0;
    _1E = 0;
    _28.zero();
    _34.zero();
    mWallSensor = nullptr;
    _20 = new Triangle;
    _24 = new Triangle;
}

bool MarioHang::start() {
    getPlayer()->cancelSquatMode();
    stopAnimationUpper(nullptr);
    changeAnimation("崖つかまり開始", "崖つかまり中");
    mActor->setBlendMtxTimer(mActor->getConst().getTable()->mHangBlendTime);
    _12 = 0;
    _14 = 0;
    _16 = 0;
    _1B = 0;
    _1C = 0;
    _1D = 0;
    _1A = 0;
    Mario* pPlayer = getPlayer();
    pPlayer->_74C = 0.0f;
    pPlayer->_750 = 0;
    pPlayer->_754 = 0;
    mActor->_F44 = 0;
    return true;
}

bool MarioHang::update() {
    _14++;
    mActor->damageDropThrowMemoSensor();
    getPlayer()->mDrawStates.mBits.mBit21 = 0;
    fn_803ED7E0();
    getPlayer()->setFrontVecKeepUp(-_34);
    if (fn_803ED840())
        return false;
    if (!fn_803ED870()) {
        if (fn_803ED940())
            return false;
    } else {
        fn_803ED990();
        fn_803EDA20();
    }
    if (fn_803EDAC0()) {
        fn_803ED8C0();
        return false;
    }
    if (fn_803EDD70()) {
        fn_803ED8C0();
        return false;
    }
    if (_1C) {
        fn_803ED8C0();
        return false;
    }

    switch (_12) {
    case 0:
        if (getPlayer()->mVerticalSpeed < 160.0f) {
            if (getPlayer()->getShadowNorm().dot(getGravityVec()) < -0.707f) {
                fn_803ED8C0();
                return false;
            }
        }

        if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
            _1C = 1;
        }

        if (getPlayer()->calcDistToCeilHead() < 80.0f) {
            _1C = 1;
        }

        break;
    case 1:
        if (!fn_803EDB30())
            _1C = 1;
        fn_803EDE40();
        playSound("声崖つかまり終了");
        _12 = 2;
        return true;
    case 2:
        if (!fn_803EDB30())
            _1C = 1;
        if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
            const TVec3f& velocity = getFrontVec() * 6.0f;
            addVelocity(velocity);
        }

        if (!isAnimationRun(nullptr) || isAnimationTerminate(nullptr)) {
            getPlayer()->mMovementStates.mLowBits.mBit30 = 1;
            return false;
        }

        return true;
    }

    if (_1C) {
        fn_803ED8C0();
        return false;
    }

    if (checkTrgA()) {
        tryClimb(true);
    }

    if (((getPlayer()->mMovementStates.mLowWord >> 2) & 1U)) {
        return true;
    }

    u8 direction = 0;
    if (!_16) {
        direction = getPlayer()->checkStickFrontBack();
        if (getStickP() < 0.95f) {
            direction = 0;
        }

        if (_1D) {
            direction = 2;
        }
    }

    switch (direction) {
    case 1:
        if (_14 > 18) {
            tryClimb(false);
        }

        break;
    case 2:
        if (isAnimationRun("崖つかまり開始") && getAnimator()->getFrame() < 20.0f) {
            _1D = 1;
        } else {
            const TVec3f& displacement = getPlayer()->mFrontVec * -100.0f;
            getPlayer()->addTrans(displacement);
            mActor->setBlendMtxTimer(16);
            Mario* pPlayer = getPlayer();
            pPlayer->_3C0 = 0;
            getPlayer()->mMovementStates.mHighBits.mBit14 = 0;
            getPlayer()->mMovementStates.mLowBits.mBit30 = 0;
            getPlayer()->tryDrop();
            pPlayer = getPlayer();
            pPlayer->_3CA = 120;
            return false;
        }

        break;
    }

    if (_1B && !_12 && _14 > 18) {
        _12 = 1;
        _14 = 0;
    }

    if (!_12 && _14 == 2) {
        playSound("声崖つかまり");
    }

    return true;
}

// NONMATCHING: literal relocation labels only.
void MarioHang::tryClimb(bool jump) {
    if (_12) {
        return;
    }

    if (getPlayer()->calcDistToCeil(false) < 160.0f) {
        return;
    }

    if (!fn_803EDB30())
        return;
    u32 i;
    for (i = 0; i < 3; i++) {
        bool left = fn_803EDBD0();
        bool right = fn_803EDCA0();
        if (right && left) {
            _1C = 1;
            return;
        }
        if (left) {
            getPlayer()->addTrans(getPlayer()->mSideVec * -15.0f);
        } else if (right) {
            getPlayer()->addTrans(getPlayer()->mSideVec * 15.0f);
        } else {
            break;
        }
    }

    if (i == 3) {
        _1C = 1;
        return;
    }

    if (isAnimationRun("崖つかまり開始")) {
        if (_1E && jump && getFrame() < 30.0f) {
            _16 = 5;
            changeAnimationNonStop("つかまりスリップアップ準備");
        }

        _1B = 1;
        return;
    }

    _12 = 1;
    _14 = 0;
}

bool MarioHang::close() {
    stopAnimation("崖つかまり開始");
    stopAnimation("崖つかまり中");
    stopAnimation("崖つかまり終了");
    stopAnimation("崖つかまり終了坂");
    if (((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
        changeAnimation(nullptr, "基本");
    } else {
        changeAnimation(nullptr, "落下");
    }

    getPlayer()->setWallCancel();
    mHangTimer = 120;
    if (_1A) {
        getPlayer()->tryHangSlipUp();
    } else {
        getPlayer()->mFlags14.mLowBits.mBit16 = 1;
    }

    mActor->_F44 = 1;
    return true;
}

// NONMATCHING: literal relocation labels only.
bool MarioHang::notice() {
    if (getNoticedStatus() == MarioStatus_Faint) {
        getPlayer()->addTrans(getPlayer()->mFrontVec * -150.0f);
        mActor->setBlendMtxTimer(16);
        stopAnimation("崖つかまり中", "基本");
    }

    return false;
}

// NONMATCHING: register scheduling differs around the outlined vector addition.
bool MarioHang::postureCtrl(MtxPtr pMatrix) {
    TVec3f up;
    TVec3f direction = getPlayer()->mHeadVec + _34;
    if (MR::isNearZero(direction)) {
        return false;
    }

    if (!_12) {
        if (MR::isSameDirection(getPlayer()->_368, _34, 0.01f)) {
            return false;
        }

        MR::makeMtxFrontUp(reinterpret_cast< TPos3f* >(pMatrix), -_34, getPlayer()->_368);
    } else {
        f32 blend = MR::clamp(_14 / 30.0f, 0.0f, 1.0f);
        Mario* headPlayer = getPlayer();
        Mario* bodyPlayer = getPlayer();
        MR::vecBlendSphere(bodyPlayer->_368, headPlayer->mHeadVec, &up, blend);
        if (MR::isSameDirection(up, _34, 0.01f)) {
            return false;
        }

        MR::makeMtxFrontUp(reinterpret_cast< TPos3f* >(pMatrix), -_34, up);
    }

    return true;
}

void MarioHang::fn_803ED7E0() {
    TPos3f matrix;
    matrix.identity();
    MR::makeMtxWithoutScale(&matrix, *_20->getBaseMtx());
    PSMTXMultVecSR(matrix.toMtxPtr(), &_28, &_34);
}

bool MarioHang::fn_803ED840() {
    if (_16) {
        _16--;
        if (!_16)
            _1A = 1;
    }
    return _1A;
}

bool MarioHang::fn_803ED870() {
    return MR::isSameMtx(_20->getBaseMtx()->toMtxPtr(), _20->getPrevBaseMtx()->toMtxPtr());
}

// NONMATCHING: literal relocation labels only.
void MarioHang::fn_803ED8C0() {
    getPlayer()->addTrans(getPlayer()->mFrontVec * -100.0f);
    mActor->setBlendMtxTimer(16);
    stopAnimation("崖つかまり中", "基本");
}

bool MarioHang::fn_803ED940() {
    if (getPlayer()->_8D4 == nullptr)
        return false;
    fn_803ED8C0();
    return true;
}

// NONMATCHING: equivalent scalar literal relocation label.
void MarioHang::fn_803ED990() {
    if (((getPlayer()->mMovementStates.mLowWord >> 5) & 1U) && _14 > 2) {
        const TVec3f* normal;
        MarioActor* actor = mActor;
        normal = &getPlayer()->getSideWallNorm();
        if (actor->getLastMove().dot(*normal) < 0.0f)
            _1C = 1;
    }
}

// NONMATCHING: equivalent comparison/register scheduling.
void MarioHang::fn_803EDA20() {
    Mario* player = getPlayer();
    if (!(player->_184.dot(getFrontVec()) <= 0.707f)) {
        if (!(getPlayer()->_4E4 >= 45.0f) && !isAnimationRun("崖つかまり開始"))
            _1C = 1;
    }
}

bool MarioHang::fn_803EDAC0() {
    bool result = false;
    if (getPlayer()->_8D4 != nullptr) {
        Mario* player = getPlayer();
        HitSensor* sensor = mWallSensor;
        if (sensor != player->_8D4)
            result = true;
    }
    return result;
}

bool MarioHang::fn_803EDB30() {
    Triangle triangle;
    return !getPlayer()->fn_803B2770(nullptr, &triangle, getTrans() - getGravityVec() * 30.0f, getFrontVec() * 60.0f);
}

bool MarioHang::fn_803EDBD0() {
    Triangle triangle;
    Mario* player = getPlayer();
    Mario* sidePlayer = getPlayer();
    return getPlayer()->fn_803B2770(nullptr, &triangle, getTrans() - getGravityVec() * 30.0f - sidePlayer->mSideVec * 15.0f,
                                  player->mSideVec * 55.0f);
}

#pragma dont_inline on
bool MarioHang::fn_803EDCA0() {
    Triangle triangle;
    Mario* player = getPlayer();
    Mario* sidePlayer = getPlayer();
    return getPlayer()->fn_803B2770(nullptr, &triangle, getTrans() - getGravityVec() * 30.0f + sidePlayer->mSideVec * 15.0f,
                                  player->mSideVec * -55.0f);
}

bool MarioHang::fn_803EDD70() {
    Triangle triangle;
    return getPlayer()->fn_803B2770(nullptr, &triangle, getTrans() + getGravityVec() * 30.0f + getFrontVec() * 60.0f,
                                  -getFrontVec() * 100.0f);
}

#pragma dont_inline off

// NONMATCHING: equivalent early return branch folding.
void MarioHang::fn_803EDE40() {
    if (calcAngleD(getPlayer()->_368) > 5.0f) {
        changeAnimationNonStop("崖つかまり終了坂");
        changeAnimation(nullptr, "基本");
    } else {
        changeAnimation("崖つかまり終了", "基本");
    }
}
