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
    this->_10_HIGH_WORD |= 0x800000;
    return true;
}

void Mario::fn_803A96D0(f32 arg1) {
    MR::fn_8005A7F0(this->mActor, arg1);
}

bool Mario::fn_803AA2C0() {
    return false;
}

void Mario::fn_803AC550() {
    this->mActor->fn_803BB5F0();
}

void Mario::fn_803AC810() {
    this->mTeresa->isTeresaAccel();
}

void Mario::fn_803B1710() {
    this->_10_HIGH_WORD &= 0xFFCFFFFF;
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
    this->mMovementStates_LOW_WORD &= 0xFFFFFBFF;
    this->mMovementStates_HIGH_WORD &= 0xFFFFFFBF;
    (*reinterpret_cast<s32**>(reinterpret_cast<u8*>(this) + 0x478))[1] = -1;
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
    if (this->getPlayer()->mMovementStates_HIGH_WORD >> 10 & 1) {
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
    if (!(this->_20_LOW_WORD >> 21 & 1) || !this->isAnimationRun("しゃがみ基本")) {
        ret = true;
    }
    return ret;
}

void Mario::fn_803AD120(const Triangle* pTri) {
    if (this->isSlipPolygon(pTri) == 0x20 && this->mTargetWalkSpeedIndex != 0) {
        this->_3F4 = 0.2f;
    }
}

void Mario::fn_803B14E0() {
    this->mActor->fn_803B9A30();
    this->_10_HIGH_WORD &= 0xFFDFFFFF;
    this->mActor->fn_803B9830()->fn_8041BB90();
    this->changeAnimation("落下", (const char*)nullptr);
}

void Mario::fn_803B1720() {
    if (this->getCurrentStatus() != 0x26 && (this->mMovementStates_LOW_WORD >> 30 & 1)) {
        this->changeStatus(this->_7B4);
        this->stopWalk();
    }
}

void Mario::fn_803B1780() {
    if (this->getCurrentStatus() == 0x26) {
        this->_7B4->fn_804090C0();
    }
}

void Mario::fn_803B17C0() {
    if (this->getCurrentStatus() == 0x26) {
        this->closeStatus(this->_7B4);
    }
}

void Mario::fn_803B1800() {
    if (this->getCurrentStatus() == 0x27) {
        *reinterpret_cast<f32*>(reinterpret_cast<u8*>(this->_7B8) + 0x14) = 1.0f;
        return;
    }
    this->changeStatus(this->_7B8);
}

bool Mario::fn_803B1850() const {
    return this->getCurrentStatus() == 0x27;
}

void Mario::fn_803B1880() {
    if (this->getCurrentStatus() != 0x28) {
        this->changeStatus(this->_7BC);
    }
}


void Mario::fn_803B1B20() {
    if (this->fn_803B1B70()) {
        this->closeStatus(this->_7C0);
    }
}

bool Mario::fn_803B1B70() const {
    return this->getCurrentStatus() == 0x29;
}


f32 Mario::fn_803B1BA0() {
    if (this->isPlayerModeCloud() && this->_430 == 0) {
        return 24.0f;
    }
    return reinterpret_cast<const f32*>(this->mActor->mConst->getTable())[this->_430];
}

// NONMATCHING: the target keeps the Yoshi flag in a register variable (inlined bool helper); 64%
bool Mario::fn_803B1C10() {
    bool ret = false;
    bool yoshi = false;
    if (this->isPlayerModeYoshi() && FLD<s32>(this->mActor->fn_803B97F0(), 0x134) > 0) {
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
    if (this->isPlayerModeYoshi() && FLD<s32>(this->mActor->fn_803B97F0(), 0x134) > 0) {
        yoshi = true;
    }
    if (yoshi && !(this->getPlayer()->_10_HIGH_WORD >> 21 & 1)) {
        ret = true;
    }
    return ret;
}

bool Mario::fn_803B26C0() {
    if (this->isPlayerModeHopper() && this->_A6C[0xC] != 0) {
        return true;
    }
    if (this->isPlayerModeInvincible() && this->_A6C[0xD] != 0) {
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
    HitSensor* pSensor = MR::fn_80038D20(this->mGroundPolygon);
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



// NONMATCHING: jump-end and punch-landing gate chain approximated from the draft
s32 Mario::fn_803A2D80() {
    if ((FLD<u32>(this, 0x10) >> 5) & 1) {
        this->tryJump();
        if (this->isAnimationRun("その場足踏み上半身")) {
            this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
        }
        FLD<u32>(this, 0x420) = 0;
        FLD<TVec3f>(this, 0x184) = FLD<TVec3f>(this, 0x184);
        FLD<u32>(this, 0x10) &= 0xFFFFFFDF;
    }
    if (this->mActor->isRequestJump() == false && !((FLD<u32>(this, 0x1C) >> 21) & 1) && !((FLD<u32>(this, 0x10) >> 7) & 1)) {
    }
    if (((FLD<u32>(this, 0xC) >> 21) & 1) && this->calcDistToCeil(false) < 160.0f) {
        this->mActor->sendMsgUpperPunch(FLD<HitSensor*>(this, 0x730));
        this->changeAnimation("しゃがみアッパー", (const char*)nullptr);
    }
    if (this->isStatusActive(0x1F)) {
        this->mSkate->fn_803F4AF0();
        this->closeStatus(this->mSkate);
    }
    if (FLD<u32>(this, 0x436) != 0) {
        if (FLD<f32>(this, 0x1C8) == 0.5f) {
            this->fn_803A2EF0();
        }
    }
    this->fn_803A2FC0();
    return 0;
}

// NONMATCHING: sound selection approximated from the draft
void Mario::fn_803A2EF0() {
    if (this->checkSquat(false)) {
        FLD<u32>(this, 0x436) = 0;
        this->mWalkSpeed = 5.0f;
        FLD<u16>(this, 0x434) = 0xB4;
        this->playSound("声物ジャンプ", -1);
        this->playSound("ダッシュ加速強成功", -1);
        this->playSound("スペシャルダッシュ強", -1);
    }
    FLD<u32>(this, 0x436) = 0;
    this->mWalkSpeed = 2.0f;
    FLD<u16>(this, 0x434) = 0x3C;
    this->playSound("スペシャルダッシュ弱", -1);
    this->playSound("ダッシュ加速強成功", -1);
}

// NONMATCHING: drop/wall-slide counter gating approximated
s32 Mario::fn_803A3030() {
    if (!((FLD<u32>(this, 0xC) >> 30) & 1)) {
        if (FLD<u16>(this, 0x3C0) != 0 && !((FLD<u32>(this, 0x10) >> 28) & 1) && MR::isNearZero(FLD<TVec3f>(this, 0x8F8), 0.001f)) {
            FLD<u16>(this, 0x3C0) -= 1;
        }
        if (FLD<u32>(this->mActor, 0x4E0) == 0) {
            this->tryDrop();
            FLD<u32>(this, 0x420) = 0;
        }
    }
    void* ent = marioCurTableEntry(this);
    if (FLD<f32>(this, 0x1C8) < FLD<f32>(ent, 0x488)) {
        FLD<u16>(this, 0x3C0) = 0;
    } else if ((FLD<u32>(this, 0x1C) >> 21) & 1) {
        FLD<u16>(this, 0x3C0) = 0;
    } else {
        u16 t = FLD<u16>(ent, 0x92);
        if (FLD<u16>(this, 0x3C0) < t) {
            FLD<u16>(this, 0x3C0) = t;
        }
    }
    return 0;
}

// NONMATCHING: angle-based run-cancel selection approximated
f32 Mario::fn_803A3780(f32 fself) {
    f32 var_f31 = MR::diffAngleAbs(fself, FLD<f32>(this, 0x2B4));
    if (this->isAnimationRun("その場足踏み")) {
        var_f31 = 1.0f;
    }
    if (this->isAnimationRun("ターンブレーキ")) {
        var_f31 = 1.0f;
    }
    if (this->isSwimming()) {
        var_f31 = 1.0f;
    }
    if (this->mActor->isRequestSpin() && (FLD<u32>(this, 0xC) >> 31) != 0 && this->mWorldPadDir.dot(FLD<TVec3f>(this, 0x16C)) < 0.0f) {
        var_f31 = 1.0f;
    }
    if ((FLD<u32>(this, 0x10) >> 20) & 1) {
        var_f31 = 1.0f;
    }
    return var_f31;
}

// NONMATCHING: the stage of the face-direction test is approximated from the draft
bool Mario::fn_803A4210() {
    Mario* m = reinterpret_cast<Mario*>(this);
    u32 pw = FLD<u32>(this->getPlayer(), 0x14);
    bool var_r31 = false;
    if (!((pw >> 16) & 1) && ((pw >> 15) & 1) && !((pw >> 21) & 1) && FLD<f32>(m, 0x8F0) == 0.0f && m->checkStickFrontBack() == 2) {
        var_r31 = true;
    }
    return var_r31;
}

void Mario::startBas(const char* pName, bool loop, f32 startFrame, f32 speed) {
    if (FLD<u32>(this->mActor, 0x6C) != 0) {
        MR::startBas(this->mActor, pName, loop, startFrame, speed);
        this->_970 = pName;
    }
}

bool Mario::taskOnHipDropBlurHopper(u32) {
    if (!getPlayer()->mMovementStates._B || mMovementStates._1 || isStatusActive(MarioStatus_Swim)) {
        if (gIsLuigi) {
            stopEffect("ホッパー尻落ルイージ");
        } else {
            stopEffect("ホッパー尻落");
        }
        return false;
    }
    return true;
}

// NONMATCHING: the per-frame reset chain is approximated from the draft
void Mario::fn_803ACC40() {
    FLD<f32>(this, 0x3F4) = 0.0f;
    if (!((FLD<u32>(this, 0xC) >> 21) & 1)) {
        this->fn_803ACD80();
    }
    if (this->isAnimationRun("壁押し")) {
        this->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
    this->fn_803ACCE0();
    this->fn_803AD120(nullptr);
    this->decideSquatWalkAnimation();
    this->fn_803AD4F0();
    FLD<f32>(this, 0x2C4) = this->getAnimator()->fn_803DD460();
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
    FLD<u32>(this, 0x10) &= 0xFDFFFFFF;
}

// NONMATCHING: squat-end transition approximated
void Mario::fn_803ACE60() {
    this->stopAnimation("飛び込み準備", (const char*)nullptr);
    if (this->isAnimationRun("Squat")) {
        this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
    }
    this->changeAnimation((const char*)nullptr, "基本");
    this->stopAnimation("サマーソルト", (const char*)nullptr);
    this->fn_803AF2F0(nullptr);
    FLD<u32>(this, 0x10) &= 0xFDFFFFFF;
    if (this->isPlayerModeYoshi()) {
        if (!((FLD<u32>(this, 0x20) >> 21) & 1)) {
            MR::startActionSound(reinterpret_cast<const LiveActor*>(this->mActor->fn_803B97F0()), "声しゃがむ", -1, -1, -1);
        }
    } else if (((FLD<u32>(this, 0xC) >> 31) == 0) && !this->isAnimationRun("ヨッシー泥移動")) {
        this->playSound("声しゃがむ", -1);
    }
}

// NONMATCHING: stick/brake speed table selection approximated
void Mario::fn_803ACF70() {
    void* e = marioCurTableEntry(this);
    if (this->mWalkSpeed < FLD<f32>(e, 0x380)) {
        if (this->isStickOn()) {
            this->mWalkSpeed = FLD<f32>(e, 0x38C);
            this->fn_803AF2F0(nullptr);
            this->mTargetWalkSpeedIndex = 1;
            FLD<u32>(this, 0x10) |= 0x02000000;
        }
        this->fn_803AF2F0(nullptr);
        this->mTargetWalkSpeedIndex = 0;
        FLD<u32>(this, 0x10) &= 0xFDFFFFFF;
    }
    if (!this->isStickOn()) {
        this->fn_803AF2F0(nullptr);
        this->mTargetWalkSpeedIndex = 0;
    }
    if ((FLD<u32>(this, 0x10) >> 25) & 1) {
        f32 f2 = FLD<f32>(e, 0x38C);
        f32 f1 = FLD<f32>(e, 0x388) - f2;
        this->mWalkSpeed = f2 + FLD<f32>(this, 0x1C8) * f1;
        this->fn_803AF2F0(nullptr);
        this->mTargetWalkSpeedIndex = 1;
    }
}

// NONMATCHING: animator speed scaling approximated
void Mario::fn_803AD0B0() {
    f32 var_f31 = this->mWalkSpeed * (60.0f / FLD<f32>(marioCurTableEntry(this), 0x384));
    if (!((FLD<u32>(this, 0x10) >> 25) & 1)) {
        var_f31 = 0.0f;
    }
    this->getAnimator()->setSpeed(var_f31);
}

// NONMATCHING: yoshi-mud effect gate approximated
void Mario::fn_803AD4F0() {
    if (this->isPlayerModeYoshi() && this->mTargetWalkSpeedIndex != 0 && ((FLD<u32>(this->getPlayer(), 0x14) >> 30) & 1)) {
        f32 f2 = FLD<f32>(this, 0x2C4);
        if (f2 > this->getAnimator()->fn_803DD460() || (f2 < 30.0f && this->getAnimator()->fn_803DD460() == 30.0f)) {
            this->playEffect("ヨッシー泥移動");
        }
    }
}

// NONMATCHING: same gate as fn_803AD4F0 with the extra speed-index test
void Mario::fn_803AD9C0() {
    if (this->isPlayerModeYoshi() && this->mTargetWalkSpeedIndex != 0 && ((FLD<u32>(this->getPlayer(), 0x14) >> 30) & 1)) {
        f32 f2 = FLD<f32>(this, 0x2C4);
        if (f2 > this->getAnimator()->fn_803DD460() || (f2 < 30.0f && this->getAnimator()->fn_803DD460() == 30.0f)) {
            this->playEffect("ヨッシー泥移動");
        }
    }
}

// NONMATCHING: animator speed setter selected by the walk-speed table
void Mario::fn_803AF2F0(const f32* pSpeedTable) {
    f32 var_f31;
    if (FLD<u16>(this, 0x3FA) != 0) {
        var_f31 = 0.5f;
    } else {
        var_f31 = FLD<f32>(marioCurTableEntry(this), 0x358);
    }
    this->getAnimator()->setBlendWeight(pSpeedTable, var_f31);
}

// NONMATCHING: stick-to-camera re-centering approximated from the draft
void Mario::fn_803A56C0() {
    TVec3f sp14(this->mStickPos.x, FLD<f32>(this, 0x1C4), 0.0f);
    f32 sp8 = FLD<TVec3f>(this, 0x368).dot(this->getCamDirX());
    f32 spC = FLD<TVec3f>(this, 0x368).dot(this->getCamDirY());
    TVec3f camv(sp8, 0.0f, spC);
    MR::normalizeOrZero(&camv);
    f32 ang = MR::diffAngleAbs(camv, sp14);
    f32 t = MR::clamp(ang, 0.0f, 1.0f);
    f32 lim = t * 1.0f + 0.5f;
    if (ang < lim || ang > 3.1415927f - lim) {
        FLD<f32>(this, 0x1C8) = 0.0f;
        FLD<f32>(this, 0x1C4) = 0.0f;
        this->mStickPos = TVec3f(0.0f, 0.0f, 0.0f);
    }
}

// NONMATCHING: 2D/wall-press side checks approximated from the draft
void Mario::fn_803A6BE0() {
    s32 var_r31 = 0;
    if (((FLD<u32>(this, 0x10) >> 30) & 1) != 0) {
        var_r31 = 0;
    }
    if (FLD<s32>(this, 0x730) != 0 && var_r31 != 0) {
        FLD<TVec3f>(this->mActor, 0x6D8) = FLD<TVec3f>(this, 0x368);
        this->mActor->setPress(0, 0);
    }
}

// NONMATCHING: timer decay and slope handling approximated
void Mario::fn_803A7540() {
    if (!((FLD<u32>(this, 0x10) >> 28) & 1)) {
        FLD<TVec3f>(this, 0x1A8) = FLD<TVec3f>(this, 0x160);
        FLD<u16>(this, 0x3C2) = 0;
        FLD<f32>(this, 0x280) = 0.0f;
        FLD<TVec3f>(this, 0x284).zero();
    } else {
        FLD<u16>(this, 0x3C2) += 1;
    }
    if (FLD<u16>(this, 0x3C4) != 0) {
        FLD<u16>(this, 0x3C4) -= 1;
    }
    if (((FLD<u32>(this, 0xC) >> 30) & 1) && ((FLD<u32>(this, 0xC) >> 31) == 0)) {
        if (this->isEnableSlopeMove()) {
            this->slopeMove();
        }
        this->clearSlope();
    }
}

// NONMATCHING: yoshi morph-trigger gate approximated
void Mario::fn_803B0E60() {
    Mario* m = reinterpret_cast<Mario*>(this);
    if (this->isPlayerModeYoshi() && YoshiParam::fn_8041BD60() != 0) {
        if (this->mActor->fn_803B9810()->fn_8041F910() != 0 && this->checkTrgA() && this->checkLvlZ()) {
            m->fn_803B0EF0();
            FLD<u32>(m, 0xA91) = 1;
        }
    }
}

// NONMATCHING: yoshi mount/dismount setup approximated; vector math simplified
void Mario::fn_803B0EF0() {
    Mario* m = reinterpret_cast<Mario*>(this);
    if (this->isPlayerModeYoshi()) {
        FLD<u16>(this, 0x412) = 0;
        m->mMovementStates_HIGH_WORD |= 0x40000;
        this->mActor->mMarioAnim->fn_803DF9D0();
        this->mActor->fn_803B9810()->fn_8041FD00();
        TVec3f sp68(*m->getGravityVec());
        sp68 *= -m->mVelocity.dot(*m->getGravityVec());
        TVec3f sp5C;
        PSVECCrossProduct(&m->mSideVec, &sp68, &sp5C);
        MR::normalizeOrZero(&sp5C);
        sp5C *= YoshiParam::fn_8041BD90();
        TVec3f sp2C(*m->getGravityVec());
        sp2C *= 20.0f;
        TVec3f sp38(sp68);
        sp38 += sp5C;
        TVec3f sp44(sp38);
        sp44 += sp2C;
        this->mActor->fn_803B97F0()->fn_8040CCD0(sp44);
        f32 t31 = YoshiParam::fn_8041BD80();
        TVec3f sp8(*m->getGravityVec());
        sp8 *= t31;
        TVec3f sp14(sp68);
        sp14 -= sp5C;
        TVec3f sp20(sp14);
        sp20 -= sp8;
        FLD<TVec3f>(m, 0x304) = sp20;
        this->changeAnimation("飛び込み準備", "基本");
        this->mActor->setPlayerMode(0, true, true);
    }
}

// NONMATCHING: yoshi dismount/landing helpers, rough from the draft (vector math simplified)
void Mario::fn_803B1100() {
    if (this->isPlayerModeYoshi()) {
        FLD<u16>(this, 0x412) = 0;
        FLD<u32>(this, 0x10) |= 0x40000000;
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

// NONMATCHING: animation-state reset and the yoshi jump-rise speed update approximated
void Mario::fn_803B12B0() {
    if ((FLD<u32>(this, 0xC) >> 20) & 1) {
        FLD<u32>(this, 0x10) &= 0xFFDFFFFF;
    }
    if (!this->isPlayerModeYoshi()) {
        FLD<u32>(this, 0x10) &= 0xFFDFFFFF;
        if ((FLD<u32>(this, 0xC) >> 30) & 1) {
            this->changeAnimation("基本", (const char*)nullptr);
        }
        this->changeAnimation("落下", (const char*)nullptr);
    }
    if ((FLD<u32>(this, 0xC) >> 30) & 1) {
        FLD<u32>(this, 0x10) &= 0xFFDFFFFF;
        this->changeAnimation("基本", (const char*)nullptr);
    }
    if (!this->isAnimationRun("踏ん張り")) {
        this->changeAnimation("踏ん張り", (const char*)nullptr);
    }
    this->mActor->fn_803B9A50();
    if (this->mActor->fn_803B9A70() == 0) {
        this->fn_803B14E0();
    }
}

// NONMATCHING: yoshi-end cleanup approximated from the draft
s32 Mario::fn_803B1540() {
    Mario* m = reinterpret_cast<Mario*>(this);
    this->mActor->fn_803B9840();
    s32 r = 0;
    if (((FLD<u32>(m, 0x430 - 4) >> 0) & 1) != 0 && m->isRising()) {
        FLD<u32>(m, 0xC) |= 0x1000;
    }
    m->fn_8039A600();
    m->resetTornado();
    m->cancelTornadoJump();
    FLD<u32>(m, 0x10) = (FLD<u32>(m, 0x10) & 0xBFFFFFFF);
    FLD<u32>(m, 0x14) |= 0x300000;
    this->mActor->fn_803B9A10();
    if (FLD<u32>(m, 0x42C) == 4) {
        TVec3f sp8;
        sp8 = -FLD<TVec3f>(m, 0x220);
        m->setFrontVecKeepUp(sp8);
        FLD<u32>(m, 0x42C) = 0;
    }
    if (this->isAnimationRun("カリカリ限界")) {
        this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
    }
    this->changeAnimation("踏ん張り", (const char*)nullptr);
    FLD<u32>(m, 0x10) &= 0xFFFFFFFC;
    return r;
}

// NONMATCHING: initial status comparison and temporary register allocation.
bool Mario::fn_803B18C0() {
    if (getCurrentStatus() == 0x29) return false;
    if (mActor->_EA4) return false;
    bool actorBusy = false;
    if (mActor->_482 || mActor->_481) actorBusy = true;
    if (actorBusy) return false;
    if (getStickP() != 0.0f || checkLvlA() || checkLvlZ() || mActor->isRequestRush()) return false;
    if (!(getPlayer()->mMovementStates_LOW_WORD >> 31) && !((getPlayer()->mMovementStates_LOW_WORD >> 30) & 1)) return false;
    if (getPlayer()->isStatusActive(6)) return false;
    if (getPlayer()->isStatusActive(5)) return false;
    if (getPlayer()->isStatusActive(0x1A)) return false;
    if (getPlayer()->isStatusActive(0x1B)) return false;
    if (getPlayer()->isStatusActive(0x12)) return false;
    if (getPlayer()->isStatusActive(0x11)) return false;
    if (getPlayer()->isStatusActive(0x22)) return false;
    if ((getPlayer()->_1C_WORD >> 26) & 1) return false;
    if ((getPlayer()->mMovementStates_HIGH_WORD >> 28) & 1) return false;
    if ((getPlayer()->mMovementStates_LOW_WORD >> 21) & 1) return false;
    if (mTargetWalkSpeedIndex != 0) return false;
    changeStatus(_7C0);
    return true;
}

// ---- range B small wrappers: map-collision queries that switch to the DashYoshi-step variant while on Yoshi ----

bool Mario::fn_803B2470() {
    if (!_60D && mActor->mAxisRotationDisabled6A0) return false;
    if (!((mDrawStates_WORD >> 18) & 1)) return false;
    TVec3f difference(mHeadVec);
    difference -= _290;
    if (MR::isNearZero(difference, 0.001f)) return false;
    TVec3f sum(mHeadVec);
    sum += _290;
    return !MR::isNearZero(sum, 0.001f);
}

s32 Mario::fn_803B2530(Triangle* triangle) {
    if (fn_803EF7C0() == 0x20 || static_cast<u16>(getPlayerMode()) == 4 || isPlayerModeYoshi() || _430 == 3) return 0;
    if (triangle != nullptr && mWalkSpeed > 0.3f && !((mMovementStates_HIGH_WORD >> 28) & 1)) {
        trySquatJump();
        return 1;
    }
    if (((mMovementStates_LOW_WORD >> 21) & 1) && !((mMovementStates_HIGH_WORD >> 28) & 1)) {
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
        return MR::fn_80037E80(pOut, pTri, rStart, rDir);
    }
    return MR::getFirstPolyOnLineToMap(pOut, pTri, rStart, rDir);
}

bool Mario::fn_803B27F0(TVec3f* pOut, Triangle* pTri, const TVec3f& rStart, const TVec3f& rDir, const CollisionPartsFilterBase* pPartsFilter, const TriangleFilterBase* pTriFilter) {
    if (fn_803B2950()) {
        return MR::fn_80038060(pOut, pTri, rStart, rDir, pPartsFilter, pTriFilter);
    }
    return MR::getFirstPolyOnLineToMap(pOut, pTri, rStart, rDir, pPartsFilter, pTriFilter);
}

bool Mario::fn_803B2880(const TVec3f& rStart, const TVec3f& rEnd) {
    if (fn_803B2950()) {
        return MR::fn_800382E0(rStart, rEnd);
    }
    return MR::isExistMapCollision(rStart, rEnd);
}

bool Mario::fn_803B28E0(const TVec3f& rStart, const TVec3f& rEnd, const LiveActor* pActor) {
    if (fn_803B2950()) {
        return MR::fn_800384C0(rStart, rEnd, pActor);
    }
    return MR::isExistMapCollisionExceptActor(rStart, rEnd, pActor);
}

s32 Mario::fn_803B2950() {
    s32 var_r31 = 0;
    if (this->isPlayerModeYoshi() && FLD<s32>(this->mActor->fn_803B97F0(), 0x134) > 0) {
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
        return Collision::fn_8003B110(idx);
    }
    return Collision::getStrikeInfoMap(idx);
}

bool Mario::fn_803B2AF0(const TVec3f& rPos, HitInfo* pInfo) {
    if (fn_803B2950()) {
        return MR::fn_800386E0(rPos, pInfo);
    }
    return MR::checkStrikePointToMap(rPos, pInfo);
}

bool Mario::fn_803B2B50(const TVec3f& rStart, const TVec3f& rEnd, TVec3f* pOut, Triangle* pTri) {
    if (fn_803B2950()) {
        return MR::fn_8003A720(rStart, rEnd, pOut, pTri);
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
        TVec3f candidate(hit->_64 - mPosHolder->mPos);
        MR::vecKillElement(candidate, *getGravityVec(), &candidate);
        f32 previousDistance = previous.squared();
        if (candidate.squared() > previousDistance) return;
    }
    *_4D8 = hit->mParentTriangle;
    _50C = hit->_64;
}

void Mario::fn_803B3AE0(const HitInfo* hit, const TVec3f* origin) {
    if (isPlayerModeTeresa()) {
        TVec3f offset(hit->_64);
        offset -= *origin;
        if (hit->mParentTriangle.getNormal(0)->dot(offset) >= 0.0f) return;
    }
    *mSideWallTriangle = hit->mParentTriangle;
    _500 = hit->_64;
    mMovementStates_LOW_WORD |= 0x20;
    if (hit->isCollisionAtCorner() || hit->isCollisionAtEdge()) mMovementStates_HIGH_WORD |= 0x1000;
}

void Mario::fn_803B3BB0(const HitInfo* hit, const TVec3f* origin) {
    if ((mMovementStates_LOW_WORD >> 23) & 1) {
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
        TVec3f offset(hit->_64);
        offset -= *origin;
        if (hit->mParentTriangle.getNormal(0)->dot(offset) >= 0.0f) return;
    }
    *mBackWallTriangle = hit->mParentTriangle;
    _4F4 = hit->_64;
    mMovementStates_LOW_WORD |= 0x40;
}

void Mario::fn_803B3CE0() {
    if (mDashWallCooldown != 0) --mDashWallCooldown;
    if (isPlayerModeYoshi() && mActor->fn_803B97F0()->mDashTimer > 0) {
        if (((mMovementStates_LOW_WORD >> 23) & 1) && !isAnimationRun("壁はじき")) {
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
        if (((mMovementStates_LOW_WORD >> 30) & 1) && mGroundPolygon->isValid() && fn_803B3E90()) {
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
    if (FLD<u8>(this, 0xA7E) != 0) {
        if (this->fn_803B41F0() == 0) {
            this->stopEffect("流砂足元砂煙");
            FLD<u8>(this, 0xA7E) = 0;
        }
    } else if (this->fn_803B41F0() != 0) {
        this->playEffect("流砂足元砂煙");
        FLD<u8>(this, 0xA7E) = 1;
    }
}

// NONMATCHING: knock-back on steep normals approximated
void Mario::fn_803B44F0(TVec3f* arg1) {
    Mario* m = reinterpret_cast<Mario*>(this);
    TVec3f sp14(*arg1);
    MR::normalizeOrZero(&sp14);
    if (this->calcAngleD(sp14) > 45.0f) {
        TVec3f sp8(*arg1);
        sp8 *= 0.2f;
        m->blown(sp8);
        FLD<u16>(m, 0x402) = 0;
        FLD<u16>(m, 0x428) = 0x3C;
        FLD<u32>(m, 0x10) = (FLD<u32>(m, 0x10) | 0x100000) & 0xFFFFFFFC;
    }
    if (FLD<TVec3f>(m, 0x1FC).dot(*arg1) < 0.0f) {
        FLD<u32>(m, 0xC) |= 0x40000000;
        FLD<TVec3f>(m, 0x2D4).zero();
        this->changeAnimation("空中一回転", (const char*)nullptr);
    }
}

// NONMATCHING: head-vector collision/normal rotation approximated from the draft
void Mario::fn_803B1DE0(TVec3f* arg1) {
    TVec3f sp20(this->mHeadVec);
    MR::normalizeOrZero(&sp20);
    TVec3f sp14(*arg1);
    MR::normalizeOrZero(&sp14);
    if (this->mHeadVec.dot(*arg1) > -0.9f) {
        TVec3f sp8;
        PSVECCrossProduct(&this->mHeadVec, arg1, &sp8);
        MR::isNearZero(sp8, 0.001f);
    }
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
        if ((getPlayer()->mMovementStates_LOW_WORD >> 30) & 1) {
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

// NONMATCHING: front-wall capture and slip-floor flags approximated from the draft
void Mario::fn_803B38B0(Triangle* arg1, TVec3f* arg2) {
    *this->mFrontWallTriangle = *arg1;
    FLD<TVec3f>(this, 0x4E8) = FLD<TVec3f>(arg1, 0x64);
    TVec3f sp14(FLD<TVec3f>(arg1, 0x64));
    sp14 -= *arg2;
    MR::normalizeOrZero(&sp14);
    f32 t31 = sp14.dot(*arg1->getNormal(0));
    if (t31 < this->fn_803B3870()) {
        FLD<u32>(this, 0xC) |= 0x800000;
    } else {
        FLD<u32>(this, 0x10) |= 0x2000;
    }
    if (((FLD<u32>(this, 0xC) >> 30) & 1) && ((FLD<u32>(this, 0xC) >> 10) & 1) && ((FLD<u32>(this, 0x10) >> 6) & 1)) {
        TVec3f sp8(FLD<TVec3f>(this, 0x4A4));
        sp8 -= FLD<TVec3f>(this, 0x4E8);
        if (sp8.dot(*this->getGravityVec()) == 0.0f) {
            FLD<u32>(this, 0xC) &= 0xFF7FFFFF;
        }
    }
    if (FLD<u8>(this, 0x60D) == 0 && !((FLD<u32>(this, 0xC) >> 23) & 1)) {
        FLD<u32>(this, 0x1C) |= 0x10000;
        FLD<TVec3f>(this, 0x380) = *this->mFrontWallTriangle->getNormal(0);
    }
}

// NONMATCHING: the remaining B-range helpers are approximated; sg reports these as unmatched
f32 Mario::fn_803B2CC0(TVec3f* arg1) {
    if (arg1->dot(this->mHeadVec) < 0.0f) {
        s32 t = FLD<s32>(this->mActor, 0x664);
        if (t != 0) {
            f32 t3 = (f32)t / 15.0f;
            return t3 * FLD<f32>(marioCurTableEntry(this), 0x7BC) + (1.0f - t3) * FLD<f32>(marioCurTableEntry(this), 0x7B8);
        }
        return FLD<f32>(marioCurTableEntry(this), 0x7B8);
    }
    if (arg1->dot(this->mHeadVec) < 0.99f) {
        return FLD<f32>(marioCurTableEntry(this), 0x7BC);
    }
    if (this->mTargetWalkSpeedIndex <= 2) {
        return FLD<f32>(marioCurTableEntry(this), 0x7C0);
    }
    return 0.0f;
}

// NONMATCHING: stubs for the three helpers whose bodies are not recovered (return the draft's default)
// NONMATCHING: register allocation / constant pool labels
bool Mario::fn_803B3040(TVec3f* pOutPos, Triangle* pOutTri) {
    TVec3f start;
    TVec3f dir;
    if (((FLD<u32>(this, 0xC) >> 23) & 1) && FLD<Triangle*>(this, 0x4C8) != nullptr) {
        pOutPos->set(FLD<TVec3f>(this, 0x4E4));
        start = FLD<TVec3f>(this->mActor, 0x5F4);
        *pOutTri = *FLD<Triangle*>(this, 0x4C8);
        return true;
    }
    if (this->isPlayerModeYoshi()) {
        TVec3f up(FLD<TVec3f>(this, 0x208));
        up.scale(115.0f);
        dir = up;
    } else if (((FLD<u32>(this, 0xC) >> 16) & 1) && !MR::isNearZero(FLD<TVec3f>(this, 0x328).dot(*this->getGravityVec()), 0.001f)) {
        MR::vecKillElement(FLD<TVec3f>(this, 0x328), *this->getGravityVec(), &dir);
        dir.setLength(200.0f);
    } else {
        TVec3f up(FLD<TVec3f>(this, 0x208));
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
    MR::isNearZero(FLD<TVec3f>(this, 0x1D8), 0.001f);
    if (__fabs(FLD<TVec3f>(this, 0x1D8).dot(*pOutTri->getNormal(0))) >= 0.1f) {
        return false;
    }
    TVec3f diff(*pOutPos);
    diff -= start;
    FLD<f32>(this, 0x4E0) = diff.dot(FLD<TVec3f>(this, 0x208));
    return true;
}

// NONMATCHING: register allocation / constant pool labels
void Mario::fn_803B3280(const TVec3f& rPos, Triangle* pTri) {
    Triangle tri;
    TVec3f up(FLD<TVec3f>(this, 0x208));
    up.scale(20.0f);
    MR::isNearZero(FLD<TVec3f>(this, 0x1D8), 0.001f);
    TVec3f back(FLD<TVec3f>(this, 0x1D8));
    back.scale(200.0f);
    TVec3f p(rPos);
    p -= back;
    TVec3f start(p);
    start += up;
    MR::isNearZero(FLD<TVec3f>(this, 0x1D8), 0.001f);
    TVec3f dir(FLD<TVec3f>(this, 0x1D8));
    dir.scale(210.0f);
    TVec3f hit;
    if (!this->fn_803B2B50(start, dir, &hit, &tri)) {
        FLD<f32>(this, 0x4DC) = 400.0f;
        return;
    }
    MR::isNearZero(FLD<TVec3f>(this, 0x1D8), 0.001f);
    if (FLD<TVec3f>(this, 0x1D8).dot(*tri.getNormal(0)) > -0.9f) {
        FLD<f32>(this, 0x4DC) = 400.0f;
        return;
    }
    MR::isNearZero(FLD<TVec3f>(this, 0x1D8), 0.001f);
    TVec3f diff(rPos);
    diff -= hit;
    f32 dist = diff.dot(FLD<TVec3f>(this, 0x1D8));
    FLD<f32>(this, 0x4DC) = dist;
    if (dist > 0.0f) {
        FLD<u32>(this, 0xC) |= 0x400;
    }
    f32 offset = 0.0f;
    MR::isNearZero(FLD<TVec3f>(this, 0x1D8), 0.001f);
    f32 angle = this->marioAcos(-FLD<TVec3f>(this, 0x1D8).dot(*tri.getNormal(0)));
    if (angle < 1.5707964f && angle != 0.0f) {
        offset = FLD<f32>(this, 0x4DC) / (JMASinRadian(angle) / JMACosRadian(angle));
    }
    TVec3f savedHit(hit);
    TVec3f lift(FLD<TVec3f>(this, 0x208));
    lift.scale(15.0f + offset);
    hit -= lift;
    MR::isNearZero(FLD<TVec3f>(this, 0x1D8), 0.001f);
    TVec3f inward(FLD<TVec3f>(this, 0x1D8));
    inward.scale(5.0f);
    TVec3f rayStart(hit);
    rayStart -= inward;
    TVec3f rayDir(*this->getGravityVec());
    rayDir.scale(10.0f);
    if (!this->fn_803B2880(rayStart, rayDir)) {
        hit.set(savedHit);
        *FLD<Triangle*>(this, 0x478) = tri;
        return;
    }
    FLD<TVec3f>(this, 0x4A0) = hit;
    if (!((FLD<u32>(this, 0xC) >> 30) & 1) || this->calcAngleD(FLD<TVec3f>(this, 0x368)) < 25.0f) {
        FLD<u32>(this, 0x10) |= 0x40;
    }
    *FLD<Triangle*>(this, 0x478) = tri;
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
            if (((FLD<u32>(this, 0x10) >> 8) & 1) && __fabs(pInfo->mParentTriangle.getNormal(0)->dot(FLD<TVec3f>(this, 0x694))) > ref) {
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
        if ((FLD<u32>(this, 0xC) >> 30) & 1) {
            ref = this->calcAngleD(FLD<TVec3f>(this, 0x368));
            if (ref < minAngle) {
                minAngle = ref;
            }
            if (ref > maxAngle) {
                maxAngle = ref;
            }
        }
        f32 range = maxAngle - minAngle;
        if (range < 60.0f && range > 5.0f && maxAngle < 80.0f) {
            FLD<u32>(this, 0x1C) |= 0x10000;
            if (maxAngle > ref) {
                FLD<TVec3f>(this, 0x380) = *this->fn_803B2AA0(maxIdx)->mParentTriangle.getNormal(0);
            } else if (minAngle < ref) {
                FLD<TVec3f>(this, 0x380) = *this->fn_803B2AA0(minIdx)->mParentTriangle.getNormal(0);
            }
        }
        if (((FLD<u32>(this, 0xC) >> 30) & 1) && maxAngle > 60.0f && maxAngle != ref && maxAngle < 80.0f) {
            TVec3f hitPos(this->fn_803B2AA0(maxIdx)->_64);
            TVec3f toPos(rPos);
            toPos -= hitPos;
            if (!MR::normalizeOrZero(&toPos)) {
                FLD<u32>(this, 0x1C) |= 0x100;
                MR::isNearZero(FLD<TVec3f>(this, 0x1D8), 0.001f);
                TVec3f side;
                PSVECCrossProduct(this->fn_803B2AA0(maxIdx)->mParentTriangle.getNormal(0), &FLD<TVec3f>(this, 0x1D8), &side);
                MR::normalize(&side);
                TVec3f front;
                PSVECCrossProduct(&FLD<TVec3f>(this, 0x368), &side, &front);
                MR::normalize(&front);
                FLD<TVec3f>(this, 0x2C4) = front;
            }
        }
    }
}

// NONMATCHING: constant pool labels
void Mario::fn_803B3EE0(TVec3f* pPos, f32* pHeight) {
    f32 offset = 110.0f;
    if ((FLD<u32>(this, 0xC) >> 21) & 1) {
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
        pPos->set(FLD<TVec3f>(this->mActor, 0x600));
        if (FLD<u16>(FLD<void*>(this, 0x7C4), 0x18) == 0) {
            *pHeight = 50.0f;
        }
    }
    if ((FLD<u32>(this, 0x10) & 3) != 0) {
        this->mActor->calcHeadPos();
        TVec3f side(FLD<TVec3f>(this, 0x16C));
        side.scale(2.0f);
        TVec3f head(FLD<TVec3f>(this->mActor, 0x600));
        head += side;
        pPos->set(head);
    }
    if (this->getPlayerMode() == 4 && !((FLD<u32>(this->getPlayer(), 0x10) >> 28) & 1) && !((FLD<u32>(this->getPlayer(), 0xC) >> 21) & 1) &&
        !this->isStatusActive(28) && !this->isStatusActive(21) && !this->isStatusActive(27) && FLD<u8>(this->mActor, 0xECD) == 0) {
        pPos->set(FLD<TVec3f>(this->mActor, 0x600));
    }
    if (this->isPlayerModeYoshi() && this->isStatusActive(39) && !MR::isDead(FLD<LiveActor*>(this->mActor->fn_803B9830(), 0x18))) {
        MarioMtxPos mtx;
        mtx.fn_80409700(MR::getJointMtx(FLD<LiveActor*>(this->mActor->fn_803B9830(), 0x18), "Center"));
        TVec3f yAxis;
        mtx.fn_80409680(&yAxis);
        TVec3f zAxis;
        mtx.fn_804096A0(&zAxis);
        mtx.fn_80409660(pPos);
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
        TVec3f swim(FLD<TVec3f>(this, 0x1F0));
        swim.scale(60.0f);
        TVec3f pos(this->mPosHolder->mPos);
        pos += swim;
        pPos->set(pos);
    }
}

// NONMATCHING: register allocation
s32 Mario::fn_803B4300(TVec3f* pOutVec, MarioStrikeEntry* pOut, s32, const TVec3f& rPos, f32 radius) {
    s32 count = this->fn_803B29B0(rPos, radius);
    f32 limit;
    if (this->isPlayerModeYoshi() && this->isStatusActive(39) && !MR::isDead(FLD<LiveActor*>(this->mActor->fn_803B9830(), 0x18))) {
        limit = 0.707f;
    } else if ((FLD<u32>(this, 0xC) >> 31) != 0) {
        limit = 0.0f;
    } else {
        limit = -0.707f;
    }
    s32 n = 0;
    pOutVec->zero();
    for (s32 i = 0; i < count; i++) {
        const HitInfo* pInfo = this->fn_803B2AA0(i);
        const TVec3f* pNrm = pInfo->mParentTriangle.getNormal(0);
        if (MR::isThroughPolygon(&pInfo->mParentTriangle)) {
            continue;
        }
        if (this->isThroughWall(&pInfo->mParentTriangle)) {
            continue;
        }
        if (!this->isSwimming() && (FLD<u32>(this, 0x10) & 3) == 0 && pNrm->dot(*this->getGravityVec()) < limit) {
            continue;
        }
        TVec3f rest;
        f32 depth = pInfo->_60;
        if (MR::vecKillElement(*pOutVec, *pNrm, &rest) >= depth) {
            continue;
        }
        TVec3f push(*pNrm);
        push.scale(depth);
        TVec3f sum(rest);
        sum += push;
        *pOutVec = sum;
        pOut[n].mTriangle = pInfo->mParentTriangle;
        pOut[n].mHitPos = pInfo->_64;
        pOut[n].mDepth = pInfo->_60;
        n++;
    }
    return n;
}

// NONMATCHING: skid/splash dust by speed band; the two animator-frame branches are approximated from the draft
void Mario::fn_803AD5B0() {
    Mario* m = reinterpret_cast<Mario*>(this);
    if (m->mTargetWalkSpeedIndex != 0) {
        u32 dm = FLD<u32>(m, 0x1C);
        if (((dm >> 13) & 1) || ((dm >> 12) & 1)) {
            if (FLD<f32>(m, 0x738) < 160.0f) {
                if (this->isPlayerModeYoshi()) {
                    if (m->mPrevAnimFrame > this->getAnimator()->fn_803DD460()) {
                        u8 idx = m->mTargetWalkSpeedIndex;
                        if (idx >= 2) {
                            if (idx < 6) {
                                this->playSound("水はね", -1);
                                this->playEffect("水はね左");
                            } else {
                                this->playSound("水跳ね左足", -1);
                                this->playEffect("水跳ね左足小");
                            }
                        }
                        f32 t = FLD<f32>(m, 0x738);
                        TVec3f sp98(FLD<TVec3f>(m, 0x368));
                        sp98 *= t;
                        TVec3f spB0(m->mGroundPos);
                        spB0 += sp98;
                        this->playEffectSRT("水波紋", 0.2f, FLD<TVec3f>(m, 0x73C), spB0);
                    }
                } else {
                    if (m->mPrevAnimFrame > this->getAnimator()->fn_803DD460()) {
                        u8 idx = m->mTargetWalkSpeedIndex;
                        if (idx >= 2) {
                            if (idx < 6) {
                                this->playSound("水跳ね右足", -1);
                                this->playEffect("水はね右弱");
                            } else {
                                this->playSound("水跳ね右足", -1);
                                this->playEffect("水はね右");
                            }
                        }
                        f32 t = FLD<f32>(m, 0x738);
                        TVec3f sp74(FLD<TVec3f>(m, 0x368));
                        sp74 *= t;
                        TVec3f sp80(FLD<TVec3f>(m, 0x208));
                        sp80 *= 20.0f;
                        TVec3f sp8C(m->mGroundPos);
                        sp8C -= sp80;
                        TVec3f sp98(sp8C);
                        sp98 += sp74;
                        this->playEffectSRT("水波紋", 0.2f, FLD<TVec3f>(m, 0x73C), sp98);
                    }
                    if (m->mPrevAnimFrame < 30.0f) {
                        if (this->getAnimator()->fn_803DD460() == 30.0f) {
                            u8 idx = m->mTargetWalkSpeedIndex;
                            if (idx >= 2) {
                                if (idx < 6) {
                                    this->playSound("水跳ね左足", -1);
                                    this->playEffect("水はね左弱");
                                } else {
                                    this->playSound("水跳ね左足", -1);
                                    this->playEffect("水はね左");
                                }
                            }
                            f32 t = FLD<f32>(m, 0x738);
                            TVec3f sp44(FLD<TVec3f>(m, 0x368));
                            sp44 *= t;
                            TVec3f sp50(FLD<TVec3f>(m, 0x208));
                            sp50 *= 20.0f;
                            TVec3f sp5C(m->mGroundPos);
                            sp5C += sp50;
                            TVec3f sp68(sp5C);
                            sp68 += sp44;
                            this->playEffectSRT("水波紋", 0.2f, FLD<TVec3f>(m, 0x73C), sp68);
                        }
                    }
                }
            }
        }
    }
}
