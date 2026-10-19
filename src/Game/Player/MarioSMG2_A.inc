// SMG2-only Mario:: functions (fn_ range A), written from the asm. Included at the end of Mario.cpp (unity TU).
// Named Yoshi member views retain canonical compiler-derived offsets.
template <typename T, typename P>
inline T& FLD(P* p, u32 off) { return *reinterpret_cast<T*>(reinterpret_cast<u8*>(const_cast<void*>(static_cast<const void*>(p))) + off); }


void Mario::fn_803880D0() {
    if ((this->mFlags20.mWord >> 15) & 1) {
        this->mPosHolder->fn_80406040();
        this->addTrans(this->mVelocity, nullptr);
        if (this->calcDistToCeil(false) < 200.0f) {
            this->mVelocity.zero();
            this->stopWalk();
        }
        this->mPosHolder->fn_80406080();
    }
}

void Mario::fn_80388150() {
    if (((this->mMovementStates.mHighWord >> 8) & 1) || ((this->mFlags14.mLowWord >> 10) & 1)) {
        TVec3f sp38(this->mPosHolder->mPos);
        sp38 -= this->_688;
        MR::vecKillElement(sp38, this->mSwimSideVec694, &sp38);
        TVec3f* temp_r31 = &this->mPosHolder->mPos;
        TVec3f sp14(this->_688);
        sp14 += sp38;
        TVec3f sp20(sp14);
        sp20 -= *temp_r31;
        this->mPosHolder->fn_80406010(sp20);
        MR::vecKillElement(this->mVelocity, this->mSwimSideVec694, &this->mVelocity);
        TVec3f sp8(this->_694);
        sp8 -= this->_688;
        TVec3f sp2C;
        f32 temp_f1 = MR::vecKillElement(sp8, this->mSwimSideVec694, &sp2C);
        f32 var_f31 = temp_f1;
        if (temp_f1 > 1.0f) {
            var_f31 = 1.0f;
        } else if (temp_f1 < -1.0f) {
            var_f31 = -1.0f;
        }
        sp2C = this->mSwimSideVec694;
        sp2C.setLength(var_f31);
        this->_688 += sp2C;
    }
}

void Mario::fn_803885F0() {
    if (!((this->mMovementStates.mHighWord >> 29) & 1)) {
        f32 temp_f31 = 0.0f;
        u32 var_r29 = 0U;
        while (var_r29 < this->_578) {
            Triangle* tri = this->_57C[var_r29];
            TVec3f sp8;
            if (MR::vecKillElement(this->mVelocity, *tri->getNormal(0), &sp8) < temp_f31) {
                this->mVelocity = sp8;
                this->mSwim->fn_803F7C20(tri->getNormal(0));
            }
            var_r29 += 1;
        }
    }
}

bool Mario::fn_803890B0() {
    if (this->isStatusActive(6)) {
        return false;
    }
    return !this->isStatusActive(0x27);
}

void Mario::fn_803896E0() {
    this->_898 = 0;
    if (!((this->mMovementStates.mLowWord >> 9) & 1) && MR::testFpViewStartTrigger()) {
        if (MR::isPossibleToShiftToFirstPersonCamera() == false) {
            if (!MR::isDemoActive()) {
                if (!MR::isStageStoryBook() && !MR::isStageFileSelect() && !MR::isStageEpilogueDemo() && !MR::isStageStaffRoll()) {
                    MR::startSystemSE("SE_SY_CAMERA_NG", -1, -1);
                }
                this->_898 = 1;
            }
        } else {
            this->tryFpViewMode();
        }
    }
}

void Mario::fn_803897A0() {
    u32 temp_r4 = this->mMovementStates.mHighWord;
    if ((temp_r4 >> 28) & 1) {
        if (!((this->mMovementStates.mLowWord >> 30) & 1)) {
            return;
        }
        if ((temp_r4 >> 27) & 1) {
            if (!this->isSlipPolygon(this->_460)) {
                return;
            }
            const TVec3f* nrm = this->_460->getNormal(0);
            f32 temp_f31 = MR::vecKillElement(this->mVelocity, *nrm, &this->mVelocity);
            nrm = this->_460->getNormal(0);
            TVec3f sp14(this->_368);
            sp14 -= *nrm;
            TVec3f sp20(sp14);
            sp20 *= 0.5f;
            f32 temp_f31_2 = MR::abs(temp_f31);
            TVec3f sp8(sp20);
            sp8 *= temp_f31_2;
            this->mVelocity += sp8;
        }
    }
}

void Mario::fn_80389D90(f32 (*m)[4], const TVec3f* col2, const TVec3f* col1, const TVec3f* col0) {
    TVec3f a;
    TVec3f b;
    TVec3f c;
    a = *col0;
    c = *col2;
    b = *col1;
    m[0][0] = a.x;
    m[1][0] = a.y;
    m[2][0] = a.z;
    m[0][1] = b.x;
    m[1][1] = b.y;
    m[2][1] = b.z;
    m[0][2] = c.x;
    m[1][2] = c.y;
    m[2][2] = c.z;
}


void Mario::fn_8038A560(f32 (*arg1)[4], s32 arg2) {
    if (!this->isNonFixHeadVec() && arg2 == 0 && !((this->mMovementStates.mLowWord >> 9) & 1)) {
        this->fixHeadFrontVecByGravity();
    }
    if (!this->updatePosture(arg1)) {
        this->createDirectionMtx(arg1);
        PSMTXConcat(_C4, arg1, arg1);
        this->fn_8038A600();
    }
}

void Mario::fn_8038A840(void*, f32 x, f32 y, f32 z) {
    this->_58.set(x, y, z);
    MR::orderRotateMtx(0, this->_58, this->_64);
    MR::orderRotateMtx(0, -this->_58, this->_94);
}

void Mario::fn_8038A9D0() {
    TMtx34f sp18;
    PSMTXRotAxisRad(sp18, &this->mHeadVec, this->_74C);
    TVec3f sp8;
    PSMTXMultVec(sp18, &this->mFrontVec, &sp8);
    this->setFrontVecKeepUp(sp8);
    this->_74C = 0.0f;
}

void Mario::fn_8038AB10() {
    u32 count = this->_754;
    if (count != 0) {
        f32 phase = this->_74C;
        this->_754 = count - 1;
        this->_750 = 0;
        this->_74C = phase + ((3.1415927f - phase) / (f32)count);
    }
}

void Mario::fn_8038AB70() {
    u32 count = this->_750;
    if (count != 0) {
        u32 remaining = count - 1;
        this->_750 = remaining;
        this->_74C = (this->_74C * (f32)remaining) / (f32)count;
        if (remaining == 0) {
            this->setFrontVecKeepUp(this->mFrontVec);
        }
    }
}

void MarioFlags3::fn_8038C890() {
    _0 = 0;
    _1 = 0;
    _2 = 0;
}

void Mario::fn_803929A0() {
    this->fn_803B3010();
    Triangle sp18;
    TVec3f sp8;
    if (this->fn_803B3040(&sp8, &sp18)) {
        this->fn_803B3280(sp8, &sp18);
    }
}

bool Mario::fn_80393C40() {
    if (((u32)this->mFlags14.mLowWord >> 31) != 0) {
        return true;
    }
    return (u32)(this->getCurrentStatus() - 8) <= 2U;
}

bool Mario::fn_803949E0() {
    if (this->isEnableAddDamage() == false) {
        return false;
    }
    if (this->isStatusActive(6)) {
        this->mMovementStates.mLowWord |= 0x10;
        this->_7C4.zero();
        return this->checkWaterDamage();
    }
    return this->doFireDanceWithInitialDamage(1);
}

bool Mario::fn_80395310() {
    if (this->mMovementStates.mLowWord & 1) {
        return false;
    }
    if (this->isInvincible() != 0) {
        return false;
    }
    u32 temp_r3 = this->mMovementStates.mLowWord;
    if (((temp_r3 >> 31) != 0) || !((temp_r3 >> 30) & 1)) {
        return false;
    }
    if (this->getCurrentStatus() == 0xE) {
        return false;
    }
    if (this->getCurrentStatus() == 0xF) {
        return false;
    }
    if (this->getCurrentStatus() == 8) {
        return false;
    }
    if ((this->mMovementStates.mLowWord >> 4) & 1) {
        return false;
    }
    this->mActor->fn_803B94E0();
    this->stopJump();
    this->stopWalk();
    this->changeStatus(this->mStun);
    return true;
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

void Mario::fn_8039CFD0() {
    this->_4B0 = this->mPosHolder->mPos;
}

void Mario::fn_803A1E40() {
    u16 temp_r4 = this->_3D4;
    if (temp_r4 != 0) {
        this->_3D4 = temp_r4 - 1;
    }
}

void Mario::fn_803A1F60() {
    u16 temp_r4 = this->_3D2;
    if (temp_r4 != 0) {
        this->_3D2 = temp_r4 - 1;
    }
}

void Mario::fn_803A2CF0() {
    if (this->_960 == 35) {
        this->mMovementStates.mHighWord |= 0x400;
    }
}

void Mario::fn_803A2420() {
    this->mDrawStates.mWord &= 0xFFFBFFFF;
    this->_40E = 0;
    this->mFlags14.mLowWord &= 0xFFF7FFFF;
}

void Mario::fn_803A1FC0() {
    if (!(this->mWalkSpeed >= 0.1f)) {
        this->mWalkSpeed = 0.0f;
        this->_3D2 = 0;
    }
}

void Mario::fn_803A2690(f32* p) {
    if (this->mActor->mBeeWallWalk != 0) {
        if (*p > 0.1f) {
            *p = 0.1f;
        }
    }
}

bool Mario::fn_8039A430() {
    if (!((this->mMovementStates.mLowWord >> 8) & 1)) {
        return false;
    }
    this->mMovementStates.mLowWord &= 0xFFFFFEFF;
    return this->mWall->startJump();
}

s32 Mario::fn_803A2D10() {
    s32 r = 0;
    if (!((this->mDrawStates.mWord >> 26) & 1) && !((this->mMovementStates.mHighWord >> 28) & 1)) {
        if (this->_754 != 0) {
            r = 1;
        }
    }
    return r;
}

void Mario::fn_803A1F80() {
    f32 temp_f1 = this->mStickPos.z;
    const MarioConstTable* ent = this->mActor->mConst->getTable();
    if (temp_f1 < ent->mTurnSlipNeutral) {
        this->mMovementStates.mLowWord |= 0x8000;
    }
}

void Mario::fn_803A2CC0() {
    if (!((this->mMovementStates.mHighWord >> 28) & 1)) {
        return;
    }
    this->mTargetWalkSpeedIndex = 0;
    this->mWalkSpeed = 0.0f;
    this->playSound("坂滑り", -1);
}

void Mario::fn_803A1A20(f32* out) {
    this->mActor->getStickValue(&out[0], &out[1]);
    out[2] = 0.0f;
}

void Mario::fn_803A2530(TVec3f* out) {
    if (this->_524 == this->_528) {
        *out = this->_22C;
    } else {
        *out = this->mFrontVec;
    }
}

void Mario::fn_803A23F0(TVec3f* out) {
    if ((this->mMovementStates.mHighWord >> 28) & 1) {
        out->set(this->mFrontVec);
    } else {
        *out = this->_22C;
    }
}

void Mario::fn_8039A460() {
    if (this->_544 <= 5) {
    }
}

static inline MarioConstTable* marioCurTableEntry(Mario* self) {
    return self->mActor->mConst->getTable();
}


void Mario::fn_8039A5B0() {
    if (this->beeMarioOnAir()) {
        this->_76C = this->mActor->mConst->getTable()->mBeeGravityReviveTime;
    }
}

void Mario::fn_803A1D60() {
    PSVECCrossProduct(&this->_214, &this->_22C, &this->_3D8);
    this->fn_803A1A60(&this->_244);
    this->_238 = this->_22C;
}

bool Mario::fn_803902B0() {
    const HitSensor* sensor = this->mGroundPolygon->mSensor;
    return sensor != nullptr && MR::isSensorPressObj(sensor);
}

bool Mario::fn_80391260(const Triangle* triangle) {
    if ((mMovementStates.mHighWord >> 8) & 1) {
        if (MR::abs(triangle->getNormal(0)->dot(mSwimSideVec694)) > 0.707f) return true;
    }
    return false;
}

void Mario::fn_803912D0(const HitInfo* hit, const TVec3f& normal, const TVec3f& direction) {
    if (fn_80391330(hit, normal, direction)) {
        *_4DC = hit->mParentTriangle;
        _518 = hit->mHitPos;
    }
}

bool Mario::fn_8039A3D0() {
    bool r = false;
    if (this->_3BC > 10 && this->checkTrgA()) {
        r = true;
    }
    return r;
}

bool Mario::fn_803A0FF0() {
    u16 v = this->_40C;
    if (v == 0) {
        return false;
    }
    if (this->mStickPos.z < 0.75f) {
        this->_40C = 0;
        return false;
    }
    this->_40C = v - 1;
    this->mStickPos.x = 0.0f;
    this->mStickPos.y = 0.0f;
    this->mStickPos.z = 0.0f;
    return true;
}

bool Mario::fn_803A2270() {
    TVec3f sp8 = -this->mFrontVec;
    f32 d = MR::diffAngleAbsHorizontal(this->_8F8, sp8, *this->getGravityVec());
    return d < 0.7853982f;
}


void Mario::fn_8039A020() {
    if (this->mFlags14.mLowWord & 1) {
        if (this->mActor->fn_803BE0E0() == nullptr) {
            this->mMovementStates.mLowWord &= 0xFFFFEFFF;
        }
    } else {
        if (this->mActor->fn_803BE0D0() == nullptr) {
            this->mMovementStates.mLowWord &= 0xFFFFEFFF;
        }
    }
}

void Mario::fn_8039A090() {
    if (this->checkTrgA() || this->mActor->fn_803BE0C0() != nullptr) {
        this->_558 = this->mActor->_37C;
        if (this->isEnableFutureJump()) {
            this->mMovementStates.mHighWord |= 0x80;
        }
    }
}

f32 Mario::fn_803A26C0(f32 x) {
    MarioConstTable* e = marioCurTableEntry(this);
    f32 f4 = e->mStickHeavyMaxAngle;
    f32 f3 = e->mStickHeavyMinRatio;
    f32 f0 = e->mStickHeavyMinAngle;
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

void Mario::fn_803A24C0(TVec3f* arg) {
    f32 angle = 0.12f;
    if (this->_274 == 0) {
        angle = -0.12f;
    }
    TMtx34f matrix;
    PSMTXRotAxisRad(matrix, &this->mHeadVec, angle);
    PSMTXMultVecSR(matrix, &this->_214, arg);
}

void Mario::fn_803A2450(const TVec3f* arg) {
    f32 d = this->mFrontVec.dot(*arg);
    if (!(d >= 0.0f) && !this->isAnimationRun("ブレーキ")) {
        this->_74C = 3.1415927f;
        this->_750 = 10;
    }
}

void Mario::fn_803A2860() {
    if (!this->isAnimationRun("その場足踏み") && this->_214.dot(this->_22C) > 0.99f) {
        this->mMovementStates.mLowWord &= 0xEFFFFFFF;
    } else {
        this->mMovementStates.mLowWord |= 0x10000000;
    }
}


void Mario::fn_803A2720() {
    if ((this->getPlayer()->mFlags14.mLowWord >> 13) & 1) {
        f32 d = MR::diffAngleAbs(this->mFrontVec, this->_22C);
        f32 c = MR::clamp(d / 3.1415927f, 0.0f, 1.0f);
        this->mWalkSpeed = this->mWalkSpeed * (1.0f - 0.1f * c);
    }
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

void Mario::fn_803A11A0(TVec3f* arg) {
    f32 sp12 = this->mStickPos.x;
    f32 sp8 = this->mStickPos.y;
    if (!(this->getPlayer()->mFlags14.mLowWord & 0x4000)) {
        this->fn_803A1240(&sp12, &sp8);
        this->calcMoveDir(sp12, sp8, arg, true);
        this->calcShadowDir(*arg, &this->_22C);
    }
}

bool Mario::fn_80394550(const TVec3f* arg) {
    if (this->damage(*arg)) {
        if (this->isStatusActive(6)) {
            this->mSwim->fn_803F7C00();
        } else {
            MarioConstTable* e = marioCurTableEntry(this);
            this->mDamage->setVecSize(e->mJumpDistLargeDamage, e->mJumpHeightLargeDamage);
            this->playSound("投げられ", -1);
        }
        return true;
    }
    return false;
}

bool Mario::fn_80390210(const Triangle* triangle) {
    if (MR::isSensorPressObj(triangle->mSensor)) {
        return true;
    }
    if (this->_95C->getCode(triangle) == 0x1D) {
        return true;
    }
    if (this->_95C->getCode(triangle) == 0x27) {
        return true;
    }
    if (this->fn_803902B0()) {
        return true;
    }
    return this->_960 == 0x1D || this->_960 == 0x27;
}

void Mario::fn_803A1A60(TVec3f* arg) {
    TVec3f s;
    this->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    PSVECCrossProduct(&s, arg, &this->_250);
}

bool Mario::fn_803A19C0() {
    TVec3f s;
    this->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    return s.dot(this->_244) > 0.0f;
}

bool Mario::fn_803A1050() {
    if ((this->mMovementStates.mHighWord >> 8) & 1) {
        f32 d = MR::diffAngleAbsHorizontal(this->_214, this->_22C, this->mSwimSideVec694);
        return d > 1.5707964f;
    }
    f32 d2 = MR::diffAngleAbsHorizontal(this->_214, this->_22C, *this->getGravityVec());
    return d2 > 0.17453293f;
}

void Mario::fn_803A0D90(TVec3f* arg) {
    if (MR::isNearZero(this->_328, 0.001f)) {
        this->_328 = this->mFrontVec;
        *arg = this->_328;
        this->calcShadowDir(this->_328, &this->_22C);
        this->_40C = 10;
    }
}


bool Mario::fn_803A0E10() {
    if (this->mTargetWalkSpeedIndex == 0) {
        return false;
    }
    if (this->_71D != 0) {
        return false;
    }
    if (this->isAnimationRun("ブレーキ")) {
        return false;
    }
    if (this->isAnimationRun("ターンブレーキ")) {
        return false;
    }
    if ((this->mMovementStates.mLowWord >> 15) & 1) {
        return false;
    }
    if ((this->mMovementStates.mLowWord >> 16) & 1) {
        return false;
    }
    if (this->mWalkSpeed >= 0.05f) {
        return false;
    }
    if ((this->mDrawStates.mWord >> 26) & 1) {
        return false;
    }
    return true;
}

void Mario::fn_8039AB50() {
    if (this->_430 == 13 && ((this->mMovementStates.mHighWord >> 31) & 1)) {
        return;
    }
    TVec3f sp32;
    f32 old = MR::vecKillElement(this->mJumpVec, *this->getGravityVec(), &sp32);
    f32 limit = this->fn_8039AC10();
    if (old >= limit) {
        TVec3f sp8(*this->getGravityVec());
        sp8 *= limit;
        TVec3f sp20(sp8);
        sp20 += sp32;
        this->mJumpVec = sp20;
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
    return (n + 2.0f) * 0.5f;
}

void Mario::fn_803A27A0(TVec3f* arg) {
    if (MR::diffAngleAbs(this->mFrontVec, this->_22C) < 2.5f) {
        return;
    }
    if (this->_3D4 != marioCurTableEntry(this)->mWeakTurnTime) {
        return;
    }
    if ((this->mDrawStates.mWord >> 18) & 1) {
        return;
    }
    TVec3f sp8;
    PSVECCrossProduct(&this->_214, arg, &sp8);
    f32 d = sp8.dot(this->mHeadVec);
    this->_274 = d > 0.0f;
    this->mFlags14.mLowWord |= 0x80000;
}

f32 Mario::fn_8039AC10() {
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

bool Mario::fn_8039A100() {
    if (this->_430 == 8 && this->_42A == 0) {
        u16 timer = this->_426;
        if (timer != 0) {
            return timer > this->mActor->mConst->getTable()->mTornadoBoostTimer - 16;
        }
        return true;
    }
    if ((this->mMovementStates.mLowWord >> 12) & 1) {
        return true;
    }
    if ((this->mMovementStates.mLowWord >> 17) & 1) {
        return true;
    }
    if (!this->isRising()) {
        return true;
    }
    return this->_430 == 2;
}

void Mario::fn_803A10D0(s32 arg) {
    MarioConstTable* e = marioCurTableEntry(this);
    u32 dur = this->mActor->mBeeWallWalk == 0 ? 30 : e->mStandingTurnTime;
    if (arg == 0) {
        this->setFrontVecKeepUp(this->_22C, dur);
        if (this->isEnableTurn()) {
            this->changeAnimation("その場足踏み", (const char*)nullptr);
            if (!this->isAnimationRun("カリカリ限界") && !this->isPlayerModeYoshi()) {
                this->fn_803EEC20("その場足踏み上半身", 0);
            }
        }
    }
}

void Mario::fn_803A1C90() {
    TVec3f sp8;
    PSVECCrossProduct(&this->_214, &this->_22C, &sp8);
    if (sp8.dot(this->_3D8) < 0.0f) {
        if (!this->fn_803A19C0()) {
            if (!(this->_214.dot(this->_22C) < 0.0f)) {
                this->_3D4 = 0;
            }
        }
    } else {
        u16 v = this->_3D4;
        if (v < marioCurTableEntry(this)->mWeakTurnTime) {
            this->_3D4 = v + 1;
            this->fn_803A1D60();
        }
    }
}

bool Mario::fn_803A1920() {
    TVec3f stick;
    this->mActor->getStickValue(&stick.x, &stick.y);
    stick.z = 0.0f;
    return !this->_250.epsilonEquals(stick, 0.000003814697265625f);
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
    this->mFlags14.mLowWord &= 0xFFF7FFFF;
    this->mMovementStates.mLowWord &= 0xEFFFFFFF;
}

void Mario::fn_803A1BB0() {
    if (this->_3D0 == 0) {
        return;
    }
    this->mFlags14.mLowWord &= 0xFFF7FFFF;
    this->mMovementStates.mLowWord &= 0xEFFFFFFF;
    f32 d = MR::diffAngleAbs(this->_214, this->_22C);
    if (MR::isInRange(d, 0.0f, 0.707f)) {
        TVec3f s;
        PSVECCrossProduct(&this->_214, &this->_22C, &s);
        if (this->_274 == 0) {
            if (s.dot(this->mHeadVec) < 0.0f) {
                this->mFlags14.mLowWord &= 0xFFF7FFFF;
            }
        } else {
            if (s.dot(this->mHeadVec) > 0.0f) {
                this->mFlags14.mLowWord &= 0xFFF7FFFF;
            }
        }
    }
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


s32 Mario::fn_803A1E60(f32 f) {
    MarioConstTable* e = marioCurTableEntry(this);
    f32 v = JMACosRadian(e->mTurnSlipAngle);
    s32 r31 = 0;
    if (f < v && !((this->mMovementStates.mLowWord >> 26) & 1)) {
        if (this->mWalkSpeed > e->mTurnSlipNeutral) {
            if (this->_38 > this->mStickPos.z) {
                if (!((this->mDrawStates.mWord >> 18) & 1)) {
                    r31 = 1;
                }
            }
        }
    }
    return r31;
}

// NONMATCHING: branch shape of the 0x3D4 == 0 block approximated
void Mario::fn_803A1FF0(f32 x, s32 arg) {
    if ((this->mMovementStates.mHighWord >> 28) & 1) {
        this->fn_803A1AC0();
        this->fn_803A1C90();
        x *= this->fn_803A1DB0();
    } else {
        this->fn_803A1E40();
        if (this->_3D4 == 0) {
            this->_244.zero();
            PSVECCrossProduct(&this->_214, &this->_22C, &this->_3D8);
            x *= 0.01f;
            this->_238 = this->_22C;
            f32 k = 0.3f;
            if ((this->mDrawStates.mWord >> 18) & 1) {
                if (this->fn_803A2270()) {
                    k = 0.1f;
                }
            }
            x *= k;
        }
    }
    this->fn_803A22D0(x, arg);
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
            if (this->isAnimationRun("その場足踏み上半身")) {
                this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
            }
        }
    }
}


void Mario::fn_803A22D0(f32 x, s32 arg) {
    TVec3f sp8;
    if ((this->mFlags14.mLowWord >> 12) & 1) {
        this->fn_803A2420();
        if ((this->mMovementStates.mHighWord >> 8) & 1) {
            this->fn_803A23F0(&sp8);
            this->fn_803A2420();
            this->fn_803A2450(&sp8);
        }
    } else if ((this->mFlags14.mLowWord >> 13) & 1) {
        this->fn_803A24C0(&sp8);
    } else {
        f32 d = MR::diffAngleAbs(this->mFrontVec, this->_22C);
        if (d < 0.1f) {
            this->fn_803A2530(&sp8);
        } else {
            this->fn_803A2570(x, &sp8);
            this->fn_803A2720();
            this->fn_803A27A0(&sp8);
            this->fn_803A2860();
        }
    }
    if (arg == 0) {
        this->setFrontVecKeepUp(sp8);
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
        if (this->mActor->fn_803B97F0()->mDashTimer > 0) {
            s *= 0.5f;
        }
    }
    if (!MR::vecBlendSphere(arg, this->mFrontVec, this->_22C, s)) {
        this->_3D4 = 0;
        MR::vecRotAxis(this->_214, this->_22C, this->mHeadVec, arg, 0.3926991f);
    }
}

// NONMATCHING: the wall-normal slide tail is approximated from the asm
void Mario::fn_803A28D0(TVec3f* arg) {
    this->fn_803A2A70();
    this->fn_803A2C40(arg);
    if ((this->mMovementStates.mHighWord >> 12) & 1) {
        if (this->_4E0 > 120.0f) {
            const TVec3f* n = this->mFrontWallTriangle->getNormal(0);
            if (this->mVelocity.dot(*n) < 0.0f) {
                TVec3f sp20(this->mPosHolder->mPos);
                sp20 -= this->_4E8;
                TVec3f sp56;
                MR::vecKillElement(sp20, *n, &sp56);
                MR::vecKillElement(sp56, *this->getGravityVec(), &sp56);
                MR::vecKillElement(sp56, this->mFrontVec, &sp56);
                MR::normalizeOrZero(&sp56);
                if (!MR::isNearZero(sp56, 0.001f)) {
                    f32 m = PSVECMag(&this->mVelocity);
                    TVec3f a(sp56);
                    a.scale(m);
                    this->mVelocity = a;
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
            }
        }
    }
}


void Mario::fn_803A1AC0() {
    if (MR::isNearZero(this->mStickPos.z, 0.001f)) {
        this->mFlags14.mLowWord &= 0xFFF7FFFF;
        this->mMovementStates.mLowWord &= 0xEFFFFFFF;
    } else if (MR::isNearZero(this->_244, 0.001f) && !((this->mMovementStates.mHighWord >> 8) & 1)) {
        if (this->fn_803A1920()) {
            this->mMovementStates.mLowWord |= 0x10000000;
        } else {
            this->fn_803A1BB0();
        }
    } else if (this->fn_803A19C0() && !((this->mMovementStates.mHighWord >> 8) & 1)) {
        this->mMovementStates.mLowWord |= 0x10000000;
    } else {
        this->fn_803A1BB0();
    }
    this->fn_803A1A20(&this->_250.x);
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

f32 Mario::fn_8039AA40() {
    if (this->getPlayerMode() == 6) {
        return 0.0f;
    }
    f32 ratio = 1.0f;
    if (this->_430 == 13 && (((this->mMovementStates.mLowWord >> 17) & 1) || !this->isRising())) {
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

// ---- rough bodies (NONMATCHING) for the remaining range-A functions ----


s32 Mario::fn_8038A8A0(f32 (*matrix)[4], TVec3f* offset) {
    offset->zero();
    if (this->isNonFixHeadVec()) {
        PSMTXIdentity(matrix);
        return 0;
    }
    if (MR::isNearZero(this->_74C, 0.001f) && this->_754 == 0) {
        PSMTXIdentity(matrix);
        this->_750 = 0;
        this->_74C = 0.0f;
        return 1;
    }
    bool apply = false;
    if (this->_750 == 0 && this->_754 == 0) {
        apply = true;
    }
    if (apply) {
        this->fn_8038A9D0();
    }
    TVec3f up(0.0f, 1.0f, 0.0f);
    if (this->fn_8038AA30()) {
        PSMTXRotAxisRad(matrix, &up, -this->_74C);
    } else {
        PSMTXRotAxisRad(matrix, &up, this->_74C);
    }
    this->fn_8038AB10();
    this->fn_8038AB70();
    return 1;
}

bool Mario::fn_8038AA30() {
    if (!((this->mMovementStates.mHighWord >> 8) & 1) || this->isStatusActive(0x22)) {
        return false;
    }
    f32 alignment = this->mFrontVec.dot(this->getCamDirX());
    if (this->_1FC.dot(this->getCamDirY()) < 0.0f) {
        alignment = -alignment;
    }
    return (alignment < 0.0f && this->_74C < 0.0f) || (alignment > 0.0f && this->_74C > 0.0f);
}

void Mario::fn_8038C540() {
    if (this->mStateFlags._2 != 0) {
        AudSeLifeEntry* p = this->mLifeAlertCounter;
        if (p == nullptr || p->mIsUsed == 0) {
            this->mLifeAlertCounter = MR::fn_8005BC60(0x258);
        }
        this->mLifeAlertCounter->add(1);
        s32 count = this->mLifeAlertCounter->mFrame;
        s32 v = MR::clamp(0x1A4 - count, 0, 0x1A4);
        if (count % 74 == 5) {
            MR::startActionSound(this->mActor, "LifeAlertTrg", v, -1, -1);
        }
    } else {
        AudSeLifeEntry* p = this->mLifeAlertCounter;
        if (p != nullptr && p->mIsUsed != 0) {
            p->reset();
        }
    }
}

bool Mario::fn_8038CCF0() {
    if (this->isPlayerModeTeresa()) {
        return false;
    }
    if (this->isAnimationRun("着地") || this->isAnimationRun("着地B") || this->isAnimationRun("着地C") ||
        this->isAnimationRun("着地ターン") || this->isAnimationRun("着地幅とび")) {
        return true;
    }
    if (this->isAnimationRun("すべり着地")) {
        return true;
    }
    if (this->isAnimationRun(this->_728)) {
        return true;
    }
    if (this->isAnimationRun("ハード着地")) {
        return true;
    }
    if (this->isAnimationRun("逆着地")) {
        return true;
    }
    if (this->isAnimationRun("ショート着地")) {
        return true;
    }
    return MR::isBckPlaying(this->mActor, "pulloutgraspend");
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

// NONMATCHING: the target-speed and sound branches are approximated
void Mario::fn_803A0ED0(TVec3f* arg1, s32 arg2) {
    if (this->fn_803A0FF0()) {
        *arg1 = this->_328;
    }
    this->calcMoveDir(this->mStickPos.x, this->mStickPos.y, arg1, true);
    this->calcShadowDir(*arg1, &this->_22C);
    this->calcShadowDir(this->mFrontVec, &this->_214);
    if (this->fn_803A1050()) {
        this->fn_803A10D0(arg2);
    }
    if (arg2 == 0) {
        this->setFrontVecKeepUp(this->_22C);
    }
    if (this->isAnimationRun("食べる") || this->isAnimationRun("噛む") || this->isAnimationRun("飲み下す")) {
        this->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
}

// NONMATCHING: table lookups follow the draft; the speed-band selection is approximated
f32 Mario::fn_803A1800() {
    MarioConstTable* e = marioCurTableEntry(this);
    if ((this->mMovementStates.mLowWord >> 16) & 1) {
        return e->mTurnAngleSpeedTornado;
    }
    if ((this->mMovementStates.mHighWord >> 28) & 1) {
        return e->mTurnAngleSpeedSlowWalk;
    }
    if (this->mWalkSpeed > e->mFastTurnSpeed) {
        if (this->isStatusActive(0x1F)) {
            return 0.5f * e->mTurnAngleSpeed3;
        }
        if (this->mTargetWalkSpeedIndex < 5) {
            return e->mGravityStandard;
        }
        return e->mTurnAngleSpeed3;
    }
    return e->mTurnAngleSpeed2;
}

// NONMATCHING: the reset/timer chain is approximated from the draft
void Mario::fn_8039A480() {
    if (this->_430 == 8) {
        if (this->checkTrgZ()) {
            this->resetTornado();
            this->cancelTornadoJump();
            this->stopAnimation((const char*)nullptr, (const char*)nullptr);
            this->changeAnimationInterpoleFrame(1);
            this->mMovementStates.mHighWord |= 0x800000;
            this->playSound("声小ジャンプ", -1);
            if (this->_410 != 0) {
                this->playEffect("スピンライト消去");
            }
        } else {
            this->mMovementStates.mLowWord &= 0xFFFFBFFF;
        }
        if (this->_410 != 0) {
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

// NONMATCHING: saved registers and temporary allocation; recovered full collision response.
void Mario::fn_803882A0() {
    s32 rising = 0;
    if (isRising()) {
        rising = 1;
    }
    if (mActor->_334 != 0) {
        this->_578 = 0;
    }
    if (!((this->mMovementStates.mHighWord >> 29) & 1)) {
        for (u32 i = 0; i < this->_578; i++) {
            Triangle* tri = this->_57C[i];
            TVec3f clipped;
            if (MR::vecKillElement(this->mVelocity, *tri->getNormal(0), &clipped) < 0.0f) {
                this->mVelocity = clipped;
                if (!((this->mFlags20.mWord >> 18) & 1)) {
                    TVec3f projected;
                    f32 depth = MR::vecKillElement(this->mJumpVec, *tri->getNormal(0), &projected);
                    if (depth <= 0.0f) {
                        TVec3f normal(*tri->getNormal(0));
                        normal *= depth;
                        TVec3f half(normal);
                        half *= 0.5f;
                        TVec3f result(projected);
                        result += half;
                        this->mJumpVec = result;
                    }
                } else if (MR::diffAngleAbs(*getGravityVec(), this->mJumpVec) < 0.10471976f) {
                    TVec3f tangent;
                    MR::vecKillElement(*tri->getNormal(0), *getGravityVec(), &tangent);
                    f32 length = PSVECMag(&this->mJumpVec);
                    MR::normalizeOrZero(&tangent);
                    this->mJumpVec += tangent;
                    this->mJumpVec.setLength(length);
                }
                if (rising) {
                    if (tri->getNormal(0)->dot(*getGravityVec()) > 0.707f) {
                        mActor->sendMsgUpperPunch(tri->mSensor);
                    }
                }
                if (isPlayerModeBee() && mActor->mBeeWallWalk == 0) {
                    const char* code = MR::getWallCodeString(tri);
                    if (code && strcmp(code, "Fur") == 0) {
                        Triangle hitTriangle;
                        TVec3f negativeNormal(-*tri->getNormal(0));
                        TVec3f ray(negativeNormal);
                        ray *= 300.0f;
                        TVec3f hitPos;
                        const TVec3f& origin = mActor->_240;
                        bool hit;
                        if (fn_803B2950()) {
                            hit = MR::getFirstPolyOnLineToDashYoshiStepAndMap(&hitPos, &hitTriangle, origin, ray);
                        } else {
                            hit = MR::getFirstPolyOnLineToMap(&hitPos, &hitTriangle, origin, ray);
                        }
                        if (hit) {
                            const char* hitCode = MR::getWallCodeString(&hitTriangle);
                            if (hitCode && strcmp(hitCode, "Fur") == 0) {
                                mActor->entryWallWalkMode(hitPos, *hitTriangle.getNormal(0));
                            }
                        }
                    }
                }
            }
        }
    }
}

// NONMATCHING: 0x274 bookkeeping approximated
// NONMATCHING: equivalent early-exit branch orientation and stack allocation.
void Mario::fn_803886B0() {
    if ((mFlags20.mWord >> 19) & 1) {
        return;
    }
    if (mActor->mBinderDirection5DC.dot(_368) >= 0.707f) {
        return;
    }
    if (MR::isNearZero(mActor->mBinderDirection5DC, 1.0f)) {
        return;
    }
    TVec3f direction(mActor->mBinderDirection5DC);
    MR::normalizeOrZero(&direction);
    TVec3f horizontal;
    f32 component = MR::vecKillElement(mVelocity, direction, &horizontal);
    if (component >= 0.0f) {
        return;
    }
    mVelocity = horizontal;
    MR::normalizeOrZero(&direction);
    f32 dot = direction.dot(mFrontVec);
    f32 weight = 1.0f - MR::clamp(1.4285715f * (-dot - 0.3f), 0.0f, 1.0f);
    if (!((mMovementStates.mLowWord >> 23) & 1U) && weight < 0.1f) {
        if (((mMovementStates.mLowWord >> 5) & 1U)) {
            TVec3f correction(getSideWallNorm());
            correction *= -component;
            mVelocity += correction;
        } else {
            s32 count = mActor->mBinder->mPlaneNum;
            if (count != 0) {
                f32 bestDot = 0.0f;
                u32 bestIndex = 0;
                for (u32 i = 0; i < count; i++) {
                    f32 facing = mActor->mBinder->getPlane(i)->mParentTriangle.getNormal(0)->dot(mFrontVec);
                    if (facing < bestDot) {
                        bestIndex = i;
                        bestDot = facing;
                    }
                }
                const TVec3f& contact = mActor->mBinder->getPlane(bestIndex)->mHitPos;
                TVec3f delta(mPosHolder->mPos);
                delta -= contact;
                TVec3f correction;
                MR::vecKillElement(delta, *getGravityVec(), &correction);
                MR::vecKillElement(correction, direction, &correction);
                if (!MR::isNearZero(correction)) {
                    correction.setLength(-component);
                    mVelocity += correction;
                }
            }
        }
    }
    mTargetWalkSpeedIndex = 0;
    mWalkSpeed *= weight;
}

// NONMATCHING: equivalent comet-condition boolean lowering and unused pointer argument setup.
void Mario::fn_80388DB0() {
    if (!((this->mMovementStates.mLowWord >> 9) & 1)) {
        if (this->checkDamage() == false) {
            if (this->isPlayerModeYoshi()) {
                this->mActor->fn_803B9810()->fn_8041DF40();
            }
            if (!this->isPlayerModeYoshi() || this->mActor->fn_803B9810()->fn_8041F8A0() == 0) {
                if (_97C != nullptr) {
                    this->sendStateMsg(2);
                } else if ((this->mMovementStates.mLowWord >> 31) != 0) {
                    this->procJump(false);
                    this->checkTornado();
                    this->checkHang();
                    this->checkWallStick();
                    this->fn_803B0E60();
                } else {
                    this->fn_803A0BE0();
                    this->updateWalkSpeed();
                    this->fn_803B0E60();
                }
            }
            this->fn_803B3CE0();
            if ((this->mMovementStates.mLowWord >> 30) & 1) {
                this->mMovementStates.mHighWord |= 0xC000;
                this->doFrontStep();
            }
            if (this->isStatusActive(0x28)) {
                this->mActor->fn_803B9A30();
                mFlags14.mHighWord &= 0xFFDFFFFF;
                this->mActor->fn_803B9830()->fn_8041BB90();
                return;
            }
            u32 t = this->mMovementStates.mLowWord;
            if (((t >> 30) & 1) || !((t >> 12) & 1)) {
                mFlags14.mHighWord &= 0xFFEFFFFF;
            }
        }
        this->fn_803A7540();
        if (this->getPlayerMode() != 6 && ((this->mMovementStates.mLowWord >> 23) & 1) && this->mFrontWallTriangle->isValid() &&
            this->mFrontWallTriangle->getNormal(0)->dot(this->mVelocity) < 0.0f) {
            MR::vecKillElement(this->mVelocity, *this->mFrontWallTriangle->getNormal(0), &this->mVelocity);
        }
        if (this->mActor->fn_803B9840() != 0) {
            this->mActor->fn_803B9830()->fn_8041BBB0();
        }
        if (this->isStickOn()) {
            if (this->isAnimationRun("水泳陸うちあげ")) {
                this->stopAnimation((const char*)nullptr, (const char*)nullptr);
            }
        }
        if (!((this->mDrawStates.mWord >> 27) & 1)) {
            this->_3FE = 0;
        }
        if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
            if (mCometRestrictions != 0 && mPreviousCometRestrictions == 0) {
                MR::addCometTimer(3600);
                MR::pauseCometTimer();
            }
            if (mCometRestrictions == 0 && mPreviousCometRestrictions != 0) {
                MR::resumeCometTimer();
            }
        }
        mPreviousCometRestrictions = mCometRestrictions;
        this->callExtraTasks(0);
        this->beeMarioOnGround();
        this->checkSpecialWaitAnimation();
        this->fn_803B4280();
        if (!((this->mDrawStates.mWord >> 9) & 1)) {
            this->retainMoveDir(mStickPos.x, mStickPos.y, nullptr);
        }
    }
}

// NONMATCHING: the two-stage neighbourhood test is approximated (returns the matched-triangle flag)

// NONMATCHING: independent loads and register scheduling; unused legacy argument.
void Mario::fn_8038C620(s32) {
    if (this->mStateFlags._1 != 0) {
        MR::startLevelSoundPlayer("JumpFromYoshi", -1, -1);
    } else if (this->mStateFlags._0 != 0) {
        if (this->isPlayerModeYoshi()) {
            if (this->mActor->fn_803B97F0()->mDashTimer > 0) {
                MR::startActionSound(this->mActor->fn_803B97F0(), "DashJump", -1, -1, -1);
            } else {
                MR::startActionSound(this->mActor->fn_803B97F0(), "JumpS", -1, -1, -1);
            }
        } else {
            this->playSound("声小ジャンプ", -1);
        }
        this->playSound("ジャンプ踏切", -1);
    }
    if (this->isPlayerModeYoshi()) {
        if (MR::isPlayerElementModeYoshiDash() && !MR::fn_804225C0()) {
            s32 v28 = MR::clamp((s32)(500.0f * MarioAccess::fn_803B57A0()), 0, 1000);
            s32 v3 = MR::clamp((s32)(400.0f * MarioAccess::fn_803B57A0()) + 200, 400, 1000);
            s32 t5 = this->_A98;
            s32 d = v3 - t5;
            s32 var_r31;
            if ((d < 0 ? -d : d) > 150) {
                var_r31 = MR::clamp(v3, t5 - 66, t5 + 66);
            } else {
                var_r31 = MR::clamp(v3, t5 - 22, t5 + 22);
            }
            s32 temp_r29 = this->mActor->fn_803B97F0()->mDashTimer;
            MR::startActionSoundPlayer("DashLV", v28, var_r31);
            MR::startActionSoundPlayer("DashLVWind", v28, -1);
            MR::startActionSoundPlayer("DashLVFire", temp_r29, -1);
            this->fn_803B2950();
            this->_A98 = var_r31;
        } else {
            this->_A98 = 0x226;
        }
        if (PSVECMag(&this->mVelocity) >= 2.0f && this->isAnimationRun("口に含む") && this->isAnimationRun("基本")) {
            if (this->getAnimator()->fn_803DF0E0() >= 2.5f) {
                MR::startActionSoundPlayer("RunMouthFull", -1, -1);
            }
        }
    }
    this->mStateFlags.fn_8038C890();
}

// NONMATCHING: the wall-contact push-out is approximated
void Mario::fn_8038E1A0(const HitInfo* hit, s32 inside) {
    const Triangle* tri = &hit->mParentTriangle;
    if (MR::isThroughPolygon(tri) || this->isThroughWall(tri)) {
        return;
    }
    if (hit->isCollisionAtEdge() || hit->isCollisionAtCorner()) {
        this->mDrawStates.mWord |= 0x400000;
        return;
    }
    const TVec3f* normal = tri->getNormal(0);
    f32 alignment = normal->dot(this->_368);
    if (!MR::isNearZero(alignment, 0.3f) || inside != 0) {
        TVec3f movement(this->_16C);
        MR::normalizeOrZero(&movement);
        TVec3f inward = -*normal;
        if (!(movement.dot(inward) < 0.707f)) {
            f32 approach = this->_16C.dot(*normal);
            f32 depth = hit->mDistance;
            if (!(MR::abs(approach) < depth) && !MR::isNearZero(depth, 1.01f)) {
                TVec3f offset(hit->mHitPos);
                offset -= this->mPosHolder->mPos;
                if (!(offset.dot(this->mHeadVec) < 0.0f)) {
                    TVec3f push(*normal);
                    push *= depth - 1.0f;
                    this->addTrans(push, nullptr);
                    u32 flags = this->mMovementStates.mLowWord;
                    if (!((flags >> 30) & 1) && (flags >> 31) != 0 && !this->isRising() && normal->dot(*this->getGravityVec()) < -0.99f) {
                        this->addVelocity(this->mFrontVec, 5.0f);
                    }
                }
            }
        }
    }
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
        mMovementStates.mHighWord &= 0xFFFFFF7F;
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
    if (_3CE > constants->mTable[constants->mCurrentTable]->mJumpConnectTime || !((mMovementStates.mLowWord >> 26) & 1)) _430 = 0;
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
        mMovementStates.mLowWord &= 0xF7FFFFFF;
        _3D2 = 0;
    }
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
        f32 f31 = this->fn_8039A810();
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
        this->addVelocity(this->mJumpVec);
        f32 f31 = this->fn_8039AA40();
        f32 f30 = this->fn_8039A810();
        TVec3f sp8(this->mActor->mAppliedGravityVec);
        sp8 *= f30;
        TVec3f sp14(sp8);
        sp14 *= f31;
        this->mJumpVec += sp14;
    }
}

// NONMATCHING: temporary/register allocation and discarded comparison bookkeeping.
void Mario::fn_8038A600() {
    HitSensor* sensor = mActor->_470;
    if (isAnimationRun("投げ")) {
        sensor = mActor->_474;
    }
    if (mActor->isPunching()) {
        sensor = mActor->_924;
    }
    if (sensor == nullptr) {
        fn_8038A840(nullptr, 0.8f * this->_58.x, 0.0f, 0.8f * this->_58.z);
        if (this->_12C != 0) {
            this->_12C--;
        }
    } else {
        TVec3f previous(this->mSideVec);
        TVec3f up(this->_1FC);
        TVec3f front(this->mFrontVec);
        const TVec3f& origin = mPosHolder->mPos;
        TVec3f direction(*MR::getSensorPos(sensor));
        direction -= origin;
        MR::normalizeOrZero(&direction);
        if (!MR::isNearZero(direction, 0.001f)) {
            TVec3f projected;
            MR::vecKillElement(direction, up, &projected);
            if (!MR::normalizeOrZero(&projected)) {
                f32 angle = JMAAcosRadian(projected.dot(front));
                TVec3f cross;
                PSVECCrossProduct(&front, &projected, &cross);
                if (cross.dot(up) < 0.0f) {
                    angle = -angle;
                }
                JMAAcosRadian(projected.dot(direction));
                bool positive = direction.dot(up) >= 0.0f;
                bool narrow = MR::abs(angle) < 1.5707964f;
                bool differs = positive != narrow;
                (void)differs;
                f32 zero = 0.0f;
                f32 limit = 0.1f * this->_406;
                if (limit > 1.0471976f) {
                    limit = 1.0471976f;
                }
                f32 clamped = MR::clamp(angle, -limit, limit);
                fn_8038A840(nullptr, clamped, 0.0f, zero);
                this->_12C = 0x10;
            }
        }
    }
}

// NONMATCHING: register allocation and position-reference load scheduling.
bool Mario::fn_80391330(const HitInfo* hit, const TVec3f& normal, const TVec3f& direction) {
    f32 angle = 180.0f * (marioAcos(-getGravityVec()->dot(normal)) / 3.14159f);
    TVec3f fromBase(hit->mHitPos);
    fromBase.sub(mPosHolder->mPos);
    MR::normalizeOrZero(&fromBase);
    bool result = false;
    bool slip = false;
    bool inFront = false;
    bool flat = false;
    if (angle >= mActor->getConst().getTable()->mFlatAngle && direction.dot(mFrontVec) > 0.0f) flat = true;
    if (flat && fromBase.dot(mFrontVec) > 0.0f) inFront = true;
    if (inFront && calcAngleD(normal) <= mActor->getConst().getTable()->mSlipAngle) slip = true;
    if (slip && MR::diffAngleAbs(_368, normal) > 30.0f) result = true;
    return result;
}

// NONMATCHING: explicit typed mode14 storage dispatch, register allocation and boolean lowering.
f32 Mario::fn_8039A810() {
    u16 timer = this->_76C;
    if (timer != 0) {
        MarioConstTable* entry = this->mActor->mConst->getTable();
        f32 ratio = static_cast<f32>(timer) / static_cast<f32>(entry->mBeeGravityReviveTime);
        f32 multiplier = this->fn_8039ADC0();
        return ((1.0f - ratio) + ratio * entry->mGravityAirWalk) * multiplier;
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

// NONMATCHING: temporary register allocation; verified collision-sensor and velocity gates.
void Mario::fn_803A02F0(s32 keepSlip, s32 hard, s32 blend) {
    u32 mode = _430;
    if (mode - 4 <= 2 || mode == 8 || mode == 2 || mode == 11) keepSlip = 1;
    if (((mFlags14.mLowWord >> 8) & 1) && _8D8 == _45C->mSensor) {
        TVec3f delta(mJumpVec);
        delta -= _8DC;
        if (delta.dot(mJumpVec) < 0.0f) mJumpVec.zero();
        else mJumpVec = delta;
    }
    if ((mFrontVec.dot(mJumpVec) > 6.0f && keepSlip == 0) || (isPlayerModeYoshi() && mActor->fn_803B97F0()->mDashTimer > 0)) {
        f32 speed = fn_803B1CA0();
        mWalkSpeed = mFrontVec.dot(mJumpVec) / speed;
        mWalkSpeed = MR::clamp(mWalkSpeed, 0.0f, 2.0f);
        stopAnimation(nullptr, "基本");
        _3FA = 0;
        _71E = 0;
        if (!((mFlags14.mLowWord >> 21) & 1)) recordTurnSlipAngle();
    } else fn_803A0490(hard, blend);
}

// NONMATCHING: temporary allocation and register scheduling.
void Mario::fn_803A0BE0() {
    this->fn_803A2CC0();
    this->mMovementStates.mHighWord = (this->mMovementStates.mHighWord & ~0x400u) | ((this->isSkatableFloor() << 10) & 0x400u);
    this->fn_803A2CF0();
    s32 sp8 = this->fn_803A2D10();
    this->fn_803A2D50();
    if (this->fn_803A2D80() == 0 && this->fn_803A3030() == 0 && this->fn_803A3130() == 0) {
        this->checkTornado();
        this->fixPositionInTower();
        TVec3f spC;
        MR::setNan(spC);
        s32 var_r31 = 0;
        f32 t31 = mStickPos.y;
        if (MR::isNearZero(mStickPos.x, 0.001f) && MR::isNearZero(t31, 0.001f)) {
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
        if (!((this->mMovementStates.mLowWord >> 27) & 1)) {
            this->fn_803A1FF0(rate, sp8);
        } else {
            this->fn_803A20E0(rate, sp8);
        }
        this->fn_803A2180();
        this->fn_803A28D0(&spC);
    }
}

// NONMATCHING: temporary allocation and register scheduling; full velocity dispatch recovered.
void Mario::fn_803A2A70() {
    MarioConst* constants = mActor->mConst;
    f32 speed = constants->mTable[constants->mCurrentTable]->mWalkSpeed;
    if ((this->mMovementStates.mHighWord >> 11) & 1) {
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
    if ((this->mDrawStates.mWord >> 26) & 1) {
        f32 walkSpeed = mWalkSpeed;
        TVec3f direction(this->_22C);
        direction *= speed;
        TVec3f scaled(direction);
        scaled *= walkSpeed;
        addVelocity(scaled);
        return;
    }
    f32 walkSpeed = mWalkSpeed;
    TVec3f direction(this->mFrontVec);
    direction *= speed;
    TVec3f scaled(direction);
    scaled *= walkSpeed;
    addVelocity(scaled);
}

// NONMATCHING: temporary/register scheduling; full landing switch and return structure recovered.
void Mario::fn_803A0490(s32 arg1, s32 arg2) {
    s32 var_r29 = arg1;
    s32 var_r28 = arg2;
    if (this->_430 == 4) {
        TVec3f sp14(-this->_220);
        this->setFrontVecKeepUp(sp14);
    }
    if (this->isAnimationRun("飛び込みジャンプ") || this->isAnimationRun("後方飛び込みジャンプ") || ((this->mFlags14.mLowWord >> 23) & 1)) {
        if (this->isAnimationRun("後方飛び込みジャンプ")) {
            TVec3f sp8(-this->mFrontVec);
            this->setFrontVecKeepUp(sp8);
        }
        if ((this->mFlags14.mLowWord >> 23) & 1) {
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
    if (!((this->mDrawStates.mWord >> 19) & 1)) {
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
                MR::startActionSound(this->mActor->fn_803B97F0(), "Land", -1, -1, -1);
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

void Mario::fn_803A1330(s32* arg1) {
    f32 alignment = this->_214.dot(this->_22C);
    if (this->_3D2 != 0) {
        alignment = this->_3E4.dot(this->_22C);
    }
    if (this->fn_803A1E60(this->_214.dot(this->_22C))) {
        this->recordTurnSlipAngle();
    } else {
        this->fn_803A1F60();
    }
    if (((this->mMovementStates.mLowWord >> 16) & 1) || ((this->mMovementStates.mHighWord >> 11) & 1)) {
        this->_3D2 = 0;
        this->mMovementStates.mLowWord &= 0xFFFF7FFF;
    }
    if (this->_3D2 != 0) {
        this->fn_803A1F80();
        this->fn_803A1FC0();
        if (this->isAnimationRun("ブレーキ") || ((this->mMovementStates.mLowWord >> 15) & 1)) {
            *arg1 = 1;
        }
    } else {
        if (!((this->mMovementStates.mHighWord >> 11) & 1) &&
            !((this->mMovementStates.mLowWord >> 21) & 1) && ((this->mMovementStates.mLowWord >> 15) & 1) &&
            !((this->mMovementStates.mLowWord >> 16) & 1) && !((this->mMovementStates.mHighWord >> 28) & 1) && !this->isStatusActive(0x1F)) {
            if (alignment < JMACosRadian(this->mActor->mConst->getTable()->mTurnSlipAngle) && !((this->mMovementStates.mLowWord >> 27) & 1)) {
                if ((this->mMovementStates.mHighWord >> 10) & 1) {
                    *arg1 = 1;
                }
                this->_220 = this->_3E4;
                if (*arg1 == 0) {
                    TVec3f front = -this->_220;
                    this->setFrontVecKeepUp(front);
                }
                if ((this->mMovementStates.mHighWord >> 10) & 1) {
                    this->_3D0 = static_cast<u16>(this->mActor->mConst->getTable()->mTurnSlipTimeB);
                    this->mMovementStates.mLowWord |= 0x08000000;
                    this->changeAnimation("ターンブレーキ滑り床", static_cast<const char*>(nullptr));
                    this->_2B8 = this->mActor->getLastMove();
                    this->stopWalk();
                    this->_754 = 10;
                    this->pushTask(&Mario::taskOnSlipTurn, 1);
                } else {
                    this->_3D0 = static_cast<u16>(this->mActor->mConst->getTable()->mTurnSlipTime);
                    this->mMovementStates.mLowWord |= 0x08000000;
                    this->mWalkSpeed = 0.0f;
                    this->changeAnimation("ターンブレーキ", static_cast<const char*>(nullptr));
                    this->playEffect("共通ブレーキ");
                }
            }
        }
        this->mMovementStates.mLowWord &= 0xFFFF7FFF;
    }
}

void Mario::fn_803A1600(s32* arg1) {
    u16 timer = this->_3D0;
    if (timer != 0) {
        if (timer == static_cast<u16>(this->mActor->mConst->getTable()->mTurnSlipTime)) {
            this->startPadVib(2);
        }
        this->_3D0 -= 1;
        if (!((this->mMovementStates.mHighWord >> 10) & 1)) {
            this->mWalkSpeed = 0.0f;
        }
    }
    if (this->_3D0 == 0) {
        if ((this->mMovementStates.mLowWord >> 27) & 1) {
            const TVec3f* gravity = this->getGravityVec();
            if (MR::diffAngleAbsHorizontal(this->getWorldPadDir(), this->mFrontVec, *gravity) > 1.5707964f) {
                this->stopAnimation("ターンブレーキ", static_cast<const char*>(nullptr));
                TVec3f front = -this->mFrontVec;
                this->setFrontVecKeepUp(front);
                this->mMovementStates.mLowWord &= 0xF7FFFFFF;
            }
            if (this->isAnimationTerminate("ターンブレーキ")) {
                this->stopAnimation("ターンブレーキ", static_cast<const char*>(nullptr));
                this->mMovementStates.mLowWord &= 0xF7FFFFFF;
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

// NONMATCHING: cached position pointers, integer conversion scheduling and stack allocation.
void Mario::fn_8038B9F0() {
    TVec3f offset;
    offset.zero();
    if (isStatusActive(MarioStatus_Blown)) {
        return;
    }
    if (mActor->_934) {
        MR::vecBlend(&_13C, _13C, offset, 0.3f);
    }
    MarioState* state = _97C;
    if (state == nullptr) {
        if (isAnimationRun("壁押し")) {
            TVec3f wallDelta(_4E8);
            wallDelta -= mPosHolder->mPos;
            TVec3f horizontal;
            MR::vecKillElement(wallDelta, mHeadVec, &horizontal);
            f32 length = PSVECMag(&horizontal);
            TVec3f wallOffset(*mFrontWallTriangle->getNormal(0));
            wallOffset *= length - 35.0f;
            offset = wallOffset;
        }
    } else if (state->mStatusId == MarioStatus_Swim) {
        f32 surfaceOffset = mSwim->mSurfaceOffset;
        const TVec3f& swimDirection = -*getGravityVec();
        TVec3f swimOffset(swimDirection);
        swimOffset *= surfaceOffset;
        offset += swimOffset;
    }
    if (((mMovementStates.mLowWord >> 30) & 1U) && !((mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31)) {
        f32 verticalSpeed = mVerticalSpeed;
        if (verticalSpeed > 5.0f && verticalSpeed < 65.0f && !isCurrentShadowFloorDangerAction()) {
            TVec3f direction(mGroundPos);
            direction -= mShadowPos;
            MR::normalizeOrZero(&direction);
            if (direction.dot(_368) < 0.1f) {
                f32 speed = mVerticalSpeed;
                TVec3f floorOffset(getAirGravityVec());
                floorOffset *= speed;
                offset += floorOffset;
            } else {
                TVec3f groundDelta(mGroundPos);
                groundDelta -= mPosHolder->mPos;
                TVec3f horizontal;
                f32 distance = MR::vecKillElement(groundDelta, *getGravityVec(), &horizontal);
                TVec3f floorOffset(*getGravityVec());
                floorOffset *= distance;
                offset += floorOffset;
            }
        }
    }
    TVec3f sinkOffset;
    sinkOffset.zero();
    if (mSinkTimer != 0) {
        TVec3f first(*getGravityVec());
        first *= static_cast<f32>(mSinkTimer);
        TVec3f second(first);
        second *= 0.00390625f;
        TVec3f third(second);
        third *= 120.0f;
        sinkOffset = third;
        f32 sink = static_cast<f32>(mSinkTimer);
        const TVec3f& sinkDirection = -_1FC;
        TVec3f fourth(sinkDirection);
        fourth *= sink;
        TVec3f fifth(fourth);
        fifth *= 0.00390625f;
        TVec3f sixth(fifth);
        sixth *= 120.0f;
        sinkOffset = sixth;
    }
    f32 blendRate = 0.2f;
    if (!isUseSimpleGroundCheck()) {
        if (fn_8038CCF0()) {
            blendRate = 0.01f;
        } else if (mTargetWalkSpeedIndex == 0 && mWalkSpeed < 0.2f && isBlendWaitGround()) {
            f32 length = PSVECMag(&offset);
            if (PSVECMag(&_148) > length) {
                blendRate = 0.5f;
            } else {
                blendRate = 0.02f * static_cast<f32>(_3CE);
                if (blendRate >= 0.2f) {
                    blendRate = 0.2f;
                }
            }
        }
    }
    MR::vecBlend(&_148, _148, offset, blendRate);
    MR::vecBlend(&_154, _154, sinkOffset, 0.2f);
    TVec3f combined(_148);
    combined += _154;
    _13C = combined;
}

void Mario::fn_80395400(u32 arg) {
    this->pushTask(&Mario::taskOnEffectCheck, arg);
}

// NONMATCHING: independent load scheduling and register allocation.
void Mario::updateWalkSpeed() {
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
    if (((mMovementStates.mLowWord >> 16) & 1) || isStatusActive(17)) {
        constants = mActor->mConst;
        target *= constants->mTable[constants->mCurrentTable]->mTornadoMultiply;
    }
    s32 wasSquatting = (mMovementStates.mLowWord >> 21) & 1;
    if (wasSquatting && ((mFlags20.mWord >> 16) & 1)) {
        fn_803A6BE0();
    } else {
        mMovementStates.mLowWord &= 0xFFDFFFFF;
        if (_436 == 0 && _434 == 0 && checkSquat(false) && mSinkTimer <= 32 && !isStatusActive(31)) {
            if (!checkLockOnHoming()) mMovementStates.mLowWord |= 0x200000;
            if (!wasSquatting) {
                u32 flags = mMovementStates.mLowWord;
                if (((flags >> 21) & 1) && (((flags >> 23) & 1) || ((mMovementStates.mHighWord >> 13) & 1))) {
                    mTargetWalkSpeedIndex = 0;
                    mWalkSpeed = 0.0f;
                }
            }
        }
        if (((mFlags20.mWord >> 16) & 1) && !((mMovementStates.mLowWord >> 21) & 1) && isAnimationRun("しゃがみ終了")) mMovementStates.mLowWord |= 0x200000;
        u32 flags = mMovementStates.mLowWord;
        if (!((flags >> 21) & 1) && wasSquatting) {
            mMovementStates.mLowWord = flags | 0x200000;
            cancelSquatMode();
            _71E = 0;
        }
    }
    if (_3D0 != 0) target = 0.0f;
    if ((mMovementStates.mLowWord >> 15) & 1) target = 0.0f;
    f32 inertia = decideInertia(target);
    if (sPreviousInertiaName != sInertiaName) sPreviousInertiaName = sInertiaName;
    if (mCometFireDanceArea && mTargetWalkSpeedIndex != 0) {
        inertia = 1.0f;
        target = inertia;
        mWalkSpeed = 1.5f;
    }
    if (!((mMovementStates.mLowWord >> 21) & 1) && (getPlayerMode() == 1 || mCometFireDanceArea)) {
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
        if (mActor->fn_803B97F0()->mDashTimer > 0) {
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
    mWalkSpeed = mWalkSpeed * inertia + target * static_cast<f32>(256 - mSinkTimer) * 0.00390625f * (1.0f - inertia);
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
        if ((mMovementStates.mLowWord >> 15) & 1) {
            sInertiaName = "ターン滑り";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaTurnSlip;
        }
        if (!((mMovementStates.mLowWord >> 15) & 1) && isAnimationRun("歩行制動ブレーキ")) {
            sInertiaName = "ターン後滑り";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaTurning;
        }
        if ((mMovementStates.mLowWord >> 27) & 1) {
            sInertiaName = "ターン中";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaJumpFinish;
        }
        if (_3CE < 30) {
            sInertiaName = "ジャンプ終了";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaReflectSlip;
        }
        if ((mMovementStates.mLowWord >> 21) & 1) {
            sInertiaName = "しゃがみ";
            c = mActor->mConst;
            inertia = c->mTable[c->mCurrentTable]->mInertiaSquat;
        }
    }
    if ((mMovementStates.mLowWord >> 16) & 1) {
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
    if (mWalkSpeed < 0.08f && _3CE > 10 && speed > 0.5f && (((mMovementStates.mLowWord >> 21) & 1) || ((mMovementStates.mLowWord >> 19) & 1))) {
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

// NONMATCHING: independent load scheduling and temporary/register allocation.
void Mario::fixHeadFrontVecByGravity() {
    TVec3f groundNormal(_368);
    MR::diffAngleAbs(mActor->mCameraBasisZ, mActor->mPostureReference1044);
    if (MR::isCameraInterpolationStateReady() && isStickOn()) {
        if (_60D != 1) _60D = 1;
        mDrawStates.mWord |= 0x40000;
        mFlags14.mLowWord |= 0x200000;
        _40E = 0;
    }
    if (_3D0 != 0 || _3D2 != 0) mDrawStates.mWord &= 0xFFFBFFFF;
    if (!_60D && mActor->mAxisRotationDisabled6A0 != 0) mDrawStates.mWord &= 0xFFFBFFFF;
    if (MR::isInAreaObj("PlayerSideRetainOffArea", getTrans())) mDrawStates.mWord &= 0xFFFBFFFF;
    if ((isStatusActive(30) || isStatusActive(29)) && _60D) _60D = 0;
    TVec3f previousHead(mHeadVec);
    TVec3f up(-*getGravityVec());
    MR::normalize(&up);
    if (mHeadVec.dot(up) < -0.9f && (mMovementStates.mLowWord >> 31)) this->_A71 = 1;
    else if ((mMovementStates.mLowWord >> 30) & 1) this->_A71 = 0;
    f32 rate = fn_803B2CC0(&up);
    if (_60D && mTargetWalkSpeedIndex != 0 && (((mMovementStates.mLowWord >> 30) & 1) || !(mMovementStates.mLowWord >> 31))) {
        if (!((mDrawStates.mWord >> 18) & 1) || !fn_803B2BD0()) {
            fn_803B23E0(&up, &mSideVec, rate);
            fixFrontVecFromUpSide();
            _29C = groundNormal;
            _290 = mSideVec;
        }
    } else {
        if (this->_A71 != 0) {
            fn_803B1EF0(&up, rate);
            setFrontVecKeepUp(mFrontVec);
        } else if (fn_803B1DE0(&up)) {
            fn_803B23E0(&up, &mFrontVec, rate);
            setFrontVecKeepUp(mFrontVec);
        } else {
            fn_803B23E0(&up, &mSideVec, rate);
            fixFrontVecFromUpSide();
        }
        MarioConst* constants = mActor->mConst;
        MarioConstTable* table = constants->mTable[constants->mCurrentTable];
        if (MR::diffAngleAbs(_43C, mFrontVec) < table->mFrontAngleFixMargin) setFrontVec(_43C);
        constants = mActor->mConst;
        table = constants->mTable[constants->mCurrentTable];
        if (MR::diffAngleAbs(mHeadVec, previousHead) < table->mHeadAngleFixMargin) {
            mHeadVec = previousHead;
            MR::normalize(&mHeadVec);
        }
        if (fn_803B2470()) {
            mSideVec = _290;
            MR::normalize(&mSideVec);
            f32 angle = marioAcos(groundNormal.dot(_29C));
            TVec3f axis;
            PSVECCrossProduct(&_29C, &groundNormal, &axis);
            MR::normalizeOrZero(&axis);
            if (MR::isNearZero(axis, 0.001f) || MR::isNearZero(_29C + groundNormal, 0.001f) || MR::isNearZero(_29C - groundNormal, 0.001f)) {
                if (_60D != 1) _60D = 1;
            } else {
                if (!MR::isNearZero(axis, 0.001f)) {
                    TMtx34f rotation;
                    PSMTXRotAxisRad(rotation, axis, angle);
                    PSMTXMultVec(rotation, mSideVec, mSideVec);
                    _29C = groundNormal;
                    _290 = mSideVec;
                }
                if (_60D != 0) _60D = 0;
            }
            fixFrontVecFromUpSide();
            return;
        }
        TVec3f axis;
        PSVECCrossProduct(&groundNormal, &_22C, &axis);
        MR::normalizeOrZero(&axis);
        if (!MR::isNearZero(axis, 0.001f)) _290 = axis;
        else _290 = mSideVec;
        if (_60D != 0) _60D = 0;
    }
}
