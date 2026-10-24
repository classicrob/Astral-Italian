// Ordered Mario continuation: target .text offsets 0x20000 onward.

void Mario::startRabbitMode() {
    if (!isStatusActive(MarioStatus_Rabbit)) {
        changeStatus(mRabbit);
    }
}

void Mario::endRabbitMode() {
    if (isStatusActive(MarioStatus_Rabbit)) {
        closeStatus(mRabbit);
    }
}

bool Mario::doRecovery() {
    if (isStatusActive(MarioStatus_Foo)) {
        return false;
    }
    if (isStatusActive(MarioStatus_Recovery)) {
        return false;
    }
    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }
    if (mActor->_934) {
        return false;
    }
    if (mActor->_EA4) {
        return false;
    }

    cancelSquatMode();
    stopWalk();
    if (mRecovery->_12) {
        changeStatus(mRecovery);
        stopJump();
    } else {
        TVec3f gravity;
        TVec3f position(*getLastSafetyTrans(&gravity));
        position += gravity * 160.0f;
        TVec3f direction(mPosHolder->mPos - position);
        MR::normalizeOrZero(&direction);
        doPointWarpRecovery(position, direction);
    }
    return true;
}

bool Mario::checkWallJumpCode() {
    if (checkWallFloorCode(11) || checkWallFloorCode(6) || checkWallFloorCode(34) || checkWallFloorCode(7) || checkWallFloorCode(8)) {
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isAnimationRun(_728)) {
            stopWalk();
            push(getWallNorm() * 10.0f);
            tryPushToVelocity();
            mJumpVec.set(-getPlayer()->getWallNorm() * 10.0f);
            MR::vecKillElement(mJumpVec, getAirGravityVec(), &mJumpVec);
            mJumpVec -= getAirGravityVec() * 10.0f;
        } else {
            mWall->initTriangleJump();
            u32 jumpType = -1;
            if (checkWallFloorCode(7)) {
                jumpType = 1;
            }
            if (checkWallFloorCode(6)) {
                jumpType = 0;
            }
            if (checkWallFloorCode(34)) {
                jumpType = 2;
            }
            if (mWall->startBackJump(jumpType)) {
                return true;
            }
        }
    }
    return false;
}

bool Mario::doSideStep() {
    if (getCurrentStatus() == MarioStatus_SideStep) {
        return false;
    }
    if (checkWallCode("NotSideStep", false)) {
        return false;
    }
    if (checkWallCode("NoAction", false)) {
        return false;
    }
    if (mActor->_468) {
        return false;
    }
    if (static_cast<u32>(mFlags20.mBits.mBit26)) {
        return false;
    }
    if (_72C < 200.0f) {
        return false;
    }
    if (getPlayerMode() == PlayerMode_Hopper) {
        return false;
    }
    if (isPlayerModeYoshi()) {
        return false;
    }
    if (calcPolygonAngleD(mFrontWallTriangle) > 91.0f) {
        return false;
    }
    f32 angle = MR::diffAngleAbs(getFrontWallNorm(), _368);
    if (MR::abs(angle - 1.5707964f) >= 0.5235988f) {
        return false;
    }
    bool cannotSideStep = !isSlipFloorCode(_960);
    if (cannotSideStep) {
        return false;
    }
    if (_960 == 2) {
        return false;
    }
    if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit23))) {
        return false;
    }
    const Triangle* wall = getPlayer()->getWallPolygon();
    if (!wall) {
        return false;
    }
    if (!MR::isSameMtx(wall->getBaseMtx()->toMtxPtr(), wall->getPrevBaseMtx()->toMtxPtr())) {
        return false;
    }
    stopWalk();
    forceStopTornado();
    changeStatus(mSideStep);
    return true;
}

bool Mario::isSkatableFloor() const {
    if (_960 == CollisionFloorCode_Ice) {
        return true;
    }

    return _960 == CollisionFloorCode_GlassIce;
}

bool Mario::doSkate() {
    changeStatus(mSkate);
    return true;
}

bool Mario::isSliderFloor() const {
    return _960 == 9;
}

bool Mario::checkSliderMode() const {
    if (!((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && ((mMovementStates.mLowWord >> 30) & 1U)) {
        if (isSliderFloor()) {
            return true;
        }
    }

    return false;
}

void Mario::startSlider() {
    if (!isStatusActive(mSlider->mStatusId)) {
        if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20))) {
            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit31))) {
                mDrawStates.mBits.mBit11 = 1;
                playSound("尻ドロップ着地");
                playSound("声尻ドロップ着地");
                playEffectRT("属性尻ドロップ", _368, mPosHolder->mPos);
                startPadVib("最強");
                startCamVib(0);
                MR::removeAllClingingKarikari();
            }
        }

        stopWalk();
        stopJump();
        changeStatus(mSlider);
    }
}

// NONMATCHING: register allocation.
void Mario::fn_803A7540() {
    if (!(static_cast<u32>(this->mMovementStates.mHighBits.mBit28))) {
        this->_1A8 = this->mVelocity;
        this->_3C2 = 0;
        this->_280 = 0.0f;
        this->_284.zero();
    } else {
        this->_3C2 += 1;
    }
    if (this->_3C4 != 0) {
        this->_3C4 -= 1;
    }
    if ((static_cast<u32>(this->mMovementStates.mLowBits.mBit30)) && ((static_cast<u32>(this->mMovementStates.mLowBits.mBit31)) == 0)) {
        if (this->isEnableSlopeMove()) {
            this->slopeMove();
        } else {
            this->clearSlope();
        }
    }
}

void Mario::clearSlope() {
    _8F0 = 0.0f;
    mMovementStates.mHighBits.mBit28 = 0;
    _8F8.zero();
    _280 = 0.0f;
    _910.zero();
    _284.zero();
    _3C2 = 0;
    _2C4.zero();
    _3C4 = 0;
    _904.zero();
}

bool Mario::isEnableSlopeMove() const {
    const bool isNotSlipFloor = !isSlipFloorCode(_960);
    if (isNotSlipFloor) {
        if (!(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
            if (MR::isNearZero(_8F8)) {
                return false;
            }
        }
    }

    const s32 status = static_cast< s32 >(getCurrentStatus());
    switch (status) {
    case 0:
    case 2:
    case 3:
    case 4:
        return true;

    case 1:
    default:
        return false;
    }
}

// NONMATCHING: independent vector/gravity evaluation order and register scheduling.
void Mario::slopeMove() {
    if (mActor->mHealth == 0) {
        return;
    }

    if (!((mMovementStates.mLowWord >> 30) & 1U)) {
        return;
    }

    f32 blend;
    f32 blendRate = MR::clamp(mWalkSpeed, 0.0f, 1.0f);

    TVec3f stack_188;
    TVec3f stack_17C;
    TVec3f stack_170;
    stack_170.cross(_368, getAirGravityVec());
    MR::normalizeOrZero(&stack_170);

    if (MR::isNearZero(stack_170)) {
        stack_17C.zero();
        _904.zero();
    } else {
        stack_17C.cross(_368, stack_170);
        MR::normalize(&stack_17C);
        _904 = stack_17C;
    }

    if (MR::isSameDirection(stack_17C, getAirGravityVec()) || MR::isOppositeDirection(stack_17C, getAirGravityVec())) {
        mMovementStates.mHighWord &= 0xEFFFFFFFU;
        _904.zero();
        stack_17C.zero();
    }

    f32 slopeAngle = calcAngleD(_368);

    if (((mDrawStates.mWord >> 8) & 1U) && slopeAngle >= 30.0f) {
        if (!((mMovementStates.mHighWord >> 28) & 1U)) {
            _8F8 = -_2C4 * 2.0f;
            mMovementStates.mHighWord |= 0x10000000U;
            _8F0 = 10.0f;
            _3C4 = 30;
        }
    }

    if (_3C4 > 0xF) {
        _8F8 += -_2C4 * 0.5f;
    }

    if (!((mDrawStates.mWord >> 19) & 1U) || !isSlipPolygon(_45C)) {
        blendRate = 1.0f;
        if (((mMovementStates.mHighWord >> 28) & 1U)) {
            if (slopeAngle >= 15.0f) {
                _8F0 -= 0.4f;
            } else {
                _8F0 -= 0.8f;
            }

            if (_8F0 < 0.0f) {
                _8F0 = 0.0f;
                mMovementStates.mHighWord &= 0xEFFFFFFFU;
            }

            mVelocity.zero();
        } else {
            _8F0 = 0.0f;
        }
    } else {
        if (!((mMovementStates.mHighWord >> 28) & 1U)) {
            if (slopeAngle >= getActor()->getConst().getTable()->mSlipAngle) {
                TVec3f stack_164;
                if (MR::vecKillElement(mVelocity, stack_17C, &stack_164) < 0.0f) {
                    _8F8 = mVelocity;
                } else {
                    _8F8.zero();
                }

                mVelocity.zero();
                mMovementStates.mHighWord |= 0x10000000U;
                _8F0 = 10.0f;
                return;
            }

            TVec3f stack_158;
            const f32 sideSpeed = MR::vecKillElement(mVelocity, stack_17C, &stack_158);

            if (MR::isInAreaObj("SlopeRunningCancelArea", mPosHolder->mPos)) {
                _8F0 = 10.0f;
            }

            if (mWalkSpeed < 0.1f) {
                _8F0 = 10.0f;
            }

            if (mTargetWalkSpeedIndex == 0 && !isAnimationRun("すべり着地")) {
                _8F0 = 10.0f;
            }

            if (slopeAngle >= 25.0f && isStickOn()) {
                const TVec3f& worldPadDir = getWorldPadDir();

                TVec3f stack_14C;
                MR::vecKillElement(stack_17C, getCamDirY(), &stack_14C);
                if (MR::isNearZero(stack_14C)) {
                    MR::vecKillElement(stack_17C, getCamDirZ(), &stack_14C);
                }

                MR::normalize(&stack_14C);
                if (stack_14C.dot(worldPadDir) < 0.0f && mFrontVec.dot(stack_14C) < 0.0f) {
                    mDrawStates.mWord |= 0x08000000U;
                }
            }

            if (!((mDrawStates.mWord >> 27) & 1U)) {
                _8F0 += 0.3f * (slopeAngle / 90.0f);
            }

            if (_8F0 >= 10.0f) {
                if (MR::vecKillElement(mActor->getLastMove(), stack_17C, &stack_158) < 0.0f) {
                    _8F8 = mActor->getLastMove();
                } else {
                    _8F8.zero();
                }

                mMovementStates.mHighWord |= 0x10000000U;
                _8F0 = 10.0f;
                mVelocity.zero();
                mDrawStates.mWord &= 0xF7FFFFFFU;
            } else if (!((mDrawStates.mWord >> 27) & 1U) && sideSpeed > 0.0f) {
                const f32 t = _8F0 / 10.0f;
                const f32 remain = 1.0f - t;

                mVelocity = stack_158 + stack_17C * sideSpeed * remain;
            }

            blendRate *= 1.0f - (_8F0 / 10.0f);

            if (((mDrawStates.mWord >> 27) & 1U)) {
                _3FE++;

                if (_3FE > getActor()->getConst().getTable()->mSlopeDashAccelTime) {
                    _3FE = getActor()->getConst().getTable()->mSlopeDashAccelTime;
                }

                _8F4 = (slopeAngle / getActor()->getConst().getTable()->mSlopeDashAngleFactor) *
                       (static_cast< f32 >(_3FE) / static_cast< f32 >(getActor()->getConst().getTable()->mSlopeDashAccelTime));
                _8F4 = 1.0f + (_8F4 * getActor()->getConst().getTable()->mSlopeDashSpeedFactor);

                const f32 accel = 1.0f + ((_8F4 - 1.0f) * getActor()->getConst().getTable()->mSlopeDashAccelFactor);
                mVelocity += -_904 * accel;

                if (isAnimationRun("がんばり走り")) {
                    stopAnimation(nullptr);
                }
            }
        }
    }

    MR::vecKillElement(_8F8, *mGroundPolygon->getNormal(0), &_8F8);

    if (((mMovementStates.mHighWord >> 28) & 1U)) {
        _71E = 0;

        const f32 moveAngle = calcAngleD(_368);
        if (((mDrawStates.mWord >> 19) & 1U)) {
            MR::vecKillElement(mVelocity, stack_17C, &mVelocity);
            mVelocity += _8F8;

            _8F8 += -stack_17C * slopeAngle * getActor()->getConst().getTable()->mSlopeAccel;
        } else {
            TVec3f stack_140;
            stack_140 = _8F8;
            if (!MR::normalizeOrZero(&stack_140)) {
                MR::vecKillElement(mVelocity, stack_140, &mVelocity);
            }

            mVelocity += _8F8;
        }

        if (MR::isNearZero(_8F8)) {
            if (!((mDrawStates.mWord >> 19) & 1U)) {
                mMovementStates.mHighWord &= 0xEFFFFFFFU;
                return;
            }
        }

        if (moveAngle > 0.0f) {
            TVec3f stack_134;
            {
                TVec3f opposite = -stack_17C;
                MR::vecKillElement(opposite, getAirGravityVec(), &stack_134);
            }

            if (MR::isNearZero(stack_17C) || (MR::isNearZero(stack_134) && !((mDrawStates.mWord >> 19) & 1U))) {
                mMovementStates.mHighWord &= 0xEFFFFFFFU;
                return;
            }

            if (MR::isNearZero(stack_134)) {
            }

            MR::normalize(&stack_134);

            const f32 speed = _8F8.length();
            MR::normalizeOrZero(&_8F8);

            const f32 ratio = MR::clamp(moveAngle / 30.0f, 0.0f, 1.0f);
            blend = ratio * getActor()->getConst().getTable()->mSlopeCurveAssist;

            MR::vecBlendSphere(_8F8, -stack_17C, &_8F8, blend);
            _8F8.setLength(speed);
        }

        if (MR::isNearZero(_8F8)) {
            mMovementStates.mHighWord &= 0xEFFFFFFFU;
            moveSlopeSlide();
            return;
        }

        if (!(_8F8.dot(stack_17C) > 0.0f)) {
            if (stack_17C.dot(-*getGravityVec()) > 0.05f) {
                TVec3f stack_128;
                MR::vecKillElement(mFrontVec, _368, &stack_128);
                MR::normalize(&stack_128);

                TVec3f stack_11C(_8F8);
                MR::normalize(&stack_11C);

                f32 turnDot = MR::cos(1.0471976f);
                if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                    turnDot = MR::cos(1.4959966f);
                }

                if (isAnimationRun("スケーティング") && _910.dot(stack_17C) > 0.0f) {
                    if (_910.length() > 2.0f) {
                        _8F8 = _910;
                    }

                    _910.zero();
                }

                if (stack_128.dot(stack_17C) < turnDot) {
                    setFrontVecKeepUp(stack_11C, 0.1f);
                    if (!isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                        changeAnimation("坂すべり下向きあおむけ", 3);
                    }

                    if (isStickOn() && _16C.dot(stack_11C) > 0.0f &&
                        MR::diffAngleAbsHorizontal(stack_11C, mFrontVec, *getGravityVec()) < 0.5235988f) {
                        setFrontVecKeepUpAngle(_16C, getActor()->getConst().getTable()->mSlipMoveTurnAngleRad);
                    }
                } else {
                    if (!isAnimationRun("坂すべり下向きあおむけ", 3)) {
                        changeAnimation("坂すべり上向きうつぶせ", 2);
                    }

                    if (_3C2 < 5) {
                        setFrontVecKeepUp(-stack_11C, 0.5f);
                    } else {
                        setFrontVecKeepUp(-stack_11C, 0.1f);
                    }

                    if (_3C2 > 10 && isStickOn() && _16C.dot(stack_11C) > 0.0f) {
                        if (MR::diffAngleAbsHorizontal(-stack_11C, mFrontVec, *getGravityVec()) < 0.5235988f) {
                            TVec3f opposite = -_16C;
                            setFrontVecKeepUpAngle(opposite, getActor()->getConst().getTable()->mSlipMoveTurnAngleRad);
                        }
                    }
                }
            } else if (((mDrawStates.mWord >> 19) & 1U)) {
                if (!isAnimationRun("スケーティング")) {
                    _910 = _8F8;
                    _8F8.zero();
                }

                changeAnimation("スケーティング");
            }
        }

        if (isStickOn() && !MR::isNearZero(stack_17C) && _3C4 == 0) {
            const TVec3f& worldPadDir = getWorldPadDir();

            TVec3f stack_110(stack_17C);
            MR::vecKillElement(stack_17C, getCamDirY(), &stack_17C);
            if (MR::isNearZero(stack_17C)) {
                MR::vecKillElement(stack_110, getCamDirZ(), &stack_17C);
            }

            MR::normalize(&stack_17C);
            if (stack_17C.dot(worldPadDir) > 0.15f) {
                playEffect("共通スリップ坂制動");
                _3D0 = getActor()->getConst().getTable()->mTurnSlipTime;
            }
        }

        _1A8.x *= 0.9f;
        _1A8.y *= 0.9f;
        _1A8.z *= 0.9f;

        TVec3f stack_104;
        if (MR::vecKillElement(_1A8, stack_17C, &stack_104) >= 0.0f) {
            stack_104 = _1A8;
        }

        addVelocity(stack_104);
    } else {
        mVelocity += _8F8;

        if (isStickOn() && _3C4 == 0) {
            _8F8.scale(getActor()->getConst().getTable()->mSlopeCancelInertia);
        } else {
            _8F8.scale(getActor()->getConst().getTable()->mSlopeFinishInertia);
        }

        const MarioConstTable* table = getActor()->getConst().getTable();
        if (_8F8.length() < table->mSlopeAnimeFinishSpeed) {
            if (isAnimationRun("坂すべり上向きうつぶせ", 2)) {
                changeAnimation("坂すべり上向き終了");
            }

            if (isAnimationRun("坂すべり下向きあおむけ", 3)) {
                changeAnimation("坂すべり下向き終了");
            }

            if (isAnimationRun("スケーティング")) {
                stopAnimation("スケーティング");
            }

            if (isStickOn() && _8F0 == 0.0f &&
                (isAnimationRun("坂すべり上向き終了") || isAnimationRun("坂すべり下向き終了"))) {
                stopAnimation(nullptr);
            }
        }
    }

    const MarioConstTable* table = getActor()->getConst().getTable();
    f32 slopeSpeedMax = table->mSlopeSpeedMax;
    if (_3D0 != 0) {
        slopeSpeedMax = table->mSlopeSpeedMaxBraking;
    }

    const f32 slopeSpeed = _8F8.length();
    if (slopeSpeed > slopeSpeedMax) {
        _8F8.setLength((0.8f * slopeSpeed) + (0.2f * slopeSpeedMax));
    }

    if (!MR::vecBlendSphere(*getGravityVec(), -_368, &stack_188, blendRate)) {
        mMovementStates.mHighWord &= 0xEFFFFFFFU;
        tryDrop();
        return;
    }

    moveSlopeSlide();
}

void Mario::moveSlopeSlide() {
    if (calcAngleD(_368) < 5.8f) {
        if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
            const TVec3f* pWorldPadDir = &getWorldPadDir();
            f32 speed = _16C.length();
            if (_910.length() < speed) {
                _910 = _16C;
            }

            _910 += *pWorldPadDir * 0.5f;

            if (_910.length() > 15.0f) {
                _910.setLength(15.0f);
            }

            addVelocity(_910);
        }
    } else if (isAnimationRun("スケーティング") || isAnimationRun("坂すべり上向きうつぶせ", 2) || isAnimationRun("坂すべり下向きあおむけ", 3)) {
        TVec3f stack_20;
        stack_20.cross(getAirGravityVec(), _368);
        MR::normalizeOrZero(&stack_20);

        const TVec3f* pWorldPadDir = &getWorldPadDir();
        if (MR::isNearZero(_280)) {
            _284 = stack_20;
        }

        if (isStickOn() && _3C4 == 0) {
            f32 dot = _284.dot(*pWorldPadDir);
            if (MR::abs(dot) > 0.2f) {
                _280 = (_280 * mActor->getConst().getTable()->mSlopeSideMoveInertia) +
                       (dot * (1.0f - mActor->getConst().getTable()->mSlopeSideMoveInertia));
            } else {
                MarioConstTable* pConstTable = mActor->getConst().getTable();
                _280 = _280 * pConstTable->mSlopeSideStopInertia;
            }
        } else {
            MarioConstTable* pConstTable = mActor->getConst().getTable();
            _280 = _280 * pConstTable->mSlopeSideStopInertia;
        }

        TVec3f stack_14;
        stack_14.cross(_284, _368);
        _284.cross(_368, stack_14);

        MarioConstTable* pConstTable = mActor->getConst().getTable();
        addVelocity(_284, _280 * pConstTable->mSlopeSideMoveSpeed);
    }
}

bool Mario::taskOnSlipTurn(u32) {
    if (!isAnimationRun("ターンブレーキ滑り床")) {
        setFrontVecKeepUp(-_220);
        _74C = 0.0f;
        _754 = 0;
        return false;
    }

    _2B8.scale(mActor->mConst->getTable()->mInertiaSlipTurn);
    addVelocity(_2B8);
    return true;
}

void Mario::lockGroundCheck(void* owner, bool simple) {
    _574 = owner;
    mFlags14.mLowBits.mBit26 = simple;
    mFlags14.mLowBits.mBit27 = 1;
    mMovementStates.mHighBits.mBit9 = simple;
}

void Mario::unlockGroundCheck(void* owner) {
    if (!owner || _574 == owner) {
        _574 = nullptr;
        mFlags14.mLowBits.mBit27 = 0;
    }
}

static inline bool queryMarioGroundLine(const Mario* mario, Triangle* triangle, const TVec3f& origin, const TVec3f& direction) {
    if (mario->fn_803B2950()) {
        return MR::getFirstPolyOnLineToDashYoshiStepAndMap(nullptr, triangle, origin, direction);
    }
    return MR::getFirstPolyOnLineToMap(nullptr, triangle, origin, direction);
}

bool Mario::isUseSimpleGroundCheck() const {
    if ((static_cast<u32>(mFlags14.mLowBits.mBit27))) {
        return (static_cast<u32>(mFlags14.mLowBits.mBit26));
    }
    if ((static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
        return true;
    }
    if (isPlayerModeTeresa()) {
        return true;
    }
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        return (static_cast<u32>(mMovementStates.mHighBits.mBit28));
    }
    if (calcAngleD(_368) > mActor->getConst().getTable()->mFlatAngle - 5.0f) {
        Triangle ground;
        bool hit = queryMarioGroundLine(this, &ground, mPosHolder->mPos - *getGravityVec() * 30.0f,
                                        *getGravityVec() * 100.0f);
        if (hit) {
            if (calcAngleD(*ground.getNormal(0)) < mActor->getConst().getTable()->mFlatAngle - 5.0f) {
                return false;
            }
        }
        return true;
    }
    fn_8038CCF0();
    return false;
}

bool Mario::checkGroundOnSlope() {
    TVec3f normal;
    if (isAnimationRun("崖ふんばり")) {
        normal = -*getGravityVec();
    } else {
        normal = *_45C->getNormal(0);
    }
    if (MR::isNearZero(normal)) {
        return false;
    }
    TVec3f front;
    MR::vecKillElement(mFrontVec, normal, &front);
    if (MR::isNearZero(front)) {
        return false;
    }
    Triangle ground;
    bool hit;
    mMovementStates.mLowBits.mBit11 = 1;
    f32 distance;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && isRising()) {
        distance = 10.0f;
    } else if (_414) {
        distance = 20.0f;
    } else {
        distance = 60.0f;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        f32 angle = calcAngleD(_368) - 30.0f;
        if (angle < 0.0f) {
            angle = 0.0f;
        }
        if (angle > 40.0f) {
            angle = 40.0f;
        }
        distance += 2.0f * angle;
    }
    if (getCurrentStatus() == MarioStatus_Slider) {
        distance = 100.0f;
    }
    s32 groundCount = 0;
    TVec3f hitPosition;
    TVec3f positionSum;
    positionSum.zero();
    TVec3f groundPosition;
    TVec3f origin(mPosHolder->mPos - *getGravityVec() * 30.0f);
    hit = fn_803B2B50(origin, *getGravityVec() * distance, &hitPosition, &ground);
    if (hit) {
        if (calcAngleD(*ground.getNormal(0)) >= 80.0f) {
            hit = false;
        }
        f32 gravityDot = getGravityVec()->dot(*ground.getNormal(0));
        if (getCurrentStatus() != MarioStatus_Slider && gravityDot > -0.1908f) {
            hit = false;
        }
        if (hit) {
            positionSum += hitPosition;
            groundCount++;
            setGroundNorm(*ground.getNormal(0));
            *mGroundPolygon = ground;
            groundPosition = hitPosition;
            mGroundPos = groundPosition;
            recordLastGround();
        }
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit18)) && getGravityVec()->dot(mGroundPos - mPosHolder->mPos) >= 0.0f) {
        mMovementStates.mLowBits.mBit18 = 0;
        TVec3f offset(mGroundPos - mPosHolder->mPos);
        f32 vertical = MR::vecKillElement(offset, *getGravityVec(), &offset);
        addTrans(*getGravityVec() * vertical);
        return true;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && isRising()) {
        return false;
    }
    if (!groundCount) {
        mMovementStates.mLowBits.mBit11 = 1;
        return false;
    }
    mMovementStates.mLowBits.mBit11 = 0;
    TVec3f down(-_368);
    TVec3f position(mGroundPos);
    if ((static_cast<u32>(mFlags14.mHighBits.mBit29))) {
        mFlags14.mHighBits.mBit29 = 0;
        mFlags20.mBits.mBit19 = 1;
    } else if (((static_cast<u32>(mMovementStates.mHighBits.mBit28)) || mTargetWalkSpeedIndex) && getPlayerMode() != PlayerMode_Teresa) {
        if (MR::abs((position - mPosHolder->mPos).dot(down)) < distance) {
            TVec3f offset(position - mPosHolder->mPos);
            f32 vertical = MR::vecKillElement(offset, down, &offset);
            if (!MR::isNearZero(vertical, 1.0f)) {
                mDrawStates.mBits.mBit31 = 0;
                TVec3f displacement(down * vertical);
                f32 ratio = MR::clamp(mWalkSpeed, 0.0f, 1.0f);
                if (!(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
                    displacement *= ratio;
                }
                bool notSlipping = !isSlipFloorCode(_960);
                if (!notSlipping && !(static_cast<u32>(mDrawStates.mBits.mBit27)) && !(static_cast<u32>(mPrevDrawStates.mBits.mBit27))) {
                    mVelocityAfter += displacement;
                }
            }
            return true;
        }
    }
    if (mVerticalSpeed < 5.0f) {
        return true;
    }
    if ((static_cast<u32>(mDrawStates.mBits.mBit25))) {
        return true;
    }
    if (groundCount) {
        return true;
    }
    return (static_cast<u32>(mDrawStates.mBits.mBit31));
}

// Nonzero selectors address the three override columns (1..3).
u32 Mario::initSoundTable(SoundList* pList, u32 globalTablePosition) {
    SoundList* pEntry;
    u32 count = 0;
    s32 i = 0;
    while (true) {
        pEntry = pList + i;
        if (pEntry->name[0] == '\0') {
            break;
        }

        pEntry->_10 = 0;
        pEntry->_14 = pEntry->_4;

        if (globalTablePosition != 0) {
            s32 swapIndex = 0;
            while (true) {
                const SoundSwapList* pSwapEntry = soundswaplist + swapIndex;
                if (pSwapEntry->name[0] == '\0') {
                    break;
                }

                if (strcmp(pEntry->name, pSwapEntry->name) == 0) {
                    u32 soundID = soundswaplist[swapIndex].soundIDs[globalTablePosition - 1];
                    if (soundID != 0) {
                        pEntry->_14 = soundID;
                    }
                    break;
                }

                swapIndex++;
            }
        }

        count++;
        i++;
    }

    return count;
}

void Mario::initSound() {
    u32 count = initSoundTable(soundlist, 0);
    _96C = new HashSortTable(count);
    for (u32 i = 0; i < count; i++) {
        _96C->add(soundlist[i].name, i, false);
    }

    _96C->sort();
    _970 = nullptr;
}

void Mario::playSoundJ(const char* pSoundName, s32 timing) {
    uintptr_t index;
    if (_96C->search(pSoundName, &index)) {
        switch (soundlist[index]._8.mPolicy & 0x3) {
        case 0:
            MR::startSound(mActor, soundlist[index]._14, timing);
            break;

        case 2:
            MR::startSystemSE(soundlist[index]._14, timing);
            break;

        case 1:
            MR::startLevelSound(mActor, soundlist[index]._14, timing);
            break;

        case 3:
            MR::startSystemLevelSE(soundlist[index]._14, timing, -1, -1);
            break;
        }

        switch (soundlist[index]._8.mPolicy & ~0x3) {
        case 0x4:
        case 0x8:
            if (((mDrawStates.mWord & Mario::DrawStates::Underwater) >> 13) || (static_cast<u32>(mDrawStates.mBits.mBit12))) {
                playSoundJ(soundlist[index]._C, -1);
            }
            break;
        }
    }

    bool isFound = _96C->search("声", pSoundName, &index);
    if (isFound) {
        MR::startSound(mActor, soundlist[index]._14, timing);
    }
}

void Mario::stopSoundJ(const char* pSoundName, u32 delay) {
    uintptr_t index;
    if (_96C->search(pSoundName, &index)) {
        switch (soundlist[index]._8.mPolicy & 0x3) {
        case 0:
            MR::stopSound(mActor, soundlist[index]._14, delay);
            break;

        case 1:
            break;

        case 2:
            MR::stopSystemSE(soundlist[index]._14, delay);
            break;

        case 3:
            break;
        }
    }

    if (_96C->search("声", pSoundName, &index)) {
        JAISoundID soundID(soundlist[index]._14);
        MR::stopSound(mActor, soundID, delay);
    }
}

void Mario::startBas(const char* pName, bool loop, f32 startFrame, f32 speed) {
    if (this->mActor->mSoundHolder != 0) {
        MR::startBas(this->mActor, pName, loop, startFrame, speed, false);
        this->_970 = pName;
    }
}

bool Mario::fn_803A9690(const char* pName) {
    if (this->_970 == nullptr) {
        return false;
    }
    return strcmp(this->_970, pName) == 0;
}

void Mario::fn_803A96D0(f32 arg1) {
    MR::skipBas(this->mActor, arg1);
}

void Mario::playSoundTeresaFlying() {
    if (getPlayerMode() != 6) {
        return;
    }

    s32 timing = 100;
    if (getCurrentStatus() == 0x1C) {
        if (mWait->_14 == 0) {
            timing = 100 - static_cast< s32 >(mWait->_16);
            if (timing < 0) {
                timing = 0;
            }
        } else {
            timing = 0;
        }
    }

    playSound("テレサ浮遊", timing);
}

void Mario::playSoundTrampleCombo(u8 combo) {
    if (combo >= 7) {
        return;
    }

    MR::startSystemSE("SE_SY_TRAMPLE_COMBO", combo);
}

// NONMATCHING: literal symbol identity; instruction words match after relocation masking.
void Mario::checkOnimasu(const HitSensor* pSensor) {
    if (strstr(MR::getSensorHost(pSensor)->mName, "オニマス") == nullptr) {
        return;
    }

    if (_5FC == nullptr) {
        _5FC = pSensor;
        _60C = 1;
    } else if (_5FC != pSensor && (*MR::getSensorPos(pSensor) - mPosHolder->mPos).length() < (_5FC->mPosition - mPosHolder->mPos).length()) {
        _5FC = pSensor;
        _60C = 1;
    }
}

void Mario::updateOnimasu() {
    if (!_5FC) {
        return;
    }

    if ((_5FC->mPosition - mPosHolder->mPos).length() >= 1000.0f) {
        _5FC = nullptr;
        return;
    }

    const HitSensor* sensor = _5FC;
    MtxPtr inverse = MR::getSensorHost(sensor)->mCollisionParts->mInvBaseMatrix;
    TVec3f localPosition;
    TVec3f previousPosition;
    previousPosition = _600;
    PSMTXMultVec(inverse, &mActor->mPosition, &localPosition);
    if (_60C) {
        _600 = localPosition;
        _60C = 0;
        return;
    }

    if (!(static_cast<u32>(mFlags14.mLowBits.mBit2))) {
        if (previousPosition.y < -400.0f && localPosition.y > -400.0f) {
            if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
                localPosition = _600;
                TVec3f position;
                PSMTXMultVec(MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix, &localPosition, &position);
                setTrans(position);
                mVelocity.zero();
            }
        } else if (previousPosition.y > 400.0f && localPosition.y <= 400.0f) {
            if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
                mFlags14.mLowBits.mBit2 = 1;
                if (previousPosition.x < -320.0f) {
                    previousPosition.x = -320.0f;
                } else if (previousPosition.x > 320.0f) {
                    previousPosition.x = 320.0f;
                }

                if (previousPosition.z < -320.0f) {
                    previousPosition.z = -320.0f;
                } else if (previousPosition.z > 320.0f) {
                    previousPosition.z = 320.0f;
                }

                localPosition = previousPosition;
            }
        } else if (MR::isInRange(localPosition.x, -400.0f, 400.0f) && MR::isInRange(localPosition.y, -400.0f, 400.0f) &&
                   MR::isInRange(localPosition.z, -400.0f, 400.0f)) {
            mVelocity.zero();
            localPosition = _600;
            TVec3f position;
            PSMTXMultVec(MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix, &localPosition, &position);
            TVec3f displacement(position - mPosHolder->mPos);
            f32 height = 10.0f + mVerticalSpeed;
            if (-displacement.dot(getShadowNorm()) > height) {
                MR::vecKillElement(displacement, getShadowNorm(), &displacement);
                displacement += -getShadowNorm() * mVerticalSpeed;
                position = mPosHolder->mPos + displacement;
            }

            setTrans(position);
        }

        bool correctX = false;
        bool correctY = false;
        bool correctZ = false;
        TVec3f axisX;
        TVec3f axisY;
        TVec3f axisZ;
        MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix.getXDir(axisX);
        MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix.getYDir(axisY);
        MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix.getZDir(axisZ);
        f32 gravityX = axisX.dot(*getGravityVec());
        f32 absGravityX = MR::abs(gravityX);
        f32 gravityY = axisY.dot(*getGravityVec());
        f32 absGravityY = MR::abs(gravityY);
        f32 gravityZ = axisZ.dot(*getGravityVec());
        f32 absGravityZ = MR::abs(gravityZ);
        f32 lower = -500.0f;
        f32 upper = -300.0f;

        if (absGravityX > absGravityY && absGravityX > absGravityZ) {
            if (gravityX > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.x, lower, upper)) {
                bool insideY = MR::isInRange(localPosition.y, -320.0f, 320.0f);
                bool insideZ = MR::isInRange(localPosition.z, -320.0f, 320.0f);
                if (insideY && insideZ) {
                    mActor->mPressNormal6D8 = axisX;
                    mActor->setPress(0, 0);
                } else {
                    if (insideZ) {
                        correctY = true;
                    }

                    if (insideY) {
                        correctZ = true;
                    }
                }
            }
        } else if (absGravityY > absGravityX && absGravityY > absGravityZ) {
            if (gravityY > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.y, lower, upper)) {
                bool insideX = MR::isInRange(localPosition.x, -320.0f, 320.0f);
                bool insideZ = MR::isInRange(localPosition.z, -320.0f, 320.0f);
                if (insideX && insideZ) {
                    mActor->mPressNormal6D8 = axisY;
                    if (gravityY < 0.0f) {
                        mActor->setPress(0, 0);
                    }
                } else {
                    if (insideZ) {
                        correctX = true;
                    }

                    if (insideX) {
                        correctZ = true;
                    }
                }
            }
        } else {
            if (gravityZ > 0.0f) {
                lower = 300.0f;
                upper = 500.0f;
            }

            if (MR::isInRange(localPosition.z, lower, upper)) {
                bool insideX = MR::isInRange(localPosition.x, -320.0f, 320.0f);
                bool insideY = MR::isInRange(localPosition.y, -320.0f, 320.0f);
                if (insideX && insideY) {
                    mActor->mPressNormal6D8 = axisZ;
                    mActor->setPress(0, 0);
                } else {
                    if (insideY) {
                        correctX = true;
                    }

                    if (insideX) {
                        correctY = true;
                    }
                }
            }
        }

        if (correctX) {
            if (localPosition.x > -400.0f && localPosition.x < -320.0f) {
                localPosition.x = -400.0f;
            } else if (localPosition.x < 400.0f && localPosition.x > 320.0f) {
                localPosition.x = 400.0f;
            }
        }

        if (correctY) {
            if (localPosition.y > -400.0f && localPosition.y < -320.0f) {
                localPosition.y = -400.0f;
            } else if (localPosition.y < 400.0f && localPosition.y > 320.0f) {
                localPosition.y = 400.0f;
            }
        }

        if (correctZ) {
            if (localPosition.z > -400.0f && localPosition.z < -320.0f) {
                localPosition.z = -400.0f;
            } else if (localPosition.z < 400.0f && localPosition.z > 320.0f) {
                localPosition.z = 400.0f;
            }
        }

        if (correctX | correctY | correctZ) {
            TVec3f position;
            PSMTXMultVec(MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix, &localPosition, &position);
            if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6)) || (static_cast<u32>(mMovementStates.mLowBits.mBit5))) {
                TVec3f displacement(position - mPosHolder->mPos);
                if (getWallNorm().dot(displacement) < 0.0f) {
                    addVelocity(-displacement);
                } else {
                    setTrans(position);
                }
            }
        }
    } else if (localPosition.y > 400.0f) {
        mFlags14.mLowBits.mBit2 = 0;
    } else {
        bool corrected = false;
        if (previousPosition.y >= -320.0f && localPosition.y < -320.0f) {
            localPosition.y = previousPosition.y;
            corrected = true;
        }

        if (previousPosition.x >= -320.0f && localPosition.x < -320.0f) {
            localPosition.x = previousPosition.x;
            corrected = true;
        }

        if (previousPosition.x <= 320.0f && localPosition.x > 320.0f) {
            localPosition.x = previousPosition.x;
            corrected = true;
        }

        if (previousPosition.z >= -320.0f && localPosition.z < -320.0f) {
            localPosition.z = previousPosition.z;
            corrected = true;
        }

        if (previousPosition.z <= 320.0f && localPosition.z > 320.0f) {
            localPosition.z = previousPosition.z;
            corrected = true;
        }

        if (corrected) {
            TVec3f position;
            PSMTXMultVec(MR::getSensorHost(sensor)->mCollisionParts->mBaseMatrix, &_600, &position);
            setTrans(position);
            localPosition = _600;
        }
    }

    _600 = localPosition;
}

bool Mario::isDossun(const Triangle* pTriangle) const {
    if (!pTriangle->isValid()) {
        return false;
    }

    if (strstr(pTriangle->mSensor->mHost->getName(), "ドッスン") != nullptr) {
        return true;
    }

    return false;
}

bool Mario::isNoWalkFallOnDossun() const {
    if (!MR::isEqualStageName("CannonFleetGalaxy")) {
        return false;
    }

    if (isDossun(mGroundPolygon) && isDossun(_45C)) {
        return true;
    }

    return false;
}

bool Mario::fn_803AA2C0() {
    return false;
}

bool Mario::isUseAnotherMovingPolygon() const {
    if (MR::isEqualStageName("BattleShipGalaxy")) {
        return true;
    }

    if (MR::isEqualStageName("SandClockGalaxy")) {
        return true;
    }

    if (MR::isEqualStageName("TriLegLv1Galaxy")) {
        return true;
    }

    return false;
}

bool Mario::fn_803AA340(const TVec3f*, TVec3f*) {
    this->isStatusActive(0x18);
    return false;
}

bool Mario::isOnimasuBinderPressSkip() const {
    if (isStatusActive(MarioStatus_SideStep)) {
        if (mFrontWallTriangle->mSensor != nullptr) {
            if (strstr(mFrontWallTriangle->mSensor->mHost->mName, "オニマス")) {
                return true;
            }
        }
    }

    return false;
}

void Mario::checkTornado() {
    if (((mMovementStates.mLowWord >> 30) & 1U)) {
        mMovementStates.mHighBits.mBit20 = 0;
    }
}

void Mario::resetTornado() {
    _530 = 0.0f;
    _534 = 0;
    _538 = 0.0f;
    mMovementStates.mLowBits.mBit16 = 0;
    _544 = 0;
    mYAngleOffset = 0.0f;
    _3F0 = 1.0f;
}

void Mario::doSpinWallEffect() {
    if ((!(static_cast<u32>(mMovementStates.mLowBits.mBit23)) || !mFrontWallTriangle->mSensor->isType(0x4F)) && (!(static_cast<u32>(mMovementStates.mLowBits.mBit6)) || !mBackWallTriangle->mSensor->isType(0x4F)) &&
        (!(static_cast<u32>(mMovementStates.mLowBits.mBit5)) || !mSideWallTriangle->mSensor->isType(0x4F))) {
        playSound("壁反射");
        playSound("声スピンキャンセル");
        playEffect("壁スパーク");
    }
}

void Mario::calcTornadoTilt() {
    bool flying = false;
    bool spinning = ((mMovementStates.mLowWord >> 16) & 1U) != 0;
    if (getPlayerMode() == PlayerMode_Bee) {
        if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && ((mMovementStates.mLowWord >> 14) & 1U)) {
            flying = true;
        }
    }

    bool tilted = spinning | flying;
    if (!isStickOn() || !tilted) {
        _548 *= mActor->getConst().getTable()->mTornadoTiltCancel;
        bool blended;
        if (tilted) {
            blended = MR::vecBlendSphere(_54C, mHeadVec, &_54C, mActor->getConst().getTable()->mTornadoTiltSpeed);
        } else {
            blended = MR::vecBlendSphere(_54C, mHeadVec, &_54C, mActor->getConst().getTable()->mTornadoTiltOffSpeed);
        }

        if (!blended) {
            _54C = mHeadVec;
        }
    } else {
        TVec3f tilt(getWorldPadDir() * mActor->getConst().getTable()->mTornadoTiltAngle +
                    mHeadVec * (1.0f - mActor->getConst().getTable()->mTornadoTiltAngle));
        MR::normalize(&tilt);
        bool blended = MR::vecBlendSphere(_54C, tilt, &_54C, mActor->getConst().getTable()->mTornadoTiltSpeed);
        MR::normalize(&_54C);
        f32 alignment = MR::abs(getWorldPadDir().dot(mFrontVec));
        _548 = _548 * mActor->getConst().getTable()->mTornadoTiltNear + alignment * (1.0f - mActor->getConst().getTable()->mTornadoTiltNear);
        if (!blended) {
            _54C = tilt;
        }
    }
}

void Mario::reflectWallOnSpinning(const TVec3f& rNormal, u16 time) {
    setFrontVecKeepUp(rNormal);
    _3F8 = time;
    _328 = mFrontVec;
    doSpinWallEffect();
}

void Mario::forceStopTornado() {
    if (((mMovementStates.mLowWord >> 16) & 1U)) {
        _40A = mActor->getConst().getTable()->mTornadoRestartTime;
    }

    resetTornado();
    if (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        cancelTornadoJump();
    }

    mDrawStates.mBits.mBit23 = 1;
}

void Mario::startRotationTask(u32 flags) {
    pushTask(&Mario::taskOnRotation, flags);
}

bool Mario::taskOnRotation(u32 flags) {
    if (flags & 4) {
        if (!isAnimationRun("ヘリコプタージャンプ")) {
            mYAngleOffset = 0.0f;
            return false;
        }

        if (isRising()) {
            mYAngleOffset += mActor->getConst().getTable()->mTrampleBegomaRotRise;
        } else {
            mYAngleOffset += mActor->getConst().getTable()->mTrampleBegomaRotFall;
        }
    }

    return true;
}

void Mario::sendStateMsg(u32 msg) {
    MarioState* pNext;

    for (MarioState* pState = _97C; pState != nullptr;) {
        pNext = pState->_8;

        if (isStatusActive(pState->mStatusId) == MarioStatus_None) {
            pState = pNext;
            continue;
        }

        if (!pState->proc(msg)) {
            if (isStatusActive(pState->mStatusId) != MarioStatus_None) {
                closeStatus(pState);
            }
        } else if (msg == MarioStateMsg_Update) {
            msg = MarioStateMsg_Keep;
        }

        pState = pNext;
    }
}

bool Mario::updatePosture(MtxPtr mtx) {
    if (_97C != nullptr) {
        return _97C->postureCtrl(mtx);
    }

    postureCtrl(mtx);

    return false;
}

void Mario::changeStatus(MarioState* pState) {
    if (isStatusActive(pState->mStatusId)) {
        return;
    }

    _980 = pState;

    sendStateMsg(MarioStateMsg_Notice);

    if (_97C) {
        pState->_8 = _97C;
    }

    _97C = pState;

    if (!pState->proc(MarioStateMsg_Start)) {
        closeStatus(pState);

        _97C = pState->_8;
    }
}

void Mario::closeStatus(MarioState* pState) {
    if (pState == nullptr) {
        while (_97C != nullptr) {
            closeStatus(_97C);
        }

        return;
    }

    MarioState* pCurr = _97C;

    if (pCurr == pState) {
        _97C = pState->_8;
    } else {
        while (pCurr != nullptr) {
            MarioState* pNext = pCurr->_8;

            if (pNext == pState) {
                break;
            }

            pCurr = pNext;
        }

        pCurr->_8 = pState->_8;
    }

    pState->_8 = nullptr;
    pState->proc(MarioStateMsg_Close);
}

u32 Mario::getCurrentStatus() const {
    MarioState* pState = _97C;

    if (pState == nullptr) {
        return MarioStatus_None;
    }

    return pState->mStatusId;
}

bool Mario::isStatusActive(u32 statusId) const {
    MarioState* pState = _97C;

    if (pState == nullptr) {
        return false;
    }

    while (pState != nullptr) {
        if (statusId == pState->mStatusId) {
            return true;
        }

        pState = pState->_8;
    }

    return false;
}

// NONMATCHING: relocation label; all scalar values verified.
void Mario::checkStep() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        return;
    }

    if (isStatusActive(MarioStatus_Hang)) {
        return;
    }

    if (isStatusActive(MarioStatus_Wall)) {
        return;
    }

    if (isStatusActive(MarioStatus_Swim)) {
        return;
    }

    if (isStatusActive(MarioStatus_Climb)) {
        return;
    }


    if (isDamaging()) {
        return;
    }

    if (isSwimming()) {
        return;
    }

    if (!MR::isNearZero(_184)) {
        return;
    }

    if (!isStickOn()) {
        return;
    }

    if (_750) {
        return;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
        return;
    }

    if (isSlipPolygon(_45C)) {
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit10))) {
        return;
    }

    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit6))) {
        return;
    }

    const TVec3f* worldPadDir = &getWorldPadDir();
    if (!(worldPadDir->dot(*mFrontWallTriangle->getNormal(0)) < -0.5f)) {
        return;
    }

    f32 stepHeight = (_4A4 - mPosHolder->mPos).dot(-*getGravityVec());
    if (stepHeight < mActor->getConst().getTable()->mWalkStepHeight) {
        startStep(_4A4);
    }
}

void Mario::startStep(const TVec3f& rVec) {
    if (getCurrentStatus() == 0x10) {
        return;
    }

    TVec3f checkPos = rVec - *getGravityVec() * 80.0f;
    HitInfo hitInfo;

    if (fn_803B2AF0(checkPos, &hitInfo)) {
        return;
    }

    u32 hitNear;
    if (fn_803B2950()) {
        hitNear = Collision::checkStrikeBallToDashYoshiStepAndMap(checkPos, 20.0f, nullptr, nullptr);
    } else {
        hitNear = Collision::checkStrikeBallToMap(checkPos, 20.0f, nullptr, nullptr);
    }
    if (hitNear) {
        return;
    }

    u32 hitFar;
    if (fn_803B2950()) {
        hitFar = Collision::checkStrikeBallToDashYoshiStepAndMap(checkPos, 50.0f, nullptr, nullptr);
    } else {
        hitFar = Collision::checkStrikeBallToMap(checkPos, 50.0f, nullptr, nullptr);
    }
    if (hitFar) {
        return;
    }

    TVec3f oldPos(mPosHolder->mPos);
    mPosHolder->fn_80405F90(rVec);
    const f32 ceilDist = calcDistToCeil(false);
    mPosHolder->fn_80405F90(oldPos);

    if (ceilDist < 200.0f) {
        return;
    }

    if (_4C8->isValid() && MR::isSensorPressObj(_4C8->mSensor)) {
        return;
    }

    if (_4D8->isValid()) {
        if (MR::isSensorPressObj(_4D8->mSensor)) {
            return;
        }

        MtxPtr prevBase = _4D8->getPrevBaseMtx()->toMtxPtr();
        MtxPtr base = _4D8->getBaseMtx()->toMtxPtr();

        if (!MR::isSameMtx(base, prevBase) && _4D8->mSensor != mGroundPolygon->mSensor) {
            return;
        }
    }

    setTrans(rVec);
    changeStatus(mStep);
    mVelocity.zero();
    _3D0 = 0;
    mMovementStates.mLowBits.mBit15 = 0;
}

void Mario::tryBeeStick(const HitSensor* pSensor) {
    bool res = mStick->setStickSensor(pSensor);
    if (res) {
        changeStatus(mStick);
    }
}

bool Mario::isSwimming() const {
    if (isStatusActive(MarioStatus_Swim)) {
        return true;
    }

    return isStatusActive(MarioStatus_Foo);
}

bool Mario::forceStartSwimAndShoot(const TVec3f& rKnockbackVec) {
    WaterInfo waterInfo;
    if (!MR::getWaterAreaObj(&waterInfo, mActor->mPosition)) {
        return false;
    }

    startSwim();
    mSwim->setDamage(rKnockbackVec, 30);
    return true;
}

bool Mario::forceExitSwim() {
    if (!isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    mSwim->mNextAction = MarioSwim::EXIT_ACTION_SURFACE;
    closeStatus(mSwim);
    return true;
}

bool Mario::checkStartSwim() {
    if (isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Climb)) {
        return false;
    }

    if ((static_cast<u32>(mFlags14.mLowBits.mBit24)) && ((getMovementStates().mLowWord >> 30) & 1U)) {
        getAnimator()->waterToGround();
        mFlags14.mLowBits.mBit24 = 0;
    }

    if (!fn_803B2950() && isPlayerModeYoshi()) {
        mActor->fn_803B9830()->fn_8041BD30();
    }

    if (mSwim->fn_803FD5E0(0)) {
        if (getPlayerMode() == 3) {
            addTrans(-*getGravityVec() * 100.0f);
            if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
                stopJump();
            }

            if (getPlayer()->isStatusActive(MarioStatus_Wall)) {
                getPlayer()->closeStatus(0);
            }

            if (getPlayer()->isStatusActive(MarioStatus_Blown)) {
                getPlayer()->closeStatus(0);
            }

            return false;
        }

        if (!fn_803B2950()) {
            startSwim();
            return true;
        }
    }

    mSwim->updateUnderWater();
    return false;
}

void Mario::startSwim() {
    if (!isStatusActive(MarioStatus_Swim)) {
        if (getPlayer()->isDamaging()) {
            mSwim->_9D = 4;
            if (_3BC >= 75) {
                playSound("水落下突入高");
            } else {
                playSound("水落下突入");
            }
        } else if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::Jumping) >> 31)) {
            mSwim->_9D = 1;
            if (_3BC >= 75) {
                playSound("水落下突入高");
            } else {
                playSound("水落下突入");
            }
        } else {
            mSwim->_9D = 0;
            playSound("水歩行突入");
        }

        mActor->fn_803B9F10();
        changeStatus(mSwim);
        clearSlope();
        stopWalk();
        stopJump();
        mSinkTimer = 0;

        mMovementStates.mHighWord &= 0xFFFFFFFCU;
        mMovementStates.mLowBits.mBit13 = 0;
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mLowBits.mBit5)) || (static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
            addVelocity(getWallNorm(), 50.0f);
        }

        getPlayer()->lockGroundCheck(mSwim, true);
        checkBaseTransBall();
        mActor->_F44 = false;
    }
}

bool Mario::checkWaterDamage() {
    if (((mMovementStates.mLowWord >> 4) & 1U)) {
        mMovementStates.mLowBits.mBit4 = 0;
        mActor->fn_803BE840(_7C4);
        mSwim->addDamage(_7C4);
        return true;
    } else if (((mMovementStates.mHighWord >> 24) & 1U)) {
        mMovementStates.mHighBits.mBit24 = 0;
        mActor->fn_803BE840(_7C4);
        mSwim->addFaint(_7C4);
        return true;
    } else {
        return false;
    }
}

void Mario::startTalk(const LiveActor* pActor) {
    stopWalk();

    mTalk->_14 = pActor;

    changeStatus(mTalk);
}

void Mario::endTalk() {
    closeStatus(mTalk);
}

void Mario::delTask(MarioModuleTask* pTask) {
    MarioModuleTask* next = pTask->mNext;
    if (next == nullptr) {
        return;
    }

    MarioModuleTask* head = _974;
    if (head == pTask) {
        if (next == pTask) {
            _974 = nullptr;
        } else {
            _974 = next;
        }

        pTask->mNext = nullptr;
        pTask->end();
        return;
    }

    while (true) {
        if (head == nullptr) {
            return;
        }

        MarioModuleTask* headNext = head->mNext;
        if (headNext == pTask) {
            if (next == pTask) {
                head->mNext = head;
            } else {
                head->mNext = next;
            }

            pTask->mNext = nullptr;
            pTask->end();
            return;
        }

        if (headNext == head) {
            return;
        }
    }
}

void Mario::execTask() {
    MarioModuleTask* task = _974;

    while (true) {
        if (task == nullptr) {
            return;
        }

        MarioModuleTask* next = task->mNext;
        if (!task->exec()) {
            delTask(task);
        }

        if (next == task) {
            return;
        }

        task = next;
    }
}

void Mario::drawTask() const {
    MarioModuleTask* task = _974;

    while (true) {
        if (task == nullptr) {
            return;
        }

        MarioModuleTask* next = task->mNext;
        task->draw();
        if (next == task) {
            return;
        }

        task = next;
    }
}

void Mario::initTask() {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        _984[i] = nullptr;
    }

    _974 = nullptr;
}

bool Mario::isActiveTask(Task task) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == task) {
            return true;
        }
    }

    return false;
}

bool Mario::isActiveTaskID(u32 id) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] != nullptr) {
            if (_A08[i] & id) {
                return true;
            }
        }
    }

    return false;
}

bool Mario::pushTask(Task task, u32 flags) {
    if (isActiveTask(task)) {
        return false;
    }

    u32 index = 0;
    for (; index < ARRAY_SIZE(_984); index++) {
        if (_984[index] == nullptr) {
            break;
        }
    }

    _984[index] = task;
    _A08[index] = flags;
    return true;
}

void Mario::popTask(Task task) {
    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == task) {
            _984[i] = nullptr;
            return;
        }
    }
}

void Mario::callExtraTasks(u32 flags) {
    execTask();

    for (u32 i = 0; i < ARRAY_SIZE(_984); i++) {
        if (_984[i] == nullptr) {
            continue;
        }

        if (flags != 0 && !(flags & _A08[i])) {
            continue;
        }

        if (!(this->*_984[i])(_A08[i])) {
            Task task = _984[i];
            popTask(task);
        }
    }
}

void Mario::startHandy() {
    pushTask(&Mario::taskOnHandy, 0x40);
}

bool Mario::taskOnHandy(u32) {
    if (!mActor->_468) {
        stopEffect("いい汗");
        return false;
    }

    if (mTargetWalkSpeedIndex > 2) {
        playEffect("いい汗");
    } else {
        stopEffect("いい汗");
    }

    return true;
}

void Mario::startHipDropBlur() {
    if (isPlayerModeHopper()) {
        if (gIsLuigi) {
            playEffect("ホッパー尻落ルイージ");
        } else {
            playEffect("ホッパー尻落");
        }

        pushTask(&Mario::taskOnHipDropBlurHopper, 0x80);
        return;
    }

    if (isPlayerModeCloud()) {
        if (gIsLuigi) {
            playEffect("雲尻落ルイージ");
        } else {
            playEffect("雲尻落");
        }
        pushTask(&Mario::taskOnHipDropBlurCloud, 0x80);
        return;
    }

    if (gIsLuigi) {
        playEffect("尻落ルイージ");
    } else {
        playEffect("尻落");
    }

    pushTask(&Mario::taskOnHipDropBlur, 0x80);
}

bool Mario::taskOnHipDropBlurHopper(u32) {
    if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20)) || (static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("ホッパー尻落ルイージ");
        } else {
            stopEffect("ホッパー尻落");
        }
        return false;
    }
    return true;
}

bool Mario::taskOnHipDropBlurCloud(u32) {
    if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20)) || (static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("雲尻落ルイージ");
        } else {
            stopEffect("雲尻落");
        }

        return false;
    }

    return true;
}

bool Mario::taskOnHipDropBlur(u32) {
    if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20)) || (static_cast<u32>(mMovementStates.mLowBits.mBit30)) || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("尻落ルイージ");
        } else {
            stopEffect("尻落");
        }

        return false;
    }

    return true;
}

// NONMATCHING: vector temporary slots, registers and literal/callback identities.
void Mario::startHipDropSlide(const HitSensor* pSensor) {
    if (isActiveTaskID(0x100)) {
        return;
    }

    if (isActiveTaskID(0x200)) {
        return;
    }

    pushTask(&Mario::taskOnHipDropSlide, 0x100);

    TVec3f dir(mPosHolder->mPos - *MR::getSensorPos(pSensor));

    if (MR::isSameDirection(dir, getAirGravityVec(), sSameDirEpsilon) || MR::isNearZero(dir, sNearZeroEpsilon)) {
        dir = -mFrontVec;
    }

    MR::normalizeOrZero(&dir);

    _A58.cross(-getAirGravityVec(), dir);
    MR::normalizeOrZero(&_A58);

    _A68 = const_cast< HitSensor* >(pSensor);
    _A4C = *MR::getSensorPos(pSensor);
    _A64 = MR::getSensorRadius(const_cast< HitSensor* >(pSensor));
    _70C = getAirGravityVec();

    changeAnimation("ヒップドロップ滑り");
}

bool Mario::taskOnHipDropSlide(u32 flags) {
    if (flags == 0x100) {
        if (!(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit20))) {
            return false;
        }
    }

    if (MR::diffAngleAbs(_70C, getAirGravityVec()) > sHipDropSlideAngle) {
        return false;
    }

    if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit30)) || !(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit31)) || isStatusActive(MarioStatus_Swim)) {
        return false;
    }

    HitSensor* sensor = _A68;
    bool enabled = sensor->mValidByHost != 0 && sensor->mValidBySystem != 0;

    if (!enabled) {
        return false;
    }

    TVec3f dir(mPosHolder->mPos);
    dir -= _A4C;
    dir.setLength(_A64);

    TVec3f dirCopy(dir);
    MR::normalizeOrZero(&dir);

    TVec3f reverseGravity = -_70C;
    if (dir.dot(reverseGravity) <= sZero) {
        return false;
    }

    Mtx rot;
    PSMTXRotAxisRad(rot, &_A58, sHipDropSlideRot);
    PSMTXMultVecSR(rot, &dir, &dir);
    dir.setLength(_A64);
    dir -= dirCopy;
    dir.setLength(sHipDropSlideLen);

    cutGravityElementFromJumpVec(true);
    cutVecElementFromJumpVec(_70C);
    addVelocity(dir);

    dirCopy.setLength(sOne);
    addVelocity(dirCopy);
    return true;
}

// NONMATCHING: vector temporary slots, registers and literal/callback identities.
void Mario::startJumpDropSlide(const HitSensor* pSensor) {
    if (isActiveTaskID(0x100)) {
        return;
    }

    if (isActiveTaskID(0x200)) {
        return;
    }

    TVec3f dir(mPosHolder->mPos - *MR::getSensorPos(pSensor));
    MR::normalizeOrZero(&dir);

    f32 dot = dir.dot(-getAirGravityVec());
    if (dot <= sJumpDropSlideDotMin) {
        return;
    }

    pushTask(&Mario::taskOnHipDropSlide, 0x200);

    TVec3f slideDir(mPosHolder->mPos - *MR::getSensorPos(pSensor));

    if (MR::isSameDirection(slideDir, getAirGravityVec(), sSameDirEpsilon) || MR::isNearZero(slideDir, sNearZeroEpsilon)) {
        slideDir = -mFrontVec;
    }

    MR::normalizeOrZero(&slideDir);

    _A58.cross(-getAirGravityVec(), slideDir);
    MR::normalizeOrZero(&_A58);

    _A68 = const_cast< HitSensor* >(pSensor);
    _A4C = *MR::getSensorPos(pSensor);
    _A64 = MR::getSensorRadius(const_cast< HitSensor* >(pSensor));
    _70C = getAirGravityVec();

    if (mActor->_468 == 0) {
        bool notHopper = !isPlayerModeHopper();
        if (notHopper && dot > sHopperJumpSlideDotMin) {
            f32 frontDot = mFrontVec.dot(slideDir);
            if (frontDot > sZero) {
                changeAnimation("ジャンプ順滑り");
            } else {
                changeAnimation("ジャンプ逆滑り");
            }
        }
    }

    mJumpVec.zero();
}

void Mario::startFreezeEnd() {
    pushTask(&Mario::taskOnFreezeEnd, 0x800);
}

bool Mario::taskOnFreezeEnd(u32) {
    return mActor->fn_803BB5F0();
}

void Mario::startTeresaMode() {
    _418 = 0;
    _428 = 0;
    getPlayer()->mMovementStates.mLowBits.mBit31 = 1;
    getPlayer()->mMovementStates.mLowBits.mBit30 = 0;
    getPlayer()->mMovementStates.mHighBits.mBit29 = 0;
    mJumpVec.zero();
    changeStatus(mTeresa);
}

void Mario::startTeresaDisappear() {
    _418 = mActor->getConst().getTable()->mTeresaWallThroughTime;
    playSound("テレサ消える");
    playSound("声トルネード");
    MR::startCSSound("CS_TERESA", nullptr, 0, 0, 1.0f);
    mTeresa->updateDropFlag();
    resetSleepTimer();
    if (_962 == 5 && mVerticalSpeed < 100.0f) {
        MR::isNearZero(mAirGravityVec, 0.001f);
        mTeresa->_34 += mAirGravityVec * 10.0f;
    }
}

bool Mario::getHitWallNorm(TVec3f* pNormal) {
    if (((getPlayer()->mMovementStates.mLowWord >> 23) & 1U)) {
        if (!isThroughWall(mFrontWallTriangle)) {
            *pNormal = *mFrontWallTriangle->getNormal(0);
            return true;
        }
    } else if (((getPlayer()->mMovementStates.mLowWord >> 6) & 1U)) {
        if (!isThroughWall(mBackWallTriangle)) {
            *pNormal = *mBackWallTriangle->getNormal(0);
            return true;
        }
    } else if (((getPlayer()->mMovementStates.mLowWord >> 5) & 1U) && !isThroughWall(mSideWallTriangle)) {
        *pNormal = *mSideWallTriangle->getNormal(0);
        return true;
    }

    return false;
}

void Mario::resetTeresaMode() {
    mActor->_F44 = true;
    _418 = 0;
    mTeresa->resetTeresaMode();
}

bool Mario::fn_803AC810() {
    return this->mTeresa->isTeresaAccel();
}

void Mario::doTeresaReflection(const TVec3f& rNormal, bool effect) {
    mTeresa->doTeresaReflection(rNormal, effect);
}

bool Mario::isBlendWaitGround() const {
    return calcAngleD(_368) > 8.0f;
}

void Mario::checkSpecialWaitAnimation() {
    if (mWait->checkStart()) {
        changeStatus(mWait);
    }
}

void Mario::resetSleepTimer() {
    mWait->_16 = 0;
    if (isStatusActive(MarioStatus_Wait)) {
        closeStatus(mWait);
    }
}

void Mario::stopWalk() {
    mTargetWalkSpeedIndex = 0;
    mWalkSpeed = 0.0f;
    _71E = 0;
    _71F = 0;
    _3D4 = 0;

    getAnimator()->initWalkWeight();
    getAnimator()->resetTilt();

    mMovementStates.mLowBits.mBit15 = 0;
    mMovementStates.mHighBits.mBit28 = 0;
    cancelSquatMode();

    mMovementStates.mLowWord &= ~Mario::MovementStates::Turning;
    _3D2 = 0;
    _8F0 = 0.0f;
    _3F4 = 0.0f;

    stopEffect("共通スリップ坂");
    stopAnimation("歩行制動ブレーキ", 1);
    stopAnimation("ブレーキ");
}

void Mario::cancelSquatMode() {
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        return;
    }

    calcDistToCeil(false);

    if ((static_cast<u32>(mFlags24.mLowBits.mBit21)) || (static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        mFlags14.mLowBits.mBit16 = 1;
    }

    mMovementStates.mLowBits.mBit21 = 0;
    mFlags24.mLowBits.mBit21 = 0;
    stopAnimation("しゃがみ基本");

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) || !(static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        if (!isSwimming()) {
            changeAnimation(nullptr, "基本");
        }

        if (!isAnimationRun("サマーソルト") && (static_cast<u32>(mFlags14.mLowBits.mBit16))) {
            if (mWalkSpeed > 0.1f) {
                if (!isPlayerModeYoshi()) {
                    fn_803EEC20("しゃがみ終了", nullptr);
                }
            } else {
                changeAnimation("しゃがみ終了");
            }
        }
    }

    mMovementStates.mHighBits.mBit25 = 0;
}

f32 Mario::getTargetWalkSpeed() const {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21))) {
        return 0.0f;
    }

    f32 targetWalkSpeed = ::sWalkTargetTable[mTargetWalkSpeedIndex];

    if (isPlayerModeYoshi()) {
        targetWalkSpeed *= mActor->fn_803B9810()->fn_8041FB30();
    }

    if (mSinkTimer != 0) {
        if (_960 == 25) {
            targetWalkSpeed = 0.0f;
        } else if (_960 == 31) {
            targetWalkSpeed = 0.0f;
        } else {
            targetWalkSpeed *= 1.0f - static_cast< f32 >(mSinkTimer) / 256.0f;
        }
    }

    targetWalkSpeed *= 1.0f - _2D0;

    if (_434 != 0) {
        targetWalkSpeed *= mActor->getConst().getTable()->mItemDashRatio;
    }

    return targetWalkSpeed;
}

void Mario::fn_803ACC40() {
    this->_3F4 = 0.0f;
    if (!(static_cast<u32>(this->mMovementStates.mLowBits.mBit21))) {
        this->fn_803ACD80();
        return;
    }
    if (this->isAnimationRun("壁押し", static_cast<u32>(0))) {
        this->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
    this->fn_803ACCE0();
    this->fn_803AD120();
    this->decideSquatWalkAnimation();
    this->fn_803AD4F0();
    this->mPrevAnimFrame = this->getAnimator()->getFrame();
}

// NONMATCHING: squat/brake animation helpers, rough from the m2c draft

void Mario::fn_803ACCE0() {
    if (this->isAnimationRun("ジャンプ")) {
        this->stopAnimation("ジャンプ", (const char*)nullptr);
    }
    if (this->fn_803ACE10()) {
        this->fn_803ACE60();
    }
    this->fn_803ACF70();
    if (this->isAnimationRun("しゃがみ基本")) {
        this->fn_803AD0B0();
    }
}

// NONMATCHING: the walk-stop animation and sounds approximated
void Mario::fn_803ACD80() {
    if (this->mWalkSpeed > 0.1f) {
        this->stopAnimation("しゃがみ", "基本");
        if (!this->isPlayerModeYoshi()) {
            this->fn_803EEC20("しゃがみ終了", 0);
        }
    } else {
        this->changeAnimation("しゃがみ終了", "基本");
    }
    this->mMovementStates.mHighBits.mBit25 = 0;
}

bool Mario::fn_803ACE10() {
    bool ret = false;
    if (!(this->mFlags24.mLowWord >> 21 & 1) || !this->isAnimationRun("しゃがみ基本")) {
        ret = true;
    }
    return ret;
}

// NONMATCHING: identical instructions apart from proven equal string-pool addresses.
void Mario::fn_803ACE60() {
    this->stopAnimation("歩行制動ブレーキ", static_cast<u32>(1));
    if (this->isAnimationRun("カリカリ限界")) {
        this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
    }
    this->changeAnimation((const char*)nullptr, "しゃがみ基本");
    this->stopAnimation("飛び込み準備", static_cast<u32>(4));
    this->fn_803AF2F0(cSquatWalkStoppedWeights);
    this->mMovementStates.mHighBits.mBit25 = 0;
    if (this->isPlayerModeYoshi()) {
        if (!(static_cast<u32>(this->mFlags24.mLowBits.mBit21))) {
            MR::startActionSound(this->mActor->getYoshi(), "Squat", -1, -1, -1);
        }
    } else if (((static_cast<u32>(this->mMovementStates.mLowBits.mBit31)) == 0) && !this->isAnimationRun("サマーソルト")) {
        this->playSound("声しゃがむ", -1);
    }
}

// NONMATCHING: independent maximum/minimum speed loads are scheduled in reverse order.
void Mario::fn_803ACF70() {
    if (mWalkSpeed < mActor->mConst->getTable()->mSpeedSquatWalkLower) {
        if (isStickOn()) {
            mWalkSpeed = mActor->mConst->getTable()->mSquatWalkMinSpeed;
            fn_803AF2F0(cSquatWalkMovingWeights);
            mTargetWalkSpeedIndex = 1;
            mMovementStates.mHighBits.mBit25 = 1;
        } else {
            fn_803AF2F0(cSquatWalkStoppedWeights);
            mTargetWalkSpeedIndex = 0;
            mMovementStates.mHighBits.mBit25 = 0;
        }
        return;
    }
    if (!isStickOn()) {
        fn_803AF2F0(cSquatWalkStoppedWeights);
        mTargetWalkSpeedIndex = 0;
        return;
    }
    if ((mMovementStates.mHighWord >> 25) & 1) {
        MarioConstTable* table = mActor->mConst->getTable();
        f32 maxSpeed = table->mSquatWalkMaxSpeed;
        f32 minSpeed = table->mSquatWalkMinSpeed;
        f32 speedRange = maxSpeed - minSpeed;
        mWalkSpeed = minSpeed + mStickPos.z * speedRange;
        fn_803AF2F0(cSquatWalkMovingWeights);
        mTargetWalkSpeedIndex = 1;
    }
}

void Mario::fn_803AD0B0() {
    f32 var_f31 = this->mWalkSpeed * (60.0f / this->mActor->mConst->getTable()->mSquatWalkStep);
    if (!(static_cast<u32>(this->mMovementStates.mHighBits.mBit25))) {
        var_f31 = 1.0f;
    }
    this->getAnimator()->setSpeed(var_f31);
}

void Mario::fn_803AD120() {
    if (this->fn_803EF7C0() == 0x20 && this->mTargetWalkSpeedIndex != 0) {
        this->_3F4 = 0.2f;
    }
}

// NONMATCHING: temporary/register allocation; SMG2 shallow-water ripple behavior recovered.
void Mario::decideSquatWalkAnimation() {
    if (mTargetWalkSpeedIndex != 0) {
        u32 flags = mDrawStates.mWord;
        if ((((flags >> 13) & 1) || ((flags >> 12) & 1)) && _738 < 160.0f) {
            if (isPlayerModeYoshi()) {
                f32 frame = getAnimator()->getFrame();
                f32 previous = mPrevAnimFrame;
                if (previous > frame || (previous < 30.0f && getAnimator()->getFrame() >= 30.0f)) {
                    playSound("水跳ね左足小");
                    playEffect("水はね弱");
                    f32 depth = _738;
                    playEffectSRT("水波紋", 0.2f, _73C, mGroundPos + _368 * depth);
                }
            } else {
                if (mPrevAnimFrame > getAnimator()->getFrame()) {
                    playSound("水跳ね左足小");
                    playEffect("水はね左弱");
                    f32 depth = _738;
                    playEffectSRT("水波紋", 0.2f, _73C, mGroundPos - mSideVec * 20.0f + _368 * depth);
                }
                if (mPrevAnimFrame < 30.0f && getAnimator()->getFrame() >= 30.0f) {
                    playSound("水跳ね右足小");
                    playEffect("水はね右弱");
                    f32 depth = _738;
                    playEffectSRT("水波紋", 0.2f, _73C, mGroundPos + mSideVec * 20.0f + _368 * depth);
                }
            }
        }
    } else if (static_cast<u32>(mDrawStates.mBits.mBit13)) {
        f32 depth = _738;
        if (depth < 160.0f && !(mActor->_37C & 0x3F)) {
            playEffectSRT("水波紋", 0.2f, _73C,
                          mShadowPos + mSideVec * (MR::getRandom() - 0.5f) * 20.0f + _368 * depth);
        }
    }
}

// NONMATCHING: equivalent direct exit replaces the target byte-gate trampoline.
void Mario::fn_803AD4F0() {
    if (isPlayerModeYoshi() && mTargetWalkSpeedIndex != 0 &&
        (static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit30)) && _A7F != 0) {
        f32 frame = getAnimator()->getFrame();
        f32 previousFrame = mPrevAnimFrame;
        if (previousFrame > frame || (previousFrame < 30.0f && getAnimator()->getFrame() >= 30.0f)) {
            playEffect("ヨッシー泥移動");
        }
    }
}

// NONMATCHING: stack allocation, cached animation frame and literal pool ordering.
void Mario::fn_803AD5B0() {
    if (mTargetWalkSpeedIndex != 0) {
        if ((static_cast<u32>(mDrawStates.mBits.mBit13)) || (static_cast<u32>(mDrawStates.mBits.mBit12))) {
            if (_738 < 160.0f) {
                if (isPlayerModeYoshi()) {
                    f32 frame = getAnimator()->getFrame();
                    f32 previousFrame = mPrevAnimFrame;
                    if (previousFrame > frame ||
                        (previousFrame < 30.0f && getAnimator()->getFrame() >= 30.0f)) {
                        u8 index = mTargetWalkSpeedIndex;
                        if (index >= 2) {
                            if (index < 6) {
                                playSound("水跳ね左足小", -1);
                                playEffect("水はね弱");
                            } else {
                                playSound("水跳ね左足", -1);
                                playEffect("水はね");
                            }
                        }
                        f32 depth = _738;
                        playEffectSRT("水波紋", 0.2f, _73C, mGroundPos + _368 * depth);
                    }
                } else {
                    if (mPrevAnimFrame > getAnimator()->getFrame()) {
                        u8 index = mTargetWalkSpeedIndex;
                        if (index >= 2) {
                            if (index < 6) {
                                playSound("水跳ね左足小", -1);
                                playEffect("水はね左弱");
                            } else {
                                playSound("水跳ね左足", -1);
                                playEffect("水はね左");
                            }
                        }
                        f32 depth = _738;
                        playEffectSRT("水波紋", 0.2f, _73C, mGroundPos - mSideVec * 20.0f + _368 * depth);
                    }
                    if (mPrevAnimFrame < 30.0f) {
                        if (getAnimator()->getFrame() >= 30.0f) {
                            u8 index = mTargetWalkSpeedIndex;
                            if (index >= 2) {
                                if (index < 6) {
                                    playSound("水跳ね右足小", -1);
                                    playEffect("水はね右弱");
                                } else {
                                    playSound("水跳ね右足", -1);
                                    playEffect("水はね右");
                                }
                            }
                            f32 depth = _738;
                            playEffectSRT("水波紋", 0.2f, _73C, mGroundPos + mSideVec * 20.0f + _368 * depth);
                        }
                    }
                }
            }
        }
    } else if (static_cast<u32>(mDrawStates.mBits.mBit13)) {
        f32 depth = _738;
        if (depth < 160.0f && (mActor->_37C & 0x3F) == 0) {
            playEffectSRT("水波紋", 0.2f, _73C,
                          mShadowPos + mSideVec * (MR::getRandom() - 0.5f) * 20.0f + _368 * depth);
        }
    }
}

// NONMATCHING: equivalent direct exit replaces the target byte-gate trampoline.
void Mario::fn_803AD9C0() {
    if (isPlayerModeYoshi() && mTargetWalkSpeedIndex != 0 &&
        (static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit30)) && _A7F != 0) {
        f32 frame = getAnimator()->getFrame();
        f32 previousFrame = mPrevAnimFrame;
        if (previousFrame > frame || (previousFrame < 30.0f && getAnimator()->getFrame() >= 30.0f)) {
            if (mTargetWalkSpeedIndex >= 2) {
                playEffect("ヨッシー泥移動");
            }
        }
    }
}

// NONMATCHING: relocation labels; speed tables and scalar values verified.
void Mario::decideWalkSpeed() {
    u32 i;
    bool canIndexDecrease = false;
    if (mTargetWalkSpeedIndex != 0) {
        f32 threshold = ::sSpeedTableB[mTargetWalkSpeedIndex - 1];
        if (mStickPos.z < threshold) {
            canIndexDecrease = true;
        }
    }

    for (i = 0; i < ARRAY_SIZE(::sSpeedTableA); i++) {
        if (mStickPos.z < ::sSpeedTableA[i]) {
            break;
        }
    }

    if (mTargetWalkSpeedIndex <= i || canIndexDecrease) {
        mTargetWalkSpeedIndex = i;
    }

    s32 clingNum = MR::getKarikariClingNum();
    if (!mActor->isEnableNerveChange()) {
        clingNum = 0;
    }
    if (clingNum != 0) {
        if (clingNum > 5) {
            clingNum = 5;
        }

        if (mTargetWalkSpeedIndex > 5 - clingNum) {
            mTargetWalkSpeedIndex = 5 - clingNum;
        }
    }

    if (((mDrawStates.mWord & Mario::DrawStates::Underwater) >> 13) && mTargetWalkSpeedIndex > 6) {
        mTargetWalkSpeedIndex = 6;
    }

    if (mActor->mBeeWallWalk != 0 && mTargetWalkSpeedIndex > 4) {
        mTargetWalkSpeedIndex = 4;
    }

    if (_960 == 32 || (isPlayerModeYoshi() && mActor->fn_803B9810()->fn_8041F8C0())) {
        if (mTargetWalkSpeedIndex > 3) {
            mTargetWalkSpeedIndex = 3;
        }

        if (mTargetWalkSpeedIndex != 0) {
            startPadVib(1);
        }

        if (mTargetWalkSpeedIndex > 2) {
            getAnimator()->setAnimationInterpoleRatio(0.5f);
        } else {
            getAnimator()->setAnimationInterpoleRatio(1.0f);
        }
    } else {
        f32 speed = getAnimator()->getAnimationInterpoleRatio();
        f32 new0C = speed;
        new0C += 0.1f;

        if (new0C > 1.0f) {
            new0C = 1.0f;
        }

        getAnimator()->setAnimationInterpoleRatio(new0C);
    }
    if (isPlayerModeYoshi() && mActor->fn_803B9810()->fn_8041FA40() && mTargetWalkSpeedIndex > 8) {
        mTargetWalkSpeedIndex = 8;
    }
    if (fn_803B1C10()) {
        mTargetWalkSpeedIndex = 7;
    }
    if (mCometFireDanceArea) {
        mTargetWalkSpeedIndex = 7;
    }
}

// NONMATCHING: register/temporary allocation and independent instruction scheduling.
#pragma dont_inline on
void Mario::decideWalkAnimation() {
    MarioConst* constants = mActor->mConst;
    f32 walkScale = constants->mTable[constants->mCurrentTable]->mWalkSpeed;
    if (mTargetWalkSpeedIndex == 0 && mWalkSpeed < 0.2f && isBlendWaitGround()) {
        getAnimator()->fn_803DE880();
    } else {
        getPlayer()->mFlags14.mLowBits.mBit16 = 0;

        if (mTargetWalkSpeedIndex == 0 && mSwim->_1B2) {
            bool prepare = !isPlayerModeBee();
            if (prepare) {
                changeAnimation("飛び込み準備", 4);
                return;
            }
        }

        if (mSinkTimer == 0) {
            if (getPlayer()->mTargetWalkSpeedIndex != 0) {
                getAnimator()->fn_803DEC10();
            }

            fn_803AF2F0(::sWeightTable[mTargetWalkSpeedIndex]);
        } else {
            f32 weights[] = {0, 0, 0, 0};
            if (mTargetWalkSpeedIndex != 0) {
                f32 f1 = 1.0f;
                if (mTargetWalkSpeedIndex <= 3) {
                    if (mTargetWalkSpeedIndex == 2) {
                        f1 = 0.75f;
                    }

                    if (mTargetWalkSpeedIndex == 3) {
                        f1 = 0.5f;
                    }

                    weights[0] = f1;
                    weights[1] = 1.0f - f1;
                } else {
                    if (mTargetWalkSpeedIndex >= 5) {
                        f1 = static_cast< f32 >(mSinkTimer) / 100.0f;
                    }

                    f1 = MR::clamp(f1, 0.0f, 1.0f);
                    weights[1] = f1;
                    weights[2] = 1.0f - f1;
                }
            } else {
                weights[3] = 1.0f;
            }

            fn_803AF2F0(&weights[0]);
        }
    }

    stopAnimation("飛び込み準備", 4);

    fn_803AD5B0();
    fn_803AD9C0();
    mPrevAnimFrame = getAnimator()->getFrame();

    f32* footStep = ::sFootStep;
    f32 f3 = 0.0f;

    if (mActor->mBeeWallWalk != 0) {
        footStep = ::sFootStepBeeWallWalk;
    }

    if (isPlayerModeYoshi()) footStep = ::sFootStepYoshi;

    for (int i = 0; i < ARRAY_SIZE(*::sWeightTable); i++) {
        f3 += footStep[i] * ::sWeightTable[mTargetWalkSpeedIndex][i];
    }

    f32 animationSpeed;

    if (f3 == 0.0f) {
        animationSpeed = 0.33f;
    } else {
        animationSpeed = 0.5f * (60.0f * ((0.01f * (mWalkSpeed * walkScale)) / f3));
    }

    if (_8F0 > 0.0f && !(static_cast<u32>(mDrawStates.mBits.mBit27))) {
        animationSpeed *= 1.0f + ((_8F0 / 10.0f) * mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mSlopeAnimeRatio);

        if (_8F0 > 5.0f && animationSpeed > 4.0f) {
            animationSpeed *= mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mSlopeSpinAnimeRatio;
            changeAnimation("がんばり走り");
            startBas("RunSlope", false, 0.0f, 0.0f);
        }
    } else if (_3FE != 0) {
        animationSpeed *= _8F4;
    }

    f32 diffFromTargetSpeed = getTargetWalkSpeed() - mWalkSpeed;

    if (mWalkSpeed > ::sWalkTargetTable[5]) {
        f32 squared = diffFromTargetSpeed * diffFromTargetSpeed;
        f32 factor = (mWalkSpeed - ::sWalkTargetTable[5]) / (1.0f - ::sWalkTargetTable[5]);
        diffFromTargetSpeed = (diffFromTargetSpeed * (1.0f - factor)) + (squared * factor);
    } else if (mWalkSpeed < ::sWalkTargetTable[3]) {
        f32 sqrt = MR::sqrt(diffFromTargetSpeed);
        f32 factor = (::sWalkTargetTable[3] - mWalkSpeed) / ::sWalkTargetTable[3];
        diffFromTargetSpeed = (diffFromTargetSpeed * factor) + (sqrt * (1.0f - factor));
    }

    f32 f4 = mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mStartSpinAnimeRatio;
    f32 f5 = 1.0f + 4.0f * diffFromTargetSpeed;

    if (f5 > 2.0f) {
        f5 = 2.0f;
    }

    if (f5 < 1.0f) {
        f5 = 1.0f;
    }

    if (getTargetWalkSpeed() < ::sWalkTargetTable[6]) {
        f5 = 1.0f;
    }

    if (fn_803EF7C0() == 32 && mTargetWalkSpeedIndex != 0) {
        f5 = 1.2f;
    }

    f32 f6 = f4 * (0.5f * (60.0f * ((getTargetWalkSpeed() * walkScale * 0.01f) / footStep[2])));
    f32 factor2 = 2.0f - f5;

    if (animationSpeed < f6) {
        animationSpeed = animationSpeed * factor2 + f6 * (1.0f - factor2);
    }

    if (mSinkTimer != 0 && mTargetWalkSpeedIndex != 0 && animationSpeed < 1.0f) {
        animationSpeed = 1.0f;
    }

    animationSpeed *= (1.0f - _2D0);
    _3F4 = f5 - 1.0f;
    getAnimator()->setSpeed(animationSpeed);

    if (!mActor->_EA4 && mTargetWalkSpeedIndex == 0 && mActor->mHealth == 1 && mActor->mMaxHealth > 2 &&
        (_970 == nullptr || strcmp(_970, "DamageWait"))) {
        getAnimator()->changeTrackAnim(3, "ダメージウエイト");
        startBas("DamageWait", false, 0.0f, 0.0f);
        mActor->fn_803BBFD0("DamageWait");
    }

    checkWallPush();

    f32 f7 = 0.9f;

    if (gIsLuigi) {
        f7 = 0.3f;
    }

    if (mTargetWalkSpeedIndex > 5 && mWalkSpeed > f7 && !(static_cast<u32>(mDrawStates.mBits.mBit27)) && !(static_cast<u32>(mMovementStates.mHighBits.mBit10))) {
        _71E = mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mBrakeFirstTimer;
    }

    if (_71E != 0) {
        _71E--;
    }

    if (mTargetWalkSpeedIndex == 0) {
        if (_71E != 0 && !isSlipPolygon(mGroundPolygon) && !(static_cast<u32>(mDrawStates.mBits.mBit26))) {
            doBrakingAnimation();
            _71F = mActor->mConst->mTable[mActor->mConst->mCurrentTable]->mBrakeSecondTimer;
        }

        _71E = 0;

        s32 clingNum = MR::getKarikariClingNum();
        if (!mActor->isEnableNerveChange()) clingNum = 0;
        if (clingNum >= 1) {
            if (!isPlayerModeYoshi()) changeAnimationUpper("カリカリ限界");
            stopAnimation("歩行制動ブレーキ", 1);
        }

        if (clingNum < 1 && isAnimationRun("カリカリ限界")) {
            stopAnimationUpper("カリカリ限界");
        }
    }
}

#pragma dont_inline reset

void Mario::doBrakingAnimation() {
    if (!isAnimationRun("歩行制動ブレーキ")) {
        playSound("ターンブレーキ", -1);
    }
    changeAnimation("歩行制動ブレーキ", 1);
    getAnimator()->setAnimationFrameCtrlAttribute(1);
    if (gIsLuigi) {
        getAnimator()->setSpeed(0.5f);
    }

    playEffect("共通ブレーキ");
    _71F = 0;
}

void Mario::checkWallPush() {
    if (mTargetWalkSpeedIndex != 0 && ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mHighBits.mBit13))) && checkWallJumpCode()) {
        return;
    }

    f32 angle = MR::diffAngleAbsHorizontal(mFrontVec, -getWallNorm(), *getGravityVec());
    bool sideStep = false;
    f32 wallPushAngleRange = getActor()->getConst().getTable()->mWallPushAngleRange;

    bool checkAngle = mTargetWalkSpeedIndex != 0 && (static_cast<u32>(mMovementStates.mLowBits.mBit23));

    if (checkAngle && angle < MR::toRadian(wallPushAngleRange)) {
        sideStep = true;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit21))) {
        sideStep = false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit19))) {
        sideStep = false;
    }

    if (calcAngleD(getWallNorm()) < getActor()->getConst().getTable()->mForceWallAngle) {
        sideStep = false;
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && mTargetWalkSpeedIndex != 0) {
            mTargetWalkSpeedIndex = 1;
            mWalkSpeed = 0.0f;
        }
    }

    if (!isAnimationRun("壁押し", 0) && sideStep) {
        doSideStep();
    }
}

void Mario::updateBrakeAnimation() {
    if (_71F != 0) {
        if (!isAnimationRun("歩行制動ブレーキ", 1)) {
            _71F = 0;
        } else {
            _71F--;
            if (!MR::isNearZero(mStickPos.z)) {
                _71F = 0;
            }

            if (_71F == 0) {
                stopAnimation(nullptr);
                stopWalk();
            }
        }
    } else if (isAnimationRun("歩行制動ブレーキ", 1) && (isAnimationTerminate(nullptr) || mTargetWalkSpeedIndex != 0)) {
        stopAnimation(nullptr);
    }

    if (!gIsLuigi) {
        return;
    }

    if (!isAnimationRun("歩行制動ブレーキ", 1) && !isAnimationRun("ブレーキ")) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mHighBits.mBit13))) {
        stopAnimation(nullptr);
        _71F = 0;
        _71E = 0;
        _3D0 = 0;
        _3D2 = 0;
    } else if (!MR::isDemoActive() && (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        playSound("ルイージ滑り");
    }
}

// NONMATCHING: independent load scheduling and register allocation.
void Mario::updateWalkSpeed() {
#ifdef GALAXY_NATIVE
    // OdysseyMovement: SMO's roll (crouch + Spin; Spin again to boost). It sets
    // the speed directly; Galaxy's walking steers it. It ends below 17 (the run
    // then slows to its cap), in the air, or when Mario stops being normal Mario.
    {
        namespace Odyssey = PetariNative::Odyssey;
        const bool canRoll = petari_mod_enabled(2) && getPlayerMode() == 0 && ((mMovementStates.mLowWord >> 30) & 1U) &&
                             !(mMovementStates.mLowWord & Mario::MovementStates::Jumping) && !((mMovementStates.mLowWord >> 27) & 1U) &&
                             mSinkTimer == 0 && !isStatusActive(17);
        const f32 unit = mActor->mConst->getTable()->mWalkSpeed;
        if (Odyssey::Live::rollStart && canRoll && !Odyssey::Live::roll.rolling) {
            Odyssey::Live::roll = Odyssey::startRoll(mWalkSpeed * unit, false);
            mMovementStates.mLowWord &= 0xFFDFFFFFU;  // _A: crouching
            mFlags24.mLowWord &= 0xFFDFFFFFU;         // _20._A
            mMovementStates.mHighWord &= 0xFDFFFFFFU;  // _26
            stopAnimation("しゃがみ基本");
            changeAnimation("飛び込み失敗回転着地");
            playSound("スピンジャンプ");
            std::fprintf(stderr, "[odyssey] roll at %.2f\n", Odyssey::Live::roll.speed);
        } else if (Odyssey::Live::rollBoost && Odyssey::Live::roll.rolling && canRoll &&
                   Odyssey::Live::roll.sinceBoost + 1 >= Odyssey::Const::SlopeRollingReStartInterval) {
            changeAnimation("飛び込み失敗回転着地");
            playSound("スピンジャンプ");
        }
        Odyssey::Live::rollStart = false;
        static const bool trace = std::getenv("PETARI_ODYSSEY_TRACE") != nullptr;
        if (trace && Odyssey::Live::roll.rolling) {
            std::fprintf(stderr, "[odyssey] roll speed %.3f walk %.3f pos (%.1f, %.1f, %.1f) ground %d jumping %d slip %d sink %d status17 %d\n",
                         Odyssey::Live::roll.speed, mWalkSpeed, mPosHolder->mPos.x, mPosHolder->mPos.y, mPosHolder->mPos.z,
                         static_cast< int >((mMovementStates.mLowWord >> 30) & 1U),
                         static_cast< int >((mMovementStates.mLowWord >> 31) & 1U), static_cast< int >((mMovementStates.mLowWord >> 27) & 1U),
                         static_cast< int >(mSinkTimer), static_cast< int >(isStatusActive(17)));
        }
        if (Odyssey::Live::roll.rolling && !canRoll) {
            Odyssey::Live::roll.rolling = false;
        }
        if (Odyssey::Live::roll.rolling) {
            // Slopes: faster down, slower up (along the roll's heading).
            if (mGroundPolygon != nullptr && mGroundPolygon->isValid()) {
                TVec3f up(-*getGravityVec());
                MR::normalizeOrZero(&up);
                const TVec3f& normal = *mGroundPolygon->getNormal(0);
                const f32 cosSlope = MR::clamp(normal.dot(up), -1.0f, 1.0f);
                TVec3f downhill(-up - normal * normal.dot(-up));  // gravity along the ground
                if (MR::normalizeOrZero(&downhill) == 0) {  // a usable direction (flat ground gives none)
                    TVec3f front(mFrontVec - normal * normal.dot(mFrontVec));
                    MR::normalizeOrZero(&front);
                    const f32 slopeDegrees = std::acos(cosSlope) * 180.0f / 3.14159265f;
                    Odyssey::Live::roll.speed += Odyssey::rollSlopeChange(slopeDegrees, front.dot(downhill));
                }
            }
            Odyssey::stepRoll(Odyssey::Live::roll, Odyssey::Live::rollBoost);
            Odyssey::Live::rollBoost = false;
            mMovementStates.mLowWord &= 0xFFDFFFFFU;  // _A
            mWalkSpeed = Odyssey::Live::roll.speed / unit;
            // Keep Galaxy's run target following the stick through the roll (its
            // animator only updates it while a walk animation runs), so the run
            // picks up where the roll ends.
            decideWalkSpeed();
            if (!Odyssey::Live::roll.rolling) {
                // The roll ended (below 17): a direction held runs on at this speed,
                // slowing to the cap; released, Galaxy's brake skid to a stop.
                stopAnimation("飛び込み失敗回転着地", "基本");
                _404 = 0;  // no slow start: the stick already holds a run
                if (!isStickOn()) {
                    doBrakingAnimation();
                    _71F = mActor->mConst->getTable()->mBrakeSecondTimer;
                }
                return;
            }
            // Galaxy has no rolling loop: restart the dive landing's forward roll
            // (about 12 frames long) for as long as the roll lasts; otherwise Mario
            // glides standing between rolls.
            if (Odyssey::Live::roll.sinceBoost % 12 == 0) {
                stopAnimation("飛び込み失敗回転着地");
                changeAnimation("飛び込み失敗回転着地");
            }
            return;
        }
        Odyssey::Live::rollBoost = false;
    }
#endif
    f32 target = getTargetWalkSpeed();
    f32 ramp = 1.0f;
    MarioConst* constants;
    if (target == 0.0f) {
        constants = mActor->mConst;
        _404 = constants->mTable[constants->mCurrentTable]->_282;
    }
    u16 remaining = _404;
    if (remaining != 0) {
        constants = mActor->mConst;
        u16 duration = constants->mTable[constants->mCurrentTable]->_282;
        _404 = remaining - 1;
        ramp = static_cast<f32>(duration - remaining) / static_cast<f32>(duration);
    }
    target *= ramp * ramp;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit16)) || isStatusActive(17)) {
        constants = mActor->mConst;
        target *= constants->mTable[constants->mCurrentTable]->mTornadoMultiply;
    }
    s32 wasSquatting = static_cast<u32>(mMovementStates.mLowBits.mBit21);
    if (wasSquatting && (static_cast<u32>(mFlags20.mBits.mBit16))) {
        fn_803A6BE0();
    } else {
        mMovementStates.mLowBits.mBit21 = 0;
        if (_436 == 0 && _434 == 0 && checkSquat(false) && mSinkTimer <= 32 && !isStatusActive(31)) {
            if (!checkLockOnHoming()) mMovementStates.mLowBits.mBit21 = 1;
            if (!wasSquatting) {
                u32 flags = mMovementStates.mLowWord;
                if (((flags >> 21) & 1) && (((flags >> 23) & 1) || (static_cast<u32>(mMovementStates.mHighBits.mBit13)))) {
                    mTargetWalkSpeedIndex = 0;
                    mWalkSpeed = 0.0f;
                }
            }
        }
        if ((static_cast<u32>(mFlags20.mBits.mBit16)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit21)) && isAnimationRun("しゃがみ終了")) mMovementStates.mLowBits.mBit21 = 1;
        u32 flags = mMovementStates.mLowWord;
        if (!((flags >> 21) & 1) && wasSquatting) {
            mMovementStates.mLowWord = flags | 0x200000;
            cancelSquatMode();
            _71E = 0;
        }
    }
    if (_3D0 != 0) target = 0.0f;
    if (static_cast<u32>(mMovementStates.mLowBits.mBit15)) target = 0.0f;
    f32 inertia = decideInertia(target);
    if (sPreviousInertiaName != sInertiaName) sPreviousInertiaName = sInertiaName;
    if (mCometFireDanceArea && mTargetWalkSpeedIndex != 0) {
        inertia = 1.0f;
        target = inertia;
        mWalkSpeed = 1.5f;
    }
    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit21)) && (getPlayerMode() == 1 || mCometFireDanceArea)) {
        f32 speed = mWalkSpeed;
        if (speed >= 0.9999f) {
            constants = mActor->mConst;
            target *= constants->mTable[constants->mCurrentTable]->mCameraAngleLimit;
            if (target > speed) inertia = 0.99f;
            if (getPlayer()->mWalkSpeed >= 1.5f) getAnimator()->changeTrackAnim(2, "メタルダッシュ");
        } else getAnimator()->fn_803DEC10();
    }
    if (fn_803B1C10()) {
        target = Yoshi::fn_8040F180();
        inertia = 0.9f;
        cancelSquatMode();
    }
    if (isPlayerModeYoshi() && getAnimator()->isAnimationRun("基本")) {
        if (mActor->getYoshi()->mDashTimer > 0) {
            getAnimator()->changeTrackAnim(0, "ヨッシーダッシュ");
            getAnimator()->changeTrackAnim(1, "ヨッシーダッシュ");
            getAnimator()->changeTrackAnim(2, "ヨッシーダッシュ");
            this->_A70 = 1;
        } else if (this->_A70 != 0) {
            getAnimator()->changeTrackAnim(0, "ヨッシー鈍行");
            getAnimator()->changeTrackAnim(1, "ヨッシー歩行");
            getAnimator()->changeTrackAnim(2, "ヨッシーラン");
            this->_A70 = 0;
        }
    }
#ifdef GALAXY_NATIVE
    // OdysseyMovement (Mods::Mod::OdysseyMovement = 2): on normal ground, SMO's
    // run: cap 3 + 11*stick (14), accelerate 14/40, slow 14/120 above the cap,
    // brake 1.4 with the stick released. Crouching, sinking, tornado and the
    // special player modes keep Galaxy's speed.
    if (petari_mod_enabled(2) && getPlayerMode() == 0 && !((mMovementStates.mLowWord >> 21) & 1U) &&
        !((mMovementStates.mLowWord >> 16) & 1U) && mSinkTimer == 0 && !isStatusActive(17)) {
        const f32 unit = mActor->mConst->getTable()->mWalkSpeed;  // Galaxy's full run, units/frame
        // The stick's tilt itself, not Galaxy's target: that one follows a run-speed
        // index only its animator updates, and ramps up from 0 after a stop
        // (_404), so right after a mod move hands over (dive or long-jump landing,
        // roll end) it reads "no stick" and Mario glides standing while braking.
        // Galaxy's turn slip and brake turn (_3D0, _10) still stop the run.
        const f32 stick = (_3D0 == 0 && !((mMovementStates.mLowWord >> 15) & 1U)) ? MR::clamp(mStickPos.z, 0.0f, 1.0f) : 0.0f;
        _404 = 0;
        decideWalkSpeed();  // keep the run animation's index on the stick, so it runs instead of standing
        if (PetariNative::Odyssey::Live::getUpFrames > 0 && --PetariNative::Odyssey::Live::getUpFrames == 0) {
            // The dive landing's forward roll is done: Galaxy would hold its last
            // (standing) frame while Mario moves on; hand over to the run.
            stopAnimation("飛び込み失敗回転着地", "基本");
        }
        if (PetariNative::Odyssey::Live::capThrowBrake > 0) {
            --PetariNative::Odyssey::Live::capThrowBrake;
            mWalkSpeed *= 0.95f;
            return;
        }
        mWalkSpeed = PetariNative::Odyssey::stepRun(mWalkSpeed * unit, stick) / unit;
        return;
    }
#endif
    mWalkSpeed = mWalkSpeed * inertia + target * static_cast<f32>(256 - mSinkTimer) * 0.00390625f * (1.0f - inertia);
}

void Mario::decideOnIceAnimation() {
    if (mTargetWalkSpeedIndex == 0) {
        if (mWalkSpeed > 0.2f && !isAnimationRun("氷上慣性走行")) {
            changeAnimationWithAttr("氷上慣性走行", 1);
            mIceAnimFoot = 1 - mIceAnimFoot;
        }
    } else {
        decideWalkAnimation();
        if (mWalkSpeed > 0.7f) {
            switch (mIceAnimFoot) {
            case 0:
                getAnimator()->changeTrackAnim(2, "氷上力行左");
                break;
            default:
                getAnimator()->changeTrackAnim(2, "氷上力行右");
            }
        }
    }

    if (mTargetWalkSpeedIndex != 0 || mWalkSpeed <= 0.2f) {
        stopAnimation("氷上慣性走行");
    }
}

// NONMATCHING: equivalent early-exit branch orientation and register allocation.
void Mario::updateOnSand() {
    if (static_cast<u32>(mMovementStates.mLowBits.mBit0)) {
        return;
    }
    if (mCometFireDanceArea) {
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        if (_960 == 27 || _960 == 28) {
            if (!strcmp(MR::getSoundCodeString(_45C), "Sand") && mSinkTimer < 64) {
                mSinkTimer++;
            }
        } else if (isCurrentFloorSink()) {
            if (mSinkTimer < 255) {
                mSinkTimer++;
                if (_960 == 25 || _960 == 31) {
                    if (_960 == 31) {
                        if (mSinkTimer == 1) {
                            playSound("声沼沈み");
                        }

                        playSound("沼強制沈み");
                    } else {
                        if (mSinkTimer == 1) {
                            playSound("声砂沈み");
                        }

                        playSound("砂強制沈み");
                    }

                    stopWalk();
                    mSinkTimer = MR::clamp(static_cast< s32 >(mSinkTimer) + 3, 0, 255);

                    if (getAirGravityVec().dot(_368) > -0.99f) {
                        TVec3f vec1;
                        MR::vecKillElement(_368, getAirGravityVec(), &vec1);
                        TVec3f vec2;
                        vec2.cross(vec1, _368);
                        MR::normalize(&vec2);
                        vec1.cross(_368, vec2);
                        addVelocity(vec1 * 6.0f);
                    }
                } else {
                    playSound("砂沈み");
                }
            } else {
                mActor->fn_803BAFE0();
                return;
            }

            if (!isAnimationRun(nullptr)) {
                getAnimator()->changeTrackAnim(1, "埋まり歩行");
            }
        } else if (_960 == 32 || _A7F != 0) {
            if (mSinkTimer < 48) {
                mSinkTimer++;
            }
        } else {
            if (mSinkTimer != 0 && !isAnimationRun(nullptr)) {
                getAnimator()->changeTrackAnim(1, "歩行");
            }

            mSinkTimer = 0;
        }
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
        mSinkTimer = 0;
    }

    if (isStatusActive(6)) {
        mSinkTimer = 0;
    }
}

void Mario::updateOnPoison() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        if (_960 == 18) {
            if (mPoisonTimer == 0) {
                mActor->decLife(0);
                playSound("毒沼ダメージ");
                playSound("ダメージ");
                playSound("声小ダメージ");
                if (mActor->mHealth == 0) {
                    mActor->forceGameOver();
                }

                startCamVib(0);
                mActor->_BC4 = 1;
            }

            if (mPoisonTimer < 255) {
                mPoisonTimer++;
            } else {
                mPoisonTimer = 0;
            }
        } else {
            mPoisonTimer = 0;
        }
    } else if ((static_cast<u32>(mMovementStates.mLowBits.mBit31)) && _3BC > 10) {
        mPoisonTimer = 0;
    }

    if (isStatusActive(6)) {
        mPoisonTimer = 0;
    }
}

void Mario::updateOnWater() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        switch (_960) {
        case 20:
        case 21:
        case 22:
            touchWater();
            _738 = 20.0f;
            _73C = _368;

            switch (_962) {
            case 20:
                _738 += 20.0f;
            case 21:
                _738 += 20.0f;
            case 22:
                mDrawStates.mWord |= Mario::DrawStates::Underwater;
            }
        }

        if (_960 == 23 && _962 == 23) {
            touchWater();
            mDrawStates.mBits.mBit12 = 1;
            _738 = 3.0f;
            _73C = _368;
        }
    }

    switch (_962) {
    case 20:
    case 21:
    case 22:
    case 23:
        mDrawStates.mBits.mBit2 = 1;
    }
}

// NONMATCHING: animator speed setter selected by the walk-speed table
void Mario::fn_803AF2F0(const f32* pSpeedTable) {
    f32 var_f31;
    if (this->_3FA != 0) {
        var_f31 = 0.5f;
    } else {
        var_f31 = this->mActor->mConst->getTable()->mWeightBlendRatio;
    }
    this->getAnimator()->setBlendWeight(pSpeedTable, var_f31);
}

bool Mario::isWalling() const {
    return getCurrentStatus() == MarioStatus_Wall;
}

// NONMATCHING: independent stack-vector argument address setup order.
void Mario::checkWallStick() {
    if (getPlayerMode() == PlayerMode_Bee) {
        checkBeeStick();
        return;
    }

    if (!checkWallJumpCode() && isEnableStickWall()) {
        if (mActor->_334) {
            TVec3f horizontal;
            f32 vertical = MR::vecKillElement(mJumpVec, mActor->mAppliedGravityVec, &horizontal);
            horizontal.setLength(-0.25f);
            mJumpVec = mActor->mAppliedGravityVec * vertical + horizontal;
            return;
        }

        changeStatus(mWall);
        stopWalk();
        mMovementStates.mLowBits.mBit31 = 0;
        mMovementStates.mLowBits.mBit20 = 0;
        mMovementStates.mLowBits.mBit25 = 0;
        mMovementStates.mLowBits.mBit8 = 0;
        fixWallingPosition(true);
        resetTornado();
    }
}

bool Mario::isEnableStickWall() {
    switch (getPlayerMode()) {
    case PlayerMode_Teresa:
        return false;
    case PlayerMode_Foo:
        isStatusActive(MarioStatus_Foo);
        return false;
    case 0xA:
        return false;
    }

    if (mWall->isCancel()) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit31)) || (static_cast<u32>(mMovementStates.mLowBits.mBit30))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit16))) {
        return false;
    }

    if ((static_cast<u32>(mDrawStates.mBits.mBit1))) {
        return false;
    }

    if (!(static_cast<u32>(mMovementStates.mLowBits.mBit23)) && !(static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit29)) && mVerticalSpeed < mActor->getConst().getTable()->mWallStickGrHeight) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit10)) && _4E0 < mActor->getConst().getTable()->mWallStickFrHeight && !getWallPolygon()->mSensor->isType(0x51)) {
        return false;
    }

    if (isAnimationRun("空中ひねり") || isAnimationRun("雲スピン空中")) {
        if (mActor->_945 < 25) {
            return false;
        }
    } else if (mActor->isPunching() && mActor->_945 < 15) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit10)) && (static_cast<u32>(mMovementStates.mLowBits.mBit29)) && (static_cast<u32>(mMovementStates.mHighBits.mBit6))) {
        TVec3f shadowDelta = _4A4 - mShadowPos;
        const TVec3f& up = -*getGravityVec();
        const MarioConstTable* table = mActor->getConst().getTable();
        if (shadowDelta.dot(up) < table->mWallStickStepHeight) {
            return false;
        }
    }

    TVec3f horizontal;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        MR::vecKillElement(mJumpVec, mActor->mAppliedGravityVec, &horizontal);
        if (mBackWallTriangle->getNormal(0)->dot(horizontal) > 0.0f) {
            return false;
        }
    }

    if (isInhibitWall()) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && calcPolygonAngleD(mFrontWallTriangle) < 80.0f) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6)) && calcPolygonAngleD(mBackWallTriangle) < 80.0f) {
        return false;
    }

    TVec3f side = getWallNorm().cross(getAirGravityVec());
    if (MR::normalizeOrZero(&side)) {
        return false;
    }

    if (!checkWallCode("NotWallSlip", true)) {
        mMovementStates.mHighBits.mBit23 = 1;
    }

    if (checkWallCode("NoAction", true)) {
        return false;
    }

    if ((getPlayerMode() != PlayerMode_Ice || !getWallPolygon()->mSensor->isType(0x51)) && isRising()) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        if (!isAnimationRun("壁ジャンプ") && getPlayer()->checkStickWallSide() != 1) {
            return false;
        }
    } else if (!isAnimationRun("壁ジャンプ") && !(static_cast<u32>(mMovementStates.mLowBits.mBit22)) && getPlayer()->checkStickWallSide() != 1) {
        return false;
    }

    if ((static_cast<u32>(mMovementStates.mHighBits.mBit15))) {
        return false;
    }

    if (mActor->_468) {
        if (!checkTrgA()) {
            return false;
        }

        mWall->initTriangleJump();
        mWall->startJump();
        return false;
    }

    TVec3f position(mActor->mCapPosition);
    f32 distance = 120.0f;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6))) {
        distance = -120.0f;
    }

    if (!fn_803B2880(position, mFrontVec * distance)) {
        return false;
    }

    return MR::getKarikariClingNum() == 0;
}

u8 Mario::checkStickWallSide() {
    if (isStickOn()) {
        const TVec3f& direction = getWorldPadDir();
        if (direction.dot(getWallNorm()) < -0.5f) {
            return 1;
        }

        if (direction.dot(getWallNorm()) > 0.5f && !(static_cast<u32>(mMovementStates.mLowBits.mBit2))) {
            return 2;
        }
    }

    return 0;
}

s32 Mario::checkStickFrontBack() {
    if (isStickOn()) {
        const TVec3f& direction = getWorldPadDir();
        if (direction.dot(-mFrontVec) < -0.5f) {
            return 1;
        }

        if (direction.dot(-mFrontVec) > 0.5f && !(static_cast<u32>(mMovementStates.mLowBits.mBit2))) {
            return 2;
        }
    }

    return 0;
}

bool Mario::fixWallingPosition(bool immediate) {
    if (!fixWallingDist()) {
        return false;
    }

    fixWallingDir(immediate);
    return true;
}

void Mario::fixWallingDir(bool immediate) {
    bool back = true;
    if (isStatusActive(MarioStatus_SideStep)) {
        back = false;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6)) && back) {
        if (immediate) {
            setFrontVecKeepUp(-*mBackWallTriangle->getNormal(0));
            return;
        }

        setFrontVecKeepUp(-*mBackWallTriangle->getNormal(0), 0.1f);
        return;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        if (immediate) {
            setFrontVecKeepUp(-*mFrontWallTriangle->getNormal(0));
            return;
        }

        setFrontVecKeepUp(-*mFrontWallTriangle->getNormal(0), 0.1f);
    }
}

// NONMATCHING: stack allocation.
bool Mario::fixWallingDist() {
    u32 backWall;
    u32 frontWall;
    frontWall = (mMovementStates.mLowWord >> 23) & 1U;
    backWall = (mMovementStates.mLowWord >> 6) & 1U;
    if (backWall & frontWall) {
        return true;
    }

    if (backWall) {
        TVec3f position(_4F4 + *mBackWallTriangle->getNormal(0) * 79.0f);
        f32 distance = MR::vecKillElement(position - mPosHolder->mPos, *mBackWallTriangle->getNormal(0), &position);
        mPosHolder->fn_80406010(*mBackWallTriangle->getNormal(0) * distance);
        if (!fixWallingTop()) {
            return false;
        }
        mPosHolder->fn_80405F90(_4F4 + *mBackWallTriangle->getNormal(0) * 60.0f - _75C * 80.0f);

    } else if (frontWall) {
        if (!fixWallingTop()) {
            return false;
        }

        if (fn_803B2880(mPosHolder->mPos, mFrontVec * 100.0f) && mFrontVec.dot(*mFrontWallTriangle->getNormal(0)) < -0.999f) {
            TVec3f position(_4E8 + *mFrontWallTriangle->getNormal(0) * 60.0f - _75C * 80.0f);
            TVec3f correction;
            MR::vecKillElement(position - mPosHolder->mPos, *getGravityVec(), &correction);
            if (!MR::isNearZero(correction)) {
                TVec3f relative(correction + mActor->mBinderDirection5DC);
                if (MR::abs(relative.x) < MR::abs(correction.x)) {
                    correction.x = relative.x;
                }

                if (MR::abs(relative.y) < MR::abs(correction.y)) {
                    correction.y = relative.y;
                }

                if (MR::abs(relative.z) < MR::abs(correction.z)) {
                    correction.z = relative.z;
                }

                addTrans(correction);
            }
        }
    }

    return true;
}

bool Mario::fixWallingTop() {
    TVec3f side = getWallNorm().cross(getAirGravityVec());
    MR::normalizeOrZero(&side);
    if (MR::isNearZero(side)) {
        return false;
    }

    _75C.cross(getWallNorm(), side);
    MR::normalizeOrZero(&_75C);
    if (MR::isNearZero(_75C)) {
        return false;
    }

    getWallNorm().dot(getAirGravityVec());
    getPlayer()->forceSetHeadVecKeepSide(_75C);
    return true;
}

bool Mario::isInhibitWall() const {
    if ((static_cast<u32>(mDrawStates.mBits.mBit29))) {
        return true;
    }

    if (checkWallCode("NotWallJump", true)) {
        return true;
    }

    return checkWallCode("NoAction", true);
}

bool Mario::checkWallFloorCode(u16 code) const {
    if (((static_cast<u32>(mMovementStates.mLowBits.mBit23)) || (static_cast<u32>(mMovementStates.mHighBits.mBit13))) && _964[0] == code) {
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit6)) && _964[1] == code) {
        return true;
    }

    if ((static_cast<u32>(mMovementStates.mLowBits.mBit5)) && _964[2] == code) {
        return true;
    }

    return false;
}

bool Mario::checkWallCode(const char* pCode, bool frontBackOnly) const {
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mBackWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    if (((mMovementStates.mLowWord >> 23) & 1U) || ((mMovementStates.mHighWord >> 13) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mFrontWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    if (frontBackOnly) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        const char* wallCode = MR::getWallCodeString(mSideWallTriangle);
        if (wallCode && strcmp(wallCode, pCode) == 0) {
            return true;
        }
    }

    return false;
}

bool Mario::checkWallCodeNorm(u16 code, TVec3f* pNormal, bool frontBackOnly) const {
    if (((mMovementStates.mLowWord >> 6) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mBackWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mBackWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    if (((mMovementStates.mLowWord >> 23) & 1U) || ((mMovementStates.mHighWord >> 13) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mFrontWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mFrontWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    if (frontBackOnly) {
        return false;
    }

    if (((mMovementStates.mLowWord >> 5) & 1U)) {
        u16 wallCode = MR::getWallCodeIndex(mSideWallTriangle);
        if (wallCode == code) {
            if (pNormal != nullptr) {
                *pNormal = *mSideWallTriangle->getNormal(0);
            }

            return true;
        }
    }

    return false;
}

void Mario::tryWallPunch() {
    if (!(static_cast<u32>(mMovementStates.mHighBits.mBit20)) && !checkWallCodeNorm(8, nullptr, false) && (!isStatusActive(MarioStatus_Skate) || !checkWallFloorCode(0x21))) {
        if (isSwimming()) {
            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit23))) {
                mSwim->hitWall(*mFrontWallTriangle->getNormal(0), mFrontWallTriangle->mSensor);
            }

            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit6))) {
                mSwim->hitWall(*mBackWallTriangle->getNormal(0), mBackWallTriangle->mSensor);
            }

            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit5))) {
                mSwim->hitWall(*mSideWallTriangle->getNormal(0), mSideWallTriangle->mSensor);
            }
        } else {
            stopWalk();
            TVec3f velocity(getWallNorm() * mActor->getConst().getTable()->mWallSpinFlipGround);
            if ((static_cast<u32>(mMovementStates.mLowBits.mBit31))) {
                velocity.scale(mActor->getConst().getTable()->mWallSpinFlipAirRatio);
            }

            velocity -= *getGravityVec() * mActor->getConst().getTable()->mWallSpinHopGround;
            tryForcePowerJump(velocity, true);
            mMovementStates.mHighBits.mBit20 = 1;
            startPadVib(2);
            playSound("壁反射");
            playSound("声スピンキャンセル");
            playEffectTrans("壁ヒット", getWallPos());
            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit23))) {
                sendPunch(mFrontWallTriangle->mSensor, mFrontWallTriangle, true);
            }

            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit6))) {
                sendPunch(mBackWallTriangle->mSensor, mBackWallTriangle, true);
            }

            if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit5))) {
                sendPunch(mSideWallTriangle->mSensor, mSideWallTriangle, true);
            }

            mActor->_EF6 = 30;
        }
    }
}

void Mario::setWallCancel() {
    mWall->_1C = 1;
    mWall->_24 = getWallNorm();
}

void Mario::keepDistFrontWall() {
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        TVec3f horizontal;
        MR::vecKillElement(_4E8 - mGroundPos, *getGravityVec(), &horizontal);
        if (horizontal.length() < 80.0f) {
            push(*mFrontWallTriangle->getNormal(0) * (80.0f - horizontal.length()));
        }
    }
}

// namespace JGeometry

bool Mario::doObjWarp(LiveActor* pActor) {
    if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    WarpPod* pPod = MR::getWarpPodManager()->getPairPod(pActor);

    if (pPod != nullptr) {
        mWarp->_4C = pActor;

        mVelocity.zero();

        stopWalk();

        mActor->setBlendMtxTimer(16);

        TVec3f axisY;
        TVec3f pairAxisY;

        MR::calcActorAxisY(&axisY, pActor);
        MR::calcActorAxisY(&pairAxisY, pPod);

        mWarp->_14 = pPod->mPosition + pairAxisY * 160.0f;
        mWarp->_20 = pActor->mPosition + axisY * 160.0f;

        MR::getRotatedAxisZ(&mWarp->_38, pPod->mRotation);
        MR::getRotatedAxisY(&mWarp->_2C, pPod->mRotation);

        mWarp->_45 = pPod->mVisibilityState;
        mWarp->_44 = pPod->mArg2;

        stopWalk();

        changeStatus(mWarp);

        stopJump();

        return true;
    }

    return false;
}

void Mario::doCubeWarp() {
    if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return;
    }

    WarpCube* pWarpCube = static_cast< WarpCube* >(MR::getAreaObj("WarpCube", mActor->_240));
    WarpCubeMgr* pWarpCubeMgr = static_cast< WarpCubeMgr* >(MR::getAreaObjManager("WarpCube"));

    if (pWarpCube != nullptr) {
        if (_56C == pWarpCube) {
            return;
        }

        WarpCube* pPairCube = pWarpCubeMgr->getPairCube(pWarpCube);
        if (pPairCube == nullptr) {
            return;
        }

        pWarpCubeMgr->startEventCamera(pWarpCube);

        TVec3f cubePos;
        MR::calcCubePos(pPairCube, &cubePos);

        mVelocity.zero();
        stopWalk();
        mActor->_38C = 10;

        _56C = pPairCube;
        mWarp->_14 = cubePos;

        MR::calcCubePos(pWarpCube, &cubePos);
        mWarp->_20 = cubePos;
        mWarp->_38.zero();
        mWarp->_2C.zero();
        mWarp->_45 = 0;
        mWarp->_44 = 0;

        stopWalk();

        changeStatus(mWarp);

        stopJump();

        return;
    }

    if (_56C != nullptr) {
        pWarpCubeMgr->setInvalidateTimer(_56C, 300);
    }

    _56C = nullptr;
}

bool Mario::doPointWarp(const TVec3f& rVec1, const TVec3f& rVec2, s32 myInt) {
    if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    mVelocity.zero();

    stopWalk();

    mActor->setBlendMtxTimer(16);

    mWarp->_14 = rVec1;
    mWarp->_20 = mPosHolder->mPos;
    mWarp->_38 = rVec2;
    mWarp->_45 = 2;
    mWarp->_44 = false;

    if (myInt != 0) {
        mWarp->_48 = myInt;
    } else {
        mWarp->_48 = 90;
    }

    changeStatus(mWarp);

    stopJump();

    return true;
}

bool Mario::doPointWarpRecovery(const TVec3f& rVec1, const TVec3f& rVec2) {
    if (((getPlayer()->getMovementStates().mLowWord & Mario::MovementStates::DebugMode) >> 9)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    if (isStatusActive(MarioStatus_Warp)) {
        return false;
    }

    mVelocity.zero();

    stopWalk();

    mActor->setBlendMtxTimer(16);

    mWarp->_14 = rVec1;
    mWarp->_20 = mPosHolder->mPos;
    mWarp->_38 = rVec2;
    mWarp->_2C.zero();
    mWarp->_45 = 3;
    mWarp->_44 = false;

    changeStatus(mWarp);

    stopJump();

    MR::startGlobalEventCameraNoTarget("引き戻し");

    return true;
}

bool Mario::isVisibleRecoveryWarpBubble() const {
    if (isStatusActive(MarioStatus_Warp) && mWarp->_45 == 3) {
        return true;
    }

    return false;
}

// NONMATCHING: flag lowering.
void Mario::fn_803B0E60() {
    if (this->isPlayerModeYoshi() && YoshiParam::fn_8041BD60() != 0) {
        if (this->mActor->fn_803B9810()->fn_8041F910() != 0 && this->checkTrgA() && this->checkLvlZ()) {
            this->fn_803B0EF0();
            this->mStateFlags._1 = true;
        }
    }
}

// NONMATCHING: temporary/register allocation.
void Mario::fn_803B0EF0() {
    if (this->isPlayerModeYoshi()) {
        this->mMorphResetTimer = 0;
        this->getPlayer()->mMovementStates.mHighBits.mBit30 = 1;
        this->getPlayer()->mMovementStates.mLowBits.mBit21 = 0;
        this->mActor->mMarioAnim->fn_803DF9D0();
        this->mActor->fn_803B9810()->fn_8041FD00();
        const TVec3f* gravity = this->getGravityVec();
        TVec3f sp68;
        JMAVECScaleAdd(gravity, &this->mVelocity, &sp68, -gravity->dot(this->mVelocity));
        TVec3f sp5C;
        PSVECCrossProduct(this->getGravityVec(), &this->mSideVec, &sp5C);
        if (!MR::normalizeOrZero(&sp5C)) {
            sp5C *= YoshiParam::fn_8041BD90();
        }
        MarioActor* actor = this->mActor;
        actor->getYoshi()->fn_8040CCD0(sp68 + sp5C + *this->getGravityVec() * 20.0f);
        f32 t31 = YoshiParam::fn_8041BD80();
        TVec3f launch = sp68 - sp5C - *this->getGravityVec() * t31;
        this->mMovementStates.mHighBits.mBit18 = 1;
        this->_304 = launch;
        this->changeAnimation("ヨッシー離脱ジャンプ", "落下");
        this->mActor->setPlayerMode(0, true, true);
        TVec3f jointPosition;
        MR::copyJointPos(this->mActor->getYoshi(), "MarioPos", &jointPosition);
        MR::setPlayerPos(jointPosition);
    }
}

// NONMATCHING: yoshi dismount/landing helpers, rough from the draft (vector math simplified)
void Mario::fn_803B1100() {
    if (this->isPlayerModeYoshi()) {
        this->mMorphResetTimer = 0;
        this->mMovementStates.mHighBits.mBit30 = 1;
        this->mActor->mMarioAnim->fn_803DF9D0();
        this->mActor->fn_803B9810()->fn_8041FD80();
        TVec3f position;
        MR::copyJointPos(this->mActor->getYoshi(), "MarioPos", &position);
        MR::setPlayerPos(position);
    }
}

// NONMATCHING: yoshi jump-release landing approximated
void Mario::fn_803B1180() {
    if (this->isPlayerModeYoshi()) {
        this->mActor->fn_803B9810()->fn_8041FD00();
        const TVec3f* gravity = this->getGravityVec();
        TVec3f sp44;
        JMAVECScaleAdd(gravity, &this->mVelocity, &sp44, -gravity->dot(this->mVelocity));
        TVec3f sp38;
        PSVECCrossProduct(this->getGravityVec(), &this->mSideVec, &sp38);
        if (!MR::normalizeOrZero(&sp38)) {
            sp38 *= YoshiParam::fn_8041BD90();
        }
        MarioActor* actor = this->mActor;
        actor->getYoshi()->fn_8040CCD0(sp44 + sp38 + *this->getGravityVec() * 20.0f);
        this->mActor->mMarioAnim->fn_803DF9D0();
        TVec3f position;
        MR::copyJointPos(this->mActor->getYoshi(), "MarioPos", &position);
        MR::setPlayerPos(position);
    }
}

// NONMATCHING: saved-register and stack allocation, pooled animation strings.
void Mario::fn_803B12B0() {
    if (static_cast<u32>(this->mMovementStates.mLowBits.mBit20)) {
        this->mFlags14.mHighBits.mBit21 = 0;
        return;
    }
    if (!this->isPlayerModeYoshi()) {
        this->mFlags14.mHighBits.mBit21 = 0;
        if (static_cast<u32>(this->mMovementStates.mLowBits.mBit30)) {
            this->changeAnimation("基本", (const char*)nullptr);
            return;
        }
        this->changeAnimation("落下", (const char*)nullptr);
        return;
    }
    if (static_cast<u32>(this->mMovementStates.mLowBits.mBit30)) {
        this->mFlags14.mHighBits.mBit21 = 0;
        this->changeAnimation("基本", (const char*)nullptr);
        return;
    }
    if (!this->isAnimationRun("踏ん張り")) {
        this->changeAnimation("踏ん張り", (const char*)nullptr);
    }
    const TVec3f& gravity = this->mActor->mAppliedGravityVec;
    TVec3f lateral;
    lateral.killElement(this->mJumpVec, gravity);
    this->mJumpVec -= lateral;
    this->mJumpVec *= 0.95f * this->mActor->fn_803B9A90();
    this->mJumpVec += lateral;
    MarioActor* actor = mActor;
    f32 maxRiseSpeed = YoshiParam::fn_8041BDB0();
    if (actor->fn_803B9A90() < maxRiseSpeed) {
        actor = mActor;
        f32 maxSpeed = YoshiParam::fn_8041BDB0();
        f32 riseSpeed = actor->fn_803B9A90();
        f32 acceleration;
        f32 ratio = (YoshiParam::fn_8041BDB0() - riseSpeed) / maxSpeed;
        acceleration = YoshiParam::fn_8041BDA0();
        this->mJumpVec += -this->mActor->mAppliedGravityVec * ratio * acceleration;
    }
    this->mActor->fn_803B9A50();
    if (this->mActor->fn_803B9A70() == 0 || !MR::testCorePadButtonA(0)) {
        this->fn_803B14E0();
    }
}

void Mario::fn_803B14E0() {
    this->mActor->fn_803B9A30();
    this->mFlags14.mHighBits.mBit21 = 0;
    this->mActor->fn_803B9830()->fn_8041BB90();
    this->changeAnimation("落下", (const char*)nullptr);
}

s32 Mario::fn_803B1540() {
    if (!this->mActor->fn_803B9840()) {
        return 0;
    }
    if (this->mActor->fn_803B9830()->fn_8041BBD0()) {
        return 0;
    }
    if (!this->isPlayerModeYoshi()) {
        return 0;
    }
    if (static_cast<u32>(this->mFlags14.mHighBits.mBit21)) {
        return 0;
    }
    if (static_cast<u32>(this->mMovementStates.mLowBits.mBit30)) {
        return 0;
    }
    if (static_cast<u32>(this->mMovementStates.mLowBits.mBit21)) {
        return 0;
    }
    if (this->_430 - 0xBU <= 2U || this->_430 == 9) {
        if (this->isRising()) {
            this->mMovementStates.mLowBits.mBit12 = 1;
            return 0;
        }
    }
    if (MR::isNearZero(this->mActor->mAppliedGravityVec, 0.001f)) {
        return 0;
    }
    this->fn_8039A600();
    this->resetTornado();
    this->cancelTornadoJump();
    this->mDrawStates.mBits.mBit23 = 1;
    this->mMovementStates.mHighBits.mBit30 = 0;
    this->mFlags14.mHighWord |= 0x300000;
    this->mActor->fn_803B9A10();
    if (this->_430 == 4) {
        this->setFrontVecKeepUp(-this->_220);
        this->_430 = 0;
    }
    if (this->isAnimationRun("カリカリ限界")) {
        this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
    }
    this->changeAnimation("踏ん張り", (const char*)nullptr);
    this->mMovementStates.mHighWord &= 0xFFFFFFFC;
    return 1;
}

void Mario::fn_803B1710() {
    this->mFlags14.mHighWord &= 0xFFCFFFFF;
}

void Mario::fn_803B1720() {
    if (this->getCurrentStatus() != 0x26 && (this->mMovementStates.mLowWord >> 30 & 1)) {
        this->changeStatus(this->mYoshiPull);
        this->stopWalk();
    }
}

void Mario::fn_803B1780() {
    if (this->getCurrentStatus() == 0x26) {
        this->mYoshiPull->fn_804090C0();
    }
}

void Mario::fn_803B17C0() {
    if (this->getCurrentStatus() == 0x26) {
        this->closeStatus(this->mYoshiPull);
    }
}

void Mario::fn_803B1800() {
    if (this->getCurrentStatus() == 0x27) {
        this->mYoshiBalloon->mAir = 1.0f;
        return;
    }
    this->changeStatus(this->mYoshiBalloon);
}

bool Mario::fn_803B1850() const {
    return this->getCurrentStatus() == 0x27;
}

void Mario::fn_803B1880() {
    if (this->getCurrentStatus() != 0x28) {
        this->changeStatus(this->mYoshiShot);
    }
}

bool Mario::fn_803B18C0() {
    if (fn_803B1B70()) return false;
    bool actorBusy;
    {
        MarioActor* actor = mActor;
        if (actor->_EA4) return false;
        actorBusy = false;
        if (actor->_482 || actor->_481) actorBusy = true;
    }
    if (actorBusy) return false;
    if (getStickP() != 0.0f || checkLvlA() || checkLvlZ() || mActor->isRequestRush()) return false;
    if ((static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit31)) || !(static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit30))) return false;
    if (getPlayer()->isStatusActive(6)) return false;
    if (getPlayer()->isStatusActive(5)) return false;
    if (getPlayer()->isStatusActive(0x1A)) return false;
    if (getPlayer()->isStatusActive(0x1B)) return false;
    if (getPlayer()->isStatusActive(0x12)) return false;
    if (getPlayer()->isStatusActive(0x11)) return false;
    if (getPlayer()->isStatusActive(0x22)) return false;
    if (static_cast<u32>(getPlayer()->mFlags20.mBits.mBit26)) return false;
    if (static_cast<u32>(getPlayer()->mMovementStates.mHighBits.mBit28)) return false;
    if (static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit21)) return false;
    if (mTargetWalkSpeedIndex != 0) return false;
    changeStatus(mYoshiDash);
    return true;
}

void Mario::fn_803B1B20() {
    if (this->fn_803B1B70()) {
        this->closeStatus(this->mYoshiDash);
    }
}

bool Mario::fn_803B1B70() const {
    return this->getCurrentStatus() == 0x29;
}

// Ordinary jump callers establish modes 0..6 or8 before this query.
f32 Mario::getJumpHeight() {
    if (this->isPlayerModeCloud() && this->_430 == 0) {
        return 24.0f;
    }
    return this->mActor->mConst->getTable()->mJumpHeight[this->_430];
}

// NONMATCHING: the target keeps the Yoshi flag in a register variable (inlined bool helper); 64%
bool Mario::fn_803B1C10() {
    bool ret = false;
    bool yoshi = false;
    if (this->isPlayerModeYoshi() && FLD<s32>(this->mActor->getYoshi(), offsetof(Yoshi, mDashTimer)) > 0) {
        yoshi = true;
    }
    if (yoshi && this->isAnimationRun("基本")) {
        ret = true;
    }
    return ret;
}

f32 Mario::fn_803B1CA0() {
    if (this->fn_803B1D60()) {
        return YoshiParam::fn_8041BDD0();
    }
    return this->mActor->mConst->getTable()->mJumpFrontSpeed;
}

f32 Mario::fn_803B1D00() {
    if (this->fn_803B1D60()) {
        return YoshiParam::fn_8041BDE0();
    }
    return this->mActor->mConst->getTable()->mJumpFrontReduction;
}

bool Mario::fn_803B1D60() {
    bool ret = false;
    bool yoshi = false;
    if (this->isPlayerModeYoshi() && FLD<s32>(this->mActor->getYoshi(), offsetof(Yoshi, mDashTimer)) > 0) {
        yoshi = true;
    }
    if (yoshi && !(this->getPlayer()->mFlags14.mHighWord >> 21 & 1)) {
        ret = true;
    }
    return ret;
}

// NONMATCHING: head-vector collision/normal rotation approximated from the draft
// NONMATCHING: independent FP load/register allocation.
bool Mario::fn_803B1DE0(TVec3f* up) {
    TVec3f head(mHeadVec);
    MR::normalizeOrZero(&head);
    TVec3f direction(*up);
    MR::normalizeOrZero(&direction);
    if (mHeadVec.dot(*up) > -0.9f) {
        TVec3f axis;
        PSVECCrossProduct(&mHeadVec, up, &axis);
        if (MR::isNearZero(axis, 0.001f)) {
            f32 front = MR::abs(up->dot(mFrontVec));
            if (MR::abs(up->dot(mSideVec)) < front) return false;
        } else {
            f32 side = MR::abs(axis.dot(mSideVec));
            if (MR::abs(axis.dot(mFrontVec)) < side) return false;
        }
    }
    return true;
}

void Mario::fn_803B1EF0(TVec3f* up, f32 rate) {
    TVec3f side;
    PSVECCrossProduct(up, &mFrontVec, &side);
    if (MR::normalizeOrZero(&side)) {
        side.set(mSideVec);
    }
    TVec3f front;
    PSVECCrossProduct(&side, up, &front);
    MR::normalizeOrZero(&front);
    PSVECCrossProduct(up, &front, &side);

    MarioRotationMatrix current;
    fn_803B2170(&current, mSideVec);
    fn_803B2190(&current, mHeadVec);
    fn_803B21B0(&current, mFrontVec);
    TQuat4f currentQuat;
    fn_803B21D0(&current, currentQuat);
    MarioRotationMatrix next;
    fn_803B2170(&next, side);
    fn_803B2190(&next, *up);
    fn_803B21B0(&next, front);
    TQuat4f nextQuat;
    fn_803B21D0(&next, nextQuat);
    TQuat4f blended;
    blended.x = currentQuat.x;
    blended.y = currentQuat.y;
    blended.z = currentQuat.z;
    blended.w = currentQuat.w;
    blended.slerp(nextQuat, rate);
    TVec3f newSide, newUp, newFront;
    blended.getXDir(newSide);
    blended.getYDir(newUp);
    blended.getZDir(newFront);
    mSideVec = newSide;
    MR::normalize(&mSideVec);
    mHeadVec = newUp;
    MR::normalize(&mHeadVec);
    setFrontVec(newFront);
}

// Column setters of a 3x3 rotation matrix.
void fn_803B2170(MarioRotationMatrix* pMtx, const TVec3f& rVec) {
    pMtx->mMtx[0][0] = rVec.x;
    pMtx->mMtx[1][0] = rVec.y;
    pMtx->mMtx[2][0] = rVec.z;
}

void fn_803B2190(MarioRotationMatrix* pMtx, const TVec3f& rVec) {
    pMtx->mMtx[0][1] = rVec.x;
    pMtx->mMtx[1][1] = rVec.y;
    pMtx->mMtx[2][1] = rVec.z;
}

void fn_803B21B0(MarioRotationMatrix* pMtx, const TVec3f& rVec) {
    pMtx->mMtx[0][2] = rVec.x;
    pMtx->mMtx[1][2] = rVec.y;
    pMtx->mMtx[2][2] = rVec.z;
}

// NONMATCHING: scalar literal symbol identities; instruction words match after relocation masking.
void fn_803B21D0(const MarioRotationMatrix* matrix, TQuat4f& rDest) {
    if ((*matrix)(0, 0) + (*matrix)(1, 1) + (*matrix)(2, 2) >= 0.0f) {
        f32 root = (*matrix)(0, 0) + (*matrix)(1, 1) + (*matrix)(2, 2);
        root += 1.0f;
        root = JGeometry::TUtil< f32 >::sqrt(root);
        f32 scale = 0.5f / root;
        rDest.w = 0.5f * root;
        rDest.x = scale * ((*matrix)(2, 1) - (*matrix)(1, 2));
        rDest.y = scale * ((*matrix)(0, 2) - (*matrix)(2, 0));
        rDest.z = scale * ((*matrix)(1, 0) - (*matrix)(0, 1));
    } else {
        f32 maximum = fn_80386D30((*matrix)(0, 0), (*matrix)(1, 1), (*matrix)(2, 2));

        if (maximum == (*matrix)(0, 0)) {
            f32 root = JGeometry::TUtil< f32 >::sqrt(1.0f + ((*matrix)(0, 0) - ((*matrix)(1, 1) + (*matrix)(2, 2))));
            f32 scale = 0.5f / root;
            rDest.x = 0.5f * root;
            rDest.y = scale * ((*matrix)(0, 1) + (*matrix)(1, 0));
            rDest.z = scale * ((*matrix)(2, 0) + (*matrix)(0, 2));
            rDest.w = scale * ((*matrix)(2, 1) - (*matrix)(1, 2));
        } else if (maximum == (*matrix)(1, 1)) {
            f32 root = JGeometry::TUtil< f32 >::sqrt(1.0f + ((*matrix)(1, 1) - ((*matrix)(2, 2) + (*matrix)(0, 0))));
            f32 scale = 0.5f / root;
            rDest.y = 0.5f * root;
            rDest.z = scale * ((*matrix)(1, 2) + (*matrix)(2, 1));
            rDest.x = scale * ((*matrix)(0, 1) + (*matrix)(1, 0));
            rDest.w = scale * ((*matrix)(0, 2) - (*matrix)(2, 0));
        } else {
            f32 root = JGeometry::TUtil< f32 >::sqrt(1.0f + ((*matrix)(2, 2) - ((*matrix)(0, 0) + (*matrix)(1, 1))));
            f32 scale = 0.5f / root;
            rDest.z = 0.5f * root;
            rDest.x = scale * ((*matrix)(2, 0) + (*matrix)(0, 2));
            rDest.y = scale * ((*matrix)(1, 2) + (*matrix)(2, 1));
            rDest.w = scale * ((*matrix)(1, 0) - (*matrix)(0, 1));
        }
    }
}

void Mario::fn_803B23E0(const TVec3f* direction, const TVec3f* axis, f32 rate) {
    TVec3f blended;
    if (!MR::vecBlendSphere(&blended, mHeadVec, *direction, rate)) {
        MR::vecRotAxis(mHeadVec, *direction, *axis, &blended, rate);
    }
    mHeadVec = blended;
    MR::normalize(&mHeadVec);
}

bool Mario::fn_803B2470() {
    if (!_60D && mActor->mAxisRotationDisabled6A0) return false;
    if (!(static_cast<u32>(mDrawStates.mBits.mBit18))) return false;
    TVec3f difference(mHeadVec);
    difference -= _290;
    if (MR::isNearZero(difference, 0.001f)) return false;
    TVec3f sum(mHeadVec);
    sum += _290;
    return !MR::isNearZero(sum, 0.001f);
}

s32 Mario::fn_803B2530(bool squat) {
    if (fn_803EF7C0() == 0x20 || static_cast<u16>(getPlayerMode()) == 4 || isPlayerModeYoshi() || _430 == 3) return 0;
    if (squat && mWalkSpeed > 0.3f && !(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
        trySquatJump();
        return 1;
    }
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit21)) && !(static_cast<u32>(mMovementStates.mHighBits.mBit28))) {
        tryBackJump();
        return 1;
    }
    return 0;
}

void Mario::fn_803B2610() {
    mCometNoSpinArea = MR::isInAreaObj("CometNoSpinArea", getTrans());
    mCometFireDanceArea = MR::isInAreaObj("CometFireDanceArea", getTrans());
    mCometRainbowArea = MR::isInAreaObj("CometRainbowArea", getTrans()) || MR::isGalaxyRainbowCometAppearInCurrentStage();
    mCometHopperArea = MR::isInAreaObj("CometHopperArea", getTrans());
}

bool Mario::fn_803B26C0() {
    if (this->isPlayerModeHopper() && this->mCometHopperApplied != 0) {
        return true;
    }
    if (this->isPlayerModeInvincible() && this->mCometRainbowApplied != 0) {
        return true;
    }
    return false;
}

bool Mario::fn_803B2730() {
    return strcmp(MR::getSoundCodeString(this->_45C), "Sand") == 0;
}

bool Mario::fn_803B2770(TVec3f* pOut, Triangle* pTri, const TVec3f& rStart, const TVec3f& rDir) {
    if (fn_803B2950()) {
        return MR::getFirstPolyOnLineToDashYoshiStepAndMap(pOut, pTri, rStart, rDir);
    }
    return MR::getFirstPolyOnLineToMap(pOut, pTri, rStart, rDir);
}

bool Mario::fn_803B27F0(TVec3f* pOut, Triangle* pTri, const TVec3f& rStart, const TVec3f& rDir, const CollisionPartsFilterBase* pPartsFilter, const TriangleFilterBase* pTriFilter) {
    if (fn_803B2950()) {
        return MR::getFirstPolyOnLineToDashYoshiStepAndMap(pOut, pTri, rStart, rDir, pPartsFilter, pTriFilter);
    }
    return MR::getFirstPolyOnLineToMap(pOut, pTri, rStart, rDir, pPartsFilter, pTriFilter);
}

bool Mario::fn_803B2880(const TVec3f& rStart, const TVec3f& rEnd) {
    if (fn_803B2950()) {
        return MR::isExistDashYoshiStepAndMapCollision(rStart, rEnd);
    }
    return MR::isExistMapCollision(rStart, rEnd);
}

bool Mario::fn_803B28E0(const TVec3f& rStart, const TVec3f& rEnd, const LiveActor* pActor) {
    if (fn_803B2950()) {
        return MR::isExistDashYoshiStepAndMapCollisionExceptActor(rStart, rEnd, pActor);
    }
    return MR::isExistMapCollisionExceptActor(rStart, rEnd, pActor);
}

s32 Mario::fn_803B2950() const {
    s32 var_r31 = 0;
    if (this->isPlayerModeYoshi() && this->mActor->getYoshi()->mDashTimer > 0) {
        var_r31 = 1;
    }
    return var_r31;
}

s32 Mario::fn_803B29B0(const TVec3f& rPos, f32 radius) {
    if (fn_803B2950()) {
        return Collision::checkStrikeBallToDashYoshiStepAndMap(rPos, radius, nullptr, nullptr);
    }
    return Collision::checkStrikeBallToMap(rPos, radius, nullptr, nullptr);
}

s32 Mario::fn_803B2A20(const TVec3f& rPos, f32 radius, f32 thickness) {
    if (fn_803B2950()) {
        return Collision::checkStrikeBallToDashYoshiStepAndMapWithThickness(rPos, radius, thickness, nullptr, nullptr);
    }
    return Collision::checkStrikeBallToMapWithThickness(rPos, radius, thickness, nullptr, nullptr);
}

const HitInfo* Mario::fn_803B2AA0(u32 idx) {
    if (fn_803B2950()) {
        return Collision::getStrikeInfoDashYoshiStepAndMap(idx);
    }
    return Collision::getStrikeInfoMap(idx);
}

bool Mario::fn_803B2AF0(const TVec3f& rPos, HitInfo* pInfo) {
    if (fn_803B2950()) {
        return MR::checkStrikePointToDashYoshiStepAndMap(rPos, pInfo);
    }
    return MR::checkStrikePointToMap(rPos, pInfo);
}

bool Mario::fn_803B2B50(const TVec3f& rStart, const TVec3f& rEnd, TVec3f* pOut, Triangle* pTri) {
    if (fn_803B2950()) {
        return MR::getFirstPolyOnLineBFastToDashYoshiStepAndMap(rStart, rEnd, pOut, pTri);
    }
    return MR::getFirstPolyOnLineBFast(rStart, rEnd, pOut, pTri);
}

// NONMATCHING: argument setup instruction order.
s32 Mario::fn_803B2BD0() {
    MR::isNearZero(mAirGravityVec, 0.001f);
    TVec3f up(mAirGravityVec);
    if (MR::normalizeOrZero(&up)) return 1;
    up = -up;
    TVec3f front;
    PSVECCrossProduct(&_290, &up, &front);
    if (MR::normalizeOrZero(&front)) return 1;
    TVec3f side;
    PSVECCrossProduct(&up, &front, &side);
    if (MR::normalizeOrZero(&side)) return 1;
    _290 = side;
    mSideVec = _290;
    MR::normalize(&mSideVec);
    return 0;
}

// NONMATCHING: literal symbol identities; instruction words match after relocation masking.
f32 Mario::fn_803B2CC0(TVec3f* arg1) {
    if ((static_cast<u32>(mFlags14.mLowBits.mBit12)) && (static_cast<u32>(mMovementStates.mLowBits.mBit20))) {
        return 1.0f;
    }
    if (arg1->dot(this->mHeadVec) < 0.0f) {
        u16 t = this->mActor->_334;
        if (t != 0) {
            f32 t3 = (f32)t / 15.0f;
            return t3 * mActor->mConst->getTable()->mRotateHeadVecSpeedByGravityM + (1.0f - t3) * mActor->mConst->getTable()->mRotateHeadVecSpeedByGravityL;
        }
        return mActor->mConst->getTable()->mRotateHeadVecSpeedByGravityL;
    }
    if (arg1->dot(this->mHeadVec) < 0.99f) {
        return mActor->mConst->getTable()->mRotateHeadVecSpeedByGravityM;
    }
    if (this->mTargetWalkSpeedIndex <= 2) {
        return mActor->mConst->getTable()->mRotateHeadVecSpeedByGravityS;
    }
    return 1.0f;
}

bool Mario::fn_803B2E10(const Triangle* pTri) {
    bool ret = false;
    if (MR::isSensorPressObj(pTri->mSensor) || MR::getFloorCodeIndex(pTri) == 29 || MR::getFloorCodeIndex(pTri) == 39) {
        ret = true;
    }
    return ret;
}

s32 Mario::fn_803B2E80(TVec3f* direction) {
    if (PSVECMag(direction) >= 80.0f) return 0;
    s32 result = 0;
    if (direction->dot(*getGravityVec()) > 0.0f && direction->dot(mFrontVec) > 0.707f) result = 1;
    return result;
}

void Mario::fn_803B2F20(const HitInfo* hit) {
    if (_4D8->isValid()) {
        TVec3f previous(_50C - mPosHolder->mPos);
        MR::vecKillElement(previous, *getGravityVec(), &previous);
        TVec3f candidate(hit->mHitPos - mPosHolder->mPos);
        MR::vecKillElement(candidate, *getGravityVec(), &candidate);
        f32 previousDistance = previous.squared();
        if (candidate.squared() > previousDistance) return;
    }
    *_4D8 = hit->mParentTriangle;
    _50C = hit->mHitPos;
}

void Mario::fn_803B3010() {
    this->mMovementStates.mLowBits.mBit10 = 0;
    this->mMovementStates.mHighBits.mBit6 = 0;
    this->_47C->mIdx = -1;
    this->_4E0 = 0.0f;
}

// NONMATCHING: register allocation / constant pool labels
bool Mario::fn_803B3040(TVec3f* pOutPos, Triangle* pOutTri) {
    TVec3f start;
    TVec3f dir;
    bool hit;
    if ((static_cast<u32>(this->mMovementStates.mLowBits.mBit23)) && this->mFrontWallTriangle != nullptr) {
        pOutPos->set(this->_4E8);
        start = this->mActor->_240;
        *pOutTri = *this->mFrontWallTriangle;
        hit = true;
    } else {
        if (this->isPlayerModeYoshi()) {
            TVec3f up(this->mFrontVec);
            up.scale(115.0f);
            dir = up;
        } else if ((static_cast<u32>(this->mMovementStates.mLowBits.mBit16)) && !MR::isNearZero(this->_328.dot(*this->getGravityVec()), 0.001f)) {
            MR::vecKillElement(this->_328, *this->getGravityVec(), &dir);
            dir.setLength(200.0f);
        } else {
            TVec3f up(this->mFrontVec);
            up.scale(100.0f);
            dir = up;
        }
        TVec3f origin(this->mPosHolder->mPos - *this->getGravityVec() * 8.0f);
        start = origin;
        hit = this->fn_803B2770(pOutPos, pOutTri, start, dir);
    }
    if (!hit) {
        return false;
    }
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    if (MR::abs(this->mAirGravityVec.dot(*pOutTri->getNormal(0))) >= 0.1f) {
        return false;
    }
    this->_4E4 = (*pOutPos - start).dot(this->mFrontVec);
    return true;
}

// NONMATCHING: register allocation / constant pool labels
void Mario::fn_803B3280(const TVec3f& rPos, Triangle* pTri) {
    Triangle tri;
    TVec3f hit;
    TVec3f start = rPos - getAirGravityVec() * 200.0f + mFrontVec * 20.0f;
    TVec3f direction(getAirGravityVec() * 210.0f);
    if (!fn_803B2B50(start, direction, &hit, &tri)) {
        this->_4E0 = 400.0f;
        return;
    }
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    if (this->mAirGravityVec.dot(*tri.getNormal(0)) > -0.9f) {
        this->_4E0 = 400.0f;
        return;
    }
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    this->_4E0 = (rPos - hit).dot(this->mAirGravityVec);
    if (this->_4E0 > 0.0f) {
        this->mMovementStates.mLowBits.mBit10 = 1;
    }
    f32 offset = 0.0f;
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    f32 angle = this->marioAcos(-this->mAirGravityVec.dot(*pTri->getNormal(0)));
    if (angle < 1.5707964f && angle != 0.0f) {
        offset = this->_4E0 / (JMASinRadian(angle) / JMACosRadian(angle));
    }
    TVec3f savedHit(hit);
    hit -= mFrontVec * (15.0f + offset);
    if (!fn_803B2880(hit - getAirGravityVec() * 5.0f, *getGravityVec() * 10.0f)) {
        hit.set(savedHit);
        *this->_47C = tri;
        return;
    }
    this->_4A4 = hit;
    if (!(static_cast<u32>(this->mMovementStates.mLowBits.mBit30)) || this->calcAngleD(this->_368) < 25.0f) {
        this->mMovementStates.mHighBits.mBit6 = 1;
    }
    *this->_47C = tri;
}

// NONMATCHING: float register allocation / constant pool labels
void Mario::fn_803B35D0(const TVec3f& rPos, u32 count) {
    f32 minAngle = 180.0f;
    f32 maxAngle = 0.0f;
    f32 ref = 0.707f;
    u32 minIdx = 0;
    u32 maxIdx = 0;
    if (count != 0) {
        for (u32 i = 0; i < count; i++) {
            const HitInfo* pInfo = this->fn_803B2AA0(i);
            if ((static_cast<u32>(this->mMovementStates.mHighBits.mBit8)) && __fabs(pInfo->mParentTriangle.getNormal(0)->dot(this->mSwimSideVec694)) > ref) {
                continue;
            }
            f32 angle = this->calcAngleD(*pInfo->mParentTriangle.getNormal(0));
            if (angle < minAngle) {
                minAngle = angle;
                minIdx = i;
            }
            if (angle > maxAngle) {
                maxAngle = angle;
                maxIdx = i;
            }
        }
        if (static_cast<u32>(this->mMovementStates.mLowBits.mBit30)) {
            ref = this->calcAngleD(this->_368);
            if (ref < minAngle) {
                minAngle = ref;
            }
            if (ref > maxAngle) {
                maxAngle = ref;
            }
        }
        f32 range = maxAngle - minAngle;
        if (range < 60.0f && range > 5.0f && maxAngle < 80.0f) {
            this->mDrawStates.mBits.mBit16 = 1;
            if (maxAngle > ref) {
                this->_380 = *this->fn_803B2AA0(maxIdx)->mParentTriangle.getNormal(0);
            } else if (minAngle < ref) {
                this->_380 = *this->fn_803B2AA0(minIdx)->mParentTriangle.getNormal(0);
            }
        }
        if ((static_cast<u32>(this->mMovementStates.mLowBits.mBit30)) && maxAngle > 60.0f && maxAngle != ref && maxAngle < 80.0f) {
            TVec3f hitPos(this->fn_803B2AA0(maxIdx)->mHitPos);
            TVec3f toPos(rPos);
            toPos -= hitPos;
            if (!MR::normalizeOrZero(&toPos)) {
                this->mDrawStates.mBits.mBit8 = 1;
                MR::isNearZero(this->mAirGravityVec, 0.001f);
                TVec3f side;
                PSVECCrossProduct(this->fn_803B2AA0(maxIdx)->mParentTriangle.getNormal(0), &this->mAirGravityVec, &side);
                MR::normalize(&side);
                TVec3f front;
                PSVECCrossProduct(&this->_368, &side, &front);
                MR::normalize(&front);
                this->_2C4 = front;
            }
        }
    }
}

f32 Mario::fn_803B3870() {
    if (this->isStatusActive(1)) {
        return -0.9f;
    }
    return -0.999f;
}

// NONMATCHING: repeated flag loads and boolean materialization.
void Mario::fn_803B38B0(const HitInfo* hit, const TVec3f* center) {
    *mFrontWallTriangle = hit->mParentTriangle;
    _4E8 = hit->mHitPos;
    TVec3f direction(hit->mHitPos);
    direction.sub(*center);
    MR::normalizeOrZero(&direction);
    f32 facing = direction.dot(*hit->mParentTriangle.getNormal(0));
    if (facing < fn_803B3870()) mMovementStates.mLowBits.mBit23 = 1;
    else mMovementStates.mHighBits.mBit13 = 1;
    if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && (static_cast<u32>(mMovementStates.mLowBits.mBit10)) && (static_cast<u32>(mMovementStates.mHighBits.mBit6))) {
        TVec3f delta(_4A4);
        delta.sub(_4E8);
        if (delta.dot(*getGravityVec()) >= 0.0f) mMovementStates.mLowBits.mBit23 = 0;
    }
    if (!_60D && !(static_cast<u32>(mMovementStates.mLowBits.mBit23))) {
        if (calcAngleD(*mFrontWallTriangle->getNormal(0)) < mActor->getConst().getTable()->mSlipAngle &&
            !isSlipFloorCode(_95C->getCode(mFrontWallTriangle)) &&
            hit->mParentTriangle.getNormal(0)->dot(mFrontVec) < 0.0f && direction.dot(mFrontVec) > 0.0f &&
            direction.dot(*hit->mParentTriangle.getNormal(0)) < 0.0f) {
            mDrawStates.mBits.mBit16 = 1;
            _380 = *mFrontWallTriangle->getNormal(0);
        }
    }
}

void Mario::fn_803B3AE0(const HitInfo* hit, const TVec3f* origin) {
    if (isPlayerModeTeresa()) {
        TVec3f offset(hit->mHitPos);
        offset -= *origin;
        if (hit->mParentTriangle.getNormal(0)->dot(offset) >= 0.0f) return;
    }
    *mSideWallTriangle = hit->mParentTriangle;
    _500 = hit->mHitPos;
    mMovementStates.mLowBits.mBit5 = 1;
    if (hit->isCollisionAtCorner() || hit->isCollisionAtEdge()) mMovementStates.mHighBits.mBit12 = 1;
}

void Mario::fn_803B3BB0(const HitInfo* hit, const TVec3f* origin) {
    if (static_cast<u32>(mMovementStates.mLowBits.mBit23)) {
        TVec3f back(_4F4);
        back -= *origin;
        TVec3f front(_4E8);
        front -= *origin;
        if (back.dot(front) >= 0.0f) {
            f32 frontLength = PSVECMag(&front);
            if (PSVECMag(&back) >= frontLength) return;
        }
    }
    if (isPlayerModeTeresa()) {
        TVec3f offset(hit->mHitPos);
        offset -= *origin;
        if (hit->mParentTriangle.getNormal(0)->dot(offset) >= 0.0f) return;
    }
    *mBackWallTriangle = hit->mParentTriangle;
    _4F4 = hit->mHitPos;
    mMovementStates.mLowBits.mBit6 = 1;
}

void Mario::fn_803B3CE0() {
    if (mDashWallCooldown != 0) --mDashWallCooldown;
    if (isPlayerModeYoshi() && mActor->getYoshi()->mDashTimer > 0) {
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit23)) && !isAnimationRun("壁はじき")) {
            if (!mActor->sendMsgToSensor(mFrontWallTriangle->mSensor, 0xF7) && mDashWallCooldown == 0) {
                MR::tryRumblePadStrong(mActor, 0);
                MR::shakeCameraNormal();
                mActor->mDashWallHit1053 = 1;
                TVec3f velocity(*mFrontWallTriangle->getNormal(0));
                velocity.scale(5.0f);
                doFlipJump(velocity);
                if (!mDashWallSoundPlayed) {
                    MR::startActionSoundPlayer("DashHitWall", -1, -1);
                    mDashWallSoundPlayed = true;
                }
                mDashWallCooldown = 30;
            }
        } else {
            mDashWallSoundPlayed = false;
        }
        if ((static_cast<u32>(mMovementStates.mLowBits.mBit30)) && mGroundPolygon->isValid() && fn_803B3E90()) {
            mActor->getYoshi()->fn_8040F320();
            return;
        }
        mActor->getYoshi()->fn_8040F370();
        return;
    }
    mDashWallSoundPlayed = false;
    if (isPlayerModeYoshi()) mActor->getYoshi()->fn_8040F370();
}

bool Mario::fn_803B3E90() {
    HitSensor* pSensor = MR::getSensor(this->mGroundPolygon);
    if (pSensor != nullptr) {
        LiveActor* pHost = MR::getSensorHost(pSensor);
        if (pHost != nullptr) {
            return MR::isEqualString(pHost->mName, "YoshiDashStep");
        }
    }
    return false;
}

// NONMATCHING: constant pool labels
void Mario::fn_803B3EE0(TVec3f* pPos, f32* pHeight) {
    f32 offset = 110.0f;
    if (static_cast<u32>(this->mMovementStates.mLowBits.mBit21)) {
        offset = 50.0f;
    }
    *pHeight = 40.0f;
    pPos->set(this->mPosHolder->mPos - *this->getGravityVec() * offset);
    if (this->isStatusActive(2)) {
        this->mActor->calcHeadPos();
        pPos->set(this->mActor->mCapPosition);
        if (this->mDamage->_18 == 0) {
            *pHeight = 50.0f;
        }
    }
    if ((this->mMovementStates.mHighWord & 3) != 0) {
        this->mActor->calcHeadPos();
        pPos->set(this->mActor->mCapPosition + this->_16C * 2.0f);
    }
    if (this->getPlayerMode() == 4 && !((this->getPlayer()->mMovementStates.mHighWord >> 28) & 1) && !((this->getPlayer()->mMovementStates.mLowWord >> 21) & 1) &&
        !this->isStatusActive(28) && !this->isStatusActive(21) && !this->isStatusActive(27) && this->mActor->mBeeWallWalk == 0) {
        pPos->set(this->mActor->mCapPosition);
    }
    if (this->isPlayerModeYoshi() && this->isStatusActive(39) && !MR::isDead(this->mActor->fn_803B9830()->mRideModelActor)) {
        MarioMtxPos mtx;
        mtx.fn_80409700(MR::getJointMtx(this->mActor->fn_803B9830()->mRideModelActor, "Center"));
        TVec3f yAxis;
        mtx.getYAxis(&yAxis);
        TVec3f zAxis;
        mtx.getZAxis(&zAxis);
        mtx.getTranslation(pPos);
        pPos->add(yAxis * 50.0f - zAxis * 20.0f);
        *pHeight = 80.0f;
    }
    if (this->isSwimming()) {
        pPos->set(this->mPosHolder->mPos + this->mHeadVec * 60.0f);
    }
}

s32 Mario::fn_803B41F0() {
    s32 result = 0;
    s32 active = 0;
    if (mSinkTimer != 0) {
        if (static_cast<u32>(getPlayer()->mMovementStates.mLowBits.mBit30)) {
            active = 1;
        }
    }
    if (active != 0) {
        if (fn_803B2730()) {
            result = 1;
        }
    }
    return result;
}

void Mario::fn_803B4280() {
    if (this->_A7E != 0) {
        if (this->fn_803B41F0() == 0) {
            this->stopEffect("流砂足元砂煙");
            this->_A7E = 0;
        }
    } else if (this->fn_803B41F0() != 0) {
        this->playEffect("流砂足元砂煙");
        this->_A7E = 1;
    }
}

// NONMATCHING: register allocation
s32 Mario::fn_803B4300(TVec3f* pOutVec, MarioStrikeEntry* pOut, s32, const TVec3f& rPos, f32 radius) {
    u32 count = this->fn_803B29B0(rPos, radius);
    f32 limit;
    if (this->isPlayerModeYoshi() && this->isStatusActive(39) && !MR::isDead(this->mActor->fn_803B9830()->mRideModelActor)) {
        limit = 0.707f;
    } else if ((static_cast<u32>(this->mMovementStates.mLowBits.mBit31)) != 0) {
        limit = 0.0f;
    } else {
        limit = -0.707f;
    }
    s32 n = 0;
    pOutVec->zero();
    for (u32 i = 0; i < count; i++) {
        const HitInfo* pInfo = this->fn_803B2AA0(i);
        const TVec3f* pNrm = pInfo->mParentTriangle.getNormal(0);
        if (MR::isThroughPolygon(&pInfo->mParentTriangle)) {
            continue;
        }
        if (this->isThroughWall(&pInfo->mParentTriangle)) {
            continue;
        }
        if (!this->isSwimming() && (this->mMovementStates.mHighWord & 3) == 0 && pNrm->dot(*this->getGravityVec()) < limit) {
            continue;
        }
        TVec3f rest;
        f32 depth = pInfo->mDistance;
        if (MR::vecKillElement(*pOutVec, *pNrm, &rest) >= depth) {
            continue;
        }
        TVec3f push(*pNrm);
        push.scale(depth);
        TVec3f sum(rest);
        sum += push;
        *pOutVec = sum;
        pOut[n].mTriangle = pInfo->mParentTriangle;
        pOut[n].mHitPos = pInfo->mHitPos;
        pOut[n].mDepth = pInfo->mDistance;
        n++;
    }
    return n;
}

// NONMATCHING: register allocation and flag lowering.
void Mario::fn_803B44F0(TVec3f* arg1) {
    TVec3f sp14(*arg1);
    MR::normalizeOrZero(&sp14);
    if (this->calcAngleD(sp14) > 45.0f) {
        TVec3f sp8(*arg1);
        sp8 *= 0.2f;
        this->blown(sp8);
        this->_402 = 0;
        this->_428 = 0x3C;
        this->mMovementStates.mHighWord = (this->mMovementStates.mHighWord | 0x100000U) & 0xFFFFFFFCU;
    } else if (this->_1FC.dot(*arg1) < 0.0f) {
        this->mMovementStates.mLowBits.mBit30 = 1;
        this->mJumpVec.zero();
        this->changeAnimation("空中一回転", static_cast<const char*>(nullptr));
    }
}

void Mario::fn_803B45E0(const char* pName, const char* arg) {
    this->changeAnimation(pName, (const char*)nullptr);
    this->mActor->fn_803BC440(arg);
}

void Mario::fn_803B4630(const char* pName) {
    this->changeAnimation(pName, (const char*)nullptr);
    this->mActor->clearNullAnimation(-3);
}

void Mario::fn_803B4670(const char* pName) {
    this->changeAnimationUpper(pName, nullptr);
    this->mActor->clearNullAnimation(-3);
}

void Mario::addTrans(const TVec3f& rShift) {
    mPosHolder->fn_80406010(rShift);
    TVec3f _148shift(_148);
    _148shift -= rShift;
    if (MR::abs(_148.x) <= MR::abs(_148shift.x)) {
        _148.x = 0.0f;
    }

    if (MR::abs(_148.y) <= MR::abs(_148shift.y)) {
        _148.y = 0.0f;
    }

    if (MR::abs(_148.z) <= MR::abs(_148shift.z)) {
        _148.z = 0.0f;
    }
}

void Mario::setTrans(const TVec3f& rShift) {
    const TVec3f& position = mPosHolder->mPos;
    TVec3f reqShift(rShift);
    reqShift -= position;
    mPosHolder->fn_80405F90(rShift);
    TVec3f _148shift(_148);
    _148shift -= reqShift;
    if (MR::abs(_148.x) <= MR::abs(_148shift.x)) {
        _148.x = 0.0f;
    }

    if (MR::abs(_148.y) <= MR::abs(_148shift.y)) {
        _148.y = 0.0f;
    }

    if (MR::abs(_148.z) <= MR::abs(_148shift.z)) {
        _148.z = 0.0f;
    }
}

bool Mario::fn_803B4850(const HitSensor* pSensor) const {
    if (MR::getSensorHost(pSensor) != nullptr) {
        return MR::isEqualString(MR::getSensorHost(pSensor)->mName, "溶岩分離滝");
    }
    return false;
}

// Source-only compatibility methods and existing emission helpers.

void Mario_FORCE_MATCH_SDATA2() {
    (void)0.00100000005f;
    (void)0.800000012f;
    (void)-0.514999986f;
    (void)200.0f;
    (void)28000.0f;
    (void)2499.0f;
    (void)0.104719758f;
    (void)0.707000017f;
    (void)300.0f;
    (void)1.42857146f;
    (void)0.300000012f;
    (void)0.100000001f;
    (void)150.0f;
    (void)40.0f;
    (void)160.0f;
    (void)80.0f;
    (void)90.0f;
    (void)5.0f;
    (void)1.49599659f;
    (void)170.0f;
    (void)0.785398185f;
    (void)0.99000001f;
    (void)30.0f;
    (void)15.0f;
    (void)45.0f;
    (void)60.0f;
    (void)290.0f;
    (void)260.0f;
    (void)1.04719758f;
    (void)0.200000003f;
    (void)0.0245436933f;
    (void)0.0174532924f;
    (void)0.00999999978f;
    (void)32.0f;
    (void)0.125f;
    (void)100.0f;
    (void)35.0f;
    (void)50.0f;
    (void)0.00390625f;
    (void)120.0f;
    (void)0.0199999996f;
    (void)1.5f;
    (void)2607.59448f;
    (void)-2607.59448f;
    (void)-150.0f;
}

bool Mario::isEnableCheckGround() {
    return !isStatusActive(MarioStatus_Swim);
}

// Nearly matches
void Mario::createMtxDir(MtxPtr pMtx, const TVec3f& rFront, const TVec3f& rUp, const TVec3f& rSide) {
    TVec3f side;
    TVec3f front;
    TVec3f up;
    side = rSide;
    front = rFront;
    up = rUp;
    bool isNormal = !isPlayerModeTeresa();
    if (isNormal) {
        if (((mMovementStates.mHighWord >> 8) & 1U) || ((mFlags14.mLowWord >> 10) & 1U)) {
            f32 sideAngleDiff = MR::diffAngleAbs(side, _950);
            f32 frontAngleDiff = MR::diffAngleAbs(front, _944);
            f32 upAngleDiff = MR::diffAngleAbs(up, _938);
            if (sideAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleS) {
                side = _950;
            } else {
                _950 = side;
            }

            if (frontAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleF) {
                front = _944;
            } else {
                _944 = front;
            }

            if (upAngleDiff < mActor->mConst->getTable()->mVibrationAbsorbAngleH) {
                up = _938;
            } else {
                _938 = up;
            }
        }
    }

    pMtx[0][0] = side.x;
    pMtx[1][0] = side.y;
    pMtx[2][0] = side.z;
    pMtx[0][1] = up.x;
    pMtx[1][1] = up.y;
    pMtx[2][1] = up.z;
    pMtx[0][2] = front.x;
    pMtx[1][2] = front.y;
    pMtx[2][2] = front.z;
}

bool Mario::createCorrectionMtx(MtxPtr pMtx, TVec3f* pOut) {
    pOut->zero();

    if (isNonFixHeadVec()) {
        PSMTXIdentity(pMtx);
        return false;
    }

    if (MR::isNearZero(_74C) && _754 == 0) {
        PSMTXIdentity(pMtx);
        _750 = 0;
        _74C = 0.0f;
    } else {
        if (_750 == 0 && _754 == 0) {
            TMtx34f tempMtx;
            TVec3f newFrontVec;
            PSMTXRotAxisRad(tempMtx, mHeadVec, _74C);
            PSMTXMultVec(tempMtx, mFrontVec, newFrontVec);
            setFrontVecKeepUp(newFrontVec);
            _74C = 0.0f;
        }

        TVec3f vec2(0.0f, 1.0f, 0.0f);

        if (((mMovementStates.mHighWord >> 8) & 1U) && !isStatusActive(MarioStatus_Talk)) {
            bool b1 = false;
            f32 dot = mFrontVec.dot(getCamDirX());

            if (_1FC.dot(getCamDirY()) < 0.0f) {
                dot = -dot;
            }

            if (dot < 0.0f && _74C < 0.0f) {
                b1 = true;
            } else if (dot > 0.0f && _74C > 0.0f) {
                b1 = true;
            }

            if (b1) {
                PSMTXRotAxisRad(pMtx, vec2, -_74C);
            } else {
                PSMTXRotAxisRad(pMtx, vec2, _74C);
            }
        } else {
            PSMTXRotAxisRad(pMtx, vec2, _74C);
        }

        if (_754 != 0) {
            _74C += (MR::pi() - _74C) / static_cast< f32 >(_754);
            _754--;
            _750 = 0;
        }

        if (_750 != 0) {
            _74C = _74C * static_cast< f32 >(_750 - 1) / static_cast< f32 >(_750);
            _750--;
            if (_750 == 0) {
                setFrontVecKeepUp(mFrontVec);
            }
        }
    }

    return true;
}

void Mario::createAngleMtx(MtxPtr pMtx, bool a2) {
    f32 zero;
    TVec3f vec1;
    TVec3f vec2;
    TVec3f vec3;
    TVec3f vec4;
    if (!(isNonFixHeadVec() || a2 || ((mMovementStates.mLowWord & Mario::MovementStates::DebugMode) >> 9))) {
        fixHeadFrontVecByGravity();
    }

    if (updatePosture(pMtx)) {
        return;
    }

    createDirectionMtx(pMtx);

    vec2 = _1FC;
    vec1 = mSideVec;
    vec3 = mFrontVec;

    PSMTXConcat(_C4, pMtx, pMtx);

    HitSensor* sensor = mActor->_470;

    if (isAnimationRun("投げ")) {
        sensor = mActor->_474;
    }

    if (mActor->isPunching()) {
        sensor = mActor->_924;
    }

    if (sensor != nullptr) {
        vec4 = sensor->mPosition;
    }

    if (sensor != nullptr) {
        TVec3f vec5;
        vec5.set(vec4 - mPosHolder->mPos);

        MR::normalizeOrZero(&vec5);

        if (MR::isNearZero(vec5)) {
            return;
        }

        TVec3f vec6(vec2);

        TVec3f vec7;
        MR::vecKillElement(vec5, vec6, &vec7);

        if (MR::normalizeOrZero(&vec7)) {
            return;
        }

        f32 angle = JMAAcosRadian(vec7.dot(vec3));

        TVec3f cross;
        cross.cross(vec3, vec7);

        if (cross.dot(vec2) < 0.0f) {
            angle = -angle;
        }

        f32 pitchAngle = JMAAcosRadian(vec7.dot(vec5));

        bool dotNonNeg = vec5.dot(vec6) >= 0.0f;

        if (MR::abs(angle) < HALF_PI && dotNonNeg) {
            pitchAngle = -pitchAngle;
        } else if (MR::abs(angle) >= HALF_PI && !dotNonNeg) {
            pitchAngle = -pitchAngle;
        }

        f32 f1 = 0.1f * static_cast< f32 >(_412);
        zero = 0.0f;

        if (f1 > MR::toRadian(60.0f)) {
            f1 = MR::toRadian(60.0f);
        }

        _58.x = MR::clamp(angle, -f1, f1);
        _58.z = zero;
        _58.y = 0.0f;

        MR::orderRotateMtx(0, _58, _64);
        MR::orderRotateMtx(0, -_58, _94);
        _12C = 16;
        return;
    } else if (_12C != 0) {
        _12C--;
    }

    _58.x *= 0.8f;
    _58.z *= 0.8f;
    _58.y = 0.0f;

    MR::orderRotateMtx(0, _58, _64);
    MR::orderRotateMtx(0, -_58, _94);
}

void Mario::updateLookOfs() {
    TVec3f vec;
    vec.zero();

    if (isStatusActive(MarioStatus_Blown)) {
        return;
    }

    if (mActor->_934) {
        MR::vecBlend(_13C, vec, &_13C, 0.3f);
    }

    if (_97C == nullptr) {
        if (isAnimationRun("壁押し")) {
            TVec3f vec2;
            MR::vecKillElement(_4E8 - mPosHolder->mPos, mHeadVec, &vec2);
            f32 len = vec2.length();
            vec = *mFrontWallTriangle->getNormal(0) * (len - 35.0f);
        }
    } else if (_97C->mStatusId == MarioStatus_Swim) {
        vec += -*getGravityVec() * mSwim->mSurfaceOffset;
    }

    if (((mMovementStates.mLowWord >> 30) & 1U) && !((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && mVerticalSpeed > 5.0f && mVerticalSpeed < 50.0f && !isCurrentShadowFloorDangerAction()) {
        TVec3f dirShadowToGround(mGroundPos - mShadowPos);
        MR::normalizeOrZero(&dirShadowToGround);
        if (dirShadowToGround.dot(_368) < 0.1f) {
            vec += *getGravityVec() * mVerticalSpeed;
        } else {
            TVec3f vec3;
            f32 f1 = MR::vecKillElement(mGroundPos - mPosHolder->mPos, *getGravityVec(), &vec3);
            vec += *getGravityVec() * f1;
        }
    }

    TVec3f vec4;
    vec4.zero();

    if (mSinkTimer != 0) {
        vec4 = *getGravityVec() * static_cast< f32 >(mSinkTimer) * (1.0f / 256.0f) * 120.0f;
        vec4 = -_1FC * static_cast< f32 >(mSinkTimer) * (1.0f / 256.0f) * 120.0f;
    }

    f32 blendRate = 0.2f;

    if (!isUseSimpleGroundCheck()) {
        if (getAnimator()->isLandingAnimationRun()) {
            blendRate = 0.01f;
        } else if (mTargetWalkSpeedIndex == 0 && mWalkSpeed < 0.2f && isBlendWaitGround()) {
            if (_148.length() > vec.length()) {
                blendRate = 0.5f;
            } else {
                blendRate = 0.02f * static_cast< f32 >(_3CE);

                if (blendRate >= 0.2f) {
                    blendRate = 0.2f;
                }
            }
        }
    }

    MR::vecBlend(_148, vec, &_148, blendRate);
    MR::vecBlend(_154, vec4, &_154, 0.2f);
    _13C = _148 + _154;
}

void MarioCollision_FORCE_MATCH_SDATA2() {
    (void)0.001f;
    (void)3e+01f;
    (void)0.3f;
    (void)0.707f;
    (void)1.01f;
    (void)-0.99f;
    (void)5.0f;
    (void)4e+01f;
    (void)1.1e+02f;
    (void)5e+01f;
    (void)6e+01f;
    (void)-0.707f;
    (void)45.0f;
    (void)0.2f;
    (void)1.2e+02f;
    (void)8e+01f;
    (void)0.7853982f;
    (void)1e+02f;
    (void)4e+02f;
    (void)1e+01f;
    (void)0.1f;
    (void)2.5e+03f;
    (void)1.5e+02f;
    (void)0.10471976f;
    (void)2.3561945f;
    (void)0.17f;
    (void)1.4959966f;
    (void)-3e+01f;
    (void)2e+02f;
    (void)-1e+01f;
    (void)5e+03f;
    (void)25.0f;
    (void)1.8e+02f;
    (void)3.14159f;
    (void)-0.999f;
    (void)-0.9f;
    (void)2.0943952f;
    (void)6.0f;
    (void)-6.0f;
    (void)55.0f;
    (void)0.99f;
    (void)-0.01f;
    (void)8.0f;
    (void)2e+01f;
    (void)2.1e+02f;
    (void)-2607.5945f;
    (void)2607.5945f;
    (void)15.0f;
    (void)2e+03f;
    (void)0.125f;
}

void Mario::setCameraPolygon(const Triangle* pTriangle) {
    *_468 = *pTriangle;

    if (pTriangle->mSensor == nullptr) {
        *_46C = *pTriangle;
        return;
    }

    *_46C = *pTriangle;
}

#pragma push
#pragma opt_propagation off
#pragma opt_lifetimes off
void Mario::calcFrontFloor() {
    Triangle wall;
    Triangle floor;
    TVec3f start;
    TVec3f wallPos;
    TVec3f floorPos;
    TVec3f direction;
    Triangle* pFloor = _47C;
    mMovementStates.mLowBits.mBit10 = 0;
    mMovementStates.mHighBits.mBit6 = 0;
    pFloor->mIdx = -1;
    _4E0 = 0.0f;
    bool hit;

    if (((mMovementStates.mLowWord >> 23) & 1U)) {
        wallPos = _4E8;
        start = mActor->_2A0;
        hit = true;
    } else {
        direction = mFrontVec * 100.0f;
        start = mPosHolder->mPos - *getGravityVec() * 8.0f;

        if (((mMovementStates.mLowWord >> 16) & 1U) && !MR::isNearZero(_328)) {
            MR::vecKillElement(_328, *getGravityVec(), &direction);
            direction.setLength(200.0f);
        }

        hit = MR::getFirstPolyOnLineToMap(&wallPos, &wall, start, direction);
    }

    if (hit) {
        _4E4 = (wallPos - start).dot(mFrontVec);
        TVec3f normal(*wall.getNormal(0));
        f32 dot = normal.dot(getAirGravityVec());

        if (MR::abs(dot) < 0.1f) {
            f32 angle = marioAcos(-dot);
            start = wallPos - getAirGravityVec() * 200.0f;
            start += mFrontVec * 20.0f;
            direction = getAirGravityVec() * 210.0f;
            hit = MR::getFirstPolyOnLineBFast(start, direction, &floorPos, &floor);

            if (hit && floor.getNormal(0)->dot(getAirGravityVec()) > -0.9f) {
                hit = false;
            }

            if (hit) {
                _4E0 = (wallPos - floorPos).dot(getAirGravityVec());

                if (_4E0 > 0.0f) {
                    mMovementStates.mLowBits.mBit10 = 1;
                }

                f32 offset = 0.0f;

                if (angle < 1.5707964f && angle != 0.0f) {
                    offset = _4E0 / (JMASinRadian(angle) / JMACosRadian(angle));
                }

                TVec3f originalPos(floorPos);
                floorPos -= mFrontVec * (15.0f + offset);

                if (!MR::isExistMapCollision(floorPos - getAirGravityVec() * 5.0f, *getGravityVec() * 10.0f)) {
                    floorPos = originalPos;
                } else {
                    _4A4 = floorPos;
                    mMovementStates.mHighBits.mBit6 = 1;
                    *_47C = floor;
                }

            } else {
                _4E0 = 400.0f;
            }
        }

    } else {
        _4E0 = 0.0f;
    }

    if (((mMovementStates.mLowWord >> 30) & 1U) && calcAngleD(_368) >= 25.0f) {
        mMovementStates.mHighBits.mBit6 = 0;
    }
}
#pragma pop

CubeCameraArea* Mario::getCameraCubeCode() const {
    if ((isSwimming() && mSwim->isOnWaterSurface()) || (((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && isRising())) {
        return static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosHolder->mPos - *getGravityVec() * 100.0f));
    }

    return static_cast< CubeCameraArea* >(MR::getAreaObj("CubeCamera", mPosHolder->mPos));
}

// namespace

void MarioDamage_FORCE_MATCH_SDATA2() {
    (void)10.0f;
    (void)5.0f;
    (void)0.949999988f;
    (void)30.0f;
    (void)20.0f;
    (void)50.0f;
    (void)-10.0f;
    (void)0.699999988f;
    (void)0.100000001f;
    (void)0.25f;
}

bool Mario::damageLarge(const TVec3f& rDirection) {
    if (damage(rDirection)) {
        if (isStatusActive(MarioStatus_Swim)) {
            mSwim->fn_803F7C00();
        } else {
            mDamage->setVecSize(mActor->getConst().getTable()->mJumpDistLargeDamage, mActor->getConst().getTable()->mJumpHeightLargeDamage);
            playSound("投げられ");
        }

        return true;
    }

    return false;
}

bool Mario::doFireObjHitWithInitialDamage() {
    if (isEnableAddDamage() == false) {
        return false;
    }

    return doFireDanceWithInitialDamage(1);
}

const TVec3f* Mario::getLastGroundEdgeNrm(u32 idx) const {
    return _474->getEdgeNormal(idx);
}

bool Mario::doFlow() {
    if (isStatusActive(MarioStatus_Flow)) {
        return false;
    }

    if (isAnimationRun("空中ひねり")) {
        return false;
    }

    changeStatus(mFlow);

    return true;
}

void MarioFoo_FORCE_MATCH_SDATA2() {
    (void)0.52359879f;
    (void)0.100000001f;
    (void)120.0f;
    (void)0.125f;
    (void)10.0f;
    (void)100.0f;
    (void)0.0500000007f;
    (void)5.0f;
    (void)1.04719758f;
    (void)1.10000002f;
    (void)-0.100000001f;
    (void)0.00999999978f;
    (void)25.0f;
    (void)0.980000019f;
    (void)1.01999998f;
    (void)0.899999976f;
    (void)1.20000005f;
    (void)50.0f;
    (void)1.5f;
    (void)12.0f;
    (void)2.09439516f;
    (void)0.314159274f;
    (void)0.785398185f;
    (void)0.25f;
    (void)30.0f;
    (void)0.015625f;
    (void)70.0f;
    (void)2000.0f;
    (void)255.0f;
    (void)1.00010002f;
    (void)6.0f;
}

void MarioFoo_FORCE_MATCH(TVec3f& rVec, f32 a, f32 b, f32 c) {
    rVec.setLength(a);
    rVec.setLength(b);
    rVec.setLength(c);
}

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

// namespace

void MarioJump_FORCE_MATCH_SDATA2() {
    (void)0.00100000005f;
    (void)15.0f;
    (void)0.300000012f;
    (void)4.0f;
    (void)10.0f;
    (void)0.899999976f;
    (void)500.0f;
    (void)0.100000001f;
    (void)5.0f;
    (void)100.0f;
    (void)50.0f;
    (void)150.0f;
    (void)0.600000024f;
    (void)80.0f;
    (void)20.0f;
    (void)8.0f;
    (void)0.200000003f;
    (void)0.800000012f;
    (void)0.00390625f;
    (void)45.0f;
    (void)160.0f;
    (void)1.79999995f;
    (void)0.25f;
    (void)300.0f;
    (void)200.0f;
    (void)-0.100000001f;
    (void)-0.800000012f;
    (void)30.0f;
    (void)-0.200000003f;
    (void)0.785398185f;
    (void)0.998000026f;
    (void)0.00999999978f;
    (void)6.0f;
    (void)7.0f;
    (void)90.0f;
}

bool Mario::isDigitalJump() const NO_INLINE {
    return ((mMovementStates.mLowWord >> 17) & 1U);
}



void Mario::calcShadowDir2D(const TVec3f& rVec, TVec3f* pOut) {
    *pOut = rVec;
    MR::normalizeOrZero(pOut);
}

void MarioSkate_FORCE_MATCH_SDATA2() {
    (void)20.0f;
    (void)50.0f;
    (void)1.25f;
    (void)1.5f;
    (void)1.2f;
    (void)0.04f;
    (void)0.1f;
    (void)-0.1f;
    (void)-0.3926991f;
    (void)0.3926991f;
    (void)-0.5235988f;
    (void)0.5235988f;
    (void)0.8f;
    (void)0.9f;
    (void)0.95f;
    (void)0.05f;
}

void Mario::setSeVersion(u32 version) {
    MR::setSeVersion(mActor, version);
}

bool Mario::isStageCameraRotate2D() const {
    return MR::isEqualStageName("HellProminenceGalaxy") && MR::getCurrentScenarioNo() == 3;
}

bool Mario::isHeadPushEnableArea() const {
    if (!((mMovementStates.mHighWord >> 8) & 1U)) {
        return false;
    }

    if (MR::isEqualStageName("SandClockGalaxy")) {
        if (MR::isInRange(mPosHolder->mPos.x, -7020.0f, -6920.0f)) {
            return true;
        }
    }

    return false;
}

void Mario::checkBeeStick() {
    return;
}

void MarioSwim_FORCE_MATCH_SDATA2() {
    (void)100.0f;
    (void)50.0f;
    (void)0.52359879f;
    (void)-200.0f;
    (void)0.100000001f;
    (void)0.200000003f;
    (void)10.0f;
    (void)1.74532938f;
    (void)-10.0f;
    (void)1.49599659f;
    (void)200.0f;
    (void)8.0f;
    (void)2.09439516f;
    (void)40.0f;
    (void)0.00999999978f;
    (void)0.0299999993f;
    (void)6.28318548f;
    (void)-50.0f;
    (void)0.196349546f;
    (void)30.0f;
    (void)-30.0f;
    (void)-0.100000001f;
    (void)60.0f;
    (void)1000.0f;
    (void)0.899999976f;
    (void)400.0f;
    (void)0.707000017f;
    (void)2.5f;
    (void)-400.0f;
    (void)120.0f;
    (void)0.125f;
    (void)300.0f;
    (void)0.0500000007f;
    (void)5.0f;
    (void)0.150000006f;
    (void)0.87266469f;
    (void)2.26892805f;
    (void)12.0f;
    (void)25.0f;
    (void)0.25f;
    (void)0.980000019f;
    (void)0.00100000005f;
    (void)1.04719758f;
    (void)2000.0f;
    (void)-25.0f;
    (void)0.949999988f;
    (void)0.75f;
    (void)0.300000012f;
    (void)2.61799383f;
    (void)0.99000001f;
    (void)0.995000005f;
    (void)0.959999979f;
    (void)1.65346992f;
    (void)15.0f;
    (void)110.0f;
    (void)80.0f;
    (void)-500.0f;
    (void)210.0f;
    (void)170.0f;
    (void)10000.0f;
    (void)0.970000029f;
    (void)0.800000012f;
    (void)500.0f;
    (void)1.01999998f;
    (void)1.5f;
    (void)6.0f;
    (void)0.699999988f;
    (void)0.314159274f;
    (void)0.785398185f;
    (void)2.61799407f;
    (void)16.0f;
    (void)-0.5f;
    (void)18.0f;
    (void)1.25f;
    (void)1.00010002f;
}

void Mario::setLastNonFixMoveVec(const TVec3f& rVec) {
    _334 = rVec;
}

void MarioTeresa_FORCE_MATCH_SDATA2() {
    (void)100.0f;
    (void)10.0f;
    (void)0.00100000005f;
    (void)0.00999999978f;
    (void)120.0f;
    (void)110.0f;
    (void)-0.100000001f;
    (void)0.100000001f;
    (void)50.0f;
    (void)0.939999998f;
    (void)1.5f;
    (void)0.200000003f;
    (void)200.0f;
    (void)0.99000001f;
    (void)60.0f;
    (void)-2.0f;
    (void)-0.400000006f;
    (void)-0.25f;
    (void)0.25f;
    (void)0.75f;
    (void)-0.5f;
    (void)0.0399999991f;
    (void)0.300000012f;
    (void)20.0f;
    (void)-2607.59448f;
    (void)2607.59448f;
    (void)0.707000017f;
    (void)-10.0f;
}

XjointTransform* (XanimeCore::*Mario_FORCE_MATCH_JOINT_TRANSFORM())(u32) {
    return &XanimeCore::getJointTransform;
}

// native: defined in MarioTeresa.cpp (DOL split owner); misplaced copy
#ifndef GALAXY_NATIVE
XanimeGroupInfo teresaAnimeTable[] = {
    {{"基本"}, 1.0f, 16},
    {{""}},
};


XanimeBckTable2 teresaAnime2[] = {
    {{"基本"}, {{"wait", 1.0f}, {"run", 0.0f}}},
    {{""}, {{"", 0.0f}, {nullptr, 0.0f}}},
};
#endif
