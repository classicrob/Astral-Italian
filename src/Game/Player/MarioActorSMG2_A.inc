#include "Game/Util/PlayerUtil.hpp"
// SMG2-only MarioActor functions (part A), written from the asm. Included at the end of MarioActor.cpp (unity TU).














































































































// NONMATCHING: typed count/pointer clears replace redundant vector type-pun; loop registers and scheduling differ.






























void DLchanger::addDL(J3DModelX* m) {
    m->setDynamicDL(mBuffers[mCurrentBuffer].mDL, mBuffers[mCurrentBuffer].mSize);
}






// NONMATCHING: equivalent nonnegative timer shifts and argument-address scheduling.


// NONMATCHING: Equivalent typed-member address reuse and saved-register allocation.




bool MarioActor::fn_803C1C60() {
    if (this->mSuperKinokoCollected == 0) {
        return false;
    }
    if (!MR::tryStartDemoWithoutCinemaFrame(this, "マリオスーパー化")) {
        return false;
    }
    this->mSuperKinokoCollected = 0;
    this->changeMaxLife(6);
    MR::stopAnimFrame(this);
    if (this->mMario->isPlayerModeYoshi()) {
        MR::stopAnimFrame(mYoshiHolder->mYoshi);
    }
    MR::requestPowerUpHPMeter();
    this->mMario->startPadVib("マリオ[変身]");
    MR::startSystemSE("SE_SY_SUPER_KINOKO_GET", -1, -1);
    this->_3DA = 0x5A;
    return true;
}

bool MarioActor::fn_803C1D30() {
    if (this->mPowerupCollected == 0) {
        return false;
    }
    if (!MR::tryStartDemoWithoutCinemaFrame(this, "マリオ変身")) {
        return false;
    }
    this->mTransforming = 1;
    this->mPowerupCollected = 0;
    this->_3D8 = 0x40;
    if (mPlayerMode == 6) {
        this->fn_803BB4F0();
        this->changeTeresaAnimation("change", -1);
    }
    if (mPlayerMode == 0xA) {
        MR::requestMovementOn(this);
        MR::requestMovementOn(mYoshiHolder->mYoshi);
        this->_3D8 = 0x13;
        this->mMario->clearVelocity();
        (this->mMario)->mWalkSpeed = 0.0f;
    }
    if (mPlayerMode != 0xA) {
        MR::stopAnimFrame(this);
        this->playEffect("変身");
        this->mMario->startPadVib("マリオ[変身]");
    }
    return true;
}

bool MarioActor::fn_803C1E40() {
    bool r = false;
    if ((this->_3D8 != 0 && mPlayerMode != 0) || this->_3DA != 0) {
        r = true;
    }
    return r;
}

void MarioActor::fn_803C1E80() {
    (this->mMario)->_350.zero();
    (this->mMario)->_35C.zero();
    reinterpret_cast<s16&>(this->_7DC) = 0;
    this->_930 = 0;
    mVelocity.zero();
    mLastMove.zero();
    this->_1FC = this->mPosition;
    if (this->fn_803C1F30()) {
        this->mMario->mPosHolder->fn_80405F90(this->mPosition);
        this->mMario->checkEnforceMove();
        this->mPosition = this->mMario->mPosHolder->mPos;
    }
}

bool MarioActor::fn_803C1F30() {
    Mario* m = this->mMario;
    bool r = false;
    if (((m->mMovementStates.mLowWord >> (0x1E)) & 1)) {
        MtxPtr prev = m->_45C->getPrevBaseMtx()->mMtx;
        if (!MR::isSameMtx((this->mMario)->_45C->getBaseMtx()->mMtx, prev)) {
            r = true;
        }
    }
    return r;
}

void MarioActor::fn_803C2010() {
    u16 t = this->_BC4;
    if (t != 0) {
        this->_BC4 = t - 1;
    }
}

void MarioActor::fn_803C2030() {
    if (this->_EA4 == 0) {
        if (this->_F44 == 0) {
            MR::offBind(this);
        } else {
            MR::onBind(this);
        }
    }
    this->_978 = mVelocity;
    if (((this->mMario->mDrawStates.mWord >> (0x14)) & 1)) {
        this->mBinder->_1EC._1 = false;
        return;
    }
    this->mBinder->_1EC._1 = true;
}


void MarioActor::fn_803C2320() {
    this->_37C += 1;
    if (this->_EF6 != 0) {
        this->_EF6 -= 1;
    }
    if (this->_3A8 != 0) {
        this->_3A8 -= 1;
    }
    if (this->_3AC != 0) {
        this->_3AC -= 1;
    }
    if (this->mAnimationDelayE28 != 0) {
        this->mAnimationDelayE28 -= 1;
    }
    if (this->mRushAnimationDelay1054 != 0) {
        this->mRushAnimationDelay1054 -= 1;
        if (this->mRushAnimationDelay1054 == 0) {
            this->_480 = 1;
            this->mMarioAnim->fn_803DDF60();
        }
    }
}

void MarioActor::fn_803C23B0() {
    TVec3f d(this->mPosition);
    d -= this->_1FC;
    mLastMove = d;
    this->_1FC = this->mPosition;
}

void MarioActor::fn_803C2520() {
    if (this->_B94 != 0) {
        this->_B94 -= 1;
        if (this->_B94 == 0) {
            this->mMario->stopAnimationUpper("ハンマー投げ回転中", (const char*)nullptr);
            this->mMario->stopAnimation("ハンマー投げ回転中", (const char*)nullptr);
        }
    }
}


void MarioActor::fn_803C2590() {
    if (this->_944 != 0) {
        u8 t = this->_945;
        this->_944 = this->_944 - 1;
        this->_945 = t + 1;
        this->fn_803C2610();
        this->fn_803C26A0();
    }
    if (this->fn_803C2710()) {
        this->mMario->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
}

void MarioActor::fn_803C2610() {
    if ((_946 != 0 || ((this->mMario->mMovementStates.mHighWord >> (0x14)) & 1)) && !this->mMario->isSwimming() && this->_944 == 0 &&
        mActionTable1080->selectAction("スピン回復エフェクト") == 1) {
        this->playEffect("スピンリング");
    }
}

void MarioActor::fn_803C26A0() {
    if ((this->isInPunchTimerRange() || !this->mMario->isSwimming())) {
        u32 t = (this->mMario)->mMovementStates.mLowWord;
        if (((t >> 0x17) & 1) || ((t >> 5) & 1) || ((t >> 6) & 1)) {
            this->mMario->tryWallPunch();
        }
    }
}

bool MarioActor::fn_803C2710() {
    bool requested;
    bool playerGateClear;
    bool actorGateClear;
    bool releaseAnimation;
    releaseAnimation = false;
    actorGateClear = false;
    playerGateClear = false;
    requested = false;
    if (mMario->isAnimationRun("ハンマー投げリリース") &&
        ((mMario->mMovementStates.mLowWord >> 30) & 1U)) {
        releaseAnimation = true;
    }
    if (releaseAnimation && _38C == 0) {
        actorGateClear = true;
    }
    if (actorGateClear && mMario->_420 == 0) {
        playerGateClear = true;
    }
    if (playerGateClear && mMario->isStickOn()) {
        requested = true;
    }
    return requested;
}


void MarioActor::fn_803C2900() {
    if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, "水陸移動特殊処理")) {
        WaterInfo info;
        if (!MR::getWaterAreaObj(&info, this->mPosition)) {
            this->mMario->forceExitSwim();
        }
    } else if (!mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, "水柱エフェクト")) {
        MarioSwim* sw = this->mMario->mSwim;
        s32 before = (sw)->mSwimState;
        sw->fn_803FD5E0(0);
        s32 after = (this->mMario->mSwim)->mSwimState;
        if (after != before) {
            MarioSwim* sw2 = this->mMario->mSwim;
            if ((u32)after <= 1 && (u32)(before - 2) <= 1) {
                this->playEffectRT("水面ジャンプ水柱", (sw2)->mSurfacePos, (sw2)->mSurfaceNorm);
                MarioSwim* sw3 = this->mMario->mSwim;
                this->emitEffectWaterColumn((sw3)->mSurfacePos, (sw3)->mSurfaceNorm);
            } else if ((u32)before <= 1 && (u32)(after - 2) <= 1) {
                TVec3f neg = -(this->mMario)->_328;
                this->playEffectRT("水面ジャンプ水柱", neg, (this->mMario->mSwim)->mSurfaceNorm);
                MarioSwim* sw3 = this->mMario->mSwim;
                this->emitEffectWaterColumn((sw3)->mSurfacePos, (sw3)->mSurfaceNorm);
            }
            switch (after) {
            case 0:
                MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();
                this->mMario->forceExitSwim();
                return;
            case 2:
                MR::getGameSceneLayoutHolder()->changeLifeMeterModeWater();
                break;
            }
        }
    }
}

void MarioActor::fn_803C2AA0() {
    if (this->mMario->isForceStopRush() && _934 != 0 && _924->receiveMessage(0x97, this->getSensor("Body"))) {
        RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
        this->endRush(&info);
    }
}

void MarioActor::fn_803C2B50() {
    if (_934 != 0) {
        const char* effect = "ハチ回復";
        if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, effect)) {
            u32 i = 0;
            do {
                this->mMario->incAirWalkTimer();
                i++;
            } while (i < 4);
        }
    }
}

bool MarioActor::fn_803C3260() {
    if (this->mMario->isAnimationRun("壁はじき")) {
        return false;
    }
    if (this->isJumping() && this->mMario->_428 != 0) {
        return false;
    }
    if (mPlayerMode == 9) {
        return false;
    }
    if (this->mMario->getCurrentStatus() == 1) {
        return false;
    }
    if (this->mMario->isSwimming()) {
        return false;
    }
    if (this->mMario->isStatusActive(0x18)) {
        return false;
    }
    if (this->mMario->isStatusActive(0x13)) {
        return false;
    }
    if (static_cast<s32>(_468) != 0) {
        return false;
    }
    if (this->mMario->isStatusActive(2)) {
        return false;
    }
    if (this->_3C0 != 0) {
        return false;
    }
    if (this->_EA4 != 0) {
        return false;
    }
    Mario* m = this->mMario;
    if (((m->mDrawStates.mWord >> (0x13)) & 1)) {
        return false;
    }
    return m->mCometNoSpinArea == 0;
}

bool MarioActor::fn_803C33D0() {
    bool r = false;
    bool a = false;
    bool b = false;
    if (!((this->mMario->mMovementStates.mLowWord >> (0x10)) & 1) && this->isJumping()) {
        b = true;
    }
    if (b) {
        bool c = true;
        if (this->mMario->isDamaging()) {
            bool d = false;
            if (this->mMario->isPlayerModeCloud() && MR::isSpinCloudBlockGroupEmpty() == 0) {
                d = true;
            }
            if (!d) {
                c = false;
            }
        }
        if (c) {
            a = true;
        }
    }
    if (a && !((this->mMario->mMovementStates.mHighWord >> (0x14)) & 1)) {
        r = true;
    }
    return r;
}

void MarioActor::fn_803C34F0() {
    if (this->_92C != 0) {
        this->_92C -= 1;
        if (this->_92C == 0) {
            this->decreaseLifeByOne();
            this->fn_803D9740();
        }
    }
    if (this->mHealth <= 1) {
        if (!MR::isDemoActive() && !MR::isPowerStarGetDemoActive() && !MR::isGalaxyDarkCometAppearInCurrentStage() && MR::isPermitSE() &&
            this->isEnableNerveChange()) {
            this->mMario->mStateFlags._2 = true;
        }
    }
    this->mMario->fn_8038C540();
}

void MarioActor::fn_803C38D0() {
    if (this->fn_803C3930()) {
        this->mMario->mPosHolder->fn_80405F90(this->mPosition);
        this->mMario->checkEnforceMove();
        this->mPosition = this->mMario->mPosHolder->mPos;
    }
}

bool MarioActor::fn_803C3930() {
    Mario* m = this->mMario;
    bool r = false;
    if (((m->mMovementStates.mLowWord >> (0x1E)) & 1)) {
        MtxPtr prev = m->_45C->getPrevBaseMtx()->mMtx;
        if (!MR::isSameMtx((this->mMario)->_45C->getBaseMtx()->mMtx, prev)) {
            r = true;
        }
    }
    return r;
}

void MarioActor::fn_803C3C90() {
    if (this->_3DA != 0) {
        this->_3DA -= 1;
        if (this->_3DA == 0) {
            MR::endDemo(this, "マリオスーパー化");
            MR::releaseAnimFrame(this);
            if (this->mMario->isPlayerModeYoshi()) {
                MR::releaseAnimFrame(mYoshiHolder->mYoshi);
            }
            MR::explainLifeUpIfAtFirst();
        }
    }
}

void MarioActor::fn_803C3D10() {
    if (this->_3D8 != 0) {
        if (mPlayerMode == 6) {
            if (mMario->mVerticalSpeed < mConst->getTable()->mTeresaWaitHeight) {
                this->mPosition -= *this->mMario->getGravityVec();
            }
        }
        if (!this->fn_803C3DF0() && !this->mMario->fn_803B26C0()) {
            this->fn_803C3E40();
        }
        this->_3D8 -= 1;
        if (this->_3D8 == 0 && !this->mMario->fn_803B26C0()) {
            this->fn_803C4020();
        }
        this->_3DF = 1;
        return;
    }
    this->mMario->updateMorphResetTimer();
}

bool MarioActor::fn_803C3DF0() {
    switch (mPlayerMode) {
    case 4:
        return MR::isOnGameEventFlagBeeMarioAtFirst();
    case 6:
        return MR::isOnGameEventFlagTeresaMarioAtFirst();
    case 5:
        return MR::isOnGameEventFlagHopperMarioAtFirst();
    case 2:
        return MR::isOnGameEventFlagFireMarioAtFirst();
    case 3:
        return MR::isOnGameEventFlagIceMarioAtFirst();
    case 7:
        return MR::isOnGameEventFlagFlyingMarioAtFirst();
    case 12:
        return MR::isOnGameEventFlagCloudMarioAtFirst();
    case 13:
        return MR::isOnGameEventFlagRockMarioAtFirst();
    }
    return true;
}

void MarioActor::fn_803C3E40() {
    TVec3f front;
    if (_4A4 != nullptr) {
        MR::calcFrontVec(&front, MR::getSensorHost(_4A4));
    } else if (mSensor794 != nullptr) {
        MR::calcFrontVec(&front, MR::getSensorHost(mSensor794));
    } else {
        this->getFrontVec(&front);
    }
    this->mMario->setFrontVecKeepUp(front);
    if (this->mFirstTransformCameraLatch666 == 0) {
        MR::zoomInTargetGameCamera();
        CameraTargetArg arg(nullptr, nullptr, nullptr, this);
        MR::startGlobalEventCamera("変身初出カメラ", arg, 60);
        TVec3f up = -*this->mMario->getGravityVec();
        TVec3f base(this->_240);
        f32 dist = 500.0f;
        f32 height = 50.0f;
        if (mPlayerMode == 6) {
            dist = 700.0f;
            height = 150.0f;
        }
        Mario* m = this->mMario;
        u32 st = (m)->mMovementStates.mLowWord;
        if (((st >> 0x14) & 1) && !((st >> 0x1E) & 1)) {
            base += (m)->mJumpVec;
        }
        up.scale(height);
        TVec3f eye(base);
        eye += up;
        TVec3f dir(front);
        dir.scale(dist);
        TVec3f at(base);
        at += dir;
        TVec3f tgt(at);
        tgt += up;
        TVec3f upv = -mAppliedGravityVec;
        MR::setProgrammableCameraParam("変身初出カメラ", eye, tgt, upv, true);
    }
    this->mFirstTransformCameraLatch666 = 1;
}

void MarioActor::fn_803C4020() {
    if (this->mTransforming != 0) {
        MR::endDemo(this, "マリオ変身");
        this->mTransforming = 0;
    }
    MR::releaseAnimFrame(this);
    this->_3DE = 1;
    switch (mPlayerMode) {
    case 4:
        MR::explainBeeMarioIfAtFirst();
        break;
    case 6:
        MR::explainTeresaMarioIfAtFirst();
        break;
    case 5:
        MR::explainHopperMarioIfAtFirst();
        break;
    case 2:
        MR::explainFireMarioIfAtFirst();
        break;
    case 3:
        MR::explainIceMarioIfAtFirst();
        break;
    case 7:
        MR::explainFlyingMarioIfAtFirst();
        break;
    case 1:
        MR::explainInvincibleMarioIfAtFirst();
        break;
    case 12:
        MR::explainCloudMarioIfAtFirst();
        break;
    case 13:
        MR::explainRockMarioIfAtFirst();
        break;
    }
    if (this->mFirstTransformCameraLatch666 != 0) {
        this->_338 = 10;
        this->mFirstTransformCameraLatch666 = 0;
    }
}

void MarioActor::fn_803C4100() {
    if (this->_338 != 0) {
        this->_338 -= 1;
        if (this->_338 == 0) {
            MR::zoomOutTargetGameCamera();
            MR::endGlobalEventCamera("変身初出カメラ", 60, true);
        }
    }
}

void MarioActor::calcAnimPlayerMode() {
    if (this->_3DF == 0) {
        this->_3DE = 0;
        return;
    }
    u16 mode;
    u16 t = this->_3D8;
    if ((s32)(t & 7) < ((s32)t >> 3)) {
        mode = this->_3D6;
    } else {
        mode = mPlayerMode;
    }
    this->fn_803D8A70(mode);
    switch (mode) {
    case 1:
        mAllModel->fn_803E7880(4);
        _9C0->calcAnim();
        if (this->_3DE != 0) {
            this->_A6E = 2;
        }
        break;
    case 4:
        mAllModel->fn_803E7880(2);
        break;
    case 5:
        this->_3DF = 0;
        mAllModel->fn_803E7880(5);
        break;
    case 3:
        mAllModel->fn_803E7880(3);
        break;
    case 12:
        mAllModel->fn_803E7880(7);
        break;
    case 13:
        mAllModel->fn_803E7880(8);
        break;
    default: {
        mAllModel->fn_803E7880(0);
        u32 frame = 0;
        switch (mode) {
        case 7:
            frame = 1;
            break;
        case 2:
            frame = 2;
            break;
        }
        MR::startBtp(this, "ColorChange");
        MR::setBtpFrameAndStop(this, (f32)frame);
        break;
    }
    }
    this->_3DF = 0;
    this->_3DE = 0;
}

void MarioActor::calcAnimHorrorBone() {
    if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
        Mario* m = this->mMario;
        if (m->mCometRestrictions != 0) {
            if ((mAllModel)->mCurrentModel == 6) {
                mAllModel->fn_803E7B80();
                this->playEffect("ホラーマリオ解除");
            }
        } else if (MR::isPowerStarGetDemoActive()) {
            if ((mAllModel)->mCurrentModel == 6) {
                mAllModel->fn_803E7B80();
            }
        } else {
            MarioAllModel* p = mAllModel;
            if (p->mCurrentModel != 6) {
                p->mPreviousModel = p->mCurrentModel;
                this->playEffect("ホラーマリオ変身");
            }
            mAllModel->fn_803E7880(6);
        }
    } else {
        if (this->mMario->isStatusActive(0xB)) {
            MarioAllModel* p = mAllModel;
            if (p->mCurrentModel != 1) {
                p->mPreviousModel = p->mCurrentModel;
            }
            if ((this->_37C & 3) && (this->isNerve(&NrvMarioActor::MarioActorNrvWait::sInstance) || this->isNerve(&NrvMarioActor::MarioActorNrvNoRush::sInstance))) {
                mAllModel->fn_803E7880(1);
                return;
            }
            mAllModel->fn_803E7B80();
            return;
        }
        if ((mAllModel)->mCurrentModel == 1 && this->mHealth != 0) {
            mAllModel->fn_803E7B80();
        }
    }
}

bool MarioActor::fn_803C4F70() {
    bool r = false;
    if (this->_EA5 == 0 && this->_EA4 != 0 && this->_EA6 != 0) {
        r = true;
    }
    return r;
}

bool MarioActor::fn_803C4FB0() {
    bool r = false;
    bool a = false;
    if (mPlayerMode == 4 && !this->mMario->isStatusActive(0x16) && !this->mMario->isStatusActive(0x15) && !this->mMario->isStatusActive(0x1B)) {
        Mario* m = this->mMario;
        if (!((m->mMovementStates.mHighWord >> (0x1C)) & 1) && !((m->mMovementStates.mLowWord >> (0x15)) & 1)) {
            a = true;
        }
    }
    if (a) {
        bool b = false;
        u32 st = (this->mMario)->mMovementStates.mLowWord;
        if (((st >> 0x14) & 1) && (st >> 0x1F) != 0) {
            b = true;
        }
        if (!b) {
            r = true;
        }
    }
    return r;
}


// NONMATCHING: saved-register allocation, stack slots and equivalent scheduling.
void MarioActor::fn_803C5090(MtxPtr mtx) {
    mGravityReferenceED0 = (this->mMario)->mAirGravityVec;
    f32 k = mConst->getTable()->mBeePoseHeadToFootLength;
    TVec3f grav = -*this->mMario->getGravityVec();
    TVec3f t1(grav);
    t1.scale(k);
    MtxPtr m1 = MR::tmpMtxTrans(-t1);
    f32 k2 = mConst->getTable()->mBeePoseHeadToFootLength;
    TVec3f e(mGravityReferenceED0);
    e.scale(k2);
    TVec3f p(this->mPosition);
    p -= e;
    TVec3f d(this->_33C);
    f32 angle = 0.0f;
    d -= p;
    TVec3f axis;
    if (!MR::normalizeOrZero(&d)) {
        PSVECCrossProduct((Vec*)&(this->mMario)->mFrontVec, (Vec*)&d, (Vec*)&axis);
        if (!MR::normalizeOrZero(&axis)) {
            f32 ang = MR::acosEx(mGravityReferenceED0.dot(d));
            f32 lim = mConst->getTable()->mBeePoseDelayAngleAir;
            if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1)) {
                lim = mConst->getTable()->mBeePoseDelayAngleGround;
            }
            angle = MR::clamp(ang, 0.0f, lim);
        }
    }
    Mtx rot;
    PSMTXRotAxisRad(rot, (Vec*)&axis, angle);
    PSMTXConcat(m1, mtx, mtx);
    PSMTXConcat(rot, mtx, mtx);
    PSMTXConcat(MR::tmpMtxTrans(t1), mtx, mtx);
    f32 k3 = mConst->getTable()->mBeePoseDelayAccel;
    TVec3f s110(mGravityReferenceED0);
    s110.scale(k3);
    MR::vecKillElement(s110, this->_360, &s110);
    TVec3f b0(this->_354);
    b0 += s110;
    this->_354 = b0;
    TVec3f a4(this->_33C);
    a4 += this->_354;
    this->_33C = a4;
    f32 k4 = mConst->getTable()->mBeePoseHeadToFootLength;
    TVec3f s80(mGravityReferenceED0);
    s80.scale(k4);
    TVec3f s8c(this->mPosition);
    s8c -= s80;
    TVec3f s98(this->_33C);
    s98 -= s8c;
    this->_360 = s98;
    TVec3f s104(this->_360);
    TVec3f sF8(this->_360);
    MR::normalizeOrZero(&sF8);
    f32 ang2 = MR::acosEx(mGravityReferenceED0.dot(sF8));
    f32 lim2 = mConst->getTable()->mBeePoseLimitAngleAir;
    if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1)) {
        lim2 = mConst->getTable()->mBeePoseLimitAngleGround;
    }
    if (ang2 > lim2) {
        TVec3f sEC;
        PSVECCrossProduct((Vec*)&mGravityReferenceED0, (Vec*)&sF8, (Vec*)&sEC);
        if (MR::normalizeOrZero(&sEC)) {
            this->_360 = mGravityReferenceED0;
        } else {
            Mtx rot2;
            PSMTXRotAxisRad(rot2, (Vec*)&sEC, lim2);
            PSMTXMultVec(rot2, (Vec*)&mGravityReferenceED0, (Vec*)&this->_360);
        }
    }
    this->_360.setLength(mConst->getTable()->mBeePoseHeadToFootLength);
    if (PSVECMag((Vec*)&s104) > mConst->getTable()->mBeePoseHeadToFootLength) {
        TVec3f s68(s104);
        s68 -= this->_360;
        TVec3f n = -s68;
        this->_354 += n;
    }
    f32 k5 = mConst->getTable()->mBeePoseHeadToFootLength;
    TVec3f s44(mGravityReferenceED0);
    s44.scale(k5);
    TVec3f s50(this->mPosition);
    s50 -= s44;
    TVec3f s5c(s50);
    s5c += this->_360;
    this->_33C = s5c;
    MR::normalizeOrZero(&this->_360);
    if (PSVECMag((Vec*)&this->_354) > mConst->getTable()->mBeePoseDelaySpeedLimit) {
        this->_354.setLength(mConst->getTable()->mBeePoseDelaySpeedLimit);
    }
    f32 sc = mConst->getTable()->mBeePoseFrictionStop;
    if (mMario->mTargetWalkSpeedIndex == 0 && !this->isJumping()) {
        sc = mConst->getTable()->mBeePoseFrictionMove;
    }
    this->_354.scale(sc);
    if (!MR::isSameDirection(this->_360, (this->mMario)->mFrontVec, 0.01f)) {
        TVec3f neg = -this->_360;
        MR::makeMtxUpFront((TPos3f*)mtx, neg, (this->mMario)->mFrontVec);
    } else if (!MR::isSameDirection((this->mMario)->_1FC, (this->mMario)->mFrontVec, 0.01f)) {
        MR::makeMtxUpFront((TPos3f*)mtx, (this->mMario)->_1FC, (this->mMario)->mFrontVec);
    } else {
        MR::makeMtxUpFront((TPos3f*)mtx, (this->mMario)->mHeadVec, (this->mMario)->mFrontVec);
    }
    if (mMario->mTargetWalkSpeedIndex > 2 || this->isJumping()) {
        f32 inv = 1.0f - mConst->getTable()->mBeePoseTransBlendingRatioMove;
        TVec3f s8(this->_33C);
        s8 -= this->mPosition;
        TVec3f s14(s8);
        s14.scale(inv);
        f32 k6 = mConst->getTable()->mBeePoseTransBlendingRatioMove;
        TVec3f s20(this->_348);
        s20.scale(k6);
        TVec3f s2c(s20);
        s2c += s14;
        this->_348 = s2c;
        MR::setMtxTrans(mtx, this->_348.x, this->_348.y, this->_348.z);
        return;
    }
    this->_348.scale(mConst->getTable()->mBeePoseTransBlendingRatioStop);
    MR::setMtxTrans(mtx, this->_348.x, this->_348.y, this->_348.z);
}

void MarioActor::fn_803C56F0() {
    this->_354.zero();
    this->_348.zero();
    this->_33C = this->mPosition;
    mGravityReferenceED0 = *this->mMario->getGravityVec();
}

bool MarioActor::fn_803C5750() {
    bool r = false;
    if (_934 != 0) {
        r = _924->receiveMessage(0xA1, this->getSensor("Body"));
    }
    if (this->_EA5 != 0) {
        PSMTXCopy(mRemoteDemoMtx4E4.mMtx, this->getBaseMtx());
    }
    if (r || this->_EA5 != 0) {
        this->fn_803C58D0();
        return true;
    }
    if (_934 != 0) {
        this->fn_803C5920();
    }
    return false;
}


void MarioActor::fn_803C5820() {
    TVec3f old(this->mPosition);
    MR::extractMtxTrans(this->getBaseMtx(), &this->mPosition);
    this->mMario->invalidateRelativePosition();
    TVec3f diff(this->mPosition);
    diff -= old;
    this->_938 = diff;
    if (((this->mMario->mMovementStates.mHighWord >> (8)) & 1)) {
        (this->mMario)->_688 = this->mPosition;
    }
    this->updateGravityVec(true, true);
    this->mMario->updateGroundInfo();
}

void MarioActor::fn_803C58D0() {
    this->fn_803C59C0();
    this->fn_803C5820();
    if (this->_EA4 != 0) {
        this->_EA6 = 1;
    }
}

void MarioActor::fn_803C5920() {
    TVec3f cur;
    MR::extractMtxTrans(this->getBaseMtx(), &cur);
    TVec3f diff(this->mPosition);
    diff -= cur;
    this->_938 = diff;
}

bool MarioActor::fn_803C5990() {
    if (this->_1C0 == 0) {
        this->_A0C = 1;
        return false;
    }
    this->_A0C = 0;
    this->_1C0 = 0;
    return true;
}

void MarioActor::fn_803C59C0() {
    TVec3f front, side, head;
    TPos3f* bm = (TPos3f*)this->getBaseMtx();
    bm->getZDir(front);
    this->mMario->setFrontVec(front);
    ((TPos3f*)this->getBaseMtx())->getXDir(side);
    this->mMario->setSideVec(side);
    ((TPos3f*)this->getBaseMtx())->getYDir(head);
    this->mMario->forceSetHeadVecKeepSide(head);
    (this->mMario)->mWalkSpeed = 0.0f;
    (this->mMario)->mTargetWalkSpeedIndex = 0;
    this->mMario->stopJump();
    (this->mMario)->_334 = (this->mMario)->mFrontVec;
    if (this->mMario->isSwimming()) {
        this->mMario->mSwim->resetAndFixPose();
    }
}


void MarioActor::fn_803C5AB0(MtxPtr mtx) {
    if (this->_390 != 0) {
        this->fn_803C5B30(mtx);
    } else if (this->_394 != 0) {
        this->fn_803C5E20(mtx);
    } else if (!((this->mMario->mMovementStates.mHighWord >> (3)) & 1)) {
        this->_3B0 = 1.0f;
    }
    u32 t = this->_398;
    if (t != 0) {
        this->_398 = t - 1;
    }
}

// NONMATCHING: Typed scalar members use direct offsets instead of a saved field address.
void MarioActor::fn_803C5B30(MtxPtr mtx) {
    s32 mode = static_cast<s32>(mCounter6C8);
    reinterpret_cast<s32&>(this->_394) = 0x1E;
    switch (mode) {
    case 0:
    case 2: {
        u32 cnt = this->_390;
        if (cnt > 0xF) {
            f32 a = this->mMario->calcDistToCeil(false);
            f32 b = this->mMario->calcDistToCeilOnPress();
            f32 v1 = MR::clamp(a / 150.0f, 0.2f, 1.0f);
            if (this->_3B0 > v1) {
                this->_3B0 = v1;
            } else {
                f32 v2 = MR::clamp(b / 150.0f, 0.2f, 1.0f);
                if (this->_3B0 > v2) {
                    this->_3B0 = v2;
                }
                MR::setMtxTrans(mtx, this->mPosition.x, this->mPosition.y, this->mPosition.z);
                if ((this->mMario)->_960 != 0x1B) {
                    this->_1E0 = 1;
                }
            }
        } else {
            this->_3B0 = 0.2f + ((0.8f * (f32)(0xF - cnt)) / 15.0f);
        }
        u32 t = this->_398;
        if (t != 0) {
            this->_3B0 = this->_3B0 + 0.02f * (f32)t;
            return;
        }
        return;
    }
    case 1:
    case 3: {
        u32 cnt = this->_390;
        if (cnt > 0xF) {
            f32 w = this->mMario->calcDistWidth();
            this->mPosition = this->mMario->mPosHolder->mPos;
            f32 v = MR::clamp(w / 80.0f, 0.2f, 1.0f);
            if (this->_3B0 > v) {
                this->_3B0 = v;
            } else {
                f32 t2 = this->_3B0 - 0.01f;
                this->_3B0 = t2;
                this->_3B0 = MR::clamp(t2, 0.2f, 1.0f);
            }
        } else {
            this->_3B0 = 0.2f + ((0.8f * (f32)(0xF - cnt)) / 15.0f);
        }
        u32 t = this->_398;
        if (t != 0) {
            this->_3B0 = this->_3B0 + 0.02f * (f32)t;
            return;
        }
        break;
    }
    case 4:
        this->_3B0 = 0.2f + ((0.8f * (160.0f * (1.0f - JMACosRadian(0.5f * (((f32)this->_390 / 120.0f) * 3.1415927f))))) / 160.0f);
        break;
    }
}

void MarioActor::fn_803C5E20(MtxPtr mtx) {
    u32 t = this->_394 - 1;
    this->_394 = t;
    this->_3B0 = 1.0f + (((0.15f * (f32)this->_394) / 30.0f) * JMASinRadian((2.0f * ((f32)t * 3.1415927f)) / 15.0f));
}

void MarioActor::fn_803C5ED0(MtxPtr m1, MtxPtr m2) {
    u16 t = this->mBlendMtxTimer;
    if (t != 0) {
        f32 inverseBlend = 1.0f - this->_EA0;
        inverseBlend *= (f32)(t - 1) / (f32)t;
        f32 f = 1.0f - inverseBlend;
        this->_EA0 = f;
        MR::blendMtx(m2, m1, f, m1);
        this->mBlendMtxTimer = this->mBlendMtxTimer - 1;
    }
}

bool MarioState::passRing(const HitSensor*) {
    return false;
}

namespace MR {
    bool isCreatePlayerResourceFire();
    bool isCreatePlayerResourceYoshi();
    bool isCreatePlayerResourceCloud();
    bool isCreatePlayerResourceRock();
}















void MultiEmitter::setGlobalScale(f32 scale, s32 n) {
    TVec3f v(scale, scale, scale);
    this->setGlobalScale(v, n);
}











void MarioActor::initFireMario() {
    if (MR::isCreatePlayerResourceFire()) {
        this->initFireBall();
        this->_3D2 |= 4;
    }
}

void MarioActor::initYoshi() {
    if (MR::isCreatePlayerResourceYoshi()) {
        mYoshiHolder = new YoshiHolder(this->mMario);
        mYoshiHolder->fn_8041B700(this);
        this->_3D2 |= 0x400;
    }
}

void MarioActor::initCloudMario() {
    if (MR::isCreatePlayerResourceCloud()) {
        const char* name = "CloudMario";
        if (gIsLuigi) {
            name = "CloudLuigi";
        }
        ModelHolder* mh = new ModelHolder(name, true);
        this->mModelEEC = mh;
        mh->initWithoutIter();
        mAllModel->registerModel(7, mModelEEC, true, true);
        mModelEEC->kill();
        MR::hideModel(mModelEEC);
        MR::hideJoint(mModelEEC, "Cap0");
        MR::hideJoint(mModelEEC, "Face0");
        CloudMarioHat* hat = new CloudMarioHat("雲マリオの帽子");
        mCloudMarioHatEF0 = hat;
        hat->initWithoutIter();
        this->_3D2 |= 0x1000;
    }
}

void MarioActor::initRockMario() {
    if (MR::isCreatePlayerResourceRock()) {
        const char* name = "RockMario";
        if (gIsLuigi) {
            name = "RockLuigi";
        }
        ModelHolder* mh = new ModelHolder(name, true);
        this->mModelEF4 = mh;
        mh->initWithoutIter();
        mAllModel->registerModel(8, mModelEF4, true, true);
        mModelEF4->kill();
        MR::hideModel(mModelEF4);
        MR::hideJoint(mModelEF4, "Face0");
        this->_3D2 |= 0x2000;
    }
}

void MarioActor::fn_803C8AA0() {
    J3DModelX* m = (J3DModelX*)mAllModel->getJ3DModel();
    if (this->mFlags.mIsNoCalcView == 0) {
        m->viewCalc3(0, nullptr);
        Mtx inv1, inv2;
        PSMTXInverse(mJointTransform294.mMtx, inv1);
        PSMTXInverse(mAllModel->getJ3DModel()->getBaseTRMtx(), inv2);
        PSMTXConcat(inv1, inv2, mReflectionBaseMtx264.mMtx);
    }
}

void J3DModelX::swapDrawBuffer(u32 a) {
    if (this->_DC == a) {
        this->_DC = 0;
        return;
    }
    this->_DC = (u8)a;
}




// NONMATCHING: scalar literal symbol identities; instruction words match after relocation masking.
bool MarioActor::fn_803C9300() {
    bool c = this->mMario->isCeiling();
    if ((c || ((this->mMario->mDrawStates.mWord >> (0xA)) & 1)) && this->mBeeWallWalk == 0 && this->_9F2 == 0) {
        Mario* m = this->mMario;
        bool r = false;
        if (((m->mDrawStates.mWord >> (0xA)) & 1)) {
            r = true;
        } else {
            const char* wallCode = MR::getWallCodeString(m->_4C8);
            if (wallCode != nullptr && strcmp(wallCode, "Fur") == 0) {
                r = true;
            }
        }
        if (r) {
            Triangle tri;
            TVec3f hit;
            Mario* m2 = this->mMario;
            TVec3f dir(TVec3f(-mAppliedGravityVec));
            dir.scale(200.0f);
            if (m2->fn_803B2770(&hit, &tri, this->mPosition, dir)) {
                this->entryWallWalkMode(hit, *tri.getNormal(0));
            }
        }
    }
    return false;
}

void MarioActor::fn_803C9920(TVec3f& vec) {
    if (mPlayerMode != 4) {
        this->mBeeWallWalk = 0;
        this->_9F2 = 0;
        return;
    }
    u16 timer = this->_9F2;
    u8 prev = this->mBeeWallWalk;
    if (timer != 0) {
        this->_9F2 = timer - 1;
    }
    if (this->_9F2 != 0 || (!this->fn_803C9300() && !this->checkBeeWallStick(vec) && !this->checkBeeFloorStick(vec))) {
        this->updateBeeStickMode(vec);
    }
    u8 cur = this->mBeeWallWalk;
    if (cur == 5) {
        MR::vecBlendSphere(&vec, mAppliedGravityVec, vec, 0.1f);
        return;
    }
    if (cur != 0) {
        MR::vecBlendSphere(&vec, mAppliedGravityVec, vec, 0.3f);
        return;
    }
    if (prev != 0) {
        this->_9F2 = 0x3C;
    }
}




void J3DModelX::setDynamicDL(u8* dl, u32 size) {
    if (dl == nullptr) {
        size = 0;
    }
    _1B8 = dl;
    _1BC = size;
}

// NONMATCHING: Matrix-address reuse, equivalent bit tests, and boolean-store scheduling.
// NONMATCHING: initial zero-register reuse, load scheduling and literal identities.

