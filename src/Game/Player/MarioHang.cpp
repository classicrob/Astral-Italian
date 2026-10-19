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

bool Mario::isHanging() const {
    return getCurrentStatus() == MarioStatus_Hang;
}

bool Mario::fixHangDir(const TVec3f& rPosition, TVec3f* pDirection) {
    TVec3f horizontal(rPosition - mPosHolder->mPos);
    MR::vecKillElement(horizontal, *getGravityVec(), &horizontal);
    MR::normalizeOrZero(&horizontal);
    Triangle pTriangle;
    if (MR::getFirstPolyOnLineToMap(nullptr, &pTriangle, mPosHolder->mPos, horizontal * 150.0f)) {
        *pDirection = -*pTriangle.getNormal(0);
        mHang->recordWallPolygon(&pTriangle);
        return true;
    }

    if (pDirection->dot(horizontal) < 0.0f) {
        *pDirection = horizontal;
        return false;
    }

    return false;
}

void Mario::checkHang() {
    if (mHang->mHangTimer) {
        return;
    }

    if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && isRising()) {
        mMovementStates.mHighWord &= 0xFFFF7FFFU;
    }

    if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && _3BC > 20) {
        mMovementStates.mHighWord &= 0xFFFF7FFFU;
    }

    if (isCeiling()) {
        mMovementStates.mHighWord &= 0xFFFFBFFFU;
    }

    if (!((mMovementStates.mHighWord >> 14) & 1U)) {
        return;
    }

    if (((getPlayer()->mDrawStates.mWord & Mario::DrawStates::Underwater) >> 13)) {
        return;
    }

    if (isStatusActive(MarioStatus_Rabbit)) {
        return;
    }

    if (mActor->_468) {
        return;
    }

    if (getPlayerMode() == PlayerMode_Bee) {
        return;
    }

    if (getPlayerMode() == PlayerMode_Teresa) {
        return;
    }

    bool front = true;
    bool back = true;
    bool side = true;
    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        const Triangle* pTriangle = mSideWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            front = false;
            back = false;
        }
    }

    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        const Triangle* pTriangle = mFrontWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            side = false;
            back = false;
        }
    }

    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        const Triangle* pTriangle = mBackWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            front = false;
            side = false;
        }
    }

    TVec3f previousPosition(mPosHolder->mPos);
    if (front && ((mMovementStates.mHighWord >> 6) & 1U) && isEnableHang()) {
        TVec3f direction(_4A4 - mPosHolder->mPos);
        MR::vecKillElement(direction, getAirGravityVec(), &direction);
        MR::normalizeOrZero(&direction);
        if (getFrontWallNorm().dot(direction) >= -0.8f) {
            return;
        }

        if (!MR::isExistMapCollision(_4A4 - mFrontVec * 50.0f - getAirGravityVec() * 5.0f, mFrontVec * 60.0f)) {
            u8 started = 1;
            if (isAnimationRun("落下")) {
                started = 0;
            }

            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            setTrans(_4A4, nullptr);
            setFrontVecKeepUp(-getWallNorm());
            mHang->recordWallPolygon(mFrontWallTriangle);
            mHang->_1E = started;
        }
    } else if (back && isEnableBackHang()) {
        TVec3f position;
        getLastGroundPos(&position);
        if ((mPosHolder->mPos - position).dot(getAirGravityVec()) >= 160.0f) {
            return;
        }

        TVec3f direction(-mFrontVec);
        u32 edge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        direction = _4F4 - mActor->_2A0;
        MR::normalizeOrZero(&direction);
        u32 positionEdge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        direction = -*mBackWallTriangle->getNormal(0);
        u32 normalEdge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        TVec3f normal;
        if (edge == positionEdge && edge == normalEdge) {
            normal = *getLastGroundEdgeNrm(edge);
        } else {
            return;
        }

        if (normal.dot(getAirGravityVec()) >= 0.1f) {
            return;
        }

        if (normal.dot(mHeadVec) >= 0.1f) {
            return;
        }

        direction = -*mBackWallTriangle->getNormal(0);
        if (MR::diffAngleAbsHorizontal(normal, -direction, getAirGravityVec()) > 0.08726647f) {
            return;
        }

        TVec3f horizontal;
        f32 distance = 0.0f;
        switch (edge) {
        case 0:
        case 1:
            distance = MR::vecKillElement(*_474->calcAndGetPos(0) - position, normal, &horizontal);
            position += normal * distance;
            break;
        case 2:
            distance = MR::vecKillElement(*_474->calcAndGetPos(1) - position, normal, &horizontal);
            position += normal * distance;
            break;
        }

        if (distance > 100.0f) {
            return;
        }

        MR::vecKillElement(normal, getAirGravityVec(), &normal);
        MR::normalizeOrZero(&normal);
        if (!MR::isExistMapCollision(position - direction * 50.0f - getAirGravityVec() * 5.0f, direction * 60.0f)) {
            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            TVec3f hangDirection;
            bool found = fixHangDir(position, &hangDirection);
            setTrans(position, nullptr);
            setFrontVecKeepUp(normal);
            mMovementStates.mLowWord |= 0x00000004U;
            if (!found) {
                mHang->recordWallPolygon(mBackWallTriangle);
            }

            mHang->recordHangNorm(normal);
            mHang->_1E = 0;
        }
    } else if (side && isEnableSideHang()) {
        TVec3f position;
        getLastGroundPos(&position);
        if ((mPosHolder->mPos - position).dot(getAirGravityVec()) >= 160.0f) {
            return;
        }

        TVec3f direction(_500 - mActor->_2A0);
        MR::normalizeOrZero(&direction);
        u32 edge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        TVec3f sideDirection(mSideVec);
        if (sideDirection.dot(direction) < 0.0f) {
            direction = -sideDirection;
        } else {
            direction = sideDirection;
        }

        u32 sideEdge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        direction = -*mSideWallTriangle->getNormal(0);
        u32 normalEdge = getLastGroundEdgeIndex(mActor->_2A0, direction);
        TVec3f normal;
        if (edge == sideEdge && edge == normalEdge) {
            normal = *getLastGroundEdgeNrm(edge);
        } else {
            return;
        }

        if (normal.dot(getAirGravityVec()) >= 0.1f) {
            return;
        }

        if (normal.dot(mHeadVec) >= 0.1f) {
            return;
        }

        direction = -*mSideWallTriangle->getNormal(0);
        if (MR::diffAngleAbsHorizontal(normal, -direction, getAirGravityVec()) > 0.08726647f) {
            return;
        }

        MR::vecKillElement(normal, getAirGravityVec(), &normal);
        MR::normalizeOrZero(&normal);
        if (!MR::isExistMapCollision(position - direction * 50.0f - getAirGravityVec() * 5.0f, direction * 60.0f)) {
            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            TVec3f hangDirection;
            bool found = fixHangDir(position, &hangDirection);
            setTrans(position, nullptr);
            setFrontVecKeepUp(normal);
            mMovementStates.mLowWord |= 0x00000004U;
            if (!found) {
                mHang->recordWallPolygon(mSideWallTriangle);
            }

            mHang->recordHangNorm(normal);
            mHang->_1E = 0;
        }
    }

    if (isStatusActive(MarioStatus_Hang)) {
        _414 = 0;
        for (u32 i = 0; i < 10; i++) {
            updateGroundInfo();
            if (((mMovementStates.mLowWord >> 30) & 1U)) {
                return;
            }

            addTrans(mFrontVec * 1.0f - getAirGravityVec() * 1.0f, nullptr);
        }
    }
}

bool Mario::isEnableHang() {
    if (!((mMovementStates.mLowWord >> 23) & 1U)) {
        return false;
    }

    if (!((mMovementStates.mLowWord >> 10) & 1U)) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 20) & 1U)) {
        return false;
    }

    if (((mDrawStates.mWord >> 1) & 1U)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Recovery)) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_47C);
    if (pWallCode != nullptr) {
        if (strcmp(pWallCode, "NotGrab") == 0) {
            return false;
        }

        if (strcmp(pWallCode, "NoAction") == 0) {
            return false;
        }
    }

    TVec3f shadowPos(mShadowPos);
    TVec3f up(-getAirGravityVec());
    f32 slope = _45C->getNormal(0)->dot(up);
    f32 height = mActor->getConst().getTable()->mWallHangGrHeight;
    if (slope > 0.0f) {
        if (slope > 0.2f) {
            slope = 0.2f;
        }

        height -= 30.0f * (5.0f * slope);
    }

    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        if ((_4A4 - mShadowPos).dot(-*getGravityVec()) < height) {
            return false;
        }
    }

    f32 heightFromMario = (_4A4 - mPosHolder->mPos).dot(-*getGravityVec());
    if (heightFromMario > mActor->getConst().getTable()->mWallHangMyHeight) {
        return false;
    }

    return !isRising();
}

bool Mario::isEnableBackHang() {
    bool turn = false;
    if (!_474->isValid()) {
        return false;
    }

    if (_430 == 6) {
        if (isRising()) {
            return false;
        }

        turn = true;
    } else if (!((mMovementStates.mHighWord >> 15) & 1U)) {
        return false;
    }

    if (!((mMovementStates.mLowWord >> 6) & 1U)) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 20) & 1U)) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_474);
    if (pWallCode != nullptr && strcmp(pWallCode, "NotGrab") == 0) {
        return false;
    }

    if (isRising()) {
        return false;
    }

    if (mStickPos.z > mActor->getConst().getTable()->mWallBackHangStickPower) {
        return false;
    }

    if (mWalkSpeed > mActor->getConst().getTable()->mWallBackHangWalkSpeed) {
        return false;
    }

    TVec3f ground;
    TVec3f horizontal;
    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        if (turn) {
            ground = mPosHolder->mPos;
        } else {
            getLastGroundPos(&ground);
        }

        f32 height = MR::vecKillElement(mShadowPos - ground, *getGravityVec(), &horizontal);
        if (height < mActor->getConst().getTable()->mWallHangGrHeight) {
            return false;
        }
    }

    if (!turn) {
        if ((_4F4 - mGroundPos).dot(*getGravityVec()) < 0.0f) {
            return false;
        }
    } else {
        setFrontVecKeepUp(-mFrontVec);
        return false;
    }

    TVec3f distance;
    TVec3f position;
    getLastGroundPos(&position);
    MR::vecKillElement(position - mPosHolder->mPos, getAirGravityVec(), &distance);
    return !(distance.length() >= 84.0f);
}

bool Mario::isEnableSideHang() {
    if (((mFlags20.mWord >> 30) & 1U) || ((mFlags20.mWord >> 29) & 1U)) {
        return false;
    }

    if (!((mMovementStates.mHighWord >> 15) & 1U)) {
        return false;
    }

    if (!((mMovementStates.mLowWord >> 5) & 1U)) {
        return false;
    }

    if (checkWallCode("NotGrab", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    const char* pWallCode = MR::getWallCodeString(_474);
    if (pWallCode != nullptr) {
        if (strcmp(pWallCode, "NotGrab") == 0) {
            return false;
        }

        if (strcmp(pWallCode, "NoAction") == 0) {
            return false;
        }
    }

    if (isRising()) {
        return false;
    }

    if (mStickPos.z > mActor->getConst().getTable()->mWallBackHangStickPower) {
        return false;
    }

    if (mWalkSpeed > mActor->getConst().getTable()->mWallBackHangWalkSpeed) {
        return false;
    }

    TVec3f ground;
    TVec3f horizontal;
    if (getPlayer()->getShadowNorm().dot(*getGravityVec()) < -0.707f) {
        getLastGroundPos(&ground);
        f32 height = MR::vecKillElement(mShadowPos - ground, *getGravityVec(), &horizontal);
        if (height < mActor->getConst().getTable()->mWallHangGrHeight) {
            return false;
        }
    }

    if (!((mMovementStates.mHighWord >> 12) & 1U)) {
        return false;
    }

    if ((_500 - mGroundPos).dot(*getGravityVec()) < -20.0f) {
        return false;
    }

    TVec3f distance;
    TVec3f position;
    getLastGroundPos(&position);
    MR::vecKillElement(position - mPosHolder->mPos, getAirGravityVec(), &distance);
    return !(distance.length() >= 84.0f);
}

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

// NONMATCHING: equivalent state increment, stack and ignored addTrans tag setup.
bool MarioHang::update() {
    _14++;
    mActor->damageDropThrowMemoSensor();
    getPlayer()->mDrawStates.mWord &= 0xFFDFFFFFU;
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
        _12++;
        return true;
    case 2:
        if (!fn_803EDB30())
            _1C = 1;
        if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
            addVelocity(getFrontVec() * 6.0f);
        }

        if (!isAnimationRun(nullptr) || isAnimationTerminate(nullptr)) {
            getPlayer()->mMovementStates.mLowWord |= 0x40000000U;
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
            getPlayer()->addTrans(getPlayer()->mFrontVec * -100.0f, "Module");
            mActor->setBlendMtxTimer(16);
            Mario* pPlayer = getPlayer();
            pPlayer->_3C0 = 0;
            getPlayer()->mMovementStates.mHighWord &= 0xFFFFBFFFU;
            getPlayer()->mMovementStates.mLowWord &= 0xBFFFFFFFU;
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
        getPlayer()->mFlags14.mLowWord |= 0x00010000U;
    }

    mActor->_F44 = 1;
    return true;
}

// NONMATCHING: vector operand scheduling and ignored addTrans tag setup.
bool MarioHang::notice() {
    if (getNoticedStatus() == MarioStatus_Faint) {
        getPlayer()->addTrans(getPlayer()->mFrontVec * -150.0f, "Module");
        mActor->setBlendMtxTimer(16);
        stopAnimation("崖つかまり中", "基本");
    }

    return false;
}

// NONMATCHING: register allocation and ignored addTrans tag setup.
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
            getPlayer()->addTrans(getPlayer()->mSideVec * -15.0f, "Module");
        } else if (right) {
            getPlayer()->addTrans(getPlayer()->mSideVec * 15.0f, "Module");
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
// NONMATCHING: vector addition inlines to copy and add operations.
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
        MR::vecBlendSphere(getPlayer()->_368, getPlayer()->mHeadVec, &up, blend);
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

// NONMATCHING: boolean normalization of the byte flag.
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

// NONMATCHING: vector operand scheduling and ignored addTrans tag setup.
void MarioHang::fn_803ED8C0() {
    getPlayer()->addTrans(getPlayer()->mFrontVec * -100.0f, "Module");
    mActor->setBlendMtxTimer(16);
    stopAnimation("崖つかまり中", "基本");
}

bool MarioHang::fn_803ED940() {
    if (getPlayer()->_8D4 == nullptr)
        return false;
    fn_803ED8C0();
    return true;
}

// NONMATCHING: equivalent flag extraction.
void MarioHang::fn_803ED990() {
    if (((getPlayer()->mMovementStates.mLowWord >> 5) & 1U) && _14 > 2) {
        MarioActor* actor = mActor;
        const TVec3f& normal = getPlayer()->getSideWallNorm();
        if (actor->getLastMove().dot(normal) < 0.0f)
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

// NONMATCHING: equivalent boolean folding.
bool MarioHang::fn_803EDAC0() {
    bool result = false;
    if (getPlayer()->_8D4 != nullptr && mWallSensor != getPlayer()->_8D4)
        result = true;
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
