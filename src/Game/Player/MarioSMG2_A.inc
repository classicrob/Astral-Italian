// Ordered Mario continuation: target .text offsets 0x10000..0x1ffff.

bool Mario::doFlipWeak(const TVec3f& rVec) {
    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    mFaint->mNoDamage = true;

    if (faint(rVec)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->fn_803F7C10();
            mFaint->mNoDamage = false;
        }

        return true;
    }

    mFaint->mNoDamage = false;
    forceStopTornado();
    return false;
}

bool Mario::faint(const TVec3f& rVec) {
    _7C4 = rVec;
    if (!isEnableAddDamage()) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    if (isPlayerModeYoshi()) {
        if (isStatusActive(MarioStatus_Swim)) {
            closeStatus(mSwim);
        }
        mDamage->_34 = rVec;
    } else {
        mFaint->setVec(rVec);
    }
    stopWalk();
    forceStopTornado();
    if (isStatusActive(MarioStatus_FpView)) {
        closeStatus(mFpView);
    }
    mActor->damageDropThrowMemoSensor();

    if (getCurrentStatus() != MarioStatus_Faint) {
        mMovementStates.mHighBits.mBit24 = 1;
        return true;
    }

    return ((mMovementStates.mHighWord >> 24) & 1U);
}

bool Mario::doFlipJump(const TVec3f& rVec) {
    if (isAnimationRun("壁はじき")) {
        return false;
    }
    stopWalk();
    TVec3f velocity(rVec);
    velocity.setLength(mActor->mConst->getTable()->mWallSpinFlipGround);
    TVec3f gravity;
    MR::calcGravityVector(mActor, &gravity, nullptr, 0);
    if (!isPlayerModeYoshi() || mActor->getYoshi()->mDashTimer <= 0 || MR::abs(getGravityVec()->dot(gravity)) > 0.2f) {
        f32 hop = mActor->mConst->getTable()->mWallSpinHopGround;
        velocity -= *getGravityVec() * hop;
    }
    tryForcePowerJump(velocity, true);
    changeAnimation("壁はじき");
    stopPunch();
    mMovementStates.mHighBits.mBit20 = 1;
    _430 = 3;
    startPadVib(2);
    return true;
}

bool Mario::doFlipRot(const TVec3f& rVec) {
    getCurrentStatus();
    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    mFlip->setVec(rVec);

    stopJump();
    stopWalk();
    forceStopTornado();

    changeStatus(mFlip);

    return true;
}

bool Mario::doFlipLarge(const TVec3f& rVec) {
    if (getCurrentStatus() == MarioStatus_Damage) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_Flip) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_FireDamage) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        return false;
    }

    return flipLarge(rVec);
}

bool Mario::doFlipBackRoll(const TVec3f& rVec) {
    TVec3f vec20;
    MR::vecKillElement(rVec, getAirGravityVec(), &vec20);

    if (doFlipLarge(vec20)) {
        checkDamage();
        changeAnimationNonStop("後転ふっとび");

        mDamage->setStrings("後転ふっとび空中", "後転ふっとび着地");

        TVec3f vec28;
        MR::vecKillElement(rVec, getAirGravityVec(), &vec28);
        setFrontVecKeepUp(-vec28, static_cast< u32 >(12l));

        return true;
    }

    return false;
}

bool Mario::fn_803972D0(const TVec3f* arg1) {
    if (this->isStatusActive(0x19) != 0) {
        return false;
    }
    if (this->isAnimationRun("空中ひねり") != 0) {
        return false;
    }
    if (this->isAnimationRun("雲スピン空中") != 0) {
        return false;
    }
    if (this->isPlayerModeYoshi() != 0) {
        this->mActor->fn_803B9870();
        this->mActor->fn_803BE840(*arg1);
    }
    this->changeStatus(this->mFlow);
    return true;
}

// namespace

void Mario::tryStartFoo() {
    if (MR::getAreaObj("CelestrialArea", getTrans()) || !mFoo->_48) {
        changeStatus(mFoo);
        mFoo->setStartSpeed(mActor->getConst().getTable()->mSwimFrontMaxSpeed);
    }
}

// namespace

bool Mario::isDisableStayHere() const {
    if (((getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        return true;
    }

    if (!((getMovementStates().mLowWord >> 30) & 1U)) {
        return true;
    }

    if (_3CE < 5) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 21) & 1U)) {
        return true;
    }

    if (isDamaging()) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 4) & 1U)) {
        return true;
    }

    if (MR::getKarikariClingNum() != 0) {
        return true;
    }

    if (((getPlayer()->getMovementStates().mHighWord >> 28) & 1U)) {
        return true;
    }

    if (mWalkSpeed > 0.01f) {
        return true;
    }

    if (getStickP() != 0.0f) {
        return true;
    }

    if (((mFlags14.mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return true;
    }

    if (_960 == 27) {
        return true;
    }

    if (_960 == 28) {
        return true;
    }

    if (((getDrawStates().mWord >> 20) & 1U)) {
        return true;
    }

    if (((getMovementStates().mLowWord >> 7) & 1U)) {
        return true;
    }

    if (!MR::isNearZero(_928)) {
        return true;
    }

    if (!MR::isNearZero(_350)) {
        return true;
    }

    if (!MR::isNearZero(_35C)) {
        return true;
    }

    if (!MR::isNearZero(_184)) {
        return true;
    }

    if (mActor->mBeeWallWalk != 0) {
        return true;
    }

    if (((getMovementStates().mHighWord >> 8) & 1U)) {
        return true;
    }

    if (((getMovementStates().mHighWord >> 5) & 1U)) {
        return true;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit10))) {
        return true;
    }

    if (mSinkTimer) {
        return true;
    }

    if (mActor->_EA4) {
        return true;
    }

    return mActor->_3C0;
}

bool Mario::isDisableFpViewMode() const {
    if (_97C) {
        return true;
    }
    if (isDisableStayHere()) {
        return true;
    }
    if (isPlayerModeYoshi() && !mActor->fn_803B9810()->fn_8041F9E0()) {
        return true;
    }
    return false;
}

void Mario::tryFpViewMode() {
    if (isStatusActive(MarioStatus_FpView)) {
        return;
    }

    stopWalk();

    changeStatus(mFpView);

    MR::startSystemSE("SE_SY_CAMERA_FP_VIEW");
}

bool Mario::doFrontStep() {
    if (getCurrentStatus() == MarioStatus_FrontStep) {
        return false;
    }

    if (getCurrentStatus() == MarioStatus_SideStep) {
        return false;
    }

    if (checkWallCode("NotSideStep", false)) {
        return false;
    }

    if (checkWallCode("NoAction", false)) {
        return false;
    }

    if (mActor->_468 != 0) {
        return false;
    }

    if ((static_cast<u32>(mFlags20.mBits.mBit26))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        return false;
    }

    if (getPlayerMode() == PlayerMode_Hopper) {
        return false;
    }

    if (_960 == 2) {
        return false;
    }

    if (calcAngleD(_368) >= 30.0f) {
        return false;
    }

    if (mAirGravityVec.dot(_374) >= 0.25f) {
        return false;
    }

    Triangle triangle = Triangle();
    MarioActor* pMario = mActor;
    TVec3f* vec = &pMario->_240;
    TVec3f ray(mFrontVec * 200.0f);
    bool hit;
    if (fn_803B2950()) {
        hit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(nullptr, &triangle, *vec, ray);
    } else {
        hit = MR::getFirstPolyOnLineToMap(nullptr, &triangle, *vec, ray);
    }
    if (!hit) {
        return false;
    }

    setFrontVecKeepUp(-*MR::getNormal(&triangle));

    stopWalk();
    forceStopTornado();

    changeStatus(mFrontStep);

    return true;
}

bool Mario::isStateHanging() const {
    return getCurrentStatus() == MarioStatus_Hang;
}

bool Mario::isStateBury() const {
    return getCurrentStatus() == MarioStatus_Bury;
}

void Mario::checkHang() {
    if (mHang->mHangTimer) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && isRising()) {
        mMovementStates.mHighBits.mBit15 = 0;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && _3BC > 20) {
        mMovementStates.mHighBits.mBit15 = 0;
    }

    if (isCeiling()) {
        mMovementStates.mHighBits.mBit14 = 0;
    }

    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit14))) {
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

    if (isPlayerModeBee()) {
        return;
    }

    if (isPlayerModeTeresa()) {
        return;
    }

    if (isPlayerModeYoshi()) {
        return;
    }

    bool front = true;
    bool back = true;
    bool side = true;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
        const Triangle* pTriangle = mSideWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            front = false;
            back = false;
        }
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        const Triangle* pTriangle = mFrontWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            side = false;
            back = false;
        }
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        const Triangle* pTriangle = mBackWallTriangle;
        if (!MR::isSameMtx(pTriangle->getBaseMtx()->toMtxPtr(), pTriangle->getPrevBaseMtx()->toMtxPtr())) {
            front = false;
            side = false;
        }
    }

    TVec3f previousPosition(mPosHolder->mPos);
    if (front && (static_cast<u32>(mMovementStates.mHighBits.mBit6)) && isEnableHang()) {
        TVec3f direction(_4A4 - mPosHolder->mPos);
        MR::vecKillElement(direction, getAirGravityVec(), &direction);
        MR::normalizeOrZero(&direction);
        if (getFrontWallNorm().dot(direction) >= -0.8f) {
            return;
        }

        if (!fn_803B2880(_4A4 - mFrontVec * 50.0f - getAirGravityVec() * 5.0f, mFrontVec * 60.0f)) {
            u8 started = 1;
            if (isAnimationRun("落下")) {
                started = 0;
            }

            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            setTrans(_4A4);
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
        u32 edge = getLastGroundEdgeIndex(mActor->_240, direction);
        direction = _4F4 - mActor->_240;
        MR::normalizeOrZero(&direction);
        u32 positionEdge = getLastGroundEdgeIndex(mActor->_240, direction);
        direction = -*mBackWallTriangle->getNormal(0);
        u32 normalEdge = getLastGroundEdgeIndex(mActor->_240, direction);
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
        const TVec3f& opposite = -direction;
        if (MR::diffAngleAbsHorizontal(normal, opposite, getAirGravityVec()) > 0.08726647f) {
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
        if (!fn_803B2880(position - direction * 50.0f - getAirGravityVec() * 5.0f, direction * 60.0f)) {
            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            TVec3f hangDirection;
            bool found = fixHangDir(position, &hangDirection);
            setTrans(position);
            setFrontVecKeepUp(normal);
            mMovementStates.mLowBits.mBit2 = 1;
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

        TVec3f direction(_500 - mActor->_240);
        MR::normalizeOrZero(&direction);
        u32 edge = getLastGroundEdgeIndex(mActor->_240, direction);
        TVec3f sideDirection(mSideVec);
        if (sideDirection.dot(direction) < 0.0f) {
            direction = -sideDirection;
        } else {
            direction = sideDirection;
        }

        u32 sideEdge = getLastGroundEdgeIndex(mActor->_240, direction);
        direction = -*mSideWallTriangle->getNormal(0);
        u32 normalEdge = getLastGroundEdgeIndex(mActor->_240, direction);
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
        const TVec3f& opposite = -direction;
        if (MR::diffAngleAbsHorizontal(normal, opposite, getAirGravityVec()) > 0.08726647f) {
            return;
        }

        MR::vecKillElement(normal, getAirGravityVec(), &normal);
        MR::normalizeOrZero(&normal);
        if (!fn_803B2880(position - direction * 50.0f - getAirGravityVec() * 5.0f, direction * 60.0f)) {
            changeStatus(mHang);
            mVelocity.zero();
            stopWalk();
            stopJump();
            TVec3f hangDirection;
            bool found = fixHangDir(position, &hangDirection);
            setTrans(position);
            setFrontVecKeepUp(normal);
            mMovementStates.mLowBits.mBit2 = 1;
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
            if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
                return;
            }

            addTrans(mFrontVec * 1.0f - getAirGravityVec() * 1.0f);
        }
    }
}

bool Mario::fixHangDir(const TVec3f& rPosition, TVec3f* pDirection) {
    TVec3f horizontal(rPosition - mPosHolder->mPos);
    MR::vecKillElement(horizontal, *getGravityVec(), &horizontal);
    MR::normalizeOrZero(&horizontal);
    Triangle pTriangle;
    if (fn_803B2770(nullptr, &pTriangle, mPosHolder->mPos, horizontal * 150.0f)) {
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

bool Mario::isEnableHang() {
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit10))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit1))) {
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
        const TVec3f& shadowDelta = _4A4 - mShadowPos;
        if (shadowDelta.dot(-*getGravityVec()) < height) {
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
    } else if (!(static_cast<u32>(mMovementStates.mHighBits.mBit15))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
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
    if ((static_cast<u32>(mFlags20.mBits.mBit30)) || (static_cast<u32>(mFlags20.mBits.mBit29))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit15))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
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

    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit12))) {
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

void Mario::initMember() {
    mPosHolder = new MarioPosHolder();
    _40.zero();
    _4C.zero();
    PSMTXIdentity(_F4.toMtxPtr());
    mPosHolder->fn_80405F90(TVec3f(0.0f));
    _16C.zero();
    mVelocityAfter.zero();
    _1B4.zero();
    _274 = 0;
    _280 = 0.0f;
    _284.zero();
    _2B8.zero();
    _2C4.zero();
    _304.zero();
    mShadowPos.zero();

    _340 = 0.0f;
    _3A4.zero();
    _3B0.zero();

    _422 = 0;
    _42A = 0;
    _438 = 0;
    _448.zero();

    _454 = 0.0f;
    mGroundPos.zero();
    _498.zero();
    _4BC.zero();
    _50C.zero();
    _518.zero();
    _600.zero();
    _60C = 0;
    _638 = 0;
    _660.zero();
    _688.zero();
    _694.zero();

    mPlaneModeArea = nullptr;
    _6C8.y = 0.0f;
    _6C8.x = 0.0f;
    _6D0.zero();
    _6DC.zero();
    _6E8.zero();
    _6F4.zero();
    _700.zero();
    _70C.zero();

    _718 = 0.0f;
    _71D = 0;
    _71F = 0;
    _720 = nullptr;
    _724 = nullptr;
    _728 = nullptr;

    _75C.zero();
    _790.zero();
    _7C4.zero();
    _7D4.zero();
    PSMTXIdentity(_7E4.toMtxPtr());
    _814.zero();
    PSMTXIdentity(_824.toMtxPtr());

    mRelativeWallPos[0].zero();
    mRelativeWallPos[1].zero();
    _8DC.zero();

    _8EC = 0;
    _8F4 = 0.0f;

    for (int i = 0; i < ARRAY_SIZE(_A08); i++) {
        _A08[i] = 0;
    }

    _A34 = 0;
    _A3C = 0;

    _A40.zero();
    _A4C.zero();
    _A58.zero();
    _A64 = 0.0f;

    _A70 = 0;
    _A71 = 0;
    _A72 = 0;
    mDashWallCooldown = 0;
    _A7E = 0;
    _A7F = 0;
    _A80 = 0;
    mCometNoSpinArea = false;
    mCometFireDanceArea = false;
    mCometRainbowArea = false;
    mCometHopperArea = false;
    mCometHopperApplied = false;
    mCometRainbowApplied = false;
    mMoveInfoA84 = new MarioMoveInfoA84();
    mStateFlags.fn_8038C890();
    mDashWallSoundPlayed = false;
}

bool Mario::isRising() const {
    if (getPlayerMode() == 4 || getPlayerMode() == 6) {
        if (_16C.dot(*getGravityVec()) < 0.0f) {
            return true;
        }

        return false;
    }

    return mJumpVec.dot(*getGravityVec()) < 0.0f;
}

void Mario::checkWallRiseAndSlipFront() {
    mMovementStates.mLowBits.mBit25 = 0;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        mMovementStates.mLowBits.mBit22 = 0;
        mWalkSpeed = 0.0f;
        return;
    }

    mMovementStates.mLowBits.mBit22 = 1;
}

void Mario::tryJump() {
    bool stickJump;
    bool mud = false;
    bool squat;
    TVec3f stickTotal(_190);
    stickTotal += _19C;
    if (static_cast<u32>(mMovementStates.mLowBits.mBit31)) return;
    if (fn_80399D30()) return;
    playEffect("共通跳躍");
    stopEffectForce("スピンリング");
    squat = checkSquat(true);
    mMovementStates.mHighBits.mBit30 = 1;
    mMovementStates.mHighBits.mBit22 = 0;
    mMovementStates.mHighBits.mBit7 = 0;
    mFlags14.mHighBits.mBit28 = 1;
    mMovementStates.mLowBits.mBit20 = 0;
    fn_80399E60(&stickTotal);
    forceStopTornado();
    if (tryTurnJump()) return;
    checkWallRiseAndSlipFront();
    f32 floorAngle = calcPolygonAngleD(mGroundPolygon);
    stickJump = false;
    if (!MR::isNearZero(_8F8, 0.001f) && (static_cast<u32>(mMovementStates.mHighBits.mBit28))) stickJump = true;
    if (floorAngle < 15.0f) {
        mMovementStates.mHighBits.mBit28 = 0;
        _8F8.zero();
    }
    if (getPlayerMode() != 6 && !(static_cast<u32>(mFlags14.mLowBits.mBit5))) {
        if (((static_cast<u32>(mDrawStates.mBits.mBit19)) && isSlipPolygon(_45C)) || isStatusActive(7)) {
            MR::isNearZero(mAirGravityVec, 0.001f);
            if (MR::diffAngleAbsHorizontal(_16C, _368, mAirGravityVec) > 1.5707964f) mWalkSpeed = 0.0f;
            if (getPlayerMode() != 4) {
                _402 = 0;
                _430 = 3;
                mMovementStates.mHighBits.mBit20 = 1;
            }
        }
        if (_960 == 0x11) {
            mud = true;
            _430 = 3;
            mWalkSpeed *= 0.5f;
        }
    }
#ifdef GALAXY_NATIVE
    // OdysseyMovement: jumping out of a roll is a long jump.
    if (fn_803EF7C0() != 0x20 && getPlayerMode() != 4 && _430 != 3 && PetariNative::Odyssey::Live::roll.rolling && odysseyOn() &&
        !((mMovementStates.mHighWord >> 28) & 1U)) {
        PetariNative::Odyssey::Live::roll.rolling = false;
        trySquatJump();
        return;
    }
#endif
    if (fn_803B2530(squat)) return;
    if (static_cast<u32>(mDrawStates.mBits.mBit26)) {
        f32 walkSpeed;
        f32 speed = fn_803B1CA0();
        walkSpeed = mWalkSpeed;
        mJumpVec = _334 * walkSpeed * speed;
    } else {
        f32 walkSpeed;
        f32 speed = fn_803B1CA0();
        walkSpeed = mWalkSpeed;
        mJumpVec = mFrontVec * walkSpeed * speed;
    }
    mFlags14.mLowBits.mBit8 = 0;
    if (static_cast<u32>(mDrawStates.mBits.mBit20)) {
        recordJumpEnforceMove();
        f32 enforce;
        if (getStickP() == 0.0f) {
            mJumpVec.zero();
            enforce = 1.0f;
        } else {
            MR::isNearZero(mAirGravityVec, 0.001f);
            f32 factor = 1.0f - MR::diffAngleAbsHorizontal(mFrontVec, _184, mAirGravityVec) / 1.5707964f;
            if (factor < 0.0f) factor = 0.0f;
            f32 speed = PSVECMag(&mJumpVec);
            f32 bonus = MR::clamp(1.0f - (speed - 4.0f) / 10.0f, 0.0f, 1.0f);
            factor += bonus;
            enforce = MR::clamp(factor, 0.0f, 1.0f);
        }
        if (MR::isInAreaObj("EnforceJumpArea", mPosHolder->mPos) && _8D8 == _45C->mSensor) enforce = 1.0f;
        doEnforceJump(enforce);
    }
    if (!MR::isNearZero(stickTotal, 0.001f)) {
        addVelocity(-stickTotal);
        MR::vecKillElement(stickTotal, mActor->mAppliedGravityVec, &stickTotal);
        cutGravityElementFromJumpVec(true);
        if (stickTotal.dot(mJumpVec) > 0.0f) {
            mJumpVec += stickTotal * 1.0f;
        } else {
            mJumpVec += stickTotal * 0.5f;
        }
    }
    if (stickJump) {
        const TVec3f& airGravity = getAirGravityVec();
        TVec3f projected;
        JMAVECScaleAdd(&airGravity, &_368, &projected, -airGravity.dot(_368));
        if (!MR::normalizeOrZero(&projected) && _16C.dot(projected) > 0.0f) {
            mJumpVec += _16C;
            cutGravityElementFromJumpVec(true);
        }
    }
    _340 = 1.0f - mWalkSpeed;
    if (_340 < 0.0f) _340 = 0.0f;
    f32 height;
    f32 ratio = 1.0f;
    if (fn_803EF7C0() == 0x20) ratio = mActor->mConst->getTable()->mMudFloorJumpWeakRatio;
    if (static_cast<u32>(mFlags20.mBits.mBit20)) ratio = 0.9f;
#ifdef GALAXY_NATIVE
    // OdysseyMovement: SMO's chain rules choose single/double/triple (within 10
    // grounded frames, at full speed, within 45 degrees of the last jump).
    // getJumpHeight() below reads the new _430.
    const bool odysseyJump = petari_mod_enabled(kModOdysseyMovement) && _430 >= 0 && _430 <= 2 && !((mFlags20.mWord >> 21) & 1U) &&
                             !((mFlags20.mWord >> 20) & 1U) && fn_803EF7C0() != 0x20 && !((mFlags14.mLowWord >> 5) & 1U) &&
                             // SMG2: only plain Mario (mode 0, not riding Yoshi) gets SMO's chain;
                             // power-ups and rides keep Galaxy's jumps (docs: native/MODS.md).
                             getPlayerMode() == 0 && !isPlayerModeYoshi();
    TVec3f odysseyDir;
    f32 odysseySpeed = 0.0f;
    if (odysseyJump) {
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyDir = mJumpVec - up * up.dot(mJumpVec);
        MR::normalizeOrZero(&odysseyDir);
        if (MR::isNearZero(odysseyDir)) {
            odysseyDir = mFrontVec - up * up.dot(mFrontVec);
            MR::normalizeOrZero(&odysseyDir);
        }
        // The ground speed in units/frame (Galaxy keeps a ratio of its run speed).
        odysseySpeed = mWalkSpeed * mActor->mConst->getTable()->mWalkSpeed;
        _430 = sOdysseyGroundPoundJump
                   ? 0
                   : PetariNative::Odyssey::nextChainIndex(sOdyssey.lastChain, _3CE, odysseySpeed, odysseyDir.dot(sOdyssey.lastDir));
    }
#endif
    height = getJumpHeight();
    mJumpVec += -mActor->mAppliedGravityVec * height * ratio;
    mMovementStates.mLowBits.mBit17 = 0;
    mMovementStates.mLowBits.mBit30 = 0;
    if (getPlayerMode() != 4 && fn_803EF7C0() == 0x20) {
        _430 = 3;
        mud = true;
        mMovementStates.mHighBits.mBit20 = 1;
    }
    if (static_cast<u32>(mFlags14.mLowBits.mBit5)) _430 = 0;
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    sCappyDiveReady = false;
    sOdysseyAirFrames = 0;  // a new airtime (also after a landing that skipped procJump's, e.g. water)
    sOdysseyAirDived = false;
    if (odysseyJump) {
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        if (sOdysseyGroundPoundJump) {
            odysseyStart(PetariNative::Odyssey::Air::GroundPoundJump, odysseyDir, up, 0.0f, _430);
            sOdyssey.lastChain = -1;
        } else {
            odysseyStart(PetariNative::Odyssey::chainAir(_430), odysseyDir, up, odysseySpeed, _430);
            sOdyssey.lastChain = _430;
            sOdyssey.lastDir = odysseyDir;
        }
    }
    sOdysseyGroundPoundJump = false;
#endif
    procJump(true);
    mMovementStates.mLowBits.mBit31 = 1;
    if (_42A == 1 && _430 == 1) _430 = 2;
    switch (_430) {
    case 0:
        if (static_cast<u32>(mMovementStates.mLowBits.mBit8)) changeAnimation("壁上昇", "落下");
        else if (isPlayerModeYoshi() && mActor->getYoshi()->mDashTimer > 0) {
            MR::startCenterScreenBlur(35, 15.0f, 80, 5, 30);
            playEffect("ダッシュジャンプ煙");
            changeAnimation("ヨッシージャンプ", "落下");
        } else changeAnimation("ジャンプ", "落下");
        if (mActor->mBeeWallWalk != 0) playSound("ハチ壁ジャンプ");
        else playSound("小ジャンプ");
        if (getPlayerMode() != 6) mStateFlags._0 = true;
        if (stickJump) {
            changeAnimation("尻滑りジャンプ");
            _428 = 30;
        }
        if (getPlayerMode() == 4) {
            changeAnimation("ハチジャンプ");
            mActor->syncJumpBeeStickMode();
        }
        if (static_cast<u32>(mFlags14.mLowBits.mBit5)) changeAnimation("ショートジャンプ", "落下");
        break;
    case 1:
        changeAnimationNonStop("ジャンプB");
        changeAnimation(nullptr, "落下");
        if (isPlayerModeYoshi()) {
            if (mActor->getYoshi()->mDashTimer > 0) MR::startActionSound(mActor->getYoshi(), "DashJumpL", -1, -1, -1);
            else MR::startActionSound(mActor->getYoshi(), "JumpM", -1, -1, -1);
        } else playSound("声中ジャンプ");
        playSound("中ジャンプ");
        playSound("ジャンプ踏切");
        break;
    case 2:
        playSound("大ジャンプ");
        playSound("声大ジャンプ");
        playSound("ジャンプ踏切");
        playEffect("共通ハイジャンプ");
        if (static_cast<u32>(mFlags20.mBits.mBit20)) changeAnimation("スカイラブジャンプ", "落下");
        else if (getPlayerMode() == 1 && mWalkSpeed > 1.0f) changeAnimation("ダッシュジャンプ", "落下");
        else changeAnimation("ジャンプC", "落下");
        break;
    case 3:
        if (mud) changeAnimation("埋まり脱出ジャンプ", "落下");
        else {
            if (!isAnimationRun("坂すべり上向きうつぶせ")) changeAnimation("尻滑りジャンプ", "落下");
            else changeAnimation("腹ばいジャンプ", "落下");
            if (isPlayerModeYoshi()) MR::startActionSound(mActor->getYoshi(), "JumpHardly", -1, -1, -1);
            else playSound("声ミスジャンプ");
            playSound("ジャンプ踏切");
        }
        break;
    }
    if (_42A == 1 && _430 == 2) _430 = 1;
    if (mSwim->checkUnderWaterFull(mFrontVec) > 500.0f && mStickPos.z == 0.0f) {
        mMovementStates.mLowBits.mBit17 = 1;
        changeAnimationNonStop("飛び込みジャンプ");
        if (isPlayerModeYoshi()) MR::startActionSound(mActor->getYoshi(), "PreLandTurn", -1, -1, -1);
        else playSound("声高飛び込み");
        playSound("ジャンプ踏切");
        if (mStickPos.z < 0.1f) {
            mJumpVec += mFrontVec * 5.0f;
        }
    }
    mMovementStates.mLowBits.mBit26 = 1;
}

// NONMATCHING: sink/animation selection from the draft; the speed/timer flags are approximated
// NONMATCHING: verified string arguments use different pooled offsets.
bool Mario::fn_80399D30() {
    u8 timer = mSinkTimer;
    if (timer == 0) return false;
    u16 code = this->_960;
    if (code == 0x19) {
        if (timer > 0x20) mSinkTimer = timer - 0x20;
        if (mSinkTimer < 0x40) mSinkTimer = 0x40;
        mMovementStates.mHighBits.mBit7 = 0;
        return true;
    }
    if (code == 0x1F) return true;
    if (timer <= 100) return false;
    if (timer > 200) {
        mSinkTimer = 128;
        changeAnimation("埋まりジャンプA");
    } else {
        mSinkTimer = 32;
        changeAnimation("埋まりジャンプB");
    }
    if (this->_960 == 0x12) playSound("毒沼脱出");
    else playSound("砂脱出");
    playSound("声砂脱出");
    return true;
}

void Mario::fn_80399E60(TVec3f* stick) {
    _42A = 0;
    MarioConst* constants = mActor->mConst;
    if (_3CE > constants->mTable[constants->mCurrentTable]->mJumpConnectTime || !(static_cast<u32>(mMovementStates.mLowBits.mBit26))) _430 = 0;
    else _430++;
    if (_430 > 2) _430 = 0;
    if (_430 == 2) {
        constants = mActor->mConst;
        if (mWalkSpeed < constants->mTable[constants->mCurrentTable]->mJumpConnectSpeed) _430 = 0;
    }
    if (getPlayerMode() == 4 || getPlayerMode() == 6) _430 = 0;
    if (isPlayerModeYoshi() && _430 > 1) _430 = 0;
    u32 flags = mFlags20.mWord;
    if ((flags >> 20) & 1) {
        _430 = 2;
        MR::start2PJumpAssistJustSound();
    } else if ((flags >> 21) & 1) MR::start2PJumpAssistSound();
    if ((isPlayerModeInvincible() || this->mCometFireDanceArea != 0) && mWalkSpeed > 1.0f) {
        _430 = 1;
        _42A = 1;
    }
    if (!MR::isNearZero(*stick, 0.001f) && mSinkTimer != 0) {
        _430 = 0;
        mMovementStates.mLowBits.mBit27 = 0;
        _3D2 = 0;
    }
}

void Mario::fn_8039A020() {
    if (this->mFlags14.mLowWord & 1) {
        if (this->mActor->fn_803BE0E0() == nullptr) {
            this->mMovementStates.mLowBits.mBit12 = 0;
        }
    } else {
        if (!this->mActor->fn_803BE0D0()) {
            this->mMovementStates.mLowBits.mBit12 = 0;
        }
    }
}

void Mario::fn_8039A090() {
    if (this->checkTrgA() || this->mActor->fn_803BE0C0() != nullptr) {
        this->_558 = this->mActor->_37C;
        if (this->isEnableFutureJump()) {
            this->mMovementStates.mHighBits.mBit7 = 1;
        }
    }
}

bool Mario::fn_8039A100() {
    if (this->_430 == 8 && this->_42A == 0) {
        u16 timer = this->_426;
        if (timer == 0 || timer > this->mActor->mConst->getTable()->mTornadoBoostTimer - 16) {
            return true;
        }
        return false;
    }
    if (((this->mMovementStates.mLowWord >> 12) & 1) || ((this->mMovementStates.mLowWord >> 17) & 1) || !this->isRising()
        || this->_430 == 2) {
        return true;
    }
    return false;
}

void Mario::fn_8039A1C0() {
    if (this->isAnimationRun("ジャンプB")) {
        this->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
    if (this->mSwim->_1B2 == 0) {
        if (this->isAnimationRun("水泳スピン移動")) {
            this->stopAnimation((const char*)nullptr, (const char*)nullptr);
        }
        if (this->isAnimationRun("水泳スピンジャンプ")) {
            this->stopAnimation((const char*)nullptr, (const char*)nullptr);
        }
        if (this->isAnimationRun("水泳ジェット")) {
            this->changeAnimation("空中一回転", (const char*)nullptr);
        }
    }
    this->_3BE += 1;
}

// NONMATCHING: temporary allocation and register scheduling.
void Mario::fn_8039A2A0() {
    f32 d = this->calcDistToCeil(false);
    if (!(d >= 160.0f)) {
        f32 g = this->cutGravityElementFromJumpVec(true);
        MR::isNearZero(mAirGravityVec, 0.001f);
        TVec3f sp20(mAirGravityVec);
        sp20 *= d;
        TVec3f sp2C(this->mPosHolder->mPos);
        sp2C -= sp20;
        this->playEffectTrans("天井ヒット", sp2C);
        f32 f31 = (this->_430 == 6 || this->_430 == 4) ? 0.9f : 0.6f;
        MR::isNearZero(mAirGravityVec, 0.001f);
        TVec3f sp8(mAirGravityVec);
        sp8 *= g;
        TVec3f sp14(sp8);
        sp14 *= f31;
        this->mJumpVec += sp14;
    }
}

bool Mario::fn_8039A3D0() {
    bool r = false;
    bool timerExpired = false;
    if (_3BC > 10) timerExpired = true;
    if (timerExpired && checkTrgA()) {
        r = true;
    }
    return r;
}

bool Mario::fn_8039A430() {
    if (!((this->mMovementStates.mLowWord >> 8) & 1)) {
        return false;
    }
    this->mMovementStates.mLowBits.mBit8 = 0;
    return this->mWall->startJump();
}

void Mario::fn_8039A460() {
    if (_544 > 5) return;
    if (_430 == 8) return;
}

// NONMATCHING: string relocation identities; complete payloads match target.
void Mario::fn_8039A480() {
    if (this->_430 == 8) {
        if (this->checkTrgZ()) {
            this->resetTornado();
            this->cancelTornadoJump();
            this->stopAnimation((const char*)nullptr, (const char*)nullptr);
            this->changeAnimationInterpoleFrame(1);
            this->mDrawStates.mBits.mBit23 = 1;
            this->playSound("声小ジャンプ", -1);
            if (this->_42A != 0) {
                this->playEffect("スピンライト消去");
            }
        } else {
            this->mMovementStates.mLowBits.mBit14 = 0;
        }
        if (this->_42A != 0) {
            u16 t = this->_402;
            if (t != 0) {
                this->_402 = t - 1;
            }
            if (this->_402 == 0) {
                this->cancelTornadoJump();
                this->playEffect("スピンライト消去");
                if (this->getPlayerMode() != 4) {
                    this->_402 = marioCurTableEntry(this)->mAirWalkTime;
                }
            }
        }
    }
}

void Mario::fn_8039A5B0() {
    if (this->beeMarioOnAir()) {
        this->_76C = this->mActor->mConst->getTable()->mBeeGravityReviveTime;
    }
}

// NONMATCHING: temporary allocation and load scheduling; verified virtual addVelocity calls.
void Mario::fn_8039A600() {
    u32 t = this->mMovementStates.mLowWord;
    if ((t >> 8) & 1) {
        MarioConst* constants = mActor->mConst;
        MarioConstTable* e = constants->mTable[constants->mCurrentTable];
        f32 v1 = (f32)_3BC / (f32)e->mSlipUpSpdCtrlTimer;
        if (v1 > 1.0f) {
            v1 = 1.0f;
        }
        f32 f2 = 1.0f - v1;
        constants = mActor->mConst;
        f32 f1 = constants->mTable[constants->mCurrentTable]->mSlipUpSpdRatio;
        f32 f29 = f1 + f2 * (1.0f - f1);
        this->addVelocity(this->mJumpVec, f29);
        f32 f30 = this->fn_8039AA40();
        f32 f31 = this->calcJumpGravity();
        TVec3f sp20(this->mActor->mAppliedGravityVec);
        sp20 *= f31;
        TVec3f sp2C(sp20);
        sp2C *= f30;
        TVec3f sp38(sp2C);
        sp38 *= f29;
        this->mJumpVec += sp38;
        this->moveWallSlide(1.0f);
        return;
    }
    if (!((t >> 30) & 1)) {
#ifdef GALAXY_NATIVE
        {
            PetariNative::Odyssey::Live::noSpinAir =
                sOdyssey.active && (sOdyssey.air.kind == PetariNative::Odyssey::Air::Dive || sOdyssey.air.kind == PetariNative::Odyssey::Air::LongJump);
            PetariNative::Odyssey::Live::airKind = sOdyssey.active ? sOdyssey.air.kind : PetariNative::Odyssey::Air::None;
            if (sOdyssey.active && _430 != sOdyssey.jumpKind) {
                sOdyssey.active = false;  // the game turned the jump into something else (hip drop, ...)
                std::fprintf(stderr, "[odyssey] jump type %d became %d after %d frames: the game's own physics from here\n",
                             static_cast< int >(sOdyssey.jumpKind), static_cast< int >(_430), sOdyssey.air.frame);
            }
            ++sOdysseyAirFrames;
            if (sOdyssey.active) {
                // This frame's velocity from the model, in the current gravity frame:
                // front/side across gravity, up against it (planets turn the frame).
                // The rest of the original step (Galaxy's gravity) is skipped, as in MG.
                TVec3f up(-mActor->mAppliedGravityVec);
                MR::normalizeOrZero(&up);
                TVec3f front(sOdyssey.front - up * up.dot(sOdyssey.front));
                MR::normalizeOrZero(&front);
                if (MR::isNearZero(front)) {
                    front = mFrontVec - up * up.dot(mFrontVec);
                    MR::normalizeOrZero(&front);
                }
                sOdyssey.front = front;
                TVec3f side;
                side.cross(up, front);
                TVec3f stick;
                calcMoveDir(mStickPos.x, mStickPos.y, &stick, true);
                const f32 stickFront = std::clamp(stick.dot(front), -1.0f, 1.0f);
                const f32 stickSide = std::clamp(stick.dot(side), -1.0f, 1.0f);
                if (sOdyssey.air.up > 0.0f && calcDistToCeil(false) < 160.0f) {
                    sOdyssey.air.up = 0.0f;  // a ceiling ends the rise
                    sOdyssey.air.extending = false;
                }
                const f32 rise = PetariNative::Odyssey::stepAir(sOdyssey.air, checkLvlA(), stickFront, stickSide);
                if (sOdyssey.air.kind == PetariNative::Odyssey::Air::Dive && isAnimationRun("幅とび")) {
                    // The dive's flight (about 33 frames) is shorter than the leap (49):
                    // play it faster, then hold its last frame until the landing.
                    XanimeFrameCtrl* ctrl = getAnimator()->getXanimePlayer()->getFrameCtrl();
                    if (ctrl != nullptr) {
                        const f32 last = static_cast< f32 >(ctrl->getEnd()) - 1.0f;
                        if (ctrl->getFrame() >= last) {
                            ctrl->setFrame(last);
                            ctrl->setRate(0.0f);
                        } else {
                            ctrl->setRate(kOdysseyDiveLeapRate);
                        }
                    }
                }
                mJumpVec = front * sOdyssey.air.front + side * sOdyssey.air.side + up * rise;
                addVelocity(mJumpVec);
                static const bool trace = std::getenv("PETARI_ODYSSEY_TRACE") != nullptr;
                if (trace) {
                    XanimePlayer* player = getAnimator()->getXanimePlayer();
                    std::fprintf(stderr,
                                 "[odyssey] frame %d kind %d rise %.3f front %.3f pos (%.3f, %.3f, %.3f) vel (%.3f, %.3f, %.3f) anim %s %.1f/%d\n",
                                 sOdyssey.air.frame, static_cast< int >(sOdyssey.air.kind), rise, sOdyssey.air.front, mPosHolder->mPos.x,
                                 mPosHolder->mPos.y, mPosHolder->mPos.z, mVelocity.x, mVelocity.y, mVelocity.z,
                                 player->getCurrentAnimationName(), getAnimator()->getFrame(),
                                 player->getFrameCtrl() != nullptr ? static_cast< int >(player->getFrameCtrl()->getEnd()) : -1);
                }
                return;
            }
        }
#endif
        this->addVelocity(this->mJumpVec);
        f32 f31 = this->fn_8039AA40();
        f32 f30 = this->calcJumpGravity();
        TVec3f sp8(this->mActor->mAppliedGravityVec);
        sp8 *= f30;
        TVec3f sp14(sp8);
        sp14 *= f31;
        this->mJumpVec += sp14;
    }
}

// NONMATCHING: explicit typed mode14 storage dispatch, register allocation and boolean lowering.
f32 Mario::calcJumpGravity() {
    u16 timer = this->_76C;
    if (timer != 0) {
        MarioConstTable* entry = this->mActor->mConst->getTable();
        f32 ratio = static_cast<f32>(timer) / static_cast<f32>(entry->mBeeGravityReviveTime);
        f32 multiplier = this->fn_8039ADC0();
        f32 gravity = (1.0f - ratio) + ratio * entry->mGravityAirWalk;
        gravity *= multiplier;
        return gravity;
    }
    if (this->_3CC != 0) {
        return this->mActor->mConst->getTable()->mGravityAirWalk;
    }
    f32 gravity;
    if (this->_430 == 0xB && (this->isRising() || this->_3BE < this->mActor->mConst->getTable()->mTrampleBegomaOpenTime)) {
        gravity = this->mActor->mConst->getTable()->mGravityJumping[0];
    } else {
        u32 mode = this->_430;
        if (mode == 8 && this->_42A == 1) {
            gravity = this->mActor->mConst->getTable()->mSpinJumpGravity;
        } else if (mode == 0xC) {
            gravity = 1.8f;
        } else if (mode == 0xD) {
            gravity = this->mActor->mConst->getTable()->mGravityJumping[9];
        } else {
            MarioConstTable* entry = this->mActor->mConst->getTable();
            // Completed mode writers produce 0..14; mode14 selects the adjacent front-speed field.
            gravity = mode == 14 ? entry->mJumpFrontSpeed : entry->mGravityJumping[mode];
        }
    }
    if (this->isPlayerModeCloud() && !this->isRising()) {
        gravity *= 0.5f;
    }
    if (this->_430 == 8 && !this->isRising()) {
        gravity *= 0.25f;
    }
    return gravity * this->fn_8039ADC0();
}

f32 Mario::fn_8039AA40() {
    if (this->getPlayerMode() == 6) {
        return 0.0f;
    }
    f32 ratio = 1.0f;
    if (this->_430 == 13 && ((static_cast<u32>(this->mMovementStates.mLowBits.mBit17)) || !this->isRising())) {
        ratio = 1.0f;
    } else if (this->fn_8039A100()) {
        ratio = this->mActor->mConst->getTable()->mGravityRatioA;
    }
    ratio *= this->mActor->getGravityRatio();
    if (this->mActor->_334 != 0) {
        ratio *= 0.3f;
        if (!this->mActor->fn_803B93E0() && this->mVerticalSpeed < 300.0f) {
            ratio = 0.0f;
            if (this->mVerticalSpeed < 200.0f) {
                this->cutGravityElementFromJumpVec(true);
                ratio = -0.1f;
            }
        }
    }
    return ratio;
}

void Mario::fn_8039AB50() {
    if (this->_430 == 13 && ((this->mMovementStates.mHighWord >> 31) & 1)) {
        return;
    }
    TVec3f sp32;
    f32 old = MR::vecKillElement(this->mJumpVec, *this->getGravityVec(), &sp32);
    f32 limit = this->getMaxDropSpeed();
    if (old >= limit) {
        this->mJumpVec = *this->getGravityVec() * limit + sp32;
    }
}

f32 Mario::getMaxDropSpeed() {
    if (this->getPlayerMode() == 4 && !this->isRising()) {
        return this->mActor->mConst->getTable()->mMaxDropSpeed[4];
    }
    if (this->_430 == 11 && !this->isRising()) {
        MarioConstTable* entry = this->mActor->mConst->getTable();
        if (this->_3BE >= entry->mTrampleBegomaOpenTime) {
            return entry->mMaxDropSpeed[3];
        }
    }
    MarioConstTable* entry = this->mActor->mConst->getTable();
    u8 level = this->mActor->getGravityLevel();
    return entry->mMaxDropSpeed[level];
}

void Mario::fn_8039ACE0() {
    if (this->calcDistToCeil(false) >= 80.0f) {
        return;
    }
    bool a = false;
    if (this->_730 != nullptr && MR::isSensorPressObj(this->_730)) {
        a = true;
    }
    bool b = false;
    if (this->mGroundPolygon->mSensor != nullptr && MR::isSensorPressObj(this->mGroundPolygon->mSensor)) {
        b = true;
    }
    if (this->_730 == this->mGroundPolygon->mSensor) {
        return;
    }
    if (a || b) {
        this->mActor->setPress(0, 0);
        this->mActor->mPressNormal6D8 = *this->mGroundPolygon->getNormal(0);
    }
}

f32 Mario::fn_8039ADC0() {
    if (!this->mActor->isEnableNerveChange()) {
        return 1.0f;
    }
    if (MR::getKarikariClingNum() == 0) {
        return 1.0f;
    }
    if (this->getPlayerMode() == 4) {
        return 1.0f;
    }
    f32 n = (f32)MR::getKarikariClingNum();
    f32 factor = n + 2.0f;
    factor *= 0.5f;
    return factor;
}

// NONMATCHING: temporary/register allocation; verified full SMG2 eligibility gate.
bool Mario::tryTurnJump() {
    if (fn_803EF7C0() == 0x20) return false;
    if (getPlayerMode() == 4) return false;
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit27)) && (_3D2 == 0 || static_cast<u8>(checkStickFrontBack()) != 2)) return false;
    MarioConstTable* table = mActor->mConst->getTable();
    if (static_cast<s32>(table->mTurnSlipTime - _3D0) < table->mTurnJumpInhibitTime) {
        _3D0 = 0;
        mMovementStates.mLowBits.mBit27 = 0;
        return false;
    }
    _430 = 0x4;

    if (_3D2 != 0) {
        TVec3f horizontal;
        MR::isNearZero(mAirGravityVec, 0.001f);
        MR::vecKillElement(_3E4, mAirGravityVec, &horizontal);
        MR::normalizeOrZero(&horizontal);

        const f32 turnJumpFrontSpeed = mActor->getConst().getTable()->mTurnJumpFrontSpeed;
        mJumpVec = -horizontal * turnJumpFrontSpeed;

        setFrontVecKeepUp(-_3E4);
        _220 = _3E4;
    } else {
        TVec3f horizontal;
        MR::isNearZero(mAirGravityVec, 0.001f);
        MR::vecKillElement(_220, mAirGravityVec, &horizontal);
        MR::normalizeOrZero(&horizontal);

        const f32 turnJumpFrontSpeed = mActor->getConst().getTable()->mTurnJumpFrontSpeed;
        mJumpVec = -horizontal * turnJumpFrontSpeed;
    }

    const f32 jumpHeight = getJumpHeight();
    mJumpVec += -mActor->mAppliedGravityVec * jumpHeight;

    mFlags14.mLowBits.mBit8 = 0;
    if ((static_cast<u32>(mDrawStates.mBits.mBit20))) {
        recordJumpEnforceMove();
        doEnforceJump(1.0f);
    }

    mMovementStates.mLowBits.mBit17 = 1;
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    if (odysseyOn()) {
        // SMO's sideflip: up 32, gravity 1.0, 9 along the new heading.
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::Sideflip, mJumpVec, up, 0.0f, _430);
        sOdyssey.lastChain = -1;
    }
#endif
    procJump(true);
    changeAnimation("ターンジャンプ", "落下");
    playSound("後ジャンプ");
    playSound("ジャンプ踏切");
    playEffect("共通ハイジャンプ");
    if (isPlayerModeYoshi()) MR::startActionSound(mActor->getYoshi(), "JumpHigh", -1, -1, -1);
    else playSound("声後ジャンプ");

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    _3D0 = 0;
    mWalkSpeed = 0.0f;
    mFlags14.mLowBits.mBit21 = 0;
    return true;
}

void Mario::trySquatJump() {
    mMovementStates.mHighBits.mBit30 = 0;
    _430 = 0x5;

    mJumpVec = getAirFrontVec() * mActor->getConst().getTable()->mSquatJumpFrontSpeed;

    TVec3f stickVec(_190 + _19C);
    if (!MR::isNearZero(stickVec)) {
        addVelocity(-stickVec);

        MR::vecKillElement(stickVec, mActor->mAppliedGravityVec, &stickVec);
        cutGravityElementFromJumpVec(true);

        mJumpVec += stickVec * 1.0f;
    }

    const f32 jumpHeight = getJumpHeight();
    mJumpVec += -mActor->mAppliedGravityVec * jumpHeight;

    mFlags14.mLowBits.mBit8 = 0;
    if ((static_cast<u32>(mDrawStates.mBits.mBit20))) {
        recordJumpEnforceMove();

        TVec3f moveKeep;
        f32 gravityMove = MR::vecKillElement(_184, getAirGravityVec(), &moveKeep);

        f32 enforceScale = 1.0f - (MR::diffAngleAbs(mFrontVec, moveKeep) / 1.5707964f);
        if (enforceScale < 0.0f) {
            enforceScale = 0.0f;
        }

        const f32 jumpMag = mJumpVec.length();
        const f32 jumpScale = MR::clamp(1.0f - ((jumpMag - 4.0f) / 10.0f), 0.0f, 1.0f);
        enforceScale += jumpScale;
        enforceScale = MR::clamp(enforceScale, 0.0f, 1.0f);

        if (mJumpVec.dot(moveKeep) < 0.0f) {
            mDrawStates.mBits.mBit4 = 1;
            initActiveJumpVec();
        }

        mVelocity -= moveKeep;

        mJumpVec += moveKeep * enforceScale;

        if (gravityMove < 0.0f) {
            mJumpVec += getAirGravityVec() * gravityMove;
        }

        invalidateRelativePosition();
    }

    mMovementStates.mLowBits.mBit17 = 1;
    mMovementStates.mLowBits.mBit30 = 0;
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    if (odysseyOn()) {
        // SMO's long jump: up 12, gravity 0.48, start min(speed + 4, 14), cap 23.
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::LongJump, getAirFrontVec(), up,
                     mWalkSpeed * mActor->mConst->getTable()->mWalkSpeed, _430);
        sOdyssey.lastChain = -1;
    }
#endif
    procJump(true);
    changeAnimationNonStop("幅とび");
    changeAnimation(nullptr, "落下");
    playSound("幅ジャンプ");
    playSound("声幅ジャンプ");
    playSound("ジャンプ踏切");

    mMovementStates.mLowBits.mBit31 = 1;
    _3D0 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    return;
}

void Mario::tryBackJump() {
    mMovementStates.mHighBits.mBit30 = 1;
    mMovementStates.mLowBits.mBit21 = 0;
    _430 = 0x6;

    const f32 backSpeed = mActor->getConst().getTable()->mSquatJumpBackSpeed;
    mJumpVec = -getAirFrontVec() * backSpeed;

    const f32 jumpHeight = getJumpHeight();
    mJumpVec += -mActor->mAppliedGravityVec * jumpHeight;

    mFlags14.mLowBits.mBit8 = 0;
    if ((static_cast<u32>(mDrawStates.mBits.mBit20))) {
        recordJumpEnforceMove();
        doEnforceJump(1.0f);
    }

    mMovementStates.mLowBits.mBit17 = 1;
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    if (odysseyOn()) {
        // SMO's backflip: up 32, gravity 1.0, 5 backwards (reference: the facing).
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::Backflip, getAirFrontVec(), up, 0.0f, _430);
        sOdyssey.lastChain = -1;
    }
#endif
    procJump(true);
    changeAnimation("しゃがみジャンプ", "落下");
    playSound("後ジャンプ");
    playSound("ジャンプ踏切");
    playEffect("共通ハイジャンプ");
    if (isPlayerModeYoshi()) {
        MR::startActionSound(mActor->getYoshi(), "JumpHigh", -1, -1, -1);
    } else {
        playSound("声後ジャンプ");
    }

    if (mSwim->checkUnderWaterFull(-mFrontVec) > 500.0f && mStickPos.z == 0.0f) {
        changeAnimationNonStop("後方飛び込みジャンプ");
        if (!isPlayerModeYoshi()) {
            playSound("声高飛び込み");
        }
        playSound("ジャンプ踏切");
    }

    mMovementStates.mLowBits.mBit31 = 1;
    _3D0 = 0;
    mMovementStates.mLowBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    mWalkSpeed = 0.0f;
    return;
}

void Mario::tryTornadoJump() {
    _430 = 0x8;
    _42A = 0;

    const f32 frontSpeed = mWalkSpeed;
    const f32 tornadoSpeed = mActor->getConst().getTable()->mJumpTornadoSpeed;

    mJumpVec = mFrontVec * frontSpeed * tornadoSpeed;

    const f32 jumpHeight = getJumpHeight();
    mJumpVec += -mActor->mAppliedGravityVec * jumpHeight;

    mMovementStates.mLowBits.mBit17 = 0;
    mMovementStates.mLowBits.mBit16 = 1;
    procJump(true);
    changeAnimation("ジャンプ", "落下");
    playSound("トルネードジャンプ");
    if (isPlayerModeYoshi()) {
        MR::startActionSound(mActor->getYoshi(), "HeadShake", -1, -1, -1);
    } else {
        playSound("声スピン");
    }

    mMovementStates.mLowBits.mBit31 = 1;
    _3D0 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit2 = 0;
    mWalkSpeed = 0.0f;

    _402 = mActor->getConst().getTable()->mAirWalkTimeTornado;
    _424 = mActor->getConst().getTable()->mTornadoZeroGravityTimer;

    const f32 boosterPower = mActor->getConst().getTable()->mTornadoBoostPower;
    setRocketBooster(-(*getGravityVec()) * boosterPower, mActor->getConst().getTable()->mTornadoBoostAttn,
                     mActor->getConst().getTable()->mTornadoBoostTimer);
}

void Mario::startTornadoCentering(HitSensor* pSensor) {
    pushTask(&Mario::taskOnTornadoCentering, 0x400);
    mTornadoCenterSensor = pSensor;
    _A34 = 0x1E;
}

bool Mario::taskOnTornadoCentering(u32 a1) {
    if (_A34 != 0) {
        TVec3f sensorOffset(mTornadoCenterSensor->mHost->mPosition - mPosHolder->mPos);
        MR::vecKillElement(sensorOffset, *getGravityVec(), &sensorOffset);

        f32 mag = sensorOffset.length();
        if (mag < 10.0f) {
            mag = 10.0f;
        }

        f32 ratio = 10.0f / mag;
        if (ratio < 0.1f) {
            ratio = 0.1f;
        }

        addTrans(sensorOffset * ratio);

        _A34--;

        if (_A34 == 0) {
            return false;
        }

        return true;
    }

    return false;
}

// NONMATCHING: literal pool offsets; full string arguments verified.
void Mario::trySpinJump(u8 a1) {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return;
    }
#ifdef GALAXY_NATIVE
    // OdysseyMovement: no spin out of the mod's dive or long jump (as in SMO).
    if (sOdyssey.active && (sOdyssey.air.kind == PetariNative::Odyssey::Air::Dive ||
                            sOdyssey.air.kind == PetariNative::Odyssey::Air::LongJump)) {
        return;
    }
#endif

    if (_430 == 8) {
        return;
    }

    if (isStatusActive(0x17)) {
        return;
    }

    if (getPlayerMode() == 4) {
        return;
    }

    if (getPlayerMode() == 6) {
        return;
    }

    if (getPlayerMode() == 7 && mVerticalSpeed > 100.0f) {
        tryStartFoo();
        return;
    }

    if (isAnimationRun("水泳スピン移動")) {
        return;
    }

    if (isAnimationRun("水泳スピンジャンプ")) {
        return;
    }

#ifdef GALAXY_NATIVE
    if (std::getenv("PETARI_CAPPY_TRACE") != nullptr) {
        std::fprintf(stderr, "[cappy] mario spin (air) starts\n");
    }
#endif
    _430 = 0x8;
    _42A = a1 + 1;

    const f32 frontSpeed = mWalkSpeed;
    const f32 tornadoSpeed = mActor->getConst().getTable()->mJumpTornadoSpeed;
    mJumpVec = mFrontVec * frontSpeed * tornadoSpeed;

    const f32 spinJumpHeight = mActor->getConst().getTable()->mSpinJumpHeight;
    mJumpVec += -mActor->mAppliedGravityVec * spinJumpHeight;

    mMovementStates.mLowBits.mBit17 = 0;
    _402 = mActor->getConst().getTable()->mAirWalkTimeSpin;
    procJump(true);

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        stopAnimationUpper(nullptr);
        if (isPlayerModeCloud()) {
            changeAnimation("雲スピン空中", "落下");
        } else {
            changeAnimation("空中ひねり", "落下");
        }
        playSound("スピンジャンプ");
        if (isPlayerModeYoshi()) {
            MR::startActionSound(mActor->getYoshi(), "HeadShake", -1, -1, -1);
        } else {
            playSound("声スピン");
        }
        startPadVib(2);

        mMovementStates.mLowBits.mBit31 = 1;
        mMovementStates.mLowBits.mBit27 = 0;
        mMovementStates.mLowBits.mBit26 = 0;
        mMovementStates.mLowBits.mBit2 = 0;
        mFlags14.mLowBits.mBit6 = 1;
        _3D0 = 0;
        mWalkSpeed = 0.0f;
        mFlags14.mLowBits.mBit30 = 1;

        tryShootStickingPopo();

        if (isStatusActive(MarioStatus_Rabbit)) {
            mRabbit->hop();
        }
    }
}

void Mario::tryForceJumpDelay(const TVec3f& rVec) {
    mMovementStates.mHighBits.mBit18 = 1;
    _304 = rVec;
}

void Mario::tryFreeJumpDelay(const TVec3f& rVec) {
    mMovementStates.mHighBits.mBit17 = 1;
    _304 = rVec;
}

void Mario::tryForceJump(const TVec3f& rVec, bool a2) {
    if (rVec.dot(*getGravityVec()) > 0.0f) {
        mMovementStates.mHighBits.mBit31 = 1;
    }

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    { const u32 flagValue = static_cast<u32>(a2); mMovementStates.mLowBits.mBit17 = flagValue; }
    mMovementStates.mLowBits.mBit20 = 0;
    mJumpVec = rVec;

    if (getPlayerMode() == 4 || getPlayerMode() == 6) {
        _16C = mJumpVec;
    }

    resetTornado();

    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    mMovementStates.mLowBits.mBit12 = 1;
    mMovementStates.mLowBits.mBit14 = 0;
    mFlags14.mLowBits.mBit23 = 0;
    _3BE = 0;
    _344 = mSideVec;
    _4B0 = mPosHolder->mPos;
    _42C = 0;
    _76C = 0;
    procJump(false);
    changeAnimation(nullptr, "落下");
    mRabbit->forceJump();
    return;
}

void Mario::tryForceFreeJump(const TVec3f& rVec) {
    mMovementStates.mLowBits.mBit25 = 0;
    _430 = 0xC;
    mMovementStates.mHighBits.mBit21 = 1;
    mMovementStates.mLowBits.mBit22 = 0;

    tryForceJump(rVec, true);
    _3BC = 0;
    mMovementStates.mLowBits.mBit14 = 0;
    _402 = mActor->getConst().getTable()->mAirWalkTime;

    if (getPlayerMode() == 4) {
        playSound("ハチ体力完全回復");
    }

    _76C = 0;
    return;
}

void Mario::tryForcePowerJump(const TVec3f& rVec, bool a2) {
    mMovementStates.mLowBits.mBit25 = 0;
    _430 = 0xD;
    mMovementStates.mHighBits.mBit21 = 1;
    mMovementStates.mLowBits.mBit22 = 0;
    const bool _2FPrev = (static_cast<u32>(mMovementStates.mHighBits.mBit16));
    mMovementStates.mHighBits.mBit16 = 1;
    tryForceJump(rVec, true);
    { const u32 flagValue = static_cast<u32>(_2FPrev); mMovementStates.mHighBits.mBit16 = flagValue; }
    _3BC = 0;
    mMovementStates.mLowBits.mBit14 = 0;

    if (!a2) {
        _402 = mActor->getConst().getTable()->mAirWalkTime;

        if (getPlayerMode() == 4) {
            playSound("ハチ体力完全回復");
        }
    }

    _76C = 0;
    return;
}

void Mario::tryFreeJump(const TVec3f& rVec, bool a2) {
    _430 = 0;

    if (rVec.dot(*getGravityVec()) > 0.0f) {
        mMovementStates.mHighBits.mBit31 = 1;
    }

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit26 = 0;
    { const u32 flagValue = static_cast<u32>(a2); mMovementStates.mLowBits.mBit17 = flagValue; }
    mMovementStates.mLowBits.mBit20 = 0;
    mJumpVec = rVec;
    resetTornado();

    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    mMovementStates.mLowBits.mBit12 = 1;
    mMovementStates.mLowBits.mBit14 = 0;
    _3BE = 0;
    _76C = 0;
    _344 = mSideVec;
    procJump(false);
    changeAnimation(nullptr, "落下");
    mRabbit->forceJump();
    return;
}

void Mario::tryWallJump(const TVec3f& rVec, bool a2) {
    _430 = 0x7;

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    mMovementStates.mLowBits.mBit20 = 0;
    mMovementStates.mLowBits.mBit25 = 0;
    mMovementStates.mHighBits.mBit23 = 0;
    mFlags24.mHighBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit22 = 0;

    if (a2) {
        mMovementStates.mLowBits.mBit22 = 1;
    } else {
        mMovementStates.mLowBits.mBit22 = 0;
    }

    mJumpVec = rVec;
    _4B0 = mPosHolder->mPos;
    _3BC = 0;
    mMovementStates.mLowBits.mBit12 = 1;
    mMovementStates.mLowBits.mBit14 = 0;
    _3CA = 0;
    _3CC = 0;
    _3BE = 0;
    mMovementStates.mLowBits.mBit2 = 1;

    if (getPlayerMode() == 5) {
        changeAnimationNonStop("ホッパー壁ジャンプ");
        playSound("ホッパージャンプ");
    } else {
        changeAnimation("壁ジャンプ", "落下");
        playSound("ジャンプ踏切");
    }

    playSound("声小ジャンプ");

    if (a2) {
        setFrontVecKeepUp(rVec);
    }

    stopWalk();
    mRabbit->forceJump();
#ifdef GALAXY_NATIVE
    sOdyssey.active = false;
    if (odysseyOn()) {
        // SMO's wall jump: 8.6 away from the wall, 23 up, gravity 0.95, the stick
        // ignored for 25 frames. Galaxy's wall gives the direction (rVec).
        TVec3f up(-mActor->mAppliedGravityVec);
        MR::normalizeOrZero(&up);
        odysseyStart(PetariNative::Odyssey::Air::WallJump, rVec, up, 0.0f, _430);
        sOdyssey.lastChain = -1;
        petariCappyWallJump();
        sOdysseyAirFrames = 0;  // the wall jump's own press does not dive
    }
#endif
    return;
}

void Mario::tryStickJump(const TVec3f& rVec) {
    stopJump();
    initJumpParam();

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    mMovementStates.mLowBits.mBit20 = 0;
    _430 = 0;
    mFlags24.mHighBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit22 = 0;
    mMovementStates.mLowBits.mBit25 = 0;
    mMovementStates.mLowBits.mBit22 = 1;
    mJumpVec = rVec;
    _3CA = 0;
    _3CC = 0;
    _3BE = 0;
    mMovementStates.mLowBits.mBit12 = 1;
    mMovementStates.mLowBits.mBit14 = 0;
    mMovementStates.mLowBits.mBit2 = 1;
    _76C = mActor->getConst().getTable()->mBeeGravityReviveTime;
    _770 = 0.0f;
    _3BC = 0xA;
    playSound("ハチ壁ジャンプ");
    playSound("声小ジャンプ");
    setFrontVecKeepUp(rVec);
    changeAnimationNonStop("ハチ壁ジャンプ");
    return;
}

void Mario::trySlipUpJump() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isPlayerModeYoshi()) {
        mMovementStates.mLowBits.mBit25 = 0;
        mMovementStates.mHighBits.mBit23 = 0;
        return;
    }

    changeAnimation("スリップアップ");
    playSound("スリップアップ");
    playEffect("スリップアップ");
    startPadVib("マリオ[スリップアップ]");

    mJumpVec = -getAirGravityVec() * mActor->getConst().getTable()->mSlipUpHeight;

    f32 ratio = 1.0f;
    TVec3f predictPos(mPosHolder->mPos - *getGravityVec() * 100.0f);

    TVec3f frontVec;
    MR::vecKillElement(mJumpVec, mFrontVec, &frontVec);
    MR::vecKillElement(frontVec, *getGravityVec(), &frontVec);
    frontVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFront;
    MR::normalizeOrZero(&frontVec);

    f32 step = 50.0f;
    u32 hitCount;
    TVec3f hitPos;
    for (hitCount = 0; hitCount < 10; hitCount++) {
        Triangle* pTmpPolygon = fn_803EFA20();
        predictPos += frontVec * step;

        if (!fn_803B2B50(predictPos, *getGravityVec() * 150.0f, &hitPos, pTmpPolygon)) {
            break;
        }
    }

    ratio *= static_cast< f32 >(hitCount) / 10.0f;
    cutVecElementFromJumpVec(mFrontVec);

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit22))) {
        mJumpVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFront * ratio;
        mMovementStates.mHighBits.mBit22 = 1;
    } else {
        mJumpVec += mFrontVec * mActor->getConst().getTable()->mSlipUpFrontWeak * ratio;
    }

    mMovementStates.mLowBits.mBit25 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    initJumpParam();
    _430 = 0xA;
    return;
}

void Mario::tryHangSlipUp() {
    mMovementStates.mLowBits.mBit26 = 0;
    getPlayer()->tryJump();
    changeAnimation("つかまりスリップアップ");
    playSound("スリップアップ");
    playEffect("スリップアップ");
    startPadVib("マリオ[スリップアップ]");

    const f32 slipUpHeight = mActor->getConst().getTable()->mSlipUpHeightHang;
    mJumpVec = -getAirGravityVec() * slipUpHeight;
    cutVecElementFromJumpVec(mFrontVec);

    const f32 slipUpFront = mActor->getConst().getTable()->mSlipUpFrontHang;
    mJumpVec += mFrontVec * slipUpFront;

    mMovementStates.mLowBits.mBit25 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    _430 = 0xA;
    mActor->setBlendMtxTimer(0x8);
    mTargetWalkSpeedIndex = 0x7;
    mWalkSpeed = 0.6f;
    return;
}

void Mario::tryDrop() {
    if (((static_cast<u32>(mMovementStates.mLowBits.mBit23))) && !(static_cast<u32>(mDrawStates.mBits.mBit20))) {
        TVec3f horizontal;
        MR::vecKillElement(_4E8 - mGroundPos, *getGravityVec(), &horizontal);

        if (horizontal.dot(*mFrontWallTriangle->getNormal(0)) < 0.0f && horizontal.length() < 80.0f) {
            getPlayer()->mMovementStates.mLowBits.mBit30 = 1;
            push(*mFrontWallTriangle->getNormal(0) * (80.0f - horizontal.length()) * 0.1f);
            return;
        }
    }

    if (((static_cast<u32>(mMovementStates.mLowBits.mBit5))) && !(static_cast<u32>(mDrawStates.mBits.mBit20))) {
        TVec3f horizontal;
        MR::vecKillElement(_500 - mGroundPos, *getGravityVec(), &horizontal);

        if (horizontal.dot(*mSideWallTriangle->getNormal(0)) < 0.0f && horizontal.length() < 80.0f) {
            push(*mSideWallTriangle->getNormal(0) * (80.0f - horizontal.length()) * 0.1f);
            getPlayer()->mMovementStates.mLowBits.mBit30 = 1;
            return;
        }
    }

    mMovementStates.mHighBits.mBit30 = 1;
    mMovementStates.mLowBits.mBit20 = 0;

    mJumpVec = mFrontVec * mWalkSpeed * ::cDropFrontSpeed;

    if (MR::isNearZero(_8F8) && ((static_cast<u32>(mMovementStates.mHighBits.mBit28)))) {
        TVec3f side;
        side.cross(getAirGravityVec(), _368);
        MR::normalizeOrZero(&side);
        _8F8.cross(_368, side);
        MR::normalizeOrZero(&_8F8);
    }

    if (!MR::isNearZero(_8F8)) {
        mJumpVec = _8F8;
        cutGravityElementFromJumpVec(false);

        if (_16C.length() > 20.0f) {
            mJumpVec += -mActor->fn_803B8520() * 10.0f;
        } else {
            f32 gravScale = mActor->mBinderDirection5DC.length();
            if (gravScale > 8.0f) {
                gravScale = 8.0f;
            }

            mJumpVec += -mActor->fn_803B8520() * gravScale;
            _8F8.zero();
        }
    }

    TVec3f carryVec(_184);
    MR::vecKillElement(carryVec, *getGravityVec(), &carryVec);
    invalidateRelativePosition();

    TVec3f groundDeltaNoGravity;
    MR::vecKillElement(mGroundPos - mPosHolder->mPos, *getGravityVec(), &groundDeltaNoGravity);

    if (carryVec.dot(groundDeltaNoGravity) > 0.0f) {
        carryVec = carryVec * 0.1f;
    }

    mJumpVec += carryVec;

    _340 = 1.0f - mWalkSpeed;
    if (_340 < 0.0f) {
        _340 = 0.0f;
    }

    _430 = 0;
    procJump(true);

    mMovementStates.mLowBits.mBit31 = 1;
    mMovementStates.mLowBits.mBit26 = 0;
    mMovementStates.mLowBits.mBit17 = 1;
    mFlags14.mHighBits.mBit28 = 0;
    mMovementStates.mLowBits.mBit20 = 0;

    if (isAnimationRun("坂すべり上向きうつぶせ", 2) || isAnimationRun("坂すべり下向きあおむけ", 3)) {
        _428 = 0xF;
    }

    changeAnimation(nullptr, "落下");

    if (getPlayerMode() == 4) {
        _408 = mActor->getConst().getTable()->mBeeGravityPowerTimeD;
        _3BC = mActor->getConst().getTable()->mBeeAirWalkInhibitTimeD - 5;
    }

    if (isAnimationRun("ショート着地")) {
        mMovementStates.mHighBits.mBit28 = 1;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        stopWalk();
    }

    return;
}

void Mario::initActiveJumpVec() {
    _2EC = getAirGravityVec();
    MR::vecKillElement(mJumpVec, _2EC, &_2E0);

    if (MR::normalizeOrZero(&_2E0)) {
        _2E0 = mFrontVec;
    }
}

void Mario::fn_8039CFD0() {
    this->_4B0 = this->mPosHolder->mPos;
}

void Mario::initJumpParam() {
    const u32 flags1 = mFlags20.mWord;
    mMovementStates.mLowBits.mBit12 = 1;

    if (((static_cast<u32>(mFlags20.mBits.mBit21))) || ((static_cast<u32>(mFlags20.mBits.mBit20)))) {
        mFlags14.mLowBits.mBit0 = 1;
    }

    mMovementStates.mHighBits.mBit7 = 0;
    mFlags14.mLowBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit16 = 0;
    mFlags14.mLowBits.mBit17 = 0;

    if (!(static_cast<u32>(mDrawStates.mBits.mBit4))) {
        initActiveJumpVec();
    }

    _42C = 0;
    _3BC = 0;
    _3CA = 0;
    _3CC = 0;
    _774 = 0;
    mMovementStates.mLowBits.mBit14 = 0;
    _3BE = 0;
    _344 = mSideVec;

    _408 = 0;
    _76C = 0;
    mMovementStates.mLowBits.mBit19 = 0;
    mMovementStates.mLowBits.mBit15 = 0;
    mMovementStates.mLowBits.mBit27 = 0;
    mMovementStates.mHighBits.mBit31 = 0;
    mFlags14.mLowBits.mBit19 = 0;
    _3D2 = 0;
    _3D0 = 0;
    _3CE = 0;
    { const u32 flagValue = static_cast<u32>((static_cast<u32>(mMovementStates.mLowBits.mBit23))); mMovementStates.mHighBits.mBit21 = flagValue; }

    if (_430 != 5) {
        cancelSquatMode();
    }

    _4B0 = mPosHolder->mPos;
    _426 = 0;

    if (mSinkTimer != 0) {
        mJumpVec.scale(0.2f + (0.8f * ((0x100 - mSinkTimer) / 256.0f)));
        mSinkTimer = 0;
    }
}

bool Mario::isEnableFutureJump() const {
    if (_430 == 0x3) {
        return false;
    }

    if (!isRising()) {
        const f32 dot = mJumpVec.dot(*getGravityVec());
        const s16 limit = mActor->getConst().getTable()->mFutureJumpReqLimitTime;
        if (mVerticalSpeed < dot * static_cast< f32 >(limit)) {
            return true;
        }
    }

    return false;
}

void Mario::procJump(bool a1) {
    if((static_cast<u32>(mFlags14.mHighBits.mBit21))) fn_803B12B0();
    if((static_cast<u32>(mMovementStates.mLowBits.mBit20))) { procHipDrop();return; }
    if(_424!=0) { _424--;return; }
    procRocketBooster();
    if((static_cast<u32>(mFlags14.mLowBits.mBit12))) fixFrontVecByGravity();
    if(isPlayerModeYoshi()) {
        if(checkSquat(true)) {
            if(isAnimationRun("カリカリ限界")) stopAnimationUpper(nullptr,nullptr);
            stopAnimation(nullptr,"しゃがみ基本");
            mMovementStates.mLowBits.mBit21 = 1;
            mFlags14.mHighBits.mBit21 = 0;mFlags14.mHighBits.mBit20 = 1;
        } else if(isAnimationRun("しゃがみ基本")) {
            mMovementStates.mLowBits.mBit21 = 0;
            stopAnimation(nullptr,"落下");
        }
    } else if(isAnimationRun("しゃがみ基本")) {
        mMovementStates.mLowBits.mBit21 = 0;
        stopAnimation(nullptr,"落下");
    }
    fn_8039A020();fn_8039A090();
    if(a1) initJumpParam();
    else {
        if(mActor->fn_803BDE10() && jumpToHipDrop()) return;
        if(mActor->fn_803B9920() && fn_803B1540()) return;
    }
    if(isRising()) { fn_8039A2A0();mJumpFallTimer=0; }
    else {
        fn_8039A1C0();
        mJumpFallTimer++;
        if(mJumpFallTimer>65) MR::startActionSound(mActor,"FallWind",-1,-1,-1);
    }
    if(_3CC) _3CC--;
    if(fn_8039A3D0()) {
        if(fn_8039A430()) return;
        fn_8039A460();
    }
    mMovementStates.mLowBits.mBit13 = 0;
    fn_8039A480();fn_8039A5B0();
    if(_76C) _76C--;
    fn_8039A600();checkWallRising();checkWallJumpHit();
    if(!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) fixFrontVecFromUpSide();
    decideSlipUp();fn_8039AB50();
#ifdef GALAXY_NATIVE
    if(((mMovementStates.mLowWord >> 30) & 1U) && !a1) {
        sOdysseyLanded = sOdyssey.active ? sOdyssey.air.kind : PetariNative::Odyssey::Air::None;
        sOdyssey.active = false;
        sCappyDiveReady = false;
        sOdysseyAirFrames = 0;
        sOdysseyAirDived = false;
        PetariNative::Odyssey::Live::noSpinAir = false;
        PetariNative::Odyssey::Live::airKind = PetariNative::Odyssey::Air::None;
        doLanding();fn_8039ACE0();
    }
    else doAirWalk();
#else
    if(((mMovementStates.mLowWord >> 30) & 1U) && !a1) { doLanding();fn_8039ACE0(); }
    else doAirWalk();
#endif
    _3BC++;
}

void Mario::checkWallRising() {
}

void Mario::checkWallJumpHit() {
    if (_430 == 0x5) {
        if (((static_cast<u32>(mMovementStates.mLowBits.mBit23))) && !checkWallJumpCode()) {
            const f32 wallDot = cutVecElementFromJumpVec(getWallNorm());
            if (checkWallCode("NoAction", true)) {
                return;
            }

            mJumpVec -= getWallNorm() * wallDot;


            blown(mJumpVec * 0.2f);

            TVec3f horizontal;
            MR::vecKillElement(mJumpVec, *getGravityVec(), &horizontal);
            MR::normalizeOrZero(&horizontal);
            if (!MR::isNearZero(horizontal)) {
                setFrontVecKeepUp(-horizontal);
            }

            mMovementStates.mHighBits.mBit20 = 1;
            _402 = 0;
            _428 = 0x3C;
        }

        return;
    }

    if (!isRising() && _3BC > 10 && ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) && calcPolygonAngleD(mFrontWallTriangle) < 80.0f) {
        if (((static_cast<u32>(mFlags14.mLowBits.mBit17))) == 0) {
            mJumpVec = *mFrontWallTriangle->getNormal(0) * 5.0f + getAirGravityVec() * cutGravityElementFromJumpVec(true);
            mFlags14.mLowBits.mBit17 = 1;
            changeAnimation("壁はじき");
        } else {
            const f32 jumpMag = mJumpVec.length();
            cutVecElementFromJumpVec(*mFrontWallTriangle->getNormal(0));
            mJumpVec.setLength(jumpMag);
        }
    }
}

void Mario::decideSlipUp() {
    if (isAnimationRun("壁はじき")) {
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit22))) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit16))) {
        mMovementStates.mLowBits.mBit25 = 0;
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit25))) {
        if (checkWallCode("NotWallSlip", true)) {
            return;
        }

        if (checkWallCode("NoAction", true)) {
            return;
        }

        bool slipFront = false;
        if (isStickOn()) {
            const TVec3f& worldPadDir = getWorldPadDir();
            if (worldPadDir.dot(getWallNorm()) < -0.8f) {
                slipFront = true;
            }
        }

        if (_430 == 0x7) {
            slipFront = true;
        }

        if (((static_cast<u32>(mMovementStates.mHighBits.mBit21))) == 0 && ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) && isRising() && slipFront) {
            mMovementStates.mLowBits.mBit25 = 1;
        }

        if (isRising() && getPlayerMode() != 4 && getPlayerMode() != 6 && getPlayerMode() != 5) {
            if (_4E0 < mActor->getConst().getTable()->mSlipUpContinueHeight) {
                if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && (static_cast<u32>(mMovementStates.mLowBits.mBit22)) && slipFront && (static_cast<u32>(mMovementStates.mHighBits.mBit22))) {
                    mMovementStates.mLowBits.mBit25 = 1;
                    cutGravityElementFromJumpVec(true);

                    mJumpVec += -getAirGravityVec() * 30.0f;
                }
            }
        }
    } else if (!(static_cast<u32>(mMovementStates.mLowBits.mBit24)) && isRising() && !(static_cast<u32>(mMovementStates.mLowBits.mBit23)) && ((static_cast<u32>(mMovementStates.mLowBits.mBit10)))) {
        trySlipUpJump();
    }
}

void Mario::moveWallSlide(f32 a1) {
    TVec3f crossVec;
    const TVec3f& wallNorm = getWallNorm();
    crossVec.cross(*getGravityVec(), wallNorm);

    if (!MR::normalizeOrZero(&crossVec) && isStickOn()) {
        f32 dot = crossVec.dot(getWorldPadDir());
        const f32 absDot = MR::abs(dot);
        if (absDot > 0.2f) {
            if (dot < -0.2f) {
                dot = (0.2f + dot) / 0.8f;
            } else {
                dot = (dot - 0.2f) / 0.8f;
            }

            addVelocity(crossVec, (10.0f * dot) * a1);
        }
    }
}

bool Mario::jumpToHipDrop() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return false;
    }

    switch (_430) {
    case 5:
        return false;

    case 9:
    case 11:
    case 12:
    case 13:
        if (getPlayerMode() != 4 && isRising()) {
            return false;
        }
    }

    if (mVerticalSpeed < mActor->getConst().getTable()->mHipDropLimitHeight) {
        return false;
    }

    if (MR::isNearZero(mActor->mAppliedGravityVec)) {
        return false;
    }

    if (getPlayerMode() == 6) {
        return false;
    }

    bool isNormalDrop = false;
    if (mActor->_3E5) {
        isNormalDrop = true;
    }

    resetTornado();
    cancelTornadoJump();

    mDrawStates.mBits.mBit23 = 1;
    mMovementStates.mHighBits.mBit30 = 0;
    mMovementStates.mLowBits.mBit20 = 1;
    mJumpVec.zero();
    _422 = 0;

    if (_430 == 4) {
        setFrontVecKeepUp(-_220);
        _430 = 0;
    }

    if (isAnimationRun("カリカリ限界")) {
        stopAnimationUpper(nullptr);
    }

    if (isPlayerModeHopper()) {
        _720 = getAnimationStringPointer("ホッパーヒップドロップ開始");
        _724 = getAnimationStringPointer("ホッパーヒップドロップ");
        _728 = getAnimationStringPointer("ヒップドロップ着地");
        startHipDropBlur();
    } else if (getPlayerMode() == 4) {
        _720 = getAnimationStringPointer("ハチヒップドロップ開始");
        _724 = getAnimationStringPointer("ハチヒップドロップ");

        if (mActor->mBeeWallWalk != 0) {
            _728 = getAnimationStringPointer("ハチヒップドロップ壁着地");
        } else {
            _728 = getAnimationStringPointer("ハチヒップドロップ着地");
        }
    } else if (isNormalDrop) {
        _720 = getAnimationStringPointer("スピンヒップドロップ開始");
        _724 = getAnimationStringPointer("スピンヒップドロップ");
        _728 = getAnimationStringPointer("スピンヒップドロップ着地");
        stopEffect("スピンライト");
        mFlags14.mHighBits.mBit24 = 1;
    } else {
        _720 = getAnimationStringPointer("ヒップドロップ開始");
        _724 = getAnimationStringPointer("ヒップドロップ");
        _728 = getAnimationStringPointer("ヒップドロップ着地");
        startHipDropBlur();
    }

    changeAnimation(_720, _724);
    playSound("尻ドロップ回転");

    if (isNormalDrop) {
        playSound("声スピン尻ドロップ");
        playSound("スピン尻ドロップ回転");

    } else {
        playSound("声尻ドロップ");
    }

    _424 = mActor->getConst().getTable()->mHipDropZeroGrTime;
    mRabbit->forceJump();
    mMovementStates.mHighWord &= 0xFFFFFFFCU;
    return true;
}

void Mario::procHipDrop() {
    f32 gravityHipDrop;
#ifdef GALAXY_NATIVE
    if (odysseyOn() && getPlayerMode() == 0 && !((mMovementStates.mLowWord >> 30) & 1U) &&
        (PetariNative::Odyssey::Live::spinBuffer > 0 || mActor->isRequestSpin())) {
        PetariNative::Odyssey::Live::spinBuffer = 0;
        // SMO's dive: Spin during a ground pound in the air (wind-up or fall).
        odysseyStartDive(*this, "ground pound + Spin");
        return;
    }
    // "Jump during a ground pound dives": a new Jump press during the pound in the
    // air, once per airtime (a cap bounce gives it back), unless the pound lands
    // within the jump pre-input window: then it stays the ground-pound jump.
    if (odysseyOn() && petari_mod_enabled(kModOdysseyJumpDive) && getPlayerMode() == 0 && !((mMovementStates.mLowWord >> 30) & 1U) &&
        !isSwimming() && !sOdysseyAirDived && checkTrgA() && !odysseyGroundPoundLandingSoon(*this)) {
        odysseyStartDive(*this, "ground pound + Jump");
        return;
    }
#endif
    if (isAnimationRun(_720)) {
        if (!isAnimationTerminate(nullptr)) {
            return;
        }

        if (_424 != 0) {
            _424--;
            return;
        }

        stopAnimation(nullptr);
        return;
    }

    if (_424 != 0) {
        _424--;
        return;
    }

    if (mActor->_38C != 0) {
        return;
    }

    if (MR::isNearZero(mActor->mAppliedGravityVec)) {
        mMovementStates.mLowWord |= Mario::MovementStates::DigitalJump;
    }

    if (mActor->_334 != 0) {
        _2F8.x *= 0.5f;
        _2F8.y *= 0.5f;
        _2F8.z *= 0.5f;
        mJumpVec.x *= 0.5f;
        mJumpVec.y *= 0.5f;
        mJumpVec.z *= 0.5f;
    }

    if (((static_cast<u32>(mMovementStates.mLowBits.mBit30))) != 0 || isAnimationRun(_728)) {
        fixFrontVecByGravity();

        if (isAnimationRun(_728)) {
            bool shouldEnd = false;

            if (mActor->_334 != 0) {
                shouldEnd = true;
            } else if (((static_cast<u32>(mMovementStates.mLowBits.mBit30))) == 0 && _3BC > 3 && mVerticalSpeed > 10.0f) {
                changeAnimation(_724);
                mJumpVec = _2F8;
                goto PROC_HIP_DROP_MOVE;
            }

            u16 endLimit = 0xF;
            if (isAnimationRun("スピンヒップドロップ着地")) {
                endLimit = 0x2D;
            }

            if (_3CE > endLimit) {
                if (isStickOn()) {
                    shouldEnd = true;
                }

                if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
                    shouldEnd = true;
                }
            }

#ifdef GALAXY_NATIVE
            bool groundPoundJumpWindow = _3CE > 5;
            if (odysseyOn()) {
                groundPoundJumpWindow = PetariNative::Odyssey::groundPoundJumpAllowed(_3CE);  // SMO: landing frames 5-30
                if (groundPoundJumpWindow && checkTrgA()) {
                    shouldEnd = true;
                }
            } else
#endif
            if (_3CE > 5 && checkTrgA()) {
                shouldEnd = true;
            }

            if (isAnimationTerminate(nullptr) || shouldEnd) {
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
                stopAnimation(nullptr, "基本");
                stopEffect("属性尻ドロップ");

                if (checkTrgA()) {
#ifdef GALAXY_NATIVE
                    sOdysseyGroundPoundJump = odysseyOn() && groundPoundJumpWindow;
#endif
                    tryJump();
                    return;
                }
            }

            mWalkSpeed = 0.0f;
        } else {
            _3CE = 0;
            mWalkSpeed = 0.0f;
            _71E = 0;

            if (isCurrentFloorSand() && !isPlayerModeYoshi()) {
                changeStatus(mBury);
                return;
            }

            if (isCurrentFloorSink()) {
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
                stopAnimation(nullptr, "基本");
                mSinkTimer = 200;
                return;
            }

            playSound("尻ドロップ着地");
            playSound("声尻ドロップ着地");
            playEffectRT("属性尻ドロップ", _368, mPosHolder->mPos);
            startPadVib("最強");
            startCamVib(0);
            mDrawStates.mBits.mBit11 = 1;
            MR::removeAllClingingKarikari();

            if (_960 == 0x1B || _960 == 0x1C || _960 == 9) {
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
                stopAnimation(nullptr, "基本");
                return;
            }

            changeAnimationWithAttr(_728, 1);
            _2F8 = mJumpVec;
            clearSlope();

            if (mActor->_B90) {
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
            }
        }

        mJumpVec.zero();
    } else {
        _3BC++;

        if (MR::isNearZero(mActor->getLastMove())) {
            _422++;
            if (_422 == 0xF) {
                stopAnimation(nullptr, "基本");
                mMovementStates.mLowBits.mBit31 = 0;
                mMovementStates.mLowBits.mBit20 = 0;
                mJumpVec.zero();
                mMovementStates.mLowBits.mBit30 = 1;
                return;
            }
        }
    }

PROC_HIP_DROP_MOVE:
    TVec3f moveVec(mJumpVec);
    if (moveVec.length() >= mVerticalSpeed) {
        moveVec.setLength(mVerticalSpeed);
    }

    addVelocity(moveVec);

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        const f32 speedRate = MR::clamp(0.1f + (1.0f - (mJumpVec.length() / mActor->getConst().getTable()->mLimitSpeedHipDrop)), 0.0f, 1.0f);
        gravityHipDrop = mActor->getConst().getTable()->mGravityHipDrop;

        mJumpVec += *getGravityVec() * gravityHipDrop * speedRate;

        const f32 jumpGravity = cutGravityElementFromJumpVec(true);
        if (mJumpVec.length() > 10.0f) {
            mJumpVec.x *= 0.5f;
            mJumpVec.y *= 0.5f;
            mJumpVec.z *= 0.5f;
        }

        mJumpVec += getAirGravityVec() * jumpGravity;

#ifdef GALAXY_NATIVE
        if (odysseyOn()) {
            const f32 hipDropLimit = PetariNative::Odyssey::groundPoundFallSpeed();  // SMO: 45
            if (mJumpVec.length() > hipDropLimit) {
                mJumpVec.setLength(hipDropLimit);
            }
        } else
#endif
        if (mJumpVec.length() > mActor->getConst().getTable()->mLimitSpeedHipDrop) {
            mJumpVec.setLength(mActor->getConst().getTable()->mLimitSpeedHipDrop);
        }

        if ((static_cast<u32>(mFlags14.mHighBits.mBit24)) && isAnimationRun("スピンヒップドロップ")) {
            playSound("スピン尻ドロップ落下");

            Triangle strikeTriangles[0x20];
            HitSensor* homingSensor = nullptr;

            f32 maxAngle = 45.0f;
            f32 speed = mVerticalSpeed;
            if (speed >= 500.0f) {
                speed = 500.0f;
            }

            s32 strikeNum = fn_803B2A20(mShadowPos, speed, speed);

            for (u32 i = 0; i < strikeNum; i++) {
                const HitInfo* strikeInfo = fn_803B2AA0(i);

                if (!mActor->sendMsgToSensor(strikeInfo->mParentTriangle.mSensor, 6) &&
                    fn_803EFA30(strikeInfo->mParentTriangle.mSensor, "SHDホーミング許可")) {
                    TVec3f sensorPos(strikeInfo->mParentTriangle.mSensor->mPosition);

                    if (MR::diffAngleAbs(sensorPos - mActor->_240, *getGravityVec()) < maxAngle) {
                        homingSensor = strikeInfo->mParentTriangle.mSensor;
                    }
                }
            }

            if (homingSensor == nullptr) {
                homingSensor = mActor->mNearestUnderSensor798;
            }

            if (homingSensor != nullptr) {
                TVec3f targetJump(*MR::getSensorPos(homingSensor) - mActor->_240);
                targetJump.setLength(mJumpVec.length());

                TVec3f lastMove;
                mActor->getLastMove(&lastMove);
                if (lastMove.dot(mJumpVec) < 0.0f) {
                    const f32 hitGravity = cutGravityElementFromJumpVec(true);
                    mJumpVec = -mJumpVec + getAirGravityVec() * hitGravity;
                } else if (mJumpVec.dot(targetJump) > 0.0f) {
                    mJumpVec = targetJump;
                }
            }

            if (((static_cast<u32>(mMovementStates.mLowBits.mBit23))) || ((static_cast<u32>(mMovementStates.mLowBits.mBit5))) || ((static_cast<u32>(mMovementStates.mLowBits.mBit6)))) {
                mFlags14.mHighBits.mBit24 = 0;
            }
        }

        fixFrontVecFromUpSide();
    }
}

void Mario::doAirWalk() {
    if (isAnimationRun("水泳ジェット")) {
        return;
    }

    if (_430 == 3) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit16))) {
        return;
    }

    if (isAnimationRun("壁はじき")) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit2))) {
        if (_3BC > mActor->getConst().getTable()->mWaitNeutralTimer) {
            mMovementStates.mLowBits.mBit2 = 0;
        }

        return;
    }

    const f32 walkSpeed = mActor->getConst().getTable()->mWalkSpeed;
    if (_428 != 0) {
        _428--;
        if (_428 == 0) {
            stopAnimation("坂すべり上向きうつぶせ", 2);
            stopAnimation("坂すべり下向きあおむけ", 3);
        }

        return;
    }

    f32 speedKiller;
    TVec3f moveDir;
    TVec3f sideMove;
    calcMoveDir(mStickPos.x, mStickPos.y, &moveDir, true);

    if (mActor->_334 != 0) {
        moveDir.zero();
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit18))) {
        if (MR::diffAngleAbs(getAirGravityVec(), _2EC) > 0.7853982f) {
            TVec3f sideVec;
            sideVec.cross(mFrontVec, getAirGravityVec());
            MR::normalizeOrZero(&sideVec);

            Mtx rotMtx;
            PSMTXRotAxisRad(rotMtx, &sideVec, MR::diffAngleSignedHorizontal(_2EC, getAirGravityVec(), sideVec));

            TVec3f frontRot;
            PSMTXMultVecSR(rotMtx, &_2E0, &frontRot);
            if (frontRot.dot(mFrontVec) < 0.0f) {
                moveDir = -mFrontVec;
            } else {
                moveDir = mFrontVec;
            }
        }
    }

    MR::vecKillElement(moveDir, *getGravityVec(), &moveDir);

    bool inhibitFrontAdjust = false;
    f32 frontDot = MR::vecKillElement(moveDir, mFrontVec, &sideMove);

    if (_430 == 0xB || _430 == 8 || getPlayerMode() == 4 || getPlayerMode() == 6) {
        inhibitFrontAdjust = true;
    } else if (frontDot >= 0.0f) {
        f32 frontReduction = fn_803B1D00();
        s16 reduceBegin = mActor->getConst().getTable()->mJumpFrontReductionBeginTime;
        if (_430 == 5) {
            frontReduction = mActor->getConst().getTable()->mSquatJumpFrontReduction;
            reduceBegin = mActor->getConst().getTable()->mSquatJumpFrontReductionBTime;
        }

        if ((static_cast<u32>(mFlags14.mLowBits.mBit8)) && _8D8 == _45C->mSensor) {
            reduceBegin = 0xB4;
            if (MR::isSameMtx(_8E8->getBaseMtx()->toMtxPtr(), _8E8->getPrevBaseMtx()->toMtxPtr())) {
                MR::vecKillElement(_8DC, getAirGravityVec(), &_8DC);
                mJumpVec -= _8DC;
                _8DC.zero();
            }
        }

        if (((static_cast<u32>(mMovementStates.mLowBits.mBit14))) == 0 && _3BC > reduceBegin) {
            const f32 jumpGravity = cutGravityElementFromJumpVec(true);
            const f32 jumpMag = mJumpVec.length();
            mJumpVec.setLength(frontReduction * jumpMag);

            mJumpVec += getAirGravityVec() * jumpGravity;
        }
    } else {
        if (!isRising() && mVerticalSpeed < mActor->getConst().getTable()->mLandTurnHeight && ((static_cast<u32>(mMovementStates.mLowBits.mBit14))) == 0) {
            mMovementStates.mLowBits.mBit15 = 1;
        } else {
            _3CA++;
        }

        if (isAnimationRun("ターンジャンプ")) {
            _3CA = 0;
            speedKiller = 0.1f;
        } else if (_3CA < mActor->mConst->getTable()->mBackJumpLimitFrame) {
            const f32 remain = static_cast< f32 >(mActor->getConst().getTable()->mBackJumpLimitFrame - _3CA);
            speedKiller =
                mActor->getConst().getTable()->mBackJumpRatio * (remain / static_cast< f32 >(mActor->getConst().getTable()->mBackJumpLimitFrame));
        } else {
            speedKiller = 0.0f;
        }

        if ((static_cast<u32>(mMovementStates.mLowBits.mBit14))) {
            moveDir = sideMove + mFrontVec * mActor->getConst().getTable()->mAirWalkBackBonus * frontDot;
        } else {
            moveDir = sideMove + mFrontVec * frontDot * speedKiller;
        }

        const f32 backDot = -moveDir.dot(mFrontVec);
        if (backDot > mActor->getConst().getTable()->mMaxBackJumpSpeed) {
            moveDir.setLength(mActor->getConst().getTable()->mMaxBackJumpSpeed);
        } else if (backDot < 0.0f && _3CA < mActor->mConst->getTable()->mBackJumpLimitFrame) {
            _3CC = 0xA;
        }
    }

    TVec3f moveNorm(moveDir);
    MR::normalizeOrZero(&moveNorm);

    TVec3f jumpNorm(mJumpVec);
    MR::normalizeOrZero(&jumpNorm);

    if (!inhibitFrontAdjust) {
        f32 alignment = moveNorm.dot(jumpNorm);
        if (alignment < 0.0f) {
            const f32 jumpGravity = cutGravityElementFromJumpVec(true);
            mJumpVec += mJumpVec * mActor->getConst().getTable()->mAirWalkSpeedKiller * alignment;

            mJumpVec += getAirGravityVec() * jumpGravity;
        }
    }

    if (_430 == 8) {
        mJumpVec += moveDir * walkSpeed * 5.0f / mActor->getConst().getTable()->mAirWalkTimerFact2;
    } else if (_430 == 0xB && !isRising()) {
        mJumpVec += moveDir * walkSpeed * 5.0f / mActor->getConst().getTable()->mAirWalkTimerFact2;
        playSound("滞空中");
    } else if (getPlayerMode() == 4) {
        mJumpVec += moveDir * mActor->getConst().getTable()->mBeeAirWalkAcc;
        playSound("滞空中");
    } else if (getPlayerMode() == 6) {
        if (mJumpVec.dot(moveDir) >= 0.0f) {
            f32 frontGravity = moveDir.dot(mFrontVec);
            if (frontGravity > 0.0f) {
                frontGravity = -frontGravity;
            }

            TVec3f normJump(mJumpVec);
            if (!MR::normalizeOrZero(&normJump)) {
                f32 alignment = normJump.dot(mFrontVec);
                alignment += 1.0f;
                f32 halfAlignment = alignment / 2.0f;
                mJumpVec *= 0.998f - 0.01f * (1.0f - halfAlignment);
            }
        }
    } else {
        mJumpVec += moveDir * walkSpeed * mActor->getConst().getTable()->mAirWalkTimerFact1 /
                    (_3BC + mActor->getConst().getTable()->mAirWalkTimerFact2);
    }

    f32 accelerationRate = 1.0f;
    if (_430 != 0xB && getPlayerMode() != 6 && !isRising()) {
        addVelocity(moveDir, walkSpeed * mActor->getConst().getTable()->mAirWalkTimerFact3 * accelerationRate);
    }

    const f32 jumpGravity = cutGravityElementFromJumpVec(true);
    if (_430 == 8) {
        if (mJumpVec.length() > 6.0f) {
            mJumpVec.setLength(6.0f);
        }
    } else if (_430 == 0xB) {
        if (mJumpVec.length() > 7.0f) {
            mJumpVec.setLength(7.0f);
        }
    } else if (getPlayerMode() == 4) {
        if (_774 == 0) {
            if (mJumpVec.length() > mActor->getConst().getTable()->mBeeAirWalkLimit) {
                mJumpVec.setLength(mActor->getConst().getTable()->mBeeAirWalkLimit);
            }
        } else if (_774 > 0x1E) {
            if (mJumpVec.length() > mActor->getConst().getTable()->mMaxJumpSpeed) {
                mJumpVec.setLength(mActor->getConst().getTable()->mMaxJumpSpeed);
            }
        } else {
            f32 t = static_cast< f32 >(_774) / 30.0f;
            const f32 limit = (t * mActor->mConst->getTable()->mMaxJumpSpeed) + ((1.0f - t) * mActor->getConst().getTable()->mBeeAirWalkLimit);
            if (mJumpVec.length() > limit) {
                mJumpVec.setLength(limit);
            }
        }
    } else if (getPlayerMode() != 6) {
        if (mJumpVec.length() > mActor->getConst().getTable()->mMaxJumpSpeed) {
            mJumpVec.setLength(mActor->getConst().getTable()->mMaxJumpSpeed);
        }
    }

    mJumpVec += getAirGravityVec() * jumpGravity;
}

void Mario::stopJump() {
    fixFrontVecByGravity();
    mJumpVec.zero();

    mMovementStates.mLowBits.mBit31 = 0;
    mMovementStates.mLowBits.mBit20 = 0;
    mMovementStates.mLowBits.mBit25 = 0;

    mMovementStates.mHighBits.mBit23 = 0;
    mMovementStates.mHighBits.mBit20 = 0;

    mFlags14.mLowBits.mBit8 = 0;
    mFlags14.mLowBits.mBit6 = 0;
    mFlags14.mLowBits.mBit0 = 0;

    if (getPlayerMode() != 4) {
        _402 = mActor->getConst().getTable()->mAirWalkTime;
    }

    _3BC = 0;
    _424 = 0;
    _426 = 0;
    _428 = 0;

    if (isDefaultAnimationRun("落下")) {
        changeAnimation(nullptr, "基本");
    }

    if (isDefaultAnimationRun(_724)) {
        changeAnimation(nullptr, "基本");
    }

    mWall->_1C = 0;
}

void Mario::cancelTornadoJump() {
    _430 = 0;

    if (((static_cast<u32>(mMovementStates.mLowBits.mBit30))) == 0) {
        stopAnimation("スピンジャンプ", "落下");
    } else {
        stopAnimation("スピンジャンプ");
    }

    _4B0 = mPosHolder->mPos;
    mMovementStates.mHighBits.mBit20 = 1;
    _426 = 0;
}

void Mario::setRocketBooster(const TVec3f& rVec, f32 a2, u16 a3) {
    _426 = a3;
    _454 = a2;
    _448 = rVec;
}

void Mario::procRocketBooster() {
    if (_426 == 0) {
        return;
    }

    _426--;
    mJumpVec += _448;
    _448.scale(_454);
}

bool Mario::isSoftLandingFloor() const {
    switch (_960) {
    case 0x6:
    case 0x7:
    case 0x8:
    case 0xB:
    case 0x22:
        return true;
    default:
        return false;
    }
}

void Mario::checkAndTryForceJump() {
    // NONMATCHING: pooled literal placement.
    bool doForceJump = false;
    TVec3f forceJumpVec;

    if ((((getMovementStates().mLowWord >> 30) & 1U) && !(static_cast<u32>(mMovementStates.mLowBits.mBit31))) || isAnimationRun(_728)) {
        switch (_960) {
        case 0x6:
        case 0x22: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[0];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0x7: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[1];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0x8: {
            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[2];
            forceJumpVec = *mGroundPolygon->getNormal(0) * jumpPower;
            doForceJump = true;
            break;
        }

        case 0xB: {
            MR::vecKillElement(_16C, *getGravityVec(), &forceJumpVec);
            forceJumpVec.setLength(10.0f);

            const f32 jumpPower = mActor->getConst().getTable()->mCodeJumpPower[3];
            forceJumpVec += -(*getGravityVec()) * jumpPower;
            doForceJump = true;
            break;
        }

        default:
            break;
        }
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit18))) {
        doForceJump = false;

        if (mActor->mHealth != 0) {
            const u32 status = getCurrentStatus();
            if (status == 2 || status == 4) {
                closeStatus(nullptr);
            }

            tryForceFreeJump(_304);
        }

        mMovementStates.mHighBits.mBit18 = 0;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit17))) {
        doForceJump = false;

        if (mActor->mHealth != 0) {
            if (getCurrentStatus() == 2) {
                closeStatus(nullptr);
            }

            tryFreeJump(_304, true);
        }

        mMovementStates.mHighBits.mBit17 = 0;
    }

    if (doForceJump) {
        mActor->sendMsgToSensor(mGroundPolygon->mSensor, 2);

        if (getCurrentStatus() == 2) {
            if (mActor->mHealth == 0) {
                mActor->forceGameOverNonStop();
            }

            closeStatus(nullptr);
        }

        mMovementStates.mHighBits.mBit30 = 1;
        mMovementStates.mHighBits.mBit17 = 1;
        _304 = forceJumpVec;
        changeAnimation("フリージャンプ");

        if (mActor->isEnableNerveChange()) {
            playSound("トランポリンジャンプ大");
            if (isPlayerModeYoshi()) {
                MR::startActionSound(mActor->getYoshi(), "JumpObj", -1, -1, -1);
            } else {
                playSound("声大ジャンプ");
            }
            playSound("ジャンプ踏切");
        }
    }
}

// NONMATCHING: register allocation and independent load scheduling; SMG2 orchestration recovered.
void Mario::doLanding() {
    bool hard = false;
    bool blend = false;
    bool keepSlip = false;
    if (getPlayerMode() == 6) return;
    mMovementStates.mLowBits.mBit31 = 0;
    mFlags14.mLowBits.mBit0 = 0;
    mFlags14.mHighBits.mBit30 = 0;
    _3CE = 0;
    mFlags20.mBits.mBit13 = 1;
    TVec3f horizontal;
    TVec3f delta(mPosHolder->mPos);
    delta -= _4B0;
    f32 distance = MR::abs(delta.dot(*getGravityVec()));
    if (_3BC <= 3 && distance < 1.0f && PSVECMag(&mJumpVec) < 10.0f) {
        stopAnimation(nullptr, "基本");
        return;
    }
    if (mActor->_EEB && (static_cast<u32>(mMovementStates.mHighBits.mBit20))) {
        if ((static_cast<u32>(mFlags14.mLowBits.mBit6)) && _430 != 3) {
            mFlags14.mLowBits.mBit6 = 0;
            mActor->_946 = 16;
        }
    }
    resetTornado();
    if (mActor->mBeeWallWalk != 0) playSound("ハチ壁くっつき");
    else playSound("着地");
    if ((isAnimationRun("スケートアクセルジャンプ") || isAnimationRun("スケートジャンプ2") || isAnimationRun("スケートジャンプ3")) && (getPlayerMode() == 3 || getPlayerMode() == 0)) {
        playEffect("スケート着地");
        if (getStickP() == 0.0f) {
            mJumpVec.zero();
            stopWalk();
            changeAnimation("スケート静止着地", "基本");
            return;
        }
        changeStatus(mSkate);
        return;
    }
    if (_430 == 5 && isSkatableFloor()) {
        if (checkSquat(false)) {
            mMovementStates.mLowBits.mBit21 = 1;
            mWalkSpeed = 1.0f;
            if (isAnimationRun("カリカリ限界")) stopAnimationUpper(nullptr, nullptr);
            stopAnimation(nullptr, "しゃがみ基本");
            return;
        }
        cancelSquatMode();
        stopAnimationUpper(nullptr, nullptr);
        changeAnimation("スケート静止着地", "基本");
        return;
    }
    if (isAnimationRun("フーファイター着地")) {
        stopAnimation(nullptr, "基本");
        return;
    }
    if (static_cast<u32>(getPlayer()->mFlags14.mLowBits.mBit29)) {
        Mario* player = getPlayer();
        player->mFlags14.mLowBits.mBit29 = 0;
        _430 = 14;
        goto COMMON_LANDING;
    }
    {
        u32 type = mMovementStates.mHighWord & 3;
        if (type == 2) {
            Mario* player = getPlayer();
            player->mMovementStates.mHighWord &= ~3;
            stopWalk();
            playSound("声ステージイン着地");
            playSound("重い着地");
            changeAnimation("ステージインB", "基本");
            startCamVib(4);
            return;
        }
        if (type == 1) {
            Mario* player = getPlayer();
            player->mMovementStates.mHighWord &= ~3;
            playEffect("スーパースピンドライバ終了");
            playEffect("共通着地大");
            if (!isStickFull()) keepSlip = true;
            hard = true;
            if (_1FC.dot(_368) < 0.0f) {
                if (!isPlayerModeYoshi() && (_960 == 0xD || _960 == 0x1E || _960 == 5 || _960 == 0x17)) {
                    changeStatus(mSukekiyo);
                    return;
                }
                stopWalk();
                startCamVib(4);
                changeAnimation("ハード着地", "基本");
                _3CE += 16;
                forceSetHeadVecKeepSide(_368);
                return;
            }
            goto COMMON_LANDING;
        }
    }
    if (isCurrentFloorSink()) {
        stopAnimation(nullptr, "基本");
        mSinkTimer = 64;
        return;
    }
    if (static_cast<u16>(_960 + 0xFFE5) <= 1) {
        bool sand = strcmp(MR::getSoundCodeString(_45C), "Sand") == 0;
        if (sand) {
            stopAnimation(nullptr, "基本");
            mSinkTimer = 32;
            return;
        }
        goto COMMON_LANDING;
    }
    if (_3BC > 10) playEffect("共通着地普通");
COMMON_LANDING:
    if (static_cast<u32>(_430 - 10) > 1 && _430 != 0 && _430 != 14) {
        if (_430 != 2 && _430 != 6 && _430 != 4) startPadVib(2);
        else startPadVib(3);
    } else startPadVib(static_cast<u32>(0));
    getAnimator()->addRumblePower(5.0f, 60);
    mMovementStates.mHighBits.mBit30 = 0;
    if (getPlayerMode() != 4) _402 = mActor->mConst->getTable()->mAirWalkTime;
    else _71E = 0;
    if (!isSoftLandingFloor()) {
        const TVec3f& position = mPosHolder->mPos;
        TVec3f fall(_4B0);
        fall -= position;
        TVec3f negativeGravity(-*getGravityVec());
        f32 height = fall.dot(negativeGravity);
        if (height > mActor->mConst->getTable()->mHardLandingHeight) hard = true;
    }
    if (hard && isCurrentFloorSand() && !isPlayerModeYoshi()) {
        changeStatus(mBury);
        return;
    }
    if (isBlendWaitGround()) blend = true;
    fixFrontVecByGravity();
#ifdef GALAXY_NATIVE
    {
        // OdysseyMovement: a long jump or dive landed with a direction held goes
        // straight into a run at the landing speed (SMO keeps the momentum; the run
        // then slows to its cap at 14/120 per frame). Galaxy would drop the long
        // jump's speed and end the dive in a belly flop. The dive lands with the
        // failed dive's forward roll as the get-up. Stick released: Galaxy's landing.
        const PetariNative::Odyssey::Air landed = sOdysseyLanded;
        sOdysseyLanded = PetariNative::Odyssey::Air::None;
        if (landed == PetariNative::Odyssey::Air::Dive && isAnimationRun("幅とび")) {
            XanimeFrameCtrl* ctrl = getAnimator()->getXanimePlayer()->getFrameCtrl();
            if (ctrl != nullptr) {
                ctrl->setRate(1.0f);  // undo the dive's leap timing
            }
        }
        if (odysseyOn() && landed == PetariNative::Odyssey::Air::Dive && !hard && !isStickOn()) {
            // Stick released: the dive ends in Galaxy's belly flop (its landing for
            // a dive into shallow water), as before the leap pose.
            stopWalk();
            changeAnimation("飛び込み失敗着地", "基本");
            goto POST_LANDING;
        }
        if (odysseyOn() && !hard && isStickOn() &&
            (landed == PetariNative::Odyssey::Air::LongJump || landed == PetariNative::Odyssey::Air::Dive)) {
            TVec3f up(-mActor->mAppliedGravityVec);
            MR::normalizeOrZero(&up);
            TVec3f horizontal(mJumpVec - up * up.dot(mJumpVec));
            const f32 speed = horizontal.length();
            if (speed > 1.0f) {
                MR::normalize(&horizontal);
                setFrontVecKeepUp(horizontal);
                if ((mMovementStates.mLowWord >> 21) & 1U) {
                    cancelSquatMode();
                }
                mWalkSpeed = speed / mActor->mConst->getTable()->mWalkSpeed;
                if (landed == PetariNative::Odyssey::Air::Dive) {
                    changeAnimation("飛び込み失敗回転着地", "基本");
                    PetariNative::Odyssey::Live::getUpFrames = 12;  // then the run (Mario::updateWalkSpeed)
                } else {
                    stopAnimation(nullptr, "基本");
                }
                std::fprintf(stderr, "[odyssey] %s landed running at %.3f\n",
                             landed == PetariNative::Odyssey::Air::Dive ? "dive" : "long jump", speed);
                goto POST_LANDING;
            }
        }
    }
#endif
    if (hard || (static_cast<u32>(mDrawStates.mBits.mBit19)) || _430 == 14 || isAnimationRun("飛び込みジャンプ") || isAnimationRun("後方飛び込みジャンプ") || (static_cast<u32>(mFlags14.mLowBits.mBit23)) || isAnimationRun("水上ダメージ中")) {
        fn_803A0490(hard, blend);
        return;
    }
    if (_430 == 5 && checkSquat(false)) {
        mMovementStates.mLowBits.mBit21 = 1;
        mWalkSpeed = 1.0f;
        if (isAnimationRun("カリカリ限界")) stopAnimationUpper(nullptr, nullptr);
        stopAnimation(nullptr, "しゃがみ基本");
    } else if (((isStickFull() || _60D) && _3CA == 0) || (!(static_cast<u32>(mMovementStates.mLowBits.mBit20)) && checkSquat(false))) {
        stopAnimation(nullptr, "基本");
        if (!(static_cast<u32>(mFlags14.mLowBits.mBit21))) recordTurnSlipAngle();
        if (_430 == 5) cancelSquatMode();
    } else {
        fn_803A02F0(keepSlip, hard, blend);
    }
#ifdef GALAXY_NATIVE
POST_LANDING:;
#endif
    if (static_cast<u32>(mDrawStates.mBits.mBit19)) {
        MR::vecKillElement(mJumpVec, *getGravityVec(), &horizontal);
        if (horizontal.dot(*mGroundPolygon->getNormal(0)) > 0.0f) {
            mMovementStates.mHighBits.mBit28 = 1;
            _8F0 = 10.0f;
        } else {
            f32 ratio = (90.0f - calcPolygonAngleD(mGroundPolygon)) / 90.0f;
            mWalkSpeed *= ratio;
            mWalkSpeed = MR::clamp(mWalkSpeed, 0.0f, 2.0f);
            f32 minVelocity = 10.0f - 10.0f * ratio;
            if (_8F0 < minVelocity) _8F0 = minVelocity;
        }
        f32 length = PSVECMag(&_8F8);
        if (PSVECMag(&horizontal) < length) {
            f32 speed = PSVECMag(&horizontal);
            _8F8.setLength(0.5f * (PSVECMag(&_8F8) + speed));
        }
    } else _8F8.zero();
}

// NONMATCHING: temporary register allocation; verified collision-sensor and velocity gates.
void Mario::fn_803A02F0(s32 keepSlipArg, s32 hardArg, s32 blendArg) {
    s32 hard = hardArg;
    s32 blend = blendArg;
    s32 keepSlip = keepSlipArg;
    u32 mode = _430;
    if (mode - 4 <= 2 || mode == 8 || mode == 2 || mode == 11) keepSlip = 1;
    if ((static_cast<u32>(mFlags14.mLowBits.mBit8)) && _8D8 == _45C->mSensor) {
        TVec3f delta(mJumpVec);
        delta -= _8DC;
        if (delta.dot(mJumpVec) < 0.0f) mJumpVec.zero();
        else mJumpVec = delta;
    }
    if ((mFrontVec.dot(mJumpVec) > 6.0f && keepSlip == 0) || (isPlayerModeYoshi() && mActor->getYoshi()->mDashTimer > 0)) {
        f32 speed = fn_803B1CA0();
        mWalkSpeed = mFrontVec.dot(mJumpVec) / speed;
        mWalkSpeed = MR::clamp(mWalkSpeed, 0.0f, 2.0f);
        stopAnimation(nullptr, "基本");
        _3FA = 0;
        _71E = 0;
        if (!(static_cast<u32>(mFlags14.mLowBits.mBit21))) recordTurnSlipAngle();
    } else fn_803A0490(hard, blend);
}

// NONMATCHING: temporary/register scheduling; full landing switch and return structure recovered.
void Mario::fn_803A0490(s32 arg1, s32 arg2) {
    s32 var_r29 = arg1;
    s32 var_r28 = arg2;
    if (this->_430 == 4) {
        TVec3f sp14(-this->_220);
        this->setFrontVecKeepUp(sp14);
    }
    if (this->isAnimationRun("飛び込みジャンプ") || this->isAnimationRun("後方飛び込みジャンプ") || (static_cast<u32>(this->mFlags14.mLowBits.mBit23))) {
        if (this->isAnimationRun("後方飛び込みジャンプ")) {
            TVec3f sp8(-this->mFrontVec);
            this->setFrontVecKeepUp(sp8);
        }
        if (static_cast<u32>(this->mFlags14.mLowBits.mBit23)) {
            var_r29 = 1;
        }
        this->stopWalk();
        if (var_r29 != 0 || this->isAnimationRun("後方飛び込みジャンプ")) {
            if (var_r29 != 0) {
                this->changeAnimation("飛び込み失敗回転着地", "基本");
                this->startSlidingTask(8, 5.0f, 25);
                this->startCamVib(0);
                return;
            }
            this->changeAnimation("飛び込み失敗着地", "基本");
            return;
        }
        this->changeAnimation("飛び込み失敗着地", "基本");
        return;
    }
    if (!(static_cast<u32>(this->mDrawStates.mBits.mBit19))) {
        this->clearSlope();
        switch (this->_430) {
        case 0:
        case 7:
            this->changeAnimation("着地", "基本");
            break;
        case 1:
            this->changeAnimation("着地B", "基本");
            break;
        case 2:
            this->changeAnimation("着地C", "基本");
            break;
        case 4:
            this->changeAnimation("着地ターン", "基本");
            break;
        case 5:
            if (!this->checkSquat(false)) {
                this->cancelSquatMode();
            }
            if (this->mActor->_468 == 0) {
                this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
            }
            this->changeAnimation("着地幅とび", "基本");
            break;
        case 6:
            this->changeAnimation("しゃがみジャンプ着地", "基本");
            break;
        default:
            if (this->isAnimationRun("水上ダメージ中")) {
                this->changeAnimation("中ダメージ着地", "基本");
            } else {
                this->changeAnimation("着地", "基本");
            }
            break;
        case 3:
        case 10:
            this->stopAnimation((const char*)nullptr, "基本");
            break;
        case 14:
            this->playSound("倒れ", -1);
            this->playSound("着地", -1);
            this->playSound("声小ダメージ", -1);
            this->playEffect("共通引き戻し着地");
            this->changeAnimation("引き戻し着地", "基本");
            var_r29 = 0;
            var_r28 = 0;
            break;

        }
        this->stopWalk();
        if (var_r28 != 0) {
            this->changeAnimation("ショート着地", "基本");
        }
        if (var_r29 != 0) {
            this->changeAnimation("ハード着地", "基本");
            this->playSound("重い着地", -1);
            if (this->isPlayerModeYoshi()) {
                MR::startActionSound(this->mActor->getYoshi(), "Land", -1, -1, -1);
            } else {
                this->playSound("声着地停止", -1);
            }
            this->startCamVib(4);
            this->_3CE += 0x10;
        }
        if (this->mActor->mBeeWallWalk != 0) {
            this->changeAnimation("ハチ匍匐着地", "基本");
        }
    } else {
        if (!this->isAnimationRun("坂すべり上向きうつぶせ", 2) && !this->isAnimationRun("坂すべり下向きあおむけ", 3)) {
            this->stopAnimation((const char*)nullptr, "基本");
        }
        this->changeAnimation("すべり着地", "基本");
        this->_71E = 0;
    }
}

void Mario::startSlidingTask(u32 a1, f32 a2, u16 a3) {
    _A40 = mFrontVec * a2;
    _A3C = a3;
    pushTask(&Mario::taskOnSlide, a1);
}

bool Mario::taskOnSlide(u32 a1) {
    (void)a1;
    addVelocity(_A40);
    _A3C--;
    return _A3C != 0;
}

void MarioJump_FORCE_MATCH_DATA() {
    Mario::Task task = &Mario::taskOnWallRising;
    (void)task;
}

bool Mario::taskOnWallRising(u32 a1) {
    (void)a1;

    if (((static_cast<u32>(mMovementStates.mLowBits.mBit8))) == 0) {
        stopEffect("共通壁上昇");
        return false;
    }

    playSound("スリップ");
    return true;
}

void Mario::incAirWalkTimer() {
    const u16 timer = _402;
    const u16 limit = mActor->getConst().getTable()->mAirWalkTime;

    if (timer >= limit) {
        return;
    }

    _402 = timer + 1;

    if (getPlayerMode() != 4) {
        return;
    }

    const u16 nextTimer = _402;
    const u16 nextLimit = mActor->getConst().getTable()->mAirWalkTime;

    if (nextTimer >= nextLimit) {
        if (mAirWalkRecoverySoundArmed != 0) {
            playSound("ハチ体力完全回復");
        }
        mAirWalkRecoverySoundArmed = 0;
    } else {
        mAirWalkRecoverySoundArmed = 1;
        playSound("ハチ体力回復");
    }
}

void Mario::stopPunch() {
    if (isStatusActive(MarioStatus_Magic)) {
        closeStatus(mMagic);
    }

    MarioActor* actor = mActor;

    if (!actor->_944) {
        actor->_945 = 0;
        actor->_974 = 0;
    }

    actor->_944 = 0;
}

void Mario::startMagic() {
    if (!((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        if (!mActor->_468) {
            if (!((mMovementStates.mHighWord >> 28) & 1U)) {
                if (!isStatusActive(MarioStatus_Slider)) {
                    if (isSkatableFloor()) {
                        doSkate();
                    } else {
                        clearSlope();
                        changeStatus(mMagic);
                        stopAnimationUpper(nullptr);
                        mFlags14.mLowBits.mBit30 = 1;
                    }
                }
            }
        }
    }
}

// NONMATCHING: temporary allocation and register scheduling.
void Mario::fn_803A0BE0() {
    this->fn_803A2CC0();
    this->mMovementStates.mHighBits.mBit10 = this->isSkatableFloor();
    this->fn_803A2CF0();
    u8 sp8 = this->fn_803A2D10();
    this->fn_803A2D50();
    if (this->fn_803A2D80() == 0 && this->fn_803A3030() == 0 && this->fn_803A3130() == 0) {
        this->checkTornado();
        this->fixPositionInTower();
        TVec3f spC;
        MR::setNan(spC);
        s32 var_r31 = 0;
        f32 stickX = mStickPos.x;
        f32 t31 = mStickPos.y;
        if (MR::isNearZero(stickX, 0.001f) && MR::isNearZero(t31, 0.001f)) {
            var_r31 = 1;
        }
        if (var_r31 != 0) {
            this->fn_803A0D90(&spC);
        } else if (this->fn_803A0E10()) {
            this->fn_803A0ED0(&spC, sp8);
        } else {
            this->fn_803A11A0(&spC);
        }
        this->calcShadowDir(this->mFrontVec, &this->_214);
        this->fn_803A1330(&sp8);
        this->fn_803A1600(&sp8);
        f32 rate = this->fn_803A1800();
        if (!(static_cast<u32>(this->mMovementStates.mLowBits.mBit27))) {
            this->fn_803A1FF0(rate, sp8);
        } else {
            this->fn_803A20E0(rate, sp8);
        }
        this->fn_803A2180();
        this->fn_803A28D0(&spC);
    }
}

void Mario::fn_803A0D90(TVec3f* arg) {
    if (MR::isNearZero(this->_328, 0.001f)) {
        this->_328 = this->mFrontVec;
    }
    *arg = this->_328;
    this->calcShadowDir(this->_328, &this->_22C);
    this->_40C = 10;
}

bool Mario::fn_803A0E10() {
    bool result = false;
    if ((this->mTargetWalkSpeedIndex == 0 || this->_71D == 0) &&
        !this->isAnimationRun("ブレーキ") && !this->isAnimationRun("ターンブレーキ") &&
        !(static_cast<u32>(this->mMovementStates.mLowBits.mBit15)) &&
        !(static_cast<u32>(this->mMovementStates.mLowBits.mBit16)) &&
        this->mWalkSpeed < 0.05f && !(static_cast<u32>(this->mDrawStates.mBits.mBit26))) {
        result = true;
    }
    return result;
}

// NONMATCHING: the target-speed and sound branches are approximated
void Mario::fn_803A0ED0(TVec3f* arg1, s32 arg2) {
    if (this->fn_803A0FF0()) {
        *arg1 = this->_328;
        return;
    }
    this->calcMoveDir(this->mStickPos.x, this->mStickPos.y, arg1, true);
    this->calcShadowDir(*arg1, &this->_22C);
    this->calcShadowDir(this->mFrontVec, &this->_214);
    if (this->fn_803A1050()) {
        this->fn_803A10D0(arg2);
        return;
    }
    if (arg2 == 0) {
        this->setFrontVecKeepUp(this->_22C);
    }
    if (this->isAnimationRun("食べる") || this->isAnimationRun("噛む") || this->isAnimationRun("飲み下す")) {
        this->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
}

bool Mario::fn_803A0FF0() {
    u16 v = this->_40C;
    if (v == 0) {
        return false;
    }
    if (this->mStickPos.z >= 0.75f) {
        this->_40C = 0;
        return false;
    }
    this->_40C = v - 1;
    this->mStickPos.x = 0.0f;
    this->mStickPos.y = 0.0f;
    this->mStickPos.z = 0.0f;
    return true;
}

bool Mario::fn_803A1050() {
    if (static_cast<u32>(this->mMovementStates.mHighBits.mBit8)) {
        f32 d = MR::diffAngleAbsHorizontal(this->_214, this->_22C, this->mSwimSideVec694);
        return d > 1.5707964f;
    }
    f32 d2 = MR::diffAngleAbsHorizontal(this->_214, this->_22C, *this->getGravityVec());
    return d2 > 0.17453293f;
}

void Mario::fn_803A10D0(s32 arg) {
    // NONMATCHING: literal-pool placement; complete string identities preserved.
    MarioConstTable* e = this->mActor->mConst->getTable();
    u32 dur = e->mStandingTurnTime;
    if (this->mActor->mBeeWallWalk != 0) {
        dur = 30;
    }
    if (arg == 0) {
        this->setFrontVecKeepUp(this->_22C, dur);
    }
    if (this->isEnableTurn()) {
        this->changeAnimation("その場足踏み", (const char*)nullptr);
        if (!this->isAnimationRun("カリカリ限界") && !this->isPlayerModeYoshi()) {
            this->fn_803EEC20("その場足踏み上半身", 0);
        }
    }
}

void Mario::fn_803A11A0(TVec3f* arg) {
    f32 sp12 = this->mStickPos.x;
    f32 sp8 = this->mStickPos.y;
    u32 allowAdjustment = static_cast<u32>(this->getPlayer()->mFlags14.mLowBits.mBit14) ^ 1;
    if (allowAdjustment) {
        this->fn_803A1240(&sp12, &sp8);
    }
    this->calcMoveDir(sp12, sp8, arg, true);
    this->calcShadowDir(*arg, &this->_22C);
}

// NONMATCHING: floating-point register allocation.
void Mario::fn_803A1240(f32* a, f32* b) {
    f32 vertical = *b;
    const MarioConstTable* table = mActor->getConst().getTable();
    f32 absVertical = MR::abs(vertical);
    if (absVertical > table->mStickMarginYstart) {
        f32 horizontal = *a;
        f32 margin = table->mStickMarginX;
        if (MR::abs(horizontal) < margin) {
            *a = 0.0f;
            return;
        }
        if (horizontal > 0.0f) {
            *a = (horizontal - margin) / (1.0f - margin);
        } else {
            *a = (horizontal + margin) / (1.0f - margin);
        }
    } else if (MR::abs(*a) > table->mStickMarginXstart) {
        f32 margin = table->mStickMarginY;
        if (absVertical < margin) {
            *b = 0.0f;
            return;
        }
        if (vertical > 0.0f) {
            *b = (vertical - margin) / (1.0f - margin);
        } else {
            *b = (vertical + margin) / (1.0f - margin);
        }
    }
}

void Mario::fn_803A1330(u8* arg1) {
    f32 alignment = this->_214.dot(this->_22C);
    if (this->_3D2 != 0) {
        alignment = this->_3E4.dot(this->_22C);
    }
    if (this->fn_803A1E60(this->_214.dot(this->_22C))) {
        this->recordTurnSlipAngle();
    } else {
        this->fn_803A1F60();
    }
    if ((static_cast<u32>(this->mMovementStates.mLowBits.mBit16)) || (static_cast<u32>(this->mMovementStates.mHighBits.mBit11))) {
        this->_3D2 = 0;
        this->mMovementStates.mLowBits.mBit15 = 0;
    }
    if (this->_3D2 != 0) {
        this->fn_803A1F80();
        this->fn_803A1FC0();
        if (this->isAnimationRun("ブレーキ") || (static_cast<u32>(this->mMovementStates.mLowBits.mBit15))) {
            *arg1 = 1;
        }
    } else {
        if (!(static_cast<u32>(this->mMovementStates.mHighBits.mBit11)) &&
            !(static_cast<u32>(this->mMovementStates.mLowBits.mBit21)) && (static_cast<u32>(this->mMovementStates.mLowBits.mBit15)) &&
            !(static_cast<u32>(this->mMovementStates.mLowBits.mBit16)) && !(static_cast<u32>(this->mMovementStates.mHighBits.mBit28)) && !this->isStatusActive(0x1F)) {
            if (alignment < JMACosRadian(this->mActor->mConst->getTable()->mTurnSlipAngle) && !(static_cast<u32>(this->mMovementStates.mLowBits.mBit27))) {
                if (static_cast<u32>(this->mMovementStates.mHighBits.mBit10)) {
                    *arg1 = 1;
                }
                this->_220 = this->_3E4;
                if (*arg1 == 0) {
                    TVec3f front = -this->_220;
                    this->setFrontVecKeepUp(front);
                }
                if (static_cast<u32>(this->mMovementStates.mHighBits.mBit10)) {
                    this->_3D0 = static_cast<u16>(this->mActor->mConst->getTable()->mTurnSlipTimeB);
                    this->mMovementStates.mLowBits.mBit27 = 1;
                    this->changeAnimation("ターンブレーキ滑り床", static_cast<const char*>(nullptr));
                    this->_2B8 = this->mActor->getLastMove();
                    this->stopWalk();
                    this->_754 = 10;
                    this->pushTask(&Mario::taskOnSlipTurn, 1);
                } else {
                    this->_3D0 = static_cast<u16>(this->mActor->mConst->getTable()->mTurnSlipTime);
                    this->mMovementStates.mLowBits.mBit27 = 1;
                    this->mWalkSpeed = 0.0f;
                    this->changeAnimation("ターンブレーキ", static_cast<const char*>(nullptr));
                    this->playEffect("共通ブレーキ");
                }
            }
        }
        this->mMovementStates.mLowBits.mBit15 = 0;
    }
}

void Mario::fn_803A1600(u8* arg1) {
    u16 timer = this->_3D0;
    if (timer != 0) {
        if (timer == static_cast<u16>(this->mActor->mConst->getTable()->mTurnSlipTime)) {
            this->startPadVib(2);
        }
        this->_3D0 -= 1;
        if (!(static_cast<u32>(this->mMovementStates.mHighBits.mBit10))) {
            this->mWalkSpeed = 0.0f;
        }
    }
    if (this->_3D0 == 0) {
        if (static_cast<u32>(this->mMovementStates.mLowBits.mBit27)) {
            const TVec3f* gravity = this->getGravityVec();
            if (MR::diffAngleAbsHorizontal(this->getWorldPadDir(), this->mFrontVec, *gravity) > 1.5707964f) {
                this->stopAnimation("ターンブレーキ", static_cast<const char*>(nullptr));
                TVec3f front = -this->mFrontVec;
                this->setFrontVecKeepUp(front);
                this->mMovementStates.mLowBits.mBit27 = 0;
            }
            if (this->isAnimationTerminate("ターンブレーキ")) {
                this->stopAnimation("ターンブレーキ", static_cast<const char*>(nullptr));
                this->mMovementStates.mLowBits.mBit27 = 0;
                if (this->mStickPos.z < 0.1f) {
                    this->stopWalk();
                }
            }
        }
        if (this->isActiveTask(&Mario::taskOnSlipTurn) && this->isAnimationRun("ターンブレーキ滑り床")) {
            TVec3f front = -this->_220;
            this->setFrontVecKeepUp(front);
            *arg1 = 1;
            this->stopAnimation("ターンブレーキ滑り床", static_cast<const char*>(nullptr));
            this->_74C = 0.0f;
            this->_754 = 0;
            this->mWalkSpeed = 0.0f;
            this->popTask(&Mario::taskOnSlipTurn);
        }
    }
}

// NONMATCHING: literal symbol identity; instruction words match after relocation masking.
f32 Mario::fn_803A1800() {
    if (static_cast<u32>(this->mMovementStates.mLowBits.mBit16)) {
        return mActor->mConst->getTable()->mTurnAngleSpeedTornado;
    }
    if (static_cast<u32>(this->mMovementStates.mHighBits.mBit28)) {
        return mActor->mConst->getTable()->mTurnAngleSpeedSlowWalk;
    }
    if (this->mWalkSpeed > mActor->mConst->getTable()->mFastTurnSpeed) {
        if (this->isStatusActive(0x1F)) {
            return 0.5f * mActor->mConst->getTable()->mTurnAngleSpeed3;
        }
        if (this->mTargetWalkSpeedIndex < 5) {
            return mActor->mConst->getTable()->mGravityStandard;
        }
        return mActor->mConst->getTable()->mTurnAngleSpeed3;
    }
    return mActor->mConst->getTable()->mTurnAngleSpeed2;
}

bool Mario::fn_803A1920() {
    TVec3f stick;
    this->mActor->getStickValue(&stick.x, &stick.y);
    stick.z = 0.0f;
    bool changed = false;
    if (!JGeometry::TUtil<f32>::epsilonEquals(this->_250.x, stick.x, 0.000003814697265625f) ||
        !JGeometry::TUtil<f32>::epsilonEquals(this->_250.y, stick.y, 0.000003814697265625f) ||
        !JGeometry::TUtil<f32>::epsilonEquals(this->_250.z, stick.z, 0.000003814697265625f)) {
        changed = true;
    }
    return changed;
}

bool Mario::fn_803A19C0() {
    TVec3f s;
    this->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    return s.dot(this->_244) > 0.0f;
}

void Mario::fn_803A1A20(f32* out) {
    this->mActor->getStickValue(&out[0], &out[1]);
    out[2] = 0.0f;
}

void Mario::fn_803A1A60(TVec3f* arg) {
    TVec3f s;
    this->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    PSVECCrossProduct(&this->_250, &s, arg);
}

void Mario::fn_803A1AC0() {
    if (MR::isNearZero(this->mStickPos.z, 0.001f)) {
        this->mFlags14.mLowBits.mBit19 = 0;
        this->mMovementStates.mLowBits.mBit28 = 0;
    } else {
        bool moving = !MR::isNearZero(this->_244, 0.001f);
        if (!moving && !(static_cast<u32>(this->mMovementStates.mHighBits.mBit8))) {
            if (this->fn_803A1920()) {
                this->mMovementStates.mLowBits.mBit28 = 1;
            } else {
                this->fn_803A1BB0();
            }
        } else if (this->fn_803A19C0() && !(static_cast<u32>(this->mMovementStates.mHighBits.mBit8))) {
            this->mMovementStates.mLowBits.mBit28 = 1;
        } else {
            this->fn_803A1BB0();
        }
    }
    this->fn_803A1A20(&this->_250.x);
}

void Mario::fn_803A1BB0() {
    if (this->_3D0 != 0) {
        this->mFlags14.mLowBits.mBit19 = 0;
        this->mMovementStates.mLowBits.mBit28 = 0;
    }
    f32 d = MR::diffAngleAbs(this->_214, this->_22C);
    if (MR::isInRange(d, 0.0f, 0.707f)) {
        TVec3f s;
        PSVECCrossProduct(&this->_214, &this->_22C, &s);
        if (this->_274 != 0) {
            if (s.dot(this->mHeadVec) > 0.0f) {
                this->mFlags14.mLowBits.mBit19 = 0;
            }
        } else {
            if (s.dot(this->mHeadVec) < 0.0f) {
                this->mFlags14.mLowBits.mBit19 = 0;
            }
        }
    }
}

void Mario::fn_803A1C90() {
    TVec3f sp8;
    PSVECCrossProduct(&this->_214, &this->_22C, &sp8);
    bool negative = sp8.dot(this->_3D8) < 0.0f;
    if (negative) {
        if (!this->fn_803A19C0()) {
            bool frontNegative = this->_214.dot(this->_22C) < 0.0f;
            if (!frontNegative) {
                this->_3D4 = 0;
            }
        }
    } else {
        const MarioConstTable* table = this->mActor->mConst->getTable();
        u16 v = this->_3D4;
        if (v < table->mWeakTurnTime) {
            this->_3D4 = v + 1;
        }
    }
    this->fn_803A1D60();
}

void Mario::fn_803A1D60() {
    PSVECCrossProduct(&this->_214, &this->_22C, &this->_3D8);
    this->fn_803A1A60(&this->_244);
    this->_238 = this->_22C;
}

// NONMATCHING: prologue scalar-load scheduling only; calls and arithmetic verified.
#pragma dont_inline on
f32 Mario::fn_803A1DB0() {
    u16 v = _3D4;
    MarioConst* table = mActor->mConst;
    f32 ratio = 1.0f;
    u16 lim = table->mTable[table->mCurrentTable]->mWeakTurnTime;
    if (v < lim && mTargetWalkSpeedIndex > 4) {
        ratio = static_cast<f32>(v) / static_cast<f32>(lim);
    }
    return MR::sqrt(ratio);
}
#pragma dont_inline reset

void Mario::fn_803A1E40() {
    u16 temp_r4 = this->_3D4;
    if (temp_r4 != 0) {
        this->_3D4 = temp_r4 - 1;
    }
}

s32 Mario::fn_803A1E60(f32 f) {
    bool result = false;
    bool stick = false;
    bool speed = false;
    bool angle = false;
    if (f >= JMACosRadian(this->mActor->mConst->getTable()->mTurnSlipAngle) &&
        !((this->mMovementStates.mLowWord >> 27) & 1)) {
        angle = true;
    }
    if (angle && this->mWalkSpeed > this->mActor->mConst->getTable()->mSlipSpeed) {
        speed = true;
    }
    if (speed && this->_38 > this->mStickPos.z) {
        stick = true;
    }
    if (stick && !((this->mDrawStates.mWord >> 18) & 1)) {
        result = true;
    }
    return result;
}

void Mario::fn_803A1F60() {
    u16 temp_r4 = this->_3D2;
    if (temp_r4 != 0) {
        this->_3D2 = temp_r4 - 1;
    }
}

void Mario::fn_803A1F80() {
    f32 temp_f1 = this->mStickPos.z;
    const MarioConstTable* ent = this->mActor->mConst->getTable();
    if (temp_f1 < ent->mTurnSlipNeutral) {
        this->mMovementStates.mLowBits.mBit15 = 1;
    }
}

void Mario::fn_803A1FC0() {
    if (!(this->mWalkSpeed >= 0.1f)) {
        this->mWalkSpeed = 0.0f;
        this->_3D2 = 0;
    }
}

// NONMATCHING: branch shape of the 0x3D4 == 0 block approximated
void Mario::fn_803A1FF0(f32 x, s32 arg) {
    this->fn_803A1AC0();
    if (static_cast<u32>(this->mMovementStates.mLowBits.mBit28)) {
        this->fn_803A1C90();
        x *= this->fn_803A1DB0();
    } else {
        this->fn_803A1E40();
        if (this->_3D4 == 0) {
            this->_244.zero();
            PSVECCrossProduct(&this->_214, &this->_22C, &this->_3D8);
            x *= 0.01f;
        }
        this->_238 = this->_22C;
    }
    if (static_cast<u32>(this->mDrawStates.mBits.mBit19)) {
        x *= this->fn_803A2270() ? 0.1f : 0.3f;
    }
    this->fn_803A22D0(x, arg);
}

void Mario::fn_803A20E0(f32 rate, s32 arg) {
    if (this->_3D0 == 0) {
        TVec3f sp8;
        if (!MR::vecBlendSphere(&sp8, this->_214, this->_22C, rate)) {
            MR::vecRotAxis(this->_214, this->_22C, this->mHeadVec, &sp8, 0.3926991f);
        }
        if (arg == 0) {
            this->setFrontVecKeepUp(sp8);
        }
    }
    this->mFlags14.mLowBits.mBit19 = 0;
    this->mMovementStates.mLowBits.mBit28 = 0;
}

void Mario::fn_803A2180() {
    if (this->_750 != 0 && this->mTargetWalkSpeedIndex == 0 && this->isEnableTurn()) {
        this->changeAnimation("その場足踏み", (const char*)nullptr);
        if (!this->isAnimationRun("カリカリ限界") && !this->isPlayerModeYoshi()) {
            this->fn_803EEC20("その場足踏み上半身", 0);
        }
    } else {
        if (this->mTargetWalkSpeedIndex != 0) {
            this->stopAnimation("その場足踏み", (const char*)nullptr);
        }
        if (this->isAnimationRun("その場足踏み上半身")) {
            this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
        }
    }
}

bool Mario::fn_803A2270() {
    TVec3f sp8 = -this->mFrontVec;
    f32 d = MR::diffAngleAbsHorizontal(this->_8F8, sp8, *this->getGravityVec());
    return d < 0.7853982f;
}

void Mario::fn_803A22D0(f32 x, s32 arg) {
    TVec3f sp8;
    if (static_cast<u32>(this->mFlags14.mLowBits.mBit12)) {
        this->fn_803A2420();
    }
    // The DOL tests high-word bit 8 independently of flags14 bit 12.
    if (static_cast<u32>(this->mMovementStates.mHighBits.mBit8)) {
        this->fn_803A23F0(&sp8);
        this->fn_803A2420();
        this->fn_803A2450(&sp8);
    } else if (static_cast<u32>(this->mFlags14.mLowBits.mBit19)) {
        this->fn_803A24C0(&sp8);
    } else {
        f32 d = MR::diffAngleAbs(this->mFrontVec, this->_22C);
        bool close = d < 0.1f;
        if (close) {
            this->fn_803A2530(&sp8);
        } else {
            this->fn_803A2570(x, &sp8);
            this->fn_803A2720();
            this->fn_803A27A0(&sp8);
        }
    }
    // All vector-producing paths join here at DOL 0x803A23AC.
    this->fn_803A2860();
    if (arg == 0) {
        this->setFrontVecKeepUp(sp8);
    }
}

void Mario::fn_803A23F0(TVec3f* out) {
    if ((this->mMovementStates.mHighWord >> 28) & 1) {
        out->set(this->mFrontVec);
    } else {
        *out = this->_22C;
    }
}

void Mario::fn_803A2420() {
    this->mDrawStates.mBits.mBit18 = 0;
    this->_40E = 0;
    this->mFlags14.mLowBits.mBit19 = 0;
}

void Mario::fn_803A2450(const TVec3f* arg) {
    f32 d = this->mFrontVec.dot(*arg);
    if (!(d >= 0.0f) && !this->isAnimationRun("ブレーキ")) {
        this->_74C = 3.1415927f;
        this->_750 = 10;
    }
}

void Mario::fn_803A24C0(TVec3f* arg) {
    f32 angle = 0.12f;
    if (this->_274 == 0) {
        angle = -angle;
    }
    TMtx34f matrix;
    PSMTXRotAxisRad(matrix, &this->mHeadVec, angle);
    PSMTXMultVecSR(matrix, &this->_214, arg);
}

void Mario::fn_803A2530(TVec3f* out) {
    bool differs = _524 != _528;
    if (!differs) {
        *out = this->_22C;
    } else {
        *out = this->mFrontVec;
    }
}

void Mario::fn_803A2570(f32 x, TVec3f* arg) {
    f32 d = MR::diffAngleAbs(this->mFrontVec, this->_22C);
    f32 s = MR::clamp(x / d, 0.0f, 1.0f);
    this->fn_803A2690(&s);
    if ((this->getPlayer()->mFlags14.mLowWord >> 13) & 1) {
        s *= this->fn_803A26C0(d);
    }
    if (this->isPlayerModeYoshi()) {
        if (this->mActor->getYoshi()->mDashTimer > 0) {
            s *= 0.5f;
        }
    }
    if (!MR::vecBlendSphere(arg, this->mFrontVec, this->_22C, s)) {
        this->_3D4 = 0;
        MR::vecRotAxis(this->_214, this->_22C, this->mHeadVec, arg, 0.3926991f);
    }
}

void Mario::fn_803A2690(f32* p) {
    if (this->mActor->mBeeWallWalk != 0) {
        if (*p > 0.1f) {
            *p = 0.1f;
        }
    }
}

f32 Mario::fn_803A26C0(f32 x) {
    // NONMATCHING: floating-point register allocation only.
    MarioConstTable* e = this->mActor->mConst->getTable();
    f32 f3 = e->mStickHeavyMinRatio;
    f32 f0 = e->mStickHeavyMinAngle;
    f32 f4 = e->mStickHeavyMaxAngle;
    if (x > f4) {
        return 1.0f;
    }
    if (x > f0) {
        f32 f2 = 1.0f - f3;
        f32 t = (x - f0) / (f4 - f0);
        return f3 + f2 * t;
    }
    return f3;
}

void Mario::fn_803A2720() {
    if ((this->getPlayer()->mFlags14.mLowWord >> 13) & 1) {
        f32 d = MR::diffAngleAbs(this->mFrontVec, this->_22C);
        f32 c = MR::clamp(1.0f - (3.1415927f - d) / 3.1415927f, 0.0f, 1.0f);
        this->mWalkSpeed = this->mWalkSpeed * (1.0f - 0.1f * c);
    }
}

void Mario::fn_803A27A0(TVec3f* arg) {
    if (MR::diffAngleAbs(this->mFrontVec, this->_22C) < 2.5f) {
        return;
    }
    if (this->_3D4 != this->mActor->mConst->getTable()->mWeakTurnTime) {
        return;
    }
    if (static_cast<u32>(this->mDrawStates.mBits.mBit18)) {
        return;
    }
    TVec3f sp8;
    PSVECCrossProduct(&this->_214, arg, &sp8);
    f32 d = sp8.dot(this->mHeadVec);
    this->_274 = d > 0.0f;
    this->mFlags14.mLowBits.mBit19 = 1;
}

void Mario::fn_803A2860() {
    if (!this->isAnimationRun("その場足踏み") && this->_214.dot(this->_22C) > 0.99f) {
        this->mMovementStates.mLowBits.mBit28 = 0;
    } else {
        this->mMovementStates.mLowBits.mBit28 = 1;
    }
}

void Mario::fn_803A28D0(TVec3f* arg) {
    this->fn_803A2A70();
    this->fn_803A2C40(arg);
    if (static_cast<u32>(this->mMovementStates.mHighBits.mBit13)) {
        if (this->_4E0 > 120.0f) {
            const TVec3f* n = this->mFrontWallTriangle->getNormal(0);
            if (this->mVelocity.dot(*n) < 0.0f) {
                TVec3f sp56;
                MR::vecKillElement(this->mPosHolder->mPos - this->_4E8, *this->mFrontWallTriangle->getNormal(0), &sp56);
                MR::vecKillElement(sp56, *this->getGravityVec(), &sp56);
                MR::vecKillElement(sp56, this->mFrontVec, &sp56);
                MR::normalizeOrZero(&sp56);
                if (!MR::isNearZero(sp56, 0.001f)) {
                    f32 m = PSVECMag(&this->mVelocity);
                    this->mVelocity = sp56 * m;
                }
            }
        }
    }

    TVec3f sp44(this->mVelocity);
    f32 m2 = PSVECMag(&sp44);
    MR::normalizeOrZero(&sp44);
    TVec3f sp32;
    PSVECCrossProduct(&this->_368, &sp44, &sp32);
    MR::normalizeOrZero(&sp32);
    PSVECCrossProduct(&sp32, &this->_368, &sp44);
    sp44.setLength(m2);
    this->mVelocity = sp44;
}

// NONMATCHING: temporary allocation and register scheduling; full velocity dispatch recovered.
void Mario::fn_803A2A70() {
    MarioConst* constants = mActor->mConst;
    f32 speed = constants->mTable[constants->mCurrentTable]->mWalkSpeed;
    if (static_cast<u32>(this->mMovementStates.mHighBits.mBit11)) {
        TVec3f front(this->mFrontVec);
        TVec3f direction(this->mFrontVec);
        if (!MR::isNearZero(this->_16C, 0.001f)) {
            direction = this->_16C;
        }
        MR::normalize(&direction);
        constants = mActor->mConst;
        f32 rate = MR::clamp((1.1f - mWalkSpeed) * constants->mTable[constants->mCurrentTable]->mInertiaIceTurn, 0.0f, 1.0f);
        TVec3f blended;
        MR::vecBlendSphere(&blended, direction, front, rate);
        MR::normalize(&blended);
        blended *= speed * mWalkSpeed;
        addVelocity(blended);
        return;
    }
    if (static_cast<u32>(this->mDrawStates.mBits.mBit26)) {
        addVelocity(this->_22C * speed * mWalkSpeed);
        return;
    }
    addVelocity(this->mFrontVec * speed * mWalkSpeed);
}

void Mario::fn_803A2C40(const TVec3f* arg) {
    this->_328 = *arg;
    if ((this->mMovementStates.mLowWord >> 21) & 1) {
        if (this->mTargetWalkSpeedIndex != 0) {
            this->_334 = *arg;
        }
    } else {
        if (this->_3D2 == 0) {
            this->_334 = *arg;
        }
    }
}

void Mario::fn_803A2CC0() {
    if (!(static_cast<u32>(this->mMovementStates.mHighBits.mBit28))) {
        return;
    }
    this->mTargetWalkSpeedIndex = 0;
    this->mWalkSpeed = 0.0f;
    this->playSound("坂滑り", -1);
}

void Mario::fn_803A2CF0() {
    if (this->_960 == 35) {
        this->mMovementStates.mHighBits.mBit10 = 1;
    }
}

s32 Mario::fn_803A2D10() {
    s32 r = 0;
    if (((this->mDrawStates.mWord >> 26) & 1) || ((this->mMovementStates.mHighWord >> 28) & 1) || this->_754 != 0) {
        r = 1;
    }
    return r;
}

// SMG2-only Mario:: functions (fn_ range B), written from the asm. Included at the end of Mario.cpp (unity TU).


void Mario::fn_803A2D50() {
    if (this->_436 == 0) {
        this->_436 = 0;
        return;
    }
    this->_436--;
}

s32 Mario::fn_803A2D80() {
    if (static_cast<u32>(this->mFlags14.mLowBits.mBit5)) {
        this->tryJump();
        if (this->isAnimationRun("その場足踏み上半身")) {
            this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
        }
        this->_420 = 0;
        this->mJumpVec = this->_184;
        this->mFlags14.mLowBits.mBit5 = 0;
        return 1;
    }
    if (!this->mActor->isRequestJump() && !(static_cast<u32>(this->mFlags20.mBits.mBit21)) && !(static_cast<u32>(this->mMovementStates.mHighBits.mBit7))) {
        return 0;
    }
    if ((static_cast<u32>(this->mMovementStates.mLowBits.mBit21)) && this->calcDistToCeil(false) < 160.0f) {
        this->mActor->sendMsgUpperPunch(this->_730);
        this->changeAnimation("しゃがみアッパー", (const char*)nullptr);
        return 0;
    }
    if (this->isStatusActive(0x1F)) {
        this->mSkate->fn_803F4AF0();
        this->closeStatus(this->mSkate);
        return 0;
    }
    if (this->_436 != 0 && this->mStickPos.z >= 0.5f) {
        this->fn_803A2EF0();
        return 0;
    }
    this->fn_803A2FC0();
    return 1;
}

void Mario::fn_803A2EF0() {
    if (this->checkSquat(false)) {
        this->_436 = 0;
        this->mWalkSpeed = 5.0f;
        this->_434 = 0xB4;
        this->playSound("スペシャルダッシュ強", -1);
        this->playSound("ダッシュ加速強成功", -1);
        this->playSound("声物ジャンプ", -1);
        return;
    }
    this->_436 = 0;
    this->mWalkSpeed = 2.0f;
    this->_434 = 0x3C;
    this->playSound("スペシャルダッシュ弱", -1);
    this->playSound("声物ジャンプ", -1);
}

void Mario::fn_803A2FC0() {
    this->saveLastSafetyTrans();
    this->tryJump();
    if (this->isAnimationRun("その場足踏み上半身")) {
        this->stopAnimationUpper(nullptr, nullptr);
    }
    this->_420 = 0;
}

s32 Mario::fn_803A3030() {
    if (!(static_cast<u32>(this->mMovementStates.mLowBits.mBit30))) {
        if (this->_3C0 != 0 && !(static_cast<u32>(this->mMovementStates.mHighBits.mBit28)) && MR::isNearZero(this->_8F8, 0.001f)) {
            this->_3C0 -= 1;
            return 0;
        }
        if (!this->mActor->_EA4) {
            this->tryDrop();
            this->_420 = 0;
            return 1;
        }
        return 0;
    }
    MarioConstTable* entry = this->mActor->mConst->getTable();
    if (this->mStickPos.z < entry->mWallBackHangStickPower) {
        this->_3C0 = 0;
    } else if (static_cast<u32>(this->mDrawStates.mBits.mBit21)) {
        this->_3C0 = 0;
    } else if (this->_3C0 < entry->mDropWaitTime) {
        this->_3C0 = entry->mDropWaitTime;
    }
    return 0;
}

bool Mario::fn_803A3130() {
    if (this->_420 == 0) {
        return false;
    }
    this->_420--;
    this->mFlags14.mHighBits.mBit23 = 1;
    return true;
}

void Mario::recordTurnSlipAngle() {
    if (!mActor->isBeeWallWalk()) {
        _3E4 = mFrontVec;
        _3D2 = mActor->getConst().getTable()->mTurnReadyTime;
    }
}

// NONMATCHING: independent loads and temporary/register allocation.
f32 Mario::decideInertia(f32 speed) {
    sInertiaName = "スケート";
    if (isStatusActive(31)) return decideInertiaOnIce(speed);
    sInertiaName = "速度超過";
    if (mWalkSpeed > 1.0f) {
        MarioConst* c = mActor->mConst;
        return c->mTable[c->mCurrentTable]->mTurnAngleSpeed;
    }
    sInertiaName = "床";
    u32 flags = mMovementStates.mHighWord;
    if ((flags >> 11) & 1) return decideInertiaOnIce(speed);
    if ((flags >> 10) & 1) return decideInertiaOnSlip(speed);
    sInertiaName = "基本";
    MarioConst* c = mActor->mConst;
    MarioConstTable* table = c->mTable[c->mCurrentTable];
    f32 base = (1.0f - mWalkSpeed) * table->mInertiaStandardStop + mWalkSpeed * table->mInertiaStandardMax;
    f32 inertia = base * (1.0f - _3F4) + _3F4 * table->mInertiaOverSpeed;
    if (base != inertia) sInertiaName = "空転";
    if (speed == 0.0f) {
        sInertiaName = "停止";
        c = mActor->mConst;
        inertia = c->mTable[c->mCurrentTable]->mInertiaStop;
        if (static_cast<u32>(mMovementStates.mLowBits.mBit15)) {
            sInertiaName = "ターン滑り";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaTurnSlip;
        }
        if (!(static_cast<u32>(mMovementStates.mLowBits.mBit15)) && isAnimationRun("歩行制動ブレーキ")) {
            sInertiaName = "ターン後滑り";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaTurning;
        }
        if (static_cast<u32>(mMovementStates.mLowBits.mBit27)) {
            sInertiaName = "ターン中";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaJumpFinish;
        }
        if (_3CE < 30) {
            sInertiaName = "ジャンプ終了";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaReflectSlip;
        }
        if (static_cast<u32>(mMovementStates.mLowBits.mBit21)) {
            sInertiaName = "しゃがみ";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaSquat;
        }
    }
    if (static_cast<u32>(mMovementStates.mLowBits.mBit16)) {
        sInertiaName = "トルネード";
        c = mActor->mConst;
        if (mWalkSpeed >= speed) inertia = c->mTable[c->mCurrentTable]->mInertiaTornadoBrake;
        else inertia = c->mTable[c->mCurrentTable]->mInertiaTornadoAccel;
    }
    if (_3F8 != 0) {
        sInertiaName = "反射";
        _3F8--;
        c = mActor->mConst;
        inertia = c->mTable[c->mCurrentTable]->mInertiaStartSpin;
    }
    if (mWalkSpeed < 0.08f && _3CE > 10 && speed > 0.5f && ((static_cast<u32>(mMovementStates.mLowBits.mBit21)) || (static_cast<u32>(mMovementStates.mLowBits.mBit19)))) {
        c = mActor->mConst;
        _3FA = c->mTable[c->mCurrentTable]->mAutoBdashTime;
        mWalkSpeed = 0.08f;
    }
    if (_3FA != 0) {
        sInertiaName = "停止急発進";
        _3FA--;
        c = mActor->mConst;
        inertia = c->mTable[c->mCurrentTable]->mInertiaOverSpeed;
        if (_3FA == 0) _3FC = 60;
    }
    if (_3FC != 0) _3FC--;
    if (fn_803EF7C0() == 32 && mWalkSpeed > 0.4f) inertia *= 0.5f;
    return inertia;
}

f32 Mario::decideInertiaOnIce(f32 speed) {
    if (speed > 1.0f) {
        speed = 1.0f;
    }

    f32 inertia =
        (1.0f - speed) * mActor->getConst().getTable()->mInertiaIceStandardStop + speed * mActor->getConst().getTable()->mInertiaIceStandardMax;
    inertia = (1.0f - _3F4) * inertia + _3F4 * mActor->getConst().getTable()->mInertiaIceStartSpin;
    if (speed == 0.0f) {
        inertia = mActor->getConst().getTable()->mInertiaIceStop;
    }

    return inertia;
}

f32 Mario::decideInertiaOnSlip(f32 speed) {
    f32 inertia = (1.0f - mWalkSpeed) * mActor->getConst().getTable()->mInertiaSlipStandardStop +
                  mWalkSpeed * mActor->getConst().getTable()->mInertiaSlipStandardMax;
    inertia = (1.0f - _3F4) * inertia + _3F4 * mActor->getConst().getTable()->mInertiaSlipStartSpin;
    if (speed == 0.0f) {
        inertia = mActor->getConst().getTable()->mInertiaSlipStop;
    }

    return inertia;
}

void Mario::calcShadowDir(const TVec3f& rInput, TVec3f* pShadowDir) {
    TVec3f direction;
    direction = rInput;
    if (!MR::isNormalize(direction)) {
        MR::normalizeOrZero(&direction);
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit8)) || (static_cast<u32>(mFlags14.mLowBits.mBit10))) {
        calcShadowDir2D(direction, pShadowDir);
        return;
    }

    TVec3f normal = -*getGravityVec();
    f32 surfaceDot = MR::abs(direction.dot(_368));
    f32 gravityDot = MR::abs(direction.dot(normal));
    if (gravityDot > surfaceDot) {
        normal = _368;
    }

    TVec3f side = normal.cross(direction);
    pShadowDir->cross(side, normal);
    MR::normalizeOrZero(pShadowDir);
}

f32 Mario::fn_803A3780(f32 angle) {
    f32 difference = MR::diffAngleAbs(angle, this->_2B4);
    if (this->isAnimationRun("その場足踏み")) {
        difference = 1.0f;
    }
    if (this->isAnimationRun("ターンブレーキ")) {
        difference = 1.0f;
    }
    if (this->isSwimming()) {
        difference = 1.0f;
    }
    if (this->mActor->isRequestSpin() && (static_cast<u32>(this->mMovementStates.mLowBits.mBit31)) != 0 && this->mWorldPadDir.dot(this->_16C) < 0.0f) {
        difference = 1.0f;
    }
    if (static_cast<u32>(this->mFlags14.mLowBits.mBit20)) {
        difference = 1.0f;
    }
    return difference;
}

void Mario::retainMoveDir(f32 stickX, f32 stickY) {
    f32 angle = MR::atan2(stickY, stickX);
    f32 difference = fn_803A3780(angle);
    if ((static_cast<u32>(mFlags14.mLowBits.mBit20)) && isStickOn()) {
        mFlags14.mLowBits.mBit20 = 0;
    }
    f32 limit = 0.06f;
    if (_3CE < 2) {
        limit = 0.1f;
    }
    bool retain = true;
    mDrawStates.mBits.mBit9 = 1;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !(static_cast<u32>(mFlags14.mLowBits.mBit21))) {
        retain = false;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && (static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        retain = false;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && !isAnimationRun(nullptr)) {
        retain = false;
    }
    if (difference < limit && isStickOn() && retain) {
        if (_40E == 0 || ((static_cast<u32>(mFlags14.mLowBits.mBit21)) && (static_cast<u32>(mMovementStates.mLowBits.mBit31)))) {
            mDrawStates.mBits.mBit18 = 1;
            _40E = 0;
        } else {
            if (_40E != 0) {
                _40E--;
            }
            _29C = _368;
            _2A8 = *getGravityVec();
        }
    } else {
        if (static_cast<u32>(mMovementStates.mLowBits.mBit31)) {
            if (isStickOn()) {
                mFlags14.mLowBits.mBit21 = 0;
            }
        } else {
            mFlags14.mLowBits.mBit21 = 1;
        }
        if (_40E == 0) {
            _3D4 = 0;
        }
        _40E = 30;
        if (!isStickOn()) {
            mFlags14.mLowBits.mBit20 = 1;
        }
        _29C = _368;
        _2A8 = *getGravityVec();
    }
    _2B4 = angle;
}

void Mario::calcMoveDir(float stickX, float stickY, TVec3f* pMoveDir, bool isDisableMove) {
    if (static_cast<u32>(mMovementStates.mHighBits.mBit8)) {
        calcDir2D(stickX, stickY, pMoveDir);
        return;
    }
    if (static_cast<u32>(mFlags14.mLowBits.mBit10)) {
        calcMoveDir2D(stickX, stickY, pMoveDir);
        return;
    }
    if (static_cast<u32>(mMovementStates.mHighBits.mBit5)) {
        calcMoveDir25D(stickX, stickY, pMoveDir);
        return;
    }
    if (isDisableMove) {
        retainMoveDir(stickX, stickY);
    }
    const TVec3f& screenX = getCamDirX();
    const TVec3f& screenY = getCamDirY();
    TVec3f screenZ = -getCamDirZ();
    TVec3f groundX;
    TVec3f groundY;
    TVec3f selectedBasis;
    TVec3f up = -*getGravityVec();
    if (!MR::isNearZero(up, 0.001f)) {
        if (_398.dot(up) <= 0.0f) {
            _398 = up;
        } else {
            MR::vecBlendSphere(&_398, _398, up, 0.1f);
        }
        MR::normalizeOrZero(&_398);
    }
    f32 yAlignment = screenY.dot(_398);
    f32 zAlignment = screenZ.dot(_398);
    if (MR::abs(zAlignment) > MR::abs(yAlignment)) {
        selectedBasis = screenY;
        if (zAlignment < 0.0f) {
            selectedBasis = -selectedBasis;
        }
        groundX.cross(selectedBasis, _398);
    } else {
        selectedBasis = -screenZ;
        if (yAlignment < 0.0f) {
            selectedBasis = -selectedBasis;
        }
        groundX.cross(selectedBasis, _398);
    }
    groundY = -selectedBasis;
    MR::normalizeOrZero(&groundX);
    if (MR::isNearZero(groundX, 0.001f)) {
        groundX = screenX;
    }
    *pMoveDir = groundX * stickX - groundY * stickY;
    if (static_cast<u32>(mFlags14.mLowBits.mBit12)) {
        MR::calcPlayerKillVecOnPlane(pMoveDir, *pMoveDir, mPosHolder->mPos, _6F4, _700);
    }
}

bool Mario::checkLockOnHoming() {
    if (mStickPos.z != 0.0f) {
        return false;
    }

    if (!checkPreLvlZ()) {
        return false;
    }

    if (isSwimming()) {
        return false;
    }

    if (!mActor->_470) {
        return false;
    }

    mMovementStates.mLowBits.mBit21 = 0;
    mDrawStates.mBits.mBit26 = 1;
    doLockOnHoming();
    return true;
}

void Mario::doLockOnHoming() {
    TVec3f direction;
    direction.set(*MR::getSensorPos(mActor->_470) - mPosHolder->mPos);
    MR::normalize(&direction);
    TVec3f shadowDir;
    calcShadowDir(direction, &shadowDir);
    if (MR::diffAngleAbsHorizontal(mFrontVec, direction, mHeadVec) >= 0.05235988f) {
        if (isAnimationStop()) {
            changeAnimation("その場足踏み");
            if (!_750) {
                setFrontVecKeepUp(direction, 15UL);
                _334 = direction;
            }
        }
    }
}

bool Mario::isEnableTurn() {
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit27))) {
        return false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit26))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit11))) {
        return false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit21))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        return false;
    }

    if (isStatusActive(MarioStatus_Magic)) {
        return false;
    }

    if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
        return false;
    }

    if (isAnimationRun("坂すべり下向きあおむけ", 3)) {
        return false;
    }

    if (isAnimationRun("坂すべり下向き終了")) {
        return false;
    }

    if (isAnimationRun("坂すべり上向き終了")) {
        return false;
    }

    if (isAnimationRun("飛び込み失敗着地")) {
        return false;
    }

    if (isAnimationRun("飛び込み失敗回転着地")) {
        return false;
    }

    if (mActor->_480) {
        return false;
    }

    if (mActor->isPunching()) {
        return false;
    }

    if (mActor->isItemSwinging()) {
        return false;
    }

    if (mActor->isBeeWallWalk()) {
        return false;
    }

    if (mActor->_3C0) {
        return false;
    }

    if (mActor->_EA4) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit8)) || (static_cast<u32>(mFlags14.mLowBits.mBit10))) {
        return false;
    }

    return true;
}

void Mario::fixPositionInTower() {
    if (fn_803A4210()) {
        bool blocked = false;
        if (isStatusActiveID(0x11) || isStatusActiveID(0x1F)) {
            blocked = true;
        }
        if (!blocked) {
            fn_803A42B0();
        }
    } else {
        if (isAnimationRun("ブレーキ")) {
            doBrakingAnimation();
            mTargetWalkSpeedIndex = 0;
            return;
        }
        stopAnimation("ブレーキ滑り床", (const char*)nullptr);
    }
}

bool Mario::fn_803A4210() {
    bool result = false;
    if (!((this->getPlayer()->getMovementStates().mLowWord >> 16) & 1) && ((this->getPlayer()->getMovementStates().mLowWord >> 15) & 1)
        && !((this->getPlayer()->getMovementStates().mLowWord >> 21) & 1) && this->_8F0 == 0.0f
        && static_cast<u8>(this->checkStickFrontBack()) == 2) {
        result = true;
    }
    return result;
}

void Mario::fn_803A42B0() {
    if (this->getPlayer()->mMovementStates.mHighWord >> 10 & 1) {
        this->changeAnimation("ブレーキ滑り床", (const char*)nullptr);
        return;
    }
    this->changeAnimation("ブレーキ", (const char*)nullptr);
}

void Mario::set25Dmode(const AreaObj* pArea) {
    Mtx matrix;
    TVec3f rotation;
    MR::calcCubeRotate(pArea, &rotation);
    TVec3f normal(0.0f, 0.0f, 1.0f);
    TVec3f up(0.0f, 1.0f, 0.0f);
    MR::makeMtxTR(matrix, 0.0f, 0.0f, 0.0f, rotation.x, rotation.y, rotation.z);
    PSMTXMultVec(matrix, &normal, &normal);
    PSMTXMultVec(matrix, &up, &up);
    _6D0 = normal;
    _6DC = up;
    _6E8.cross(_6D0, _6DC);
    MR::normalize(&_6E8);
    mMovementStates.mHighBits.mBit5 = 1;
}

void Mario::update25Dmode() {
    TVec3f up;
    MR::vecKillElement(-*getGravityVec(), _6D0, &up);
    MR::normalizeOrZero(&up);
    if (MR::isNearZero(up)) {
        mAxisMode6A4 = -getGravityVec()->dot(_6D0) > 0.0f ? 4 : 5;
        return;
    }

    f32 sideDot = up.dot(_6E8);
    f32 upDot = up.dot(_6DC);
    f32 normalDot = up.dot(_6D0);
    f32 sideAbs = MR::abs(sideDot);
    f32 upAbs = MR::abs(upDot);
    f32 normalAbs = MR::abs(normalDot);
    u8 mode;
    if (sideAbs > upAbs && sideAbs > normalAbs) {
        mode = sideDot > 0.0f ? 2 : 3;
    } else if (upAbs > sideAbs && upAbs > normalAbs) {
        if (upDot > 0.0f) {
            mode = 0;
        } else {
            mode = 1;
        }
    } else {
        mode = normalDot > 0.0f ? 4 : 5;
    }

    mCandidateAxisMode6A5 = mode;
    if (mode == 2 || mode == 3) {
        if (mode == 2 || mode == 3) {
            f32 threshold = 0.965f;
            if (mAxisMode6A4 == 2 || mAxisMode6A4 == 3) {
                threshold = 0.9397f;
            }

            if (sideAbs < threshold) {
                if (upAbs > normalAbs) {
                    if (upDot > 0.0f) {
                        mode = 0;
                    } else {
                        mode = 1;
                    }
                } else {
                    return;
                }
            }
        }
    }

    if (!isStickOn()) {
        mAxisMode6A4 = mode;
        mMovementStates.mHighBits.mBit4 = 0;
    } else if (!(static_cast<u32>(mMovementStates.mHighBits.mBit4)) && mStickPos.z > 0.7f) {
        _6C8.set(mStickPos.x, mStickPos.y);
        mAxisMode6A4 = mode;
        mMovementStates.mHighBits.mBit4 = 1;
    } else {
        TVec2f stick(mStickPos.x, mStickPos.y);
        if (MR::diffAngleAbs(_6C8, stick) >= 0.7853982f) {
            mAxisMode6A4 = mode;
            _6C8.set(mStickPos.x, mStickPos.y);
        }
    }
}

void Mario::updateAxisFromMode(u8 mode) {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        return;
    }

    TVec3f side;
    TVec3f normal;
    if (mode < 4) {
        TVec3f up;
        TVec3f planeUp;
        MR::vecKillElement(-*getGravityVec(), _6D0, &up);
        MR::vecKillElement(_6DC, _6D0, &planeUp);
        MR::normalizeOrZero(&up);
        MR::normalizeOrZero(&planeUp);
        f32 dot = up.dot(planeUp);
        if (MR::abs(dot) > 0.99f) {
            switch (mode) {
            case 0:
                side = _6E8;
                normal = _6D0;
                if (dot < 0.0f) {
                    side = -side;
                }

                break;
            case 1:
                side = _6E8;
                normal = _6D0;
                if (dot > 0.0f) {
                    side = -side;
                }

                break;
            case 2:
                side = _6D0;
                normal = -_6E8;
                if (dot < 0.0f) {
                    side = -side;
                    normal = -normal;
                }

                break;
            case 3:
                side = -_6D0;
                normal = _6E8;
                if (dot < 0.0f) {
                    side = -side;
                    normal = -normal;
                }

                break;
            }
        } else {
            TVec3f axis;
            axis.cross(up, planeUp);
            MR::normalize(&axis);
            if (axis.dot(_6D0) < 0.0f) {
                axis = -axis;
            }

            TVec3f tangent;
            tangent.cross(axis, up);
            MR::normalize(&tangent);
            switch (mode) {
            case 0:
                side = tangent;
                normal = axis;
                break;
            case 1:
                side = -tangent;
                normal = axis;
                break;
            case 2:
                side = axis;
                normal = -tangent;
                break;
            case 3:
                side = -axis;
                normal = tangent;
                break;
            }
        }
    }

    _6B0 = side;
    _6BC = -normal;
}

void Mario::calcMoveDir25D(f32 x, f32 y, TVec3f* pDirection) {
    *pDirection = _6B0 * x - _6BC * y;
}

void Mario::check2DMode() {
    mMovementStates.mHighBits.mBit8 = 0;
    mFlags14.mLowBits.mBit10 = 0;

    AreaObj* previousArea = mPlaneModeArea;
    AreaObj* area = MR::getAreaObj("PlaneModeCube", mPosHolder->mPos);
    mPlaneModeArea = area;
    if (area != nullptr) {
        mMovementStates.mHighBits.mBit8 = 1;

        TVec3f rotate;
        MR::calcCubeRotate(area, &rotate);
        MR::makeMtxTR(_F4, 0.0f, 0.0f, 0.0f, rotate.x, rotate.y, rotate.z);

        if (previousArea != area) {
            _688 = mPosHolder->mPos;
            MR::getRotatedAxisZ(&mSwimSideVec694, rotate);
            mSwimSideVec694 = -mSwimSideVec694;
            MR::calcCubePos(area, &_694);
        }
    } else {
        area = MR::getAreaObj("PlaneCircularModeCube", mPosHolder->mPos);
        if (area != nullptr) {
            TVec3f rotate;
            MR::calcCubeRotate(area, &rotate);
            PSMTXCopy(MR::tmpMtxRotYDeg(rotate.y), _F4);
            mFlags14.mLowBits.mBit10 = 1;

            if (!((_28 >> 10) & 1)) {
                _688 = mPosHolder->mPos;
                MR::getRotatedAxisZ(&mSwimSideVec694, rotate);
                MR::calcCubePos(area, &_694);
            }
        }
    }

    mMovementStates.mHighBits.mBit5 = 0;
    area = MR::getAreaObj("PipeModeCube", mPosHolder->mPos);
    if (area != nullptr) {
        set25Dmode(area);
    }
    mActor->fn_803D8A30(static_cast<u32>(mMovementStates.mHighBits.mBit8), mSwimSideVec694);

}

void Mario::calcMoveDir2D(f32 a1, f32 a2, TVec3f* pOut) {
    if (a1 != 0.0f) {
        a2 = 0.0f;
    }

    TVec3f stack_8C;
    TVec3f stack_80;
    TVec3f stack_74;
    TVec3f stack_68;
    TVec3f stack_5C;

    stack_8C = mSide2D;
    stack_80 = mUp2D;
    stack_74 = mNormal2D;

    TVec3f stack_50(_660);
    stack_68.cross(stack_74, stack_50);
    MR::normalize(&stack_68);

    if (stack_50.dot(stack_80) < 0.0f) {
        stack_68 = -stack_68;
    }

    stack_5C = -stack_80;
    TVec3f stack_44(_398);

    if (_398.dot(stack_50) <= 0.0f) {
        _398 = stack_50;
    } else {
        MR::vecBlendSphere(_398, stack_50, &_398, 0.1f);
    }

    if (MR::isNearZero(_398)) {
        _398 = stack_44;
    } else {
        MR::normalize(&_398);
    }

    *pOut = stack_68 * a1 - stack_5C * a2;
}

void Mario::stick2Dadjust(f32& rStickX, f32& rStickY) {
    bool isNearHalfPi = false;

    if (((mFlags14.mLowWord & Mario::MovementStates::Turning) >> 28) && (static_cast<u32>(mMovementStates.mLowBits.mBit30)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        mFlags14.mLowWord &= ~Mario::MovementStates::Turning;
        clear2DStick();
    }

    f32 stickXInput = rStickX;
    f32 stickYInput = rStickY;

    mSide2D.set(1.0f, 0.0f, 0.0f);
    mUp2D.set(0.0f, 1.0f, 0.0f);
    mNormal2D.set(0.0f, 0.0f, -1.0f);
    PSMTXMultVecSR(_F4, &mSide2D, &mSide2D);
    PSMTXMultVecSR(_F4, &mNormal2D, &mNormal2D);

    TVec3f stack_38(0.0f, 1.0f, 0.0f);
    TVec3f* camDirZ = &getCamDirZ();
    f32 angle = MR::diffAngleSignedHorizontal(getCamDirY(), stack_38, *camDirZ);

    TVec3f stack_2C(1.0f, 0.0f, 0.0f);
    camDirZ = &getCamDirZ();
    const f32 absHorizontal = MR::diffAngleAbsHorizontal(getCamDirX(), stack_2C, *camDirZ);
    if (absHorizontal >= 3.0415928f && MR::abs(angle) >= 3.0415928f) {
        angle = 0.0f;
        rStickX = -rStickX;
    }

    Mtx rotMtx;
    PSMTXRotAxisRad(rotMtx, &mSwimSideVec694, angle);
    PSMTXMultVecSR(rotMtx, &mSide2D, &mSide2D);
    PSMTXMultVecSR(rotMtx, &mUp2D, &mUp2D);

    MR::vecKillElement(_368, mSwimSideVec694, &_660);
    if (MR::normalizeOrZero(&_660)) {
        _660 = _368;
    }

    set2Dmode(_660.dot(mUp2D) >= 0.0f);

    bool isMainY = false;
    bool isMainX = false;

    if (_66C) {
        if (MR::isNearZero(mStickPos.z)) {
            _66C = false;
        } else {
            TVec3f stickVec(rStickX, rStickY, 0.0f);
            const f32 diff = MR::diffAngleAbs(stickVec, _670);

            if (_670.x * rStickX < 0.0f) {
                _66C = false;
            } else if (_670.y * rStickY < 0.0f) {
                _66C = false;
            } else if (diff < 0.3f) {
                rStickX = _67C.x;
                rStickY = _67C.y;
            } else {
                _66C = false;
            }
        }
    }

    if (!_66C && !MR::isNearZero(mStickPos.z) && !_60E && !_60F) {
        TVec3f stickVec(rStickX, rStickY, 0.0f);
        PSMTXMultVecSR(_F4, &stickVec, &stickVec);
        PSMTXMultVecSR(rotMtx, &stickVec, &stickVec);

        TVec3f invMove(-_660);
        const f32 diff = MR::diffAngleAbs(invMove, stickVec);
        f32 clamp = MR::clamp(1.0f - (mStickPos.z * mStickPos.z), 0.0f, 1.0f);

        const f32 threshold = MR::getInterpolateValue(clamp, 0.5f, 1.5f);
        if (diff < threshold) {
            rStickX = 0.0f;
            rStickY = 0.0f;
            mStickPos.z = 0.0f;
        } else if (diff > (3.1415927f - threshold)) {
            rStickX = 0.0f;
            rStickY = 0.0f;
            mStickPos.z = 0.0f;
        } else {
            _670 = stickVec;
            MR::vecKillElement(stickVec, _660, &stickVec);
            rStickX = stickVec.x;
            rStickY = stickVec.y;
            _67C = stickVec;
            _66C = true;
        }
    }

    if (MR::abs(rStickY) > MR::abs(rStickX)) {
        isMainY = true;
        rStickX = 0.0f;
        if (rStickY > 0.0f) {
            rStickY = mStickPos.z;
        } else {
            rStickY = -mStickPos.z;
        }
    } else {
        isMainX = true;
        rStickY = 0.0f;
        if (rStickX > 0.0f) {
            rStickX = mStickPos.z;
        } else {
            rStickX = -mStickPos.z;
        }
    }

    const f32 angleMove = MR::diffAngleAbs(_660, mUp2D);
    u8 lockX = 0;
    u8 lockY = 0;

    if (!((_60E != 0 || _60F != 0) && _610 == 0 && _611 == 0)) {
        bool doSecondBlock = false;

        if ((angleMove < 0.5235988f || angleMove > 2.617994f || _610 != 0) && _611 == 0) {
            if (mPrevious2DStickY == 0.0f) {
                rStickY = 0.0f;
                if (isMainY && MR::abs(stickXInput) > 0.3f) {
                    rStickX = mStickPos.z;
                    if (stickXInput < 0.0f) {
                        rStickX = -rStickX;
                    }
                    lockX = 1;
                }
            }

        } else {
            if (!(angleMove > 1.3962635f && angleMove < 1.7453294f) && _611 == 0) {
            } else if (_610 == 0 && mPrevious2DStickX == 0.0f) {
                rStickX = 0.0f;
                if (isMainX && MR::abs(stickYInput) > 0.3f) {
                    rStickY = mStickPos.z;
                    if (stickYInput < 0.0f) {
                        rStickY = -rStickY;
                    }
                    lockY = 1;
                }
            }
        }
    }

    mPrevious2DStickX = rStickX;
    mPrevious2DStickY = rStickY;
    _610 = lockX;
    _611 = lockY;

    if (!_60E && !_60F && rStickY != 0.0f) {
        if (MR::isInRange(angleMove, 0.19634955f, 1.5707964f)) {
            if (_660.dot(mSide2D) < 0.0f) {
                _60F = 1;
            } else {
                _60F = 2;
            }
        } else if (MR::isInRange(angleMove, 1.5707964f, 2.9452431f)) {
            if (_660.dot(mSide2D) < 0.0f) {
                _60F = 2;
            } else {
                _60F = 1;
            }
        }
    }

    switch (_60F) {
    case 1:
        if (rStickY == 0.0f) {
            _60F = 0;
        } else {
            rStickX = rStickY;
        }
        break;
    case 2:
        if (rStickY == 0.0f) {
            _60F = 0;
        } else {
            rStickX = -rStickY;
            rStickY = -rStickY;
        }
        break;
    default:
        _60F = 0;
        break;
    }

    switch (_60E) {
    case 1:
        if (rStickX > 0.0f) {
            rStickX = -rStickX;
        } else {
            _60E = 0;
        }
        break;
    case 2:
        if (rStickX < 0.0f) {
            rStickX = -rStickX;
        } else {
            _60E = 0;
        }
        break;
    default:
        _60E = 0;
        break;
    }

    _614 = rStickX;
    _618 = rStickY;

    if (MR::isNearZero(angleMove - 1.5707964f, 0.0001f)) {
        isNearHalfPi = true;
    }

    if (!isNearHalfPi) {
        rStickY = 0.0f;
        mStickPos.z = MR::abs(rStickX);
    } else {
        rStickX = 0.0f;
        mStickPos.z = MR::abs(rStickY);
    }
}

void Mario::set2Dmode(bool mode) {
    if (mode == _624) {
        return;
    }

    if (_614 > 0.0f) {
        _60E = 1;
    }

    if (_614 < 0.0f) {
        _60E = 2;
    }

    _624 = mode;
}

void Mario::clear2DStick() {
    _60E = 0;
    _60F = 0;
    _610 = 0;
    _611 = 0;
    _618 = 0.0f;
    _614 = 0.0f;
    mPrevious2DStickY = 0.0f;
    mPrevious2DStickX = 0.0f;
    _624 = false;
    _61C = 0.0f;
#ifdef GALAXY_NATIVE
    // The constructor calls this before the flag has been initialised. Wii's
    // byte load/nonzero test always leaves zero here; a host bool read is UB.
    _66C = false;
#else
    if (_66C) {
        _66C = false;
    }
#endif
    _670.zero();
    _67C.zero();
}

void Mario::fn_803A56C0() {
    TVec3f stick(this->mStickPos.x, this->mStickPos.y, 0.0f);
    TVec3f camera;
    camera.x = this->_368.dot(this->getCamDirX());
    camera.y = this->_368.dot(this->getCamDirY());
    camera.z = 0.0f;
    MR::normalizeOrZero(&camera);
    f32 angle = MR::diffAngleAbs(camera, stick);
    f32 t = MR::clamp(1.0f - this->mStickPos.z * this->mStickPos.z, 0.0f, 1.0f);
    f32 limit = MR::getInterpolateValue(t, 0.5f, 1.5f);
    if (angle < limit || angle > 3.1415927f - limit) {
        this->mStickPos.z = 0.0f;
        this->mStickPos.y = 0.0f;
        this->mStickPos.x = 0.0f;
    }
}

bool Mario::checkPressDamage() {
    if(_5FC) return false;
    if(isSwimming()) return false;
    if(checkVerticalPress(false)) {
        if(isStatusActive(MarioStatus_Hang)) mHang->forceDrop();
        else {
            mActor->setPress(0,0);
            if(mActor->_390) return true;
            _350.zero();_35C.zero();
            return true;
        }
    }
    if(checkSidePressPre()) { mActor->setPress(1,0);return true; }
    return checkSidePress();
}

// NONMATCHING: temporary/register allocation and independent load scheduling.
bool Mario::checkVerticalPress(bool atCurrentPosition) {
    if (static_cast<u32>(mMovementStates.mLowBits.mBit9)) return false;
    TVec3f position;
    if (isStatusActive(5)) position = mActor->_240;
    else position = mPosHolder->mPos;
    if (!atCurrentPosition && mActor->_390 == 0) position += mVelocity;
    f32 height;
    if (static_cast<u32>(mMovementStates.mLowBits.mBit21)) height = 80.0f;
    else height = 160.0f;
    if ((static_cast<u32>(getPlayer()->mFlags20.mBits.mBit16)) && (static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit30))) {
        if (_4C8->isValid() && MR::isGroundCodePress(_4C8)) {
            if (mGroundPolygon->mSensor != _4C8->mSensor) {
                f32 distance = _72C;
                MR::isNearZero(mAirGravityVec, 0.001f);
                TVec3f negativeGravity(-mAirGravityVec);
                TVec3f ray(negativeGravity);
                ray *= distance;
                TVec3f point(getTrans());
                point += ray;
                TVec3f velocity;
                MR::calcVelocityMovingPoint(_4C8, point, &velocity);
                if (velocity.dot(ray) < 0.0f) {
                    mActor->mPressNormal6D8 = _368;
                    return true;
                }
            }
        }
    }
    TVec3f up(-*getGravityVec());
    Triangle ceiling;
    TVec3f firstRay(up);
    firstRay *= height;
    TVec3f firstOffset(up);
    firstOffset *= 10.0f;
    TVec3f firstOrigin(position);
    firstOrigin -= firstOffset;
    TVec3f ceilingPos;
    bool ceilingHit;
    if (fn_803B2950()) ceilingHit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(&ceilingPos, &ceiling, firstOrigin, firstRay);
    else ceilingHit = MR::getFirstPolyOnLineToMap(&ceilingPos, &ceiling, firstOrigin, firstRay);
    if (!ceilingHit) return false;
    TVec3f ceilingDelta(ceilingPos);
    ceilingDelta -= position;
    f32 distance = up.dot(ceilingDelta);
    const TVec3f* selected = distance < 150.0f ? &ceilingPos : &position;
    TVec3f origin(*selected);
    Triangle floor;
    TVec3f secondRay(*ceiling.getNormal(0));
    secondRay *= height;
    TVec3f secondOffset(*ceiling.getNormal(0));
    secondOffset *= 10.0f;
    TVec3f secondOrigin(origin);
    secondOrigin -= secondOffset;
    TVec3f floorPos;
    bool floorHit;
    if (fn_803B2950()) floorHit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(&floorPos, &floor, secondOrigin, secondRay);
    else floorHit = MR::getFirstPolyOnLineToMap(&floorPos, &floor, secondOrigin, secondRay);
    if (floorHit) {
        if (ceiling.mSensor != floor.mSensor && (fn_803B2E10(&ceiling) || MR::isSensorPressObj(floor.mSensor))) {
            if (!atCurrentPosition && mActor->_390 == 0) {
                if (!checkVerticalPress(true)) {
                    mVelocity.zero();
                    return false;
                }
                return true;
            }
            mActor->mPressNormal6D8 = _368;
            return true;
        }
    } else if (!atCurrentPosition) {
        if (mActor->_390 != 0) return false;
        if (!MR::isSensorPressObj(ceiling.mSensor)) return false;
        mDrawStates.mWord |= 2;
        TVec3f displacement;
        if (distance < 0.0f) {
            TVec3f delta(ceilingPos);
            delta -= position;
            displacement = delta;
            displacement.setLength(height - distance);
        } else {
            TVec3f delta(position);
            delta -= ceilingPos;
            displacement = delta;
            displacement.setLength(150.0f - distance);
        }
        if (static_cast<u32>(mMovementStates.mLowBits.mBit30)) {
            if (mGroundPolygon->mSensor == ceiling.mSensor) {
                f32 length = PSVECMag(&displacement);
                TVec3f pushDirection(*ceiling.getNormal(0));
                pushDirection *= length;
                push(pushDirection);
            } else {
                mActor->_F48 = ceiling.mSensor;
                mActor->mPressNormal6D8 = _368;
                return true;
            }
        } else {
            f32 length = PSVECMag(&displacement);
            TVec3f pushDirection(*ceiling.getNormal(0));
            pushDirection *= length;
            push(pushDirection);
        }
    }
    return false;
}

// NONMATCHING: independent loads and temporary/register allocation.
bool Mario::checkSidePressPre() {
    if (_5FC) {
        return false;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && (static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        if ((_4F4 - _4E8).length() < 100.0f && mFrontWallTriangle->mSensor != mBackWallTriangle->mSensor &&
            (MR::isSensorPressObj(mFrontWallTriangle->mSensor) || MR::isSensorPressObj(mBackWallTriangle->mSensor))) {
            if (isDossun(mFrontWallTriangle) || isDossun(mBackWallTriangle)) {
                return false;
            }
            if (mFrontWallTriangle->getNormal(0)->dot(*mBackWallTriangle->getNormal(0)) < -0.5f) {
                mActor->mPressNormal6D8 = getWallNorm();
                return true;
            }
        }
    } else if (isStatusActive(MarioStatus_Hang) && (static_cast<u32>(mMovementStates.mLowBits.mBit6)) && _8C8->isValid() && _8C8->mSensor != mBackWallTriangle->mSensor &&
               (MR::isSensorPressObj(_8C8->mSensor) || MR::isSensorPressObj(mBackWallTriangle->mSensor))) {
        if (isDossun(_8C8) || isDossun(mBackWallTriangle)) {
            return false;
        }
        TVec3f horizontal;
        MR::vecKillElement(mPosHolder->mPos - _4F4, getAirGravityVec(), &horizontal);
        if (horizontal.length() < 100.0f) {
            mActor->mPressNormal6D8 = getWallNorm();
            return true;
        }
    }
    return false;
}

#pragma opt_propagation off
// NONMATCHING: temporary/register allocation; canonical SMG2 strike records and gates recovered.
bool Mario::checkSidePress() {
    TVec3f position;
    if (isStatusActive(MarioStatus_Hang)) {
        position = mActor->_240;
    } else {
        position = mPosHolder->mPos;
    }
    f32 radius;
    f32 height;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        radius = 50.0f;
        height = 40.0f;
    } else {
        radius = 80.0f;
        height = 70.0f;
    }
    f32 ceilingDistance = calcDistToCeil(false);
    TVec3f center(position + mHeadVec * height);
    u32 hitCount = fn_803B2A20(center, radius, radius);
    if (hitCount < 2) {
        return false;
    }
    for (u32 i = 0; i < hitCount; i++) {
        const HitInfo* first = fn_803B2AA0(i);
        TVec3f firstNormal(*first->mParentTriangle.getNormal(0));
        if (MR::isEqualString(first->mParentTriangle.mSensor->mHost->mName, "バッタン")) continue;
        for (u32 j = i + 1; j < hitCount; j++) {
            const HitInfo* second = fn_803B2AA0(j);
            TVec3f secondNormal(*second->mParentTriangle.getNormal(0));
            if (MR::isEqualString(second->mParentTriangle.mSensor->mHost->mName, "バッタン")) continue;
            if (firstNormal.dot(secondNormal) >= -0.707f) {
                continue;
            }
            if ((first->mHitPos - center).dot(second->mHitPos - center) > 0.0f) {
                continue;
            }
            f32 separation = (first->mHitPos - second->mHitPos).length();
            TVec3f firstVelocity;
            TVec3f secondVelocity;
            MR::calcVelocityMovingPoint(&first->mParentTriangle, first->mHitPos, &firstVelocity);
            MR::calcVelocityMovingPoint(&second->mParentTriangle, second->mHitPos, &secondVelocity);
            if (!MR::isNearZero(firstVelocity) || !MR::isNearZero(secondVelocity)) {
                if (!MR::isNearZero(firstVelocity)) {
                    MR::normalize(&firstVelocity);
                    if (firstVelocity.dot(firstNormal) < 0.2f || firstVelocity.dot(secondNormal) > -0.2f) {
                        continue;
                    }
                }
                if (!MR::isNearZero(secondVelocity)) {
                    MR::normalize(&secondVelocity);
                    if (secondVelocity.dot(secondNormal) < 0.2f || secondVelocity.dot(firstNormal) > -0.2f) {
                        continue;
                    }
                }
                if (fn_803B4850(first->mParentTriangle.mSensor) || fn_803B4850(second->mParentTriangle.mSensor) ||
                    (first->mParentTriangle.mSensor != second->mParentTriangle.mSensor && (fn_803B2E10(&first->mParentTriangle) || fn_803B2E10(&second->mParentTriangle)))) {
                    if (isStatusActive(MarioStatus_Hang)) {
                        mMovementStates.mLowBits.mBit30 = 0;
                        closeStatus(mHang);
                    }
                    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30)) && MR::abs(firstNormal.dot(*getGravityVec())) < 0.707f) {
                        if (!first->isCollisionAtFace()) {
                            TVec3f horizontal;
                            f32 vertical = MR::vecKillElement(first->mHitPos - center, *getGravityVec(), &horizontal);
                            if (vertical != 0.0f) {
                                addVelocity(*getGravityVec() * -vertical);
                                mDrawStates.mBits.mBit1 = 1;
                                continue;
                            }
                        }
                        if (!second->isCollisionAtFace()) {
                            TVec3f horizontal;
                            f32 vertical = MR::vecKillElement(second->mHitPos - center, *getGravityVec(), &horizontal);
                            if (vertical != 0.0f) {
                                addVelocity(*getGravityVec() * -vertical);
                                mDrawStates.mBits.mBit1 = 1;
                                continue;
                            }
                        }
                    }
                    if (!isStatusActive(MarioStatus_Bury) && !mSinkTimer && ceilingDistance >= 180.0f) {
                        TVec3f firstOffset(first->mHitPos - center);
                        TVec3f secondOffset(center - second->mHitPos);
                        if (!MR::isNearZero(MR::diffAngleAbs(firstOffset, secondOffset), 0.13962634f)) {
                            MR::normalizeOrZero(&firstOffset);
                            MR::normalizeOrZero(&secondOffset);
                            addVelocity(-firstOffset * 5.0f);
                            addVelocity(secondOffset * 5.0f);
                            continue;
                        }
                    }
                    f32 angle = JMAAcosRadian(-firstNormal.dot(secondNormal));
                    if (fn_803EFA30(first->mParentTriangle.mSensor, "ナナメつぶし許可") || fn_803EFA30(second->mParentTriangle.mSensor, "ナナメつぶし許可")) {
                        angle = 0.0f;
                    }
                    if (!isStatusActive(MarioStatus_Bury) && !mSinkTimer && angle > 0.34906587f) {
                        f32 distance = (160.0f - ceilingDistance) / JMASinRadian(angle);
                        if (!(distance < 0.0f)) {
                            TVec3f firstHorizontal;
                            TVec3f secondHorizontal;
                            MR::vecKillElement(firstNormal, *getGravityVec(), &firstHorizontal);
                            MR::vecKillElement(secondNormal, *getGravityVec(), &secondHorizontal);
                            if (firstHorizontal.length() > secondHorizontal.length()) {
                                firstHorizontal.setLength(distance);
                                addVelocity(firstHorizontal);
                            } else {
                                secondHorizontal.setLength(distance);
                                addVelocity(secondHorizontal);
                            }
                        }
                        continue;
                    }
                    s32 previousFloorCode = _95C->getCode(_45C);
                    if (previousFloorCode != _95C->getCode(mGroundPolygon) && MR::isSensorPressObj(mGroundPolygon->mSensor) &&
                        mVerticalSpeed >= 200.0f) {
                        mDrawStates.mBits.mBit21 = 1;
                        TVec3f delta(mShadowPos);
                        delta -= mGroundPos;
                        MR::isNearZero(mAirGravityVec, 0.001f);
                        TVec3f horizontal;
                        MR::vecKillElement(delta, mAirGravityVec, &horizontal);
                        horizontal.setLength(10.0f);
                        addVelocity(horizontal);
                        return false;
                    }
                    if (MR::abs(firstNormal.dot(mHeadVec)) > 0.707f) {
                        mActor->setPress(0, 0);
                    } else {
                        if (!first->mParentTriangle.isHostMoved() || !second->mParentTriangle.isHostMoved()) {
                            TVec3f hitPosition;
                            if (!first->mParentTriangle.isHostMoved()) hitPosition = first->mHitPos;
                            else hitPosition = second->mHitPos;
                            TVec3f displacement(hitPosition);
                            displacement -= center;
                            mPosHolder->fn_80406010(displacement);
                        }
                        mActor->setPress(1, 0);
                    }
                    mActor->mPressNormal6D8 = firstNormal;
                    return true;
                }
            }
            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit31))) {
                MR::diffAngleAbsHorizontal(firstNormal, secondNormal, getAirGravityVec());
                f32 normalDot = firstNormal.dot(secondNormal);
                TVec3f tangent;
                PSVECCrossProduct(&firstNormal, &getAirGravityVec(), &tangent);
                MR::normalizeOrZero(&tangent);
                TVec3f firstUp = tangent.cross(firstNormal);
                MR::normalizeOrZero(&firstUp);
                PSVECCrossProduct(&secondNormal, &getAirGravityVec(), &tangent);
                MR::normalizeOrZero(&tangent);
                TVec3f secondUp = tangent.cross(secondNormal);
                MR::normalizeOrZero(&secondUp);
                TVec3f firstPosition;
                TVec3f secondPosition;
                firstPosition = first->mHitPos + firstUp * height;
                secondPosition = second->mHitPos + secondUp * height;
                f32 distance = (firstPosition - secondPosition).length();
                if (isRising()) {
                    if (normalDot < -0.707f && distance < 120.0f && separation < 100.0f) {
                        cutGravityElementFromJumpVec(true);
                    }
                } else if (normalDot < -0.707f && separation < 100.0f && distance < 80.0f) {
                    cutGravityElementFromJumpVec(true);
                    mMovementStates.mLowBits.mBit30 = 1;
                    stopJump();
                }
            }
        }
    }
    return false;
}

#pragma opt_propagation on

// NONMATCHING: saved register allocation and independent load scheduling.
void Mario::fn_803A6BE0() {
    bool press = false;
    if (_95C->getCode(_4C8) != 29) {
        if (_95C->getCode(_4C8) != 39 && MR::isSensorPressObj(_730)) {
            TVec3f projected;
            if (MR::vecKillElement(_184, *getGravityVec(), &projected) < -0.5f) press = true;
            else {
                TVec3f current;
                TVec3f previous;
                MR::extractMtxTrans(_730->mHost->mCollisionParts->mBaseMatrix, &current);
                MR::extractMtxTrans(_730->mHost->mCollisionParts->mPrevBaseMatrix, &previous);
                TVec3f movement(current);
                movement -= previous;
                if (MR::vecKillElement(movement, *getGravityVec(), &projected) < -0.5f) press = true;
            }
        }
    }
    bool grounded = static_cast<u32>(mMovementStates.mLowBits.mBit30);
    if (grounded) {
        const char* name = mGroundPolygon->mSensor->mHost->mName;
        if (strstr(name, "TriPod") || strstr(name, "Tripod")) press = false;
    }
    if (grounded && _730 == mGroundPolygon->mSensor) press = false;
    if (_730 != nullptr && press) {
        mActor->mPressNormal6D8 = _368;
        mActor->setPress(0, 0);
    }
}
