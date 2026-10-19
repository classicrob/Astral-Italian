// SMG2-only Mario:: functions (fn_ range B), written from the asm. Included at the end of Mario.cpp (unity TU).


void Mario::fn_803A2D50() {
    if (this->_436 == 0) {
        this->_436 = 0;
        return;
    }
    this->_436--;
}

bool Mario::fn_803A3130() {
    if (this->_420 == 0) {
        return false;
    }
    this->_420--;
    this->mFlags14.mHighWord |= 0x800000;
    return true;
}

void Mario::fn_803A96D0(f32 arg1) {
    MR::skipBas(this->mActor, arg1);
}

bool Mario::fn_803AA2C0() {
    return false;
}

bool Mario::fn_803AC810() {
    return this->mTeresa->isTeresaAccel();
}

void Mario::fn_803B1710() {
    this->mFlags14.mHighWord &= 0xFFCFFFFF;
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

// NONMATCHING: floating-point register allocation and independent load scheduling only.
void fn_803B21D0(const MarioRotationMatrix* matrix, TQuat4f& rDest) {
    if (matrix->mMtx[0][0] + matrix->mMtx[1][1] + matrix->mMtx[2][2] >= 0.0f) {
        f32 root = matrix->mMtx[0][0] + matrix->mMtx[1][1] + matrix->mMtx[2][2];
        root += 1.0f;
        root = JGeometry::TUtil< f32 >::sqrt(root);
        f32 scale = 0.5f / root;
        rDest.w = 0.5f * root;
        rDest.x = scale * (matrix->mMtx[2][1] - matrix->mMtx[1][2]);
        rDest.y = scale * (matrix->mMtx[0][2] - matrix->mMtx[2][0]);
        rDest.z = scale * (matrix->mMtx[1][0] - matrix->mMtx[0][1]);
    } else {
        f32 maximum = fn_80386D30(matrix->mMtx[0][0], matrix->mMtx[1][1], matrix->mMtx[2][2]);

        if (maximum == matrix->mMtx[0][0]) {
            f32 root = JGeometry::TUtil< f32 >::sqrt(1.0f + (matrix->mMtx[0][0] - (matrix->mMtx[1][1] + matrix->mMtx[2][2])));
            f32 scale = 0.5f / root;
            rDest.x = 0.5f * root;
            rDest.y = scale * (matrix->mMtx[0][1] + matrix->mMtx[1][0]);
            rDest.z = scale * (matrix->mMtx[2][0] + matrix->mMtx[0][2]);
            rDest.w = scale * (matrix->mMtx[2][1] - matrix->mMtx[1][2]);
        } else if (maximum == matrix->mMtx[1][1]) {
            f32 root = JGeometry::TUtil< f32 >::sqrt(1.0f + (matrix->mMtx[1][1] - (matrix->mMtx[2][2] + matrix->mMtx[0][0])));
            f32 scale = 0.5f / root;
            rDest.y = 0.5f * root;
            rDest.z = scale * (matrix->mMtx[1][2] + matrix->mMtx[2][1]);
            rDest.x = scale * (matrix->mMtx[0][1] + matrix->mMtx[1][0]);
            rDest.w = scale * (matrix->mMtx[0][2] - matrix->mMtx[2][0]);
        } else {
            f32 root = JGeometry::TUtil< f32 >::sqrt(1.0f + (matrix->mMtx[2][2] - (matrix->mMtx[0][0] + matrix->mMtx[1][1])));
            f32 scale = 0.5f / root;
            rDest.z = 0.5f * root;
            rDest.x = scale * (matrix->mMtx[2][0] + matrix->mMtx[0][2]);
            rDest.y = scale * (matrix->mMtx[1][2] + matrix->mMtx[2][1]);
            rDest.w = scale * (matrix->mMtx[1][0] - matrix->mMtx[0][1]);
        }
    }
}

void Mario::fn_803B3010() {
    this->mMovementStates.mLowWord &= 0xFFFFFBFF;
    this->mMovementStates.mHighWord &= 0xFFFFFFBF;
    this->_47C->mIdx = -1;
    this->_4E0 = 0.0f;
}


void Mario::fn_803A2FC0() {
    this->saveLastSafetyTrans();
    this->tryJump();
    if (this->isAnimationRun("その場足踏み上半身")) {
        this->stopAnimationUpper(nullptr, nullptr);
    }
    this->_420 = 0;
}

void Mario::fn_803A42B0() {
    if (this->getPlayer()->mMovementStates.mHighWord >> 10 & 1) {
        this->changeAnimation("ブレーキ滑り床", (const char*)nullptr);
        return;
    }
    this->changeAnimation("ブレーキ", (const char*)nullptr);
}

bool Mario::fn_803A9690(const char* pName) {
    if (this->_970 == nullptr) {
        return false;
    }
    return strcmp(this->_970, pName) == 0;
}

bool Mario::fn_803AA340(const TVec3f*, TVec3f*) {
    this->isStatusActive(0x18);
    return false;
}

bool Mario::fn_803ACE10() {
    bool ret = false;
    if (!(this->mFlags24.mLowWord >> 21 & 1) || !this->isAnimationRun("しゃがみ基本")) {
        ret = true;
    }
    return ret;
}

void Mario::fn_803AD120(const Triangle* pTri) {
    if (this->fn_803EF7C0() == 0x20 && this->mTargetWalkSpeedIndex != 0) {
        this->_3F4 = 0.2f;
    }
}

void Mario::fn_803B14E0() {
    this->mActor->fn_803B9A30();
    this->mFlags14.mHighWord &= 0xFFDFFFFF;
    this->mActor->fn_803B9830()->fn_8041BB90();
    this->changeAnimation("落下", (const char*)nullptr);
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
    if (this->isPlayerModeYoshi() && FLD<s32>(this->mActor->fn_803B97F0(), offsetof(Yoshi, mDashTimer)) > 0) {
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
    if (this->isPlayerModeYoshi() && FLD<s32>(this->mActor->fn_803B97F0(), offsetof(Yoshi, mDashTimer)) > 0) {
        yoshi = true;
    }
    if (yoshi && !(this->getPlayer()->mFlags14.mHighWord >> 21 & 1)) {
        ret = true;
    }
    return ret;
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

f32 Mario::fn_803B3870() {
    if (this->isStatusActive(1)) {
        return -0.9f;
    }
    return -0.999f;
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

bool Mario::fn_803B4850(const HitSensor* pSensor) const {
    if (MR::getSensorHost(pSensor) != nullptr) {
        return MR::isEqualString(MR::getSensorHost(pSensor)->mName, "溶岩分離滝");
    }
    return false;
}



s32 Mario::fn_803A2D80() {
    if ((this->mFlags14.mLowWord >> 5) & 1) {
        this->tryJump();
        if (this->isAnimationRun("その場足踏み上半身")) {
            this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
        }
        this->_420 = 0;
        this->mJumpVec = this->_184;
        this->mFlags14.mLowWord &= 0xFFFFFFDF;
        return 1;
    }
    if (!this->mActor->isRequestJump() && !((this->mFlags20.mWord >> 21) & 1) && !((this->mMovementStates.mHighWord >> 7) & 1)) {
        return 0;
    }
    if (((this->mMovementStates.mLowWord >> 21) & 1) && this->calcDistToCeil(false) < 160.0f) {
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

s32 Mario::fn_803A3030() {
    if (!((this->mMovementStates.mLowWord >> 30) & 1)) {
        if (this->_3C0 != 0 && !((this->mMovementStates.mHighWord >> 28) & 1) && MR::isNearZero(this->_8F8, 0.001f)) {
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
    } else if ((this->mDrawStates.mWord >> 21) & 1) {
        this->_3C0 = 0;
    } else if (this->_3C0 < entry->mDropWaitTime) {
        this->_3C0 = entry->mDropWaitTime;
    }
    return 0;
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
    if (this->mActor->isRequestSpin() && (this->mMovementStates.mLowWord >> 31) != 0 && this->mWorldPadDir.dot(this->_16C) < 0.0f) {
        difference = 1.0f;
    }
    if ((this->mFlags14.mLowWord >> 20) & 1) {
        difference = 1.0f;
    }
    return difference;
}

bool Mario::fn_803A4210() {
    if ((this->getPlayer()->mMovementStates.mLowWord >> 16) & 1) {
        return false;
    }
    if (!((this->getPlayer()->mMovementStates.mLowWord >> 15) & 1)) {
        return false;
    }
    if ((this->getPlayer()->mMovementStates.mLowWord >> 21) & 1) {
        return false;
    }
    if (this->_8F0 != 0.0f) {
        return false;
    }
    return static_cast<u8>(this->checkStickFrontBack()) == 2;
}

void Mario::startBas(const char* pName, bool loop, f32 startFrame, f32 speed) {
    if (this->mActor->mSoundHolder != 0) {
        MR::startBas(this->mActor, pName, loop, startFrame, speed, false);
        this->_970 = pName;
    }
}

bool Mario::taskOnHipDropBlurHopper(u32) {
    if (!((getPlayer()->mMovementStates.mLowWord >> 20) & 1U) || ((mMovementStates.mLowWord >> 30) & 1U) || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("ホッパー尻落ルイージ");
        } else {
            stopEffect("ホッパー尻落");
        }
        return false;
    }
    return true;
}

// NONMATCHING: explicit null materialization for an unused Triangle argument.
void Mario::fn_803ACC40() {
    this->_3F4 = 0.0f;
    if (!((this->mMovementStates.mLowWord >> 21) & 1)) {
        this->fn_803ACD80();
        return;
    }
    if (this->isAnimationRun("壁押し", static_cast<u32>(0))) {
        this->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
    this->fn_803ACCE0();
    this->fn_803AD120(nullptr);
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
    this->mMovementStates.mHighWord &= 0xFDFFFFFF;
}

namespace {
    const f32 cSquatWalkStoppedWeights[4] = {1.0f, 0.0f, 0.0f, 0.0f};
    const f32 cSquatWalkMovingWeights[4] = {0.0f, 1.0f, 0.0f, 0.0f};
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
    this->mMovementStates.mHighWord &= 0xFDFFFFFF;
    if (this->isPlayerModeYoshi()) {
        if (!((this->mFlags24.mLowWord >> 21) & 1)) {
            MR::startActionSound(this->mActor->fn_803B97F0(), "Squat", -1, -1, -1);
        }
    } else if (((this->mMovementStates.mLowWord >> 31) == 0) && !this->isAnimationRun("サマーソルト")) {
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
            mMovementStates.mHighWord |= 0x02000000;
        } else {
            fn_803AF2F0(cSquatWalkStoppedWeights);
            mTargetWalkSpeedIndex = 0;
            mMovementStates.mHighWord &= 0xFDFFFFFF;
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
        f32 minSpeed = table->mSquatWalkMinSpeed;
        f32 speedRange = table->mSquatWalkMaxSpeed - minSpeed;
        mWalkSpeed = minSpeed + mStickPos.z * speedRange;
        fn_803AF2F0(cSquatWalkMovingWeights);
        mTargetWalkSpeedIndex = 1;
    }
}

void Mario::fn_803AD0B0() {
    f32 var_f31 = this->mWalkSpeed * (60.0f / this->mActor->mConst->getTable()->mSquatWalkStep);
    if (!((this->mMovementStates.mHighWord >> 25) & 1)) {
        var_f31 = 1.0f;
    }
    this->getAnimator()->setSpeed(var_f31);
}

// NONMATCHING: equivalent direct exit replaces the target byte-gate trampoline.
void Mario::fn_803AD4F0() {
    if (isPlayerModeYoshi() && mTargetWalkSpeedIndex != 0 &&
        ((getPlayer()->mMovementStates.mLowWord >> 30) & 1) && _A7F != 0) {
        f32 frame = getAnimator()->getFrame();
        f32 previousFrame = mPrevAnimFrame;
        if (previousFrame > frame || (previousFrame < 30.0f && getAnimator()->getFrame() >= 30.0f)) {
            playEffect("ヨッシー泥移動");
        }
    }
}

// NONMATCHING: equivalent direct exit replaces the target byte-gate trampoline.
void Mario::fn_803AD9C0() {
    if (isPlayerModeYoshi() && mTargetWalkSpeedIndex != 0 &&
        ((getPlayer()->mMovementStates.mLowWord >> 30) & 1) && _A7F != 0) {
        f32 frame = getAnimator()->getFrame();
        f32 previousFrame = mPrevAnimFrame;
        if (previousFrame > frame || (previousFrame < 30.0f && getAnimator()->getFrame() >= 30.0f)) {
            if (mTargetWalkSpeedIndex >= 2) {
                playEffect("ヨッシー泥移動");
            }
        }
    }
}

// NONMATCHING: animator speed setter selected by the walk-speed table
void Mario::fn_803AF2F0(const f32* pSpeedTable) {
    f32 var_f31;
    if (this->_3FA != 0) {
        var_f31 = 0.5f;
    } else {
        var_f31 = marioCurTableEntry(this)->mWeightBlendRatio;
    }
    this->getAnimator()->setBlendWeight(pSpeedTable, var_f31);
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
    bool grounded = (mMovementStates.mLowWord >> 30) & 1;
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

// NONMATCHING: register allocation.
void Mario::fn_803A7540() {
    if (!((this->mMovementStates.mHighWord >> 28) & 1)) {
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
    if (((this->mMovementStates.mLowWord >> 30) & 1) && ((this->mMovementStates.mLowWord >> 31) == 0)) {
        if (this->isEnableSlopeMove()) {
            this->slopeMove();
        } else {
            this->clearSlope();
        }
    }
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
        this->getPlayer()->mMovementStates.mHighWord |= 0x40000000U;
        this->getPlayer()->mMovementStates.mLowWord &= 0xFFDFFFFFU;
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
        TVec3f sp2C(*this->getGravityVec());
        sp2C *= 20.0f;
        TVec3f sp38(sp68);
        sp38 += sp5C;
        TVec3f sp44(sp38);
        sp44 += sp2C;
        actor->fn_803B97F0()->fn_8040CCD0(sp44);
        f32 t31 = YoshiParam::fn_8041BD80();
        TVec3f sp8(*this->getGravityVec());
        sp8 *= t31;
        TVec3f sp14(sp68);
        sp14 -= sp5C;
        TVec3f sp20(sp14);
        sp20 -= sp8;
        this->mMovementStates.mHighWord |= 0x40000U;
        this->_304 = sp20;
        this->changeAnimation("ヨッシー離脱ジャンプ", "落下");
        this->mActor->setPlayerMode(0, true, true);
        TVec3f jointPosition;
        MR::copyJointPos(this->mActor->fn_803B97F0(), "MarioPos", &jointPosition);
        MR::setPlayerPos(jointPosition);
    }
}

// NONMATCHING: yoshi dismount/landing helpers, rough from the draft (vector math simplified)
void Mario::fn_803B1100() {
    if (this->isPlayerModeYoshi()) {
        this->mMorphResetTimer = 0;
        this->mMovementStates.mHighWord |= 0x40000000;
        this->mActor->mMarioAnim->fn_803DF9D0();
        this->mActor->fn_803B9810()->fn_8041FD80();
    }
}

// NONMATCHING: yoshi jump-release landing approximated
void Mario::fn_803B1180() {
    if (this->isPlayerModeYoshi()) {
        this->mActor->fn_803B9810()->fn_8041FD00();
        TVec3f sp44(*this->getGravityVec());
        sp44 *= -this->mVelocity.dot(*this->getGravityVec());
        TVec3f sp38;
        PSVECCrossProduct(&this->mSideVec, &sp44, &sp38);
        MR::normalizeOrZero(&sp38);
        sp38 *= YoshiParam::fn_8041BD90();
        TVec3f sp8(*this->getGravityVec());
        sp8 *= 20.0f;
        TVec3f sp14(sp44);
        sp14 += sp38;
        TVec3f sp20(sp14);
        sp20 += sp8;
        this->mActor->fn_803B97F0()->fn_8040CCD0(sp20);
        this->mActor->mMarioAnim->fn_803DF9D0();
    }
}

void Mario::fn_803B12B0() {
    if ((this->mMovementStates.mLowWord >> 20) & 1) {
        this->mFlags14.mHighWord &= 0xFFDFFFFF;
        return;
    }
    if (!this->isPlayerModeYoshi()) {
        this->mFlags14.mHighWord &= 0xFFDFFFFF;
        if ((this->mMovementStates.mLowWord >> 30) & 1) {
            this->changeAnimation("基本", (const char*)nullptr);
            return;
        }
        this->changeAnimation("落下", (const char*)nullptr);
        return;
    }
    if ((this->mMovementStates.mLowWord >> 30) & 1) {
        this->mFlags14.mHighWord &= 0xFFDFFFFF;
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
    f32 maxRiseSpeed = YoshiParam::fn_8041BDB0();
    if (this->mActor->fn_803B9A90() < maxRiseSpeed) {
        f32 maxSpeed = YoshiParam::fn_8041BDB0();
        f32 riseSpeed = this->mActor->fn_803B9A90();
        f32 ratio = (YoshiParam::fn_8041BDB0() - riseSpeed) / maxSpeed;
        f32 acceleration = YoshiParam::fn_8041BDA0();
        this->mJumpVec += -this->mActor->mAppliedGravityVec * ratio * acceleration;
    }
    this->mActor->fn_803B9A50();
    if (this->mActor->fn_803B9A70() == 0 || !MR::testCorePadButtonA(0)) {
        this->fn_803B14E0();
    }
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
    if ((this->mFlags14.mHighWord >> 21) & 1) {
        return 0;
    }
    if ((this->mMovementStates.mLowWord >> 30) & 1) {
        return 0;
    }
    if ((this->mMovementStates.mLowWord >> 21) & 1) {
        return 0;
    }
    if (this->_430 - 0xBU <= 2U || this->_430 == 9) {
        if (this->isRising()) {
            this->mMovementStates.mLowWord |= 0x1000;
            return 0;
        }
    }
    if (MR::isNearZero(this->mActor->mAppliedGravityVec, 0.001f)) {
        return 0;
    }
    this->fn_8039A600();
    this->resetTornado();
    this->cancelTornadoJump();
    this->mDrawStates.mWord |= 0x800000;
    this->mMovementStates.mHighWord &= 0xBFFFFFFF;
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

// NONMATCHING: initial status comparison and temporary register allocation.
bool Mario::fn_803B18C0() {
    if (getCurrentStatus() == 0x29) return false;
    if (mActor->_EA4) return false;
    bool actorBusy = false;
    if (mActor->_482 || mActor->_481) actorBusy = true;
    if (actorBusy) return false;
    if (getStickP() != 0.0f || checkLvlA() || checkLvlZ() || mActor->isRequestRush()) return false;
    if (!(getPlayer()->mMovementStates.mLowWord >> 31) && !((getPlayer()->mMovementStates.mLowWord >> 30) & 1)) return false;
    if (getPlayer()->isStatusActive(6)) return false;
    if (getPlayer()->isStatusActive(5)) return false;
    if (getPlayer()->isStatusActive(0x1A)) return false;
    if (getPlayer()->isStatusActive(0x1B)) return false;
    if (getPlayer()->isStatusActive(0x12)) return false;
    if (getPlayer()->isStatusActive(0x11)) return false;
    if (getPlayer()->isStatusActive(0x22)) return false;
    if ((getPlayer()->mFlags20.mWord >> 26) & 1) return false;
    if ((getPlayer()->mMovementStates.mHighWord >> 28) & 1) return false;
    if ((getPlayer()->mMovementStates.mLowWord >> 21) & 1) return false;
    if (mTargetWalkSpeedIndex != 0) return false;
    changeStatus(mYoshiDash);
    return true;
}

// ---- range B small wrappers: map-collision queries that switch to the DashYoshi-step variant while on Yoshi ----

bool Mario::fn_803B2470() {
    if (!_60D && mActor->mAxisRotationDisabled6A0) return false;
    if (!((mDrawStates.mWord >> 18) & 1)) return false;
    TVec3f difference(mHeadVec);
    difference -= _290;
    if (MR::isNearZero(difference, 0.001f)) return false;
    TVec3f sum(mHeadVec);
    sum += _290;
    return !MR::isNearZero(sum, 0.001f);
}

s32 Mario::fn_803B2530(bool squat) {
    if (fn_803EF7C0() == 0x20 || static_cast<u16>(getPlayerMode()) == 4 || isPlayerModeYoshi() || _430 == 3) return 0;
    if (squat && mWalkSpeed > 0.3f && !((mMovementStates.mHighWord >> 28) & 1)) {
        trySquatJump();
        return 1;
    }
    if (((mMovementStates.mLowWord >> 21) & 1) && !((mMovementStates.mHighWord >> 28) & 1)) {
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
    if (this->isPlayerModeYoshi() && this->mActor->fn_803B97F0()->mDashTimer > 0) {
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

void Mario::fn_803B3AE0(const HitInfo* hit, const TVec3f* origin) {
    if (isPlayerModeTeresa()) {
        TVec3f offset(hit->mHitPos);
        offset -= *origin;
        if (hit->mParentTriangle.getNormal(0)->dot(offset) >= 0.0f) return;
    }
    *mSideWallTriangle = hit->mParentTriangle;
    _500 = hit->mHitPos;
    mMovementStates.mLowWord |= 0x20;
    if (hit->isCollisionAtCorner() || hit->isCollisionAtEdge()) mMovementStates.mHighWord |= 0x1000;
}

void Mario::fn_803B3BB0(const HitInfo* hit, const TVec3f* origin) {
    if ((mMovementStates.mLowWord >> 23) & 1) {
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
    mMovementStates.mLowWord |= 0x40;
}

void Mario::fn_803B3CE0() {
    if (mDashWallCooldown != 0) --mDashWallCooldown;
    if (isPlayerModeYoshi() && mActor->fn_803B97F0()->mDashTimer > 0) {
        if (((mMovementStates.mLowWord >> 23) & 1) && !isAnimationRun("壁はじき")) {
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
        if (((mMovementStates.mLowWord >> 30) & 1) && mGroundPolygon->isValid() && fn_803B3E90()) {
            mActor->fn_803B97F0()->fn_8040F320();
            return;
        }
        mActor->fn_803B97F0()->fn_8040F370();
        return;
    }
    mDashWallSoundPlayed = false;
    if (isPlayerModeYoshi()) mActor->fn_803B97F0()->fn_8040F370();
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
        this->mMovementStates.mLowWord |= 0x40000000U;
        this->mJumpVec.zero();
        this->changeAnimation("空中一回転", static_cast<const char*>(nullptr));
    }
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

void Mario::fn_803B23E0(const TVec3f* direction, const TVec3f* axis, f32 rate) {
    TVec3f blended;
    if (!MR::vecBlendSphere(&blended, mHeadVec, *direction, rate)) {
        MR::vecRotAxis(mHeadVec, *direction, *axis, &blended, rate);
    }
    mHeadVec = blended;
    MR::normalize(&mHeadVec);
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

s32 Mario::fn_803B41F0() {
    s32 result = 0;
    s32 active = 0;
    if (mSinkTimer != 0) {
        if ((getPlayer()->mMovementStates.mLowWord >> 30) & 1) {
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

// NONMATCHING: repeated flag loads and boolean materialization.
void Mario::fn_803B38B0(const HitInfo* hit, const TVec3f* center) {
    *mFrontWallTriangle = hit->mParentTriangle;
    _4E8 = hit->mHitPos;
    TVec3f direction(hit->mHitPos);
    direction.sub(*center);
    MR::normalizeOrZero(&direction);
    f32 facing = direction.dot(*hit->mParentTriangle.getNormal(0));
    if (facing < fn_803B3870()) mMovementStates.mLowWord |= 0x00800000U;
    else mMovementStates.mHighWord |= 0x00002000U;
    if (((mMovementStates.mLowWord >> 30) & 1U) && ((mMovementStates.mLowWord >> 10) & 1U) && ((mMovementStates.mHighWord >> 6) & 1U)) {
        TVec3f delta(_4A4);
        delta.sub(_4E8);
        if (delta.dot(*getGravityVec()) >= 0.0f) mMovementStates.mLowWord &= 0xFF7FFFFFU;
    }
    if (!_60D && !((mMovementStates.mLowWord >> 23) & 1U)) {
        if (calcAngleD(*mFrontWallTriangle->getNormal(0)) < mActor->getConst().getTable()->mSlipAngle &&
            !isSlipFloorCode(_95C->getCode(mFrontWallTriangle)) &&
            hit->mParentTriangle.getNormal(0)->dot(mFrontVec) < 0.0f && direction.dot(mFrontVec) > 0.0f &&
            direction.dot(*hit->mParentTriangle.getNormal(0)) < 0.0f) {
            mDrawStates.mWord |= 0x00010000U;
            _380 = *mFrontWallTriangle->getNormal(0);
        }
    }
}

// NONMATCHING: temporary allocation and register scheduling.
f32 Mario::fn_803B2CC0(TVec3f* arg1) {
    if (((mFlags14.mLowWord >> 12) & 1) && ((mMovementStates.mLowWord >> 20) & 1)) {
        return 1.0f;
    }
    if (arg1->dot(this->mHeadVec) < 0.0f) {
        u16 t = this->mActor->_334;
        if (t != 0) {
            f32 t3 = (f32)t / 15.0f;
            return t3 * marioCurTableEntry(this)->mRotateHeadVecSpeedByGravityM + (1.0f - t3) * marioCurTableEntry(this)->mRotateHeadVecSpeedByGravityL;
        }
        return marioCurTableEntry(this)->mRotateHeadVecSpeedByGravityL;
    }
    if (arg1->dot(this->mHeadVec) < 0.99f) {
        return marioCurTableEntry(this)->mRotateHeadVecSpeedByGravityM;
    }
    if (this->mTargetWalkSpeedIndex <= 2) {
        return marioCurTableEntry(this)->mRotateHeadVecSpeedByGravityS;
    }
    return 1.0f;
}

// NONMATCHING: register allocation / constant pool labels
bool Mario::fn_803B3040(TVec3f* pOutPos, Triangle* pOutTri) {
    TVec3f start;
    TVec3f dir;
    if (((this->mMovementStates.mLowWord >> 23) & 1) && this->mFrontWallTriangle != nullptr) {
        pOutPos->set(this->_4E8);
        start = this->mActor->_240;
        *pOutTri = *this->mFrontWallTriangle;
        return true;
    }
    if (this->isPlayerModeYoshi()) {
        TVec3f up(this->mFrontVec);
        up.scale(115.0f);
        dir = up;
    } else if (((this->mMovementStates.mLowWord >> 16) & 1) && !MR::isNearZero(this->_328.dot(*this->getGravityVec()), 0.001f)) {
        MR::vecKillElement(this->_328, *this->getGravityVec(), &dir);
        dir.setLength(200.0f);
    } else {
        TVec3f up(this->mFrontVec);
        up.scale(100.0f);
        dir = up;
    }
    TVec3f down(*this->getGravityVec());
    down.scale(8.0f);
    TVec3f origin(this->mPosHolder->mPos);
    origin -= down;
    start = origin;
    if (!this->fn_803B2770(pOutPos, pOutTri, start, dir)) {
        return false;
    }
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    if (__fabs(this->mAirGravityVec.dot(*pOutTri->getNormal(0))) >= 0.1f) {
        return false;
    }
    TVec3f diff(*pOutPos);
    diff -= start;
    this->_4E4 = diff.dot(this->mFrontVec);
    return true;
}

// NONMATCHING: register allocation / constant pool labels
void Mario::fn_803B3280(const TVec3f& rPos, Triangle* pTri) {
    Triangle tri;
    TVec3f up(this->mFrontVec);
    up.scale(20.0f);
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    TVec3f back(this->mAirGravityVec);
    back.scale(200.0f);
    TVec3f p(rPos);
    p -= back;
    TVec3f start(p);
    start += up;
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    TVec3f dir(this->mAirGravityVec);
    dir.scale(210.0f);
    TVec3f hit;
    if (!this->fn_803B2B50(start, dir, &hit, &tri)) {
        this->_4E0 = 400.0f;
        return;
    }
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    if (this->mAirGravityVec.dot(*tri.getNormal(0)) > -0.9f) {
        this->_4E0 = 400.0f;
        return;
    }
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    TVec3f diff(rPos);
    diff -= hit;
    f32 dist = diff.dot(this->mAirGravityVec);
    this->_4E0 = dist;
    if (dist > 0.0f) {
        this->mMovementStates.mLowWord |= 0x400;
    }
    f32 offset = 0.0f;
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    f32 angle = this->marioAcos(-this->mAirGravityVec.dot(*tri.getNormal(0)));
    if (angle < 1.5707964f && angle != 0.0f) {
        offset = this->_4E0 / (JMASinRadian(angle) / JMACosRadian(angle));
    }
    TVec3f savedHit(hit);
    TVec3f lift(this->mFrontVec);
    lift.scale(15.0f + offset);
    hit -= lift;
    MR::isNearZero(this->mAirGravityVec, 0.001f);
    TVec3f inward(this->mAirGravityVec);
    inward.scale(5.0f);
    TVec3f rayStart(hit);
    rayStart -= inward;
    TVec3f rayDir(*this->getGravityVec());
    rayDir.scale(10.0f);
    if (!this->fn_803B2880(rayStart, rayDir)) {
        hit.set(savedHit);
        *this->_47C = tri;
        return;
    }
    this->_4A4 = hit;
    if (!((this->mMovementStates.mLowWord >> 30) & 1) || this->calcAngleD(this->_368) < 25.0f) {
        this->mMovementStates.mHighWord |= 0x40;
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
            if (((this->mMovementStates.mHighWord >> 8) & 1) && __fabs(pInfo->mParentTriangle.getNormal(0)->dot(this->mSwimSideVec694)) > ref) {
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
        if ((this->mMovementStates.mLowWord >> 30) & 1) {
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
            this->mDrawStates.mWord |= 0x10000;
            if (maxAngle > ref) {
                this->_380 = *this->fn_803B2AA0(maxIdx)->mParentTriangle.getNormal(0);
            } else if (minAngle < ref) {
                this->_380 = *this->fn_803B2AA0(minIdx)->mParentTriangle.getNormal(0);
            }
        }
        if (((this->mMovementStates.mLowWord >> 30) & 1) && maxAngle > 60.0f && maxAngle != ref && maxAngle < 80.0f) {
            TVec3f hitPos(this->fn_803B2AA0(maxIdx)->mHitPos);
            TVec3f toPos(rPos);
            toPos -= hitPos;
            if (!MR::normalizeOrZero(&toPos)) {
                this->mDrawStates.mWord |= 0x100;
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

// NONMATCHING: constant pool labels
void Mario::fn_803B3EE0(TVec3f* pPos, f32* pHeight) {
    f32 offset = 110.0f;
    if ((this->mMovementStates.mLowWord >> 21) & 1) {
        offset = 50.0f;
    }
    *pHeight = 40.0f;
    TVec3f down(*this->getGravityVec());
    down.scale(offset);
    TVec3f base(this->mPosHolder->mPos);
    base -= down;
    pPos->set(base);
    if (this->isStatusActive(2)) {
        this->mActor->calcHeadPos();
        pPos->set(this->mActor->mCapPosition);
        if (this->mDamage->_18 == 0) {
            *pHeight = 50.0f;
        }
    }
    if ((this->mMovementStates.mHighWord & 3) != 0) {
        this->mActor->calcHeadPos();
        TVec3f side(this->_16C);
        side.scale(2.0f);
        TVec3f head(this->mActor->mCapPosition);
        head += side;
        pPos->set(head);
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
        TVec3f zOff(zAxis);
        zOff.scale(20.0f);
        TVec3f yOff(yAxis);
        yOff.scale(50.0f);
        TVec3f off(yOff);
        off -= zOff;
        pPos->add(off);
        *pHeight = 80.0f;
    }
    if (this->isSwimming()) {
        TVec3f swim(this->mHeadVec);
        swim.scale(60.0f);
        TVec3f pos(this->mPosHolder->mPos);
        pos += swim;
        pPos->set(pos);
    }
}

// NONMATCHING: register allocation
s32 Mario::fn_803B4300(TVec3f* pOutVec, MarioStrikeEntry* pOut, s32, const TVec3f& rPos, f32 radius) {
    u32 count = this->fn_803B29B0(rPos, radius);
    f32 limit;
    if (this->isPlayerModeYoshi() && this->isStatusActive(39) && !MR::isDead(this->mActor->fn_803B9830()->mRideModelActor)) {
        limit = 0.707f;
    } else if ((this->mMovementStates.mLowWord >> 31) != 0) {
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

// NONMATCHING: stack allocation, cached animation frame and literal pool ordering.
void Mario::fn_803AD5B0() {
    if (mTargetWalkSpeedIndex != 0) {
        if (((mDrawStates.mWord >> 13) & 1) || ((mDrawStates.mWord >> 12) & 1)) {
            if (_738 < 160.0f) {
                if (isPlayerModeYoshi()) {
                    f32 previousFrame = mPrevAnimFrame;
                    if (previousFrame > getAnimator()->getFrame() ||
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
                        TVec3f height(_368);
                        height *= depth;
                        TVec3f position(mGroundPos);
                        position += height;
                        playEffectSRT("水波紋", 0.2f, _73C, position);
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
                        TVec3f height(_368);
                        height *= depth;
                        TVec3f side(mSideVec);
                        side *= 20.0f;
                        TVec3f offset(mGroundPos);
                        offset -= side;
                        TVec3f position(offset);
                        position += height;
                        playEffectSRT("水波紋", 0.2f, _73C, position);
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
                            TVec3f height(_368);
                            height *= depth;
                            TVec3f side(mSideVec);
                            side *= 20.0f;
                            TVec3f offset(mGroundPos);
                            offset += side;
                            TVec3f position(offset);
                            position += height;
                            playEffectSRT("水波紋", 0.2f, _73C, position);
                        }
                    }
                }
            }
        }
    } else if ((mDrawStates.mWord >> 13) & 1) {
        f32 depth = _738;
        if (depth < 160.0f && (mActor->_37C & 0x3F) == 0) {
            TVec3f height(_368);
            height *= depth;
            f32 randomOffset = MR::getRandom() - 0.5f;
            TVec3f side(mSideVec);
            side *= randomOffset;
            TVec3f scaledSide(side);
            scaledSide *= 20.0f;
            TVec3f offset(mShadowPos);
            offset += scaledSide;
            TVec3f position(offset);
            position += height;
            playEffectSRT("水波紋", 0.2f, _73C, position);
        }
    }
}
