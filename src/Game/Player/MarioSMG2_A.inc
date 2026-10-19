// SMG2-only Mario:: functions (fn_ range A), written from the asm. Included at the end of Mario.cpp (unity TU).
// PLACEHOLDER(enemies): FLD<T>(self, off) typed access to a Mario field by its SMG2 byte offset where the header name is unknown; resolve when the Mario layout is recovered and the fields are named
template <typename T, typename P>
inline T& FLD(P* p, u32 off) { return *reinterpret_cast<T*>(reinterpret_cast<u8*>(const_cast<void*>(static_cast<const void*>(p))) + off); }


void Mario::fn_803880D0() {
    if ((this->_1C_WORD >> 15) & 1) {
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
    if (((this->mMovementStates_HIGH_WORD >> 8) & 1) || ((this->_10_LOW_WORD >> 10) & 1)) {
        TVec3f sp38(this->mPosHolder->mPos);
        sp38 -= FLD<TVec3f>(this, 0x67C);
        MR::vecKillElement(sp38, FLD<TVec3f>(this, 0x694), &sp38);
        TVec3f* temp_r31 = &this->mPosHolder->mPos;
        TVec3f sp14(FLD<TVec3f>(this, 0x67C));
        sp14 += sp38;
        TVec3f sp20(sp14);
        sp20 -= *temp_r31;
        this->mPosHolder->fn_80406010(sp20);
        MR::vecKillElement(this->mVelocity, FLD<TVec3f>(this, 0x694), &this->mVelocity);
        TVec3f sp8(FLD<TVec3f>(this, 0x688));
        sp8 -= FLD<TVec3f>(this, 0x67C);
        TVec3f sp2C;
        f32 temp_f1 = MR::vecKillElement(sp8, FLD<TVec3f>(this, 0x694), &sp2C);
        f32 var_f31 = temp_f1;
        if (temp_f1 > 1.0f) {
            var_f31 = 1.0f;
        } else if (temp_f1 < -1.0f) {
            var_f31 = -1.0f;
        }
        sp2C = FLD<TVec3f>(this, 0x694);
        sp2C.setLength(var_f31);
        FLD<TVec3f>(this, 0x67C) += sp2C;
    }
}

void Mario::fn_803885F0() {
    if (!((this->mMovementStates_HIGH_WORD >> 29) & 1)) {
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
    FLD<u8>(this, 0x89C) = 0;
    if (!((this->mMovementStates_LOW_WORD >> 9) & 1) && MR::testFpViewStartTrigger()) {
        if (MR::isPossibleToShiftToFirstPersonCamera() == false) {
            if (!MR::isDemoActive()) {
                if (!MR::isStageStoryBook() && !MR::isStageFileSelect() && !MR::isStageEpilogueDemo() && !MR::isStageStaffRoll()) {
                    MR::startSystemSE("SE_SY_CAMERA_NG", -1, -1);
                }
                FLD<u8>(this, 0x89C) = 1;
            }
        } else {
            this->tryFpViewMode();
        }
    }
}

void Mario::fn_803897A0() {
    u32 temp_r4 = this->mMovementStates_HIGH_WORD;
    if ((temp_r4 >> 28) & 1) {
        if (!((this->mMovementStates_LOW_WORD >> 30) & 1)) {
        }
        if ((temp_r4 >> 27) & 1) {
            if (!this->isSlipPolygon(this->mGroundPolygon)) {
            }
            const TVec3f* nrm = this->mGroundPolygon->getNormal(0);
            f32 temp_f31 = MR::vecKillElement(this->mVelocity, *nrm, &this->mVelocity);
            nrm = this->mGroundPolygon->getNormal(0);
            TVec3f sp14(FLD<TVec3f>(this, 0x368));
            sp14 -= *nrm;
            TVec3f sp20(sp14);
            sp20 *= 0.5f;
            f32 temp_f31_2 = temp_f31 < 0.0f ? -temp_f31 : temp_f31;
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
    if (!this->isNonFixHeadVec() && arg2 == 0 && !((this->mMovementStates_LOW_WORD >> 9) & 1)) {
        this->fixHeadFrontVecByGravity();
    }
    if (!this->updatePosture(arg1)) {
        this->createDirectionMtx(arg1);
        PSMTXConcat(_C4, arg1, arg1);
        this->fn_8038A600();
    }
}

void Mario::fn_8038A840(void*, f32 x, f32 y, f32 z) {
    FLD<TVec3f>(this, 0x60).set(x, y, z);
    MR::orderRotateMtx(0, FLD<TVec3f>(this, 0x60), FLD<TMtx34f>(this, 0x6C));
    MR::orderRotateMtx(0, -FLD<TVec3f>(this, 0x60), FLD<TMtx34f>(this, 0x9C));
}

void Mario::fn_8038A9D0() {
    TMtx34f sp18;
    PSMTXRotAxisRad(sp18, &this->mHeadVec, FLD<f32>(this, 0x74C));
    TVec3f sp8;
    PSMTXMultVec(sp18, &this->mFrontVec, &sp8);
    this->setFrontVecKeepUp(sp8);
    FLD<f32>(this, 0x74C) = 0.0f;
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
    if (((u32)this->_10_LOW_WORD >> 31) != 0) {
    }
    return (u32)(this->getCurrentStatus() - 8) <= 2U;
}

bool Mario::fn_803949E0() {
    if (this->isEnableAddDamage() == false) {
    }
    if (this->isStatusActive(6)) {
        this->mMovementStates_LOW_WORD |= 0x10;
        FLD<TVec3f>(this, 0x7C4).zero();
        return this->checkWaterDamage();
    }
    return this->doFireDanceWithInitialDamage(1);
}

bool Mario::fn_80395310() {
    if (this->mMovementStates_LOW_WORD & 1) {
    }
    if (this->isInvincible() != 0) {
    }
    u32 temp_r3 = this->mMovementStates_LOW_WORD;
    if (((temp_r3 >> 31) != 0) || !((temp_r3 >> 30) & 1)) {
    }
    if (this->getCurrentStatus() == 0xE) {
    }
    if (this->getCurrentStatus() == 0xF) {
    }
    if (this->getCurrentStatus() == 8) {
    }
    if ((this->mMovementStates_LOW_WORD >> 4) & 1) {
    }
    this->mActor->fn_803B94E0();
    this->stopJump();
    this->stopWalk();
    this->changeStatus(this->mStun);
    return true;
}

bool Mario::fn_803972D0(const TVec3f* arg1) {
    if (this->isStatusActive(0x19) != 0) {
    }
    if (this->isAnimationRun("空中ひねり") != 0) {
    }
    if (this->isAnimationRun("雲スピン空中") != 0) {
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
    u16 temp_r4 = FLD<u16>(this, 0x3D4);
    if (temp_r4 != 0) {
        FLD<u16>(this, 0x3D4) = temp_r4 - 1;
    }
}

void Mario::fn_803A1F60() {
    u16 temp_r4 = FLD<u16>(this, 0x3D2);
    if (temp_r4 != 0) {
        FLD<u16>(this, 0x3D2) = temp_r4 - 1;
    }
}

void Mario::fn_803A2CF0() {
    if (FLD<u16>(this, 0x964) == 35) {
        this->mMovementStates_HIGH_WORD |= 0x400;
    }
}

void Mario::fn_803A2420() {
    this->mDrawStates_WORD &= 0xFFFBFFFF;
    FLD<u16>(this, 0x40C) = 0;
    this->_10_LOW_WORD &= 0xFFF7FFFF;
}

void Mario::fn_803A1FC0() {
    if (!(this->mWalkSpeed >= 0.1f)) {
        this->mWalkSpeed = 0.0f;
        FLD<u16>(this, 0x3D2) = 0;
    }
}

void Mario::fn_803A2690(f32* p) {
    if (FLD<u8>(this->mActor, 0xECD) != 0) {
        if (*p > 0.1f) {
            *p = 0.1f;
        }
    }
}

bool Mario::fn_8039A430() {
    if (!((this->mMovementStates_LOW_WORD >> 8) & 1)) {
        return false;
    }
    this->mMovementStates_LOW_WORD &= 0xFFFFFEFF;
    return FLD<MarioWall*>(this, 0x750)->startJump();
}

s32 Mario::fn_803A2D10() {
    s32 r = 0;
    if (!((this->mDrawStates_WORD >> 26) & 1) && !((this->mMovementStates_HIGH_WORD >> 28) & 1)) {
        if (FLD<s32>(this, 0x74C) != 0) {
            r = 1;
        }
    }
    return r;
}

void Mario::fn_803A1F80() {
    f32 temp_f1 = this->mStickPos.z;
    const MarioConstTable* ent = this->mActor->mConst->getTable();
    if (temp_f1 < ent->mTurnSlipNeutral) {
        this->mMovementStates_LOW_WORD |= 0x8000;
    }
}

void Mario::fn_803A2CC0() {
    if (!((this->mMovementStates_HIGH_WORD >> 28) & 1)) {
        return;
    }
    FLD<u8>(this, 0x714) = 0;
    this->mWalkSpeed = 0.0f;
    this->playSound("坂滑り", -1);
}

void Mario::fn_803A1A20(f32* out) {
    this->mActor->getStickValue(&out[0], &out[1]);
    out[2] = 0.0f;
}

void Mario::fn_803A2530(TVec3f* out) {
    if (FLD<f32>(this, 0x520) == FLD<f32>(this, 0x524)) {
        *out = FLD<TVec3f>(this, 0x22C);
    } else {
        *out = this->mFrontVec;
    }
}

void Mario::fn_803A23F0(TVec3f* out) {
    if ((this->mMovementStates_HIGH_WORD >> 28) & 1) {
        *out = this->mFrontVec;
    } else {
        *out = FLD<TVec3f>(this, 0x22C);
    }
}

void Mario::fn_8039A460() {
    if (FLD<u16>(this, 0x540) <= 5) {
    }
}

static inline MarioConstTable* marioCurTableEntry(Mario* self) {
    return self->mActor->mConst->getTable();
}


void Mario::fn_8039A5B0() {
    if (this->beeMarioOnAir()) {
        FLD<u16>(this, 0x764) = FLD<u16>(marioCurTableEntry(this), 0x670);
    }
}

void Mario::fn_803A1D60() {
    PSVECCrossProduct(&FLD<TVec3f>(this, 0x214), &FLD<TVec3f>(this, 0x22C), &FLD<TVec3f>(this, 0x3D8));
    this->fn_803A1A60(&FLD<TVec3f>(this, 0x244));
    FLD<TVec3f>(this, 0x238) = FLD<TVec3f>(this, 0x22C);
}

bool Mario::fn_803902B0(HitSensor* arg1) {
    bool r = false;
    const HitSensor* temp_r3 = FLD<const HitSensor*>(FLD<void*>(this, 0x460), 8);
    if (temp_r3 != nullptr && MR::isSensorPressObj(temp_r3)) {
        r = true;
    }
    return r;
}

bool Mario::fn_80391260(const Triangle* triangle) {
    if ((mMovementStates_HIGH_WORD >> 8) & 1) {
        if (MR::abs(triangle->getNormal(0)->dot(mSwimSideVec694)) > 0.707f) return true;
    }
    return false;
}

void Mario::fn_803912D0(const HitInfo* hit, const TVec3f& normal, const TVec3f& direction) {
    if (fn_80391330(hit, normal, direction)) {
        *_4DC = hit->mParentTriangle;
        _518 = hit->_64;
    }
}

bool Mario::fn_8039A3D0() {
    bool r = false;
    if (FLD<u16>(this, 0x3BC) > 10 && this->checkTrgA()) {
        r = true;
    }
    return r;
}

bool Mario::fn_803A0FF0() {
    u16 v = FLD<u16>(this, 0x40A);
    if (v == 0) {
        return false;
    }
    if (FLD<f32>(this, 0x1C8) < 0.75f) {
        FLD<u16>(this, 0x40A) = 0;
        return false;
    }
    FLD<u16>(this, 0x40A) = v - 1;
    FLD<f32>(this, 0x1C0) = 0.0f;
    FLD<f32>(this, 0x1C4) = 0.0f;
    FLD<f32>(this, 0x1C8) = 0.0f;
    return true;
}

bool Mario::fn_803A2270() {
    TVec3f sp8 = -this->mFrontVec;
    f32 d = MR::diffAngleAbsHorizontal(FLD<TVec3f>(this, 0x8FC), sp8, *this->getGravityVec());
    return d < 0.7853982f;
}


void Mario::fn_8039A020() {
    if (this->_10_LOW_WORD & 1) {
        if (this->mActor->fn_803BE0E0() == nullptr) {
            this->mMovementStates_LOW_WORD &= 0xFFFFEFFF;
        }
    } else {
        if (this->mActor->fn_803BE0D0() == nullptr) {
            this->mMovementStates_LOW_WORD &= 0xFFFFEFFF;
        }
    }
}

void Mario::fn_8039A090() {
    if (!this->checkTrgA()) {
        if (this->mActor->fn_803BE0C0() != nullptr) {
            FLD<u32>(this, 0x554) = FLD<u32>(this->mActor, 0x6AC);
            if (this->isEnableFutureJump()) {
                this->mMovementStates_HIGH_WORD |= 0x80;
            }
        }
    }
}

f32 Mario::fn_803A26C0(f32 x) {
    void* e = marioCurTableEntry(this);
    f32 f4 = FLD<f32>(e, 0x37C);
    f32 f3 = FLD<f32>(e, 0x374);
    f32 f0 = FLD<f32>(e, 0x378);
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
    if (FLD<u8>(this, 0x274) == 0) {
        TMtx34f sp8;
        PSMTXRotAxisRad(sp8, &this->mHeadVec, -0.12f);
        PSMTXMultVecSR(sp8, &FLD<TVec3f>(this, 0x214), arg);
    }
}

void Mario::fn_803A2450(const TVec3f* arg) {
    f32 d = this->mFrontVec.dot(*arg);
    if (!(d >= 0.0f) && !this->isAnimationRun("ブレーキ")) {
        FLD<f32>(this, 0x744) = 3.1415927f;
        FLD<u32>(this, 0x748) = 10;
    }
}

void Mario::fn_803A2860() {
    if (!this->isAnimationRun("その場足踏み")) {
        if (FLD<TVec3f>(this, 0x214).dot(FLD<TVec3f>(this, 0x22C)) > 0.99f) {
            this->mMovementStates_LOW_WORD &= 0xEFFFFFFF;
        } else {
            this->mMovementStates_LOW_WORD |= 0x10000000;
        }
    }
}


void Mario::fn_803A2720() {
    if ((FLD<u32>(this->getPlayer(), 0x14) >> 13) & 1) {
        f32 d = MR::diffAngleAbs(this->mFrontVec, FLD<TVec3f>(this, 0x22C));
        f32 c = MR::clamp(d / 3.1415927f, 0.0f, 1.0f);
        this->mWalkSpeed = this->mWalkSpeed * (1.0f - 0.1f * c);
    }
}

void Mario::fn_803A2C40(const TVec3f* arg) {
    FLD<TVec3f>(this, 0x328) = *arg;
    if ((this->mMovementStates_LOW_WORD >> 21) & 1) {
        if (FLD<u8>(this, 0x714) != 0) {
            FLD<TVec3f>(this, 0x334) = *arg;
        }
    } else {
        if (FLD<u16>(this, 0x3D2) == 0) {
            FLD<TVec3f>(this, 0x334) = *arg;
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
    f32 sp12 = FLD<f32>(this, 0x1C0);
    f32 sp8 = FLD<f32>(this, 0x1C4);
    if (!(FLD<u32>(this->getPlayer(), 0x14) & 0x4000)) {
        this->fn_803A1240(&sp12, &sp8);
        this->calcMoveDir(sp12, sp8, arg, true);
        this->calcShadowDir(*arg, &FLD<TVec3f>(this, 0x22C));
    }
}

bool Mario::fn_80394550(const TVec3f* arg) {
    if (this->damage(*arg)) {
        if (this->isStatusActive(6)) {
            this->mSwim->fn_803F7C00();
        } else {
            void* e = marioCurTableEntry(this);
            FLD<MarioDamage*>(this, 0x7C4)->setVecSize(FLD<f32>(e, 0x170), FLD<f32>(e, 0x16C));
            this->playSound("投げられ", -1);
        }
        return true;
    }
    return false;
}

bool Mario::fn_80390210(HitSensor* arg) {
    if (MR::isSensorPressObj(reinterpret_cast<const HitSensor*>(FLD<void*>(arg, 8)))) {
        return true;
    }
    FloorCode* fc = FLD<FloorCode*>(this, 0x960);
    if (fc->getCode(reinterpret_cast<const Triangle*>(arg)) == 0x1D) {
        return true;
    }
    if (fc->getCode(reinterpret_cast<const Triangle*>(arg)) == 0x27) {
        return true;
    }
    if (this->fn_803902B0(arg)) {
        return true;
    }
    u16 v = FLD<u16>(this, 0x964);
    return v == 0x1D || v == 0x27;
}

void Mario::fn_803A1A60(TVec3f* arg) {
    TVec3f s;
    this->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    PSVECCrossProduct(&s, arg, &FLD<TVec3f>(this, 0x250));
}

bool Mario::fn_803A19C0() {
    TVec3f s;
    this->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    return s.dot(FLD<TVec3f>(this, 0x244)) > 0.0f;
}

bool Mario::fn_803A1050() {
    if ((this->mMovementStates_HIGH_WORD >> 8) & 1) {
        f32 d = MR::diffAngleAbsHorizontal(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), FLD<TVec3f>(this, 0x694));
        return d > 1.5707964f;
    }
    f32 d2 = MR::diffAngleAbsHorizontal(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), *this->getGravityVec());
    return d2 > 0.17453293f;
}

void Mario::fn_803A0D90(TVec3f* arg) {
    if (MR::isNearZero(FLD<TVec3f>(this, 0x328), 0.001f)) {
        FLD<TVec3f>(this, 0x328) = this->mFrontVec;
        *arg = FLD<TVec3f>(this, 0x328);
        this->calcShadowDir(FLD<TVec3f>(this, 0x328), &FLD<TVec3f>(this, 0x22C));
        FLD<u16>(this, 0x40A) = 10;
    }
}


bool Mario::fn_803A0E10() {
    if (FLD<u8>(this, 0x714) == 0) {
        return false;
    }
    if (FLD<u8>(this, 0x715) != 0) {
        return false;
    }
    if (this->isAnimationRun("ブレーキ")) {
        return false;
    }
    if (this->isAnimationRun("ターンブレーキ")) {
        return false;
    }
    if ((this->mMovementStates_LOW_WORD >> 15) & 1) {
        return false;
    }
    if ((this->mMovementStates_LOW_WORD >> 16) & 1) {
        return false;
    }
    if (this->mWalkSpeed >= 0.05f) {
        return false;
    }
    if ((this->mDrawStates_WORD >> 26) & 1) {
        return false;
    }
    return true;
}

void Mario::fn_8039AB50() {
    if (FLD<u16>(this, 0x42C) == 13 && !((this->mMovementStates_HIGH_WORD >> 31) & 1)) {
        TVec3f sp32;
        f32 old = MR::vecKillElement(this->mJumpVec, *this->getGravityVec(), &sp32);
        f32 d = this->fn_8039AC10();
        if (!(old >= d)) {
            TVec3f sp8(*this->getGravityVec());
            sp8 *= d;
            TVec3f sp20(sp8);
            sp20 += sp32;
            this->mJumpVec = sp20;
        }
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
    if (MR::diffAngleAbs(this->mFrontVec, FLD<TVec3f>(this, 0x22C)) < 2.5f) {
        return;
    }
    if (FLD<u16>(this, 0x3D4) != FLD<u16>(marioCurTableEntry(this), 0x2E6)) {
        return;
    }
    if ((this->mDrawStates_WORD >> 18) & 1) {
        return;
    }
    TVec3f sp8;
    PSVECCrossProduct(&FLD<TVec3f>(this, 0x214), arg, &sp8);
    f32 d = sp8.dot(this->mHeadVec);
    FLD<u8>(this, 0x274) = d > 0.0f;
    this->_10_LOW_WORD |= 0x80000;
}

f32 Mario::fn_8039AC10() {
    void* e = marioCurTableEntry(this);
    if (this->getPlayerMode() == 4 && !this->isRising()) {
        return FLD<f32>(e, 0x8C);
    }
    if (FLD<u16>(this, 0x42C) == 11 && !this->isRising()) {
        if (FLD<u16>(this, 0x3BE) >= FLD<u16>(e, 0x138)) {
            return FLD<f32>(e, 0x88);
        }
    }
    void* e2 = marioCurTableEntry(this);
    u8 lvl = this->mActor->getGravityLevel();
    return FLD<f32>(e2, 0x7C + ((lvl << 2) & 0x3C));
}

bool Mario::fn_8039A100() {
    if (FLD<u32>(this, 0x42C) == 8 && FLD<u16>(this, 0x426) == 0) {
        s16 v = FLD<s16>(this, 0x422);
        if (v != 0) {
            return v > (s16)(FLD<u16>(marioCurTableEntry(this), 0x3F0) - 16);
        }
    }
    if ((this->mMovementStates_LOW_WORD >> 12) & 1) {
        return false;
    }
    if ((this->mMovementStates_LOW_WORD >> 17) & 1) {
        return false;
    }
    if (!this->isRising()) {
        return false;
    }
    return FLD<u32>(this, 0x42C) == 2;
}

void Mario::fn_803A10D0(s32 arg) {
    void* e = marioCurTableEntry(this);
    u32 dur = FLD<u8>(this->mActor, 0xECD) == 0 ? 30 : FLD<u16>(e, 0x2E4);
    if (arg == 0) {
        this->setFrontVecKeepUp(FLD<TVec3f>(this, 0x22C), dur);
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
    PSVECCrossProduct(&FLD<TVec3f>(this, 0x214), &FLD<TVec3f>(this, 0x22C), &sp8);
    if (sp8.dot(FLD<TVec3f>(this, 0x3D8)) < 0.0f) {
        if (!this->fn_803A19C0()) {
            if (!(FLD<TVec3f>(this, 0x214).dot(FLD<TVec3f>(this, 0x22C)) < 0.0f)) {
                FLD<u16>(this, 0x3D4) = 0;
            }
        }
    } else {
        u16 v = FLD<u16>(this, 0x3D4);
        if (v < FLD<u16>(marioCurTableEntry(this), 0x2E6)) {
            FLD<u16>(this, 0x3D4) = v + 1;
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

void Mario::fn_803A20E0(s32 arg) {
    if (FLD<u16>(this, 0x3D0) == 0) {
        TVec3f sp8;
        if (!MR::vecBlendSphere(&sp8, FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), 0.0f)) {
            MR::vecRotAxis(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), this->mHeadVec, &sp8, 0.3926991f);
            if (arg == 0) {
                this->setFrontVecKeepUp(sp8);
            }
        }
    }
    this->_10_LOW_WORD &= 0xFFF7FFFF;
    this->mMovementStates_LOW_WORD &= 0xEFFFFFFF;
}

void Mario::fn_803A1BB0() {
    if (FLD<u16>(this, 0x3D0) == 0) {
        return;
    }
    this->_10_LOW_WORD &= 0xFFF7FFFF;
    this->mMovementStates_LOW_WORD &= 0xEFFFFFFF;
    f32 d = MR::diffAngleAbs(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C));
    if (MR::isInRange(d, 0.0f, 0.707f)) {
        TVec3f s;
        PSVECCrossProduct(&FLD<TVec3f>(this, 0x214), &FLD<TVec3f>(this, 0x22C), &s);
        if (FLD<u8>(this, 0x274) == 0) {
            if (s.dot(this->mHeadVec) < 0.0f) {
                this->_10_LOW_WORD &= 0xFFF7FFFF;
            }
        } else {
            if (s.dot(this->mHeadVec) > 0.0f) {
                this->_10_LOW_WORD &= 0xFFF7FFFF;
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
    void* e = marioCurTableEntry(this);
    f32 v = JMACosRadian(FLD<f32>(e, 0x2C0));
    s32 r31 = 0;
    if (f < v && !((this->mMovementStates_LOW_WORD >> 26) & 1)) {
        if (this->mWalkSpeed > FLD<f32>(e, 0x2C8)) {
            if (FLD<f32>(this, 0x3C) > FLD<f32>(this, 0x1C8)) {
                if (!((this->mDrawStates_WORD >> 18) & 1)) {
                    r31 = 1;
                }
            }
        }
    }
    return r31;
}

// NONMATCHING: branch shape of the 0x3D4 == 0 block approximated
void Mario::fn_803A1FF0(f32 x, s32 arg) {
    if ((this->mMovementStates_HIGH_WORD >> 28) & 1) {
        this->fn_803A1AC0();
        this->fn_803A1C90();
        x *= this->fn_803A1DB0();
    } else {
        this->fn_803A1E40();
        if (FLD<u16>(this, 0x3D4) == 0) {
            FLD<TVec3f>(this, 0x244).zero();
            PSVECCrossProduct(&FLD<TVec3f>(this, 0x214), &FLD<TVec3f>(this, 0x22C), &FLD<TVec3f>(this, 0x3D8));
            x *= 0.01f;
            FLD<TVec3f>(this, 0x238) = FLD<TVec3f>(this, 0x22C);
            f32 k = 0.3f;
            if ((this->mDrawStates_WORD >> 18) & 1) {
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
    if (FLD<u32>(this, 0x748) != 0 && FLD<u8>(this, 0x714) == 0 && this->isEnableTurn()) {
        this->changeAnimation("その場足踏み", (const char*)nullptr);
        if (!this->isAnimationRun("カリカリ限界") && !this->isPlayerModeYoshi()) {
            this->fn_803EEC20("その場足踏み上半身", 0);
        }
    } else {
        if (FLD<u8>(this, 0x714) != 0) {
            this->stopAnimation("その場足踏み", (const char*)nullptr);
            if (this->isAnimationRun("その場足踏み上半身")) {
                this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
            }
        }
    }
}


void Mario::fn_803A22D0(f32 x, s32 arg) {
    TVec3f sp8;
    if ((this->_10_LOW_WORD >> 12) & 1) {
        this->fn_803A2420();
        if ((this->mMovementStates_HIGH_WORD >> 8) & 1) {
            this->fn_803A23F0(&sp8);
            this->fn_803A2420();
            this->fn_803A2450(&sp8);
        }
    } else if ((this->_10_LOW_WORD >> 13) & 1) {
        this->fn_803A24C0(&sp8);
    } else {
        f32 d = MR::diffAngleAbs(this->mFrontVec, FLD<TVec3f>(this, 0x22C));
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
    f32 d = MR::diffAngleAbs(this->mFrontVec, FLD<TVec3f>(this, 0x22C));
    f32 s = MR::clamp(x / d, 0.0f, 1.0f);
    this->fn_803A2690(&s);
    if ((FLD<u32>(this->getPlayer(), 0x14) >> 13) & 1) {
        s *= this->fn_803A26C0(d);
    }
    if (this->isPlayerModeYoshi()) {
        if (FLD<s32>(this->mActor->fn_803B97F0(), 0x134) > 0) {
            s *= 0.5f;
        }
    }
    if (!MR::vecBlendSphere(arg, this->mFrontVec, FLD<TVec3f>(this, 0x22C), s)) {
        FLD<u16>(this, 0x3D4) = 0;
        MR::vecRotAxis(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), this->mHeadVec, arg, 0.3926991f);
    }
}

// NONMATCHING: the wall-normal slide tail is approximated from the asm
void Mario::fn_803A28D0(TVec3f* arg) {
    this->fn_803A2A70();
    this->fn_803A2C40(arg);
    if ((this->mMovementStates_HIGH_WORD >> 12) & 1) {
        if (FLD<f32>(this, 0x4DC) > 120.0f) {
            const TVec3f* n = FLD<Triangle*>(this, 0x4C8)->getNormal(0);
            if (this->mVelocity.dot(*n) < 0.0f) {
                TVec3f sp20(this->mPosHolder->mPos);
                sp20 -= FLD<TVec3f>(this, 0x4E4);
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
                    PSVECCrossProduct(&FLD<TVec3f>(this, 0x368), &sp44, &sp32);
                    MR::normalizeOrZero(&sp32);
                    PSVECCrossProduct(&sp32, &FLD<TVec3f>(this, 0x368), &sp44);
                    sp44.setLength(m2);
                    this->mVelocity = sp44;
                }
            }
        }
    }
}


// NONMATCHING: the nested gate structure is approximated from the asm
void Mario::fn_803A1AC0() {
    if (MR::isNearZero(FLD<f32>(this, 0x1C8), 0.001f)) {
        this->_10_LOW_WORD &= 0xFFF7FFFF;
        this->mMovementStates_LOW_WORD &= 0xEFFFFFFF;
        return;
    }
    if (!MR::isNearZero(FLD<TVec3f>(this, 0x244), 0.001f)) {
        if (!((this->mMovementStates_HIGH_WORD >> 24) & 1) && this->fn_803A1920()) {
            this->mMovementStates_LOW_WORD |= 0x10000000;
        }
    } else {
        this->fn_803A1BB0();
    }
    this->fn_803A1A20(&FLD<f32>(this, 0x250));
}

// NONMATCHING: the second gate and the press-normal copy are approximated
void Mario::fn_8039ACE0() {
    if (this->calcDistToCeil(false) < 80.0f) {
        return;
    }
    bool a = false;
    if (FLD<void*>(this, 0x728) != nullptr && MR::isSensorPressObj(reinterpret_cast<const HitSensor*>(FLD<void*>(this, 0x728)))) {
        a = true;
    }
    bool b = false;
    Triangle* g = FLD<Triangle*>(this, 0x460);
    void* gs = FLD<void*>(g, 8);
    if (gs != nullptr && MR::isSensorPressObj(reinterpret_cast<const HitSensor*>(gs))) {
        b = true;
    }
    if (FLD<void*>(this, 0x728) == gs) {
        return;
    }
    if (!a && b) {
        this->mActor->setPress(0, 0);
        FLD<TVec3f>(this->mActor, 0x6D8) = *g->getNormal(0);
    }
}

f32 Mario::fn_8039AA40() {
    if (this->getPlayerMode() == 6) {
        return 0.0f;
    }
    f32 ratio = 1.0f;
    if (this->_430 == 13 && (((this->mMovementStates_LOW_WORD >> 17) & 1) || !this->isRising())) {
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


// NONMATCHING: follows the m2c call sequence; the isNearZero/identity branches are approximated
s32 Mario::fn_8038A8A0(f32 (*arg1)[4], TVec3f* arg2) {
    *arg2 = TVec3f(0.0f, 0.0f, 0.0f);
    if (this->isNonFixHeadVec()) {
        PSMTXIdentity(arg1);
    }
    if (MR::isNearZero(FLD<f32>(this, 0x74C), 0.001f) && FLD<s32>(this, 0x754) == 0) {
        PSMTXIdentity(arg1);
        FLD<s32>(this, 0x750) = 0;
        FLD<f32>(this, 0x74C) = 0.0f;
    }
    if (FLD<s32>(this, 0x750) == 0 && FLD<s32>(this, 0x754) == 0) {
        this->fn_8038A9D0();
    }
    TVec3f sp8(0.0f, 1.0f, 0.0f);
    if (this->fn_8038AA30()) {
        PSMTXRotAxisRad(arg1, &sp8, -FLD<f32>(this, 0x74C));
    } else {
        PSMTXRotAxisRad(arg1, &sp8, FLD<f32>(this, 0x74C));
    }
    this->fn_8038AB10();
    this->fn_8038AB70();
    return 0;
}

// NONMATCHING: branch structure of the two sign tests is approximated
bool Mario::fn_8038AA30() {
    f32 d = this->mFrontVec.dot(this->getCamDirX());
    if (FLD<TVec3f>(this, 0x1FC).dot(this->getCamDirY()) < 0.0f) {
        d = -d;
    }
    f32 t = FLD<f32>(this, 0x74C);
    return (d < 0.0f && t < 0.0f) || (d > 0.0f && t > 0.0f);
}

// NONMATCHING: sound-slot bookkeeping approximated from the m2c draft
void Mario::fn_8038C540() {
    if (FLD<u32>(this, 0xA92) != 0) {
        AudLifeAlertCounter* p = FLD<AudLifeAlertCounter*>(this, 0xA94);
        if (p == nullptr || p->_9 == 0) {
            FLD<AudLifeAlertCounter*>(this, 0xA94) = MR::fn_8005BC60(0x258);
        }
        p = FLD<AudLifeAlertCounter*>(this, 0xA94);
        p->fn_80085D30(1);
        s32 v = MR::clamp(0x1A4 - p->_0, 0, 0x1A4);
        if (p->_0 % 74 == 5) {
            MR::startActionSound(this->mActor, "LifeAlertTrg", v, -1, -1);
        }
    } else {
        AudLifeAlertCounter* p = FLD<AudLifeAlertCounter*>(this, 0xA94);
        if (p != nullptr && p->_9 != 0) {
            p->fn_80085D40();
        }
    }
}

// NONMATCHING: only the animation-run gate chain is recovered; the flags are approximated
bool Mario::fn_8038CCF0() {
    if (this->isPlayerModeTeresa()) {
    }
    if (this->isAnimationRun("着地") || this->isAnimationRun("着地B") || this->isAnimationRun("着地C") ||
        this->isAnimationRun("着地ターン") || this->isAnimationRun("着地幅とび")) {
    }
    if (this->isAnimationRun("すべり着地")) {
    }
    if (this->isAnimationRun(FLD<const char*>(this, 0x728))) {
    }
    if (this->isAnimationRun("ハード着地")) {
    }
    if (this->isAnimationRun("逆着地")) {
    }
    if (this->isAnimationRun("ショート着地")) {
    }
    return MR::isBckPlaying(this->mActor, "pulloutgraspend");
}

// NONMATCHING: timer and gate logic approximated
void Mario::fn_8039A1C0() {
    if (this->isAnimationRun("ジャンプB")) {
        this->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
    if (FLD<u8>(FLD<void*>(this, 0x888), 0x16E) == 0) {
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
    FLD<u16>(this, 0x3BE) += 1;
}

// NONMATCHING: the target-speed and sound branches are approximated
void Mario::fn_803A0ED0(TVec3f* arg1, s32 arg2) {
    if (this->fn_803A0FF0()) {
        *arg1 = FLD<TVec3f>(this, 0x328);
    }
    this->calcMoveDir(this->mStickPos.x, FLD<f32>(this, 0x1C4), arg1, true);
    this->calcShadowDir(*arg1, &FLD<TVec3f>(this, 0x22C));
    this->calcShadowDir(this->mFrontVec, &FLD<TVec3f>(this, 0x214));
    if (this->fn_803A1050()) {
        this->fn_803A10D0(arg2);
    }
    if (arg2 == 0) {
        this->setFrontVecKeepUp(FLD<TVec3f>(this, 0x22C));
    }
    if (this->isAnimationRun("食べる") || this->isAnimationRun("噛む") || this->isAnimationRun("飲み下す")) {
        this->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
}

// NONMATCHING: table lookups follow the draft; the speed-band selection is approximated
f32 Mario::fn_803A1800() {
    void* e = marioCurTableEntry(this);
    if ((FLD<u32>(this, 0xC) >> 16) & 1) {
        return FLD<f32>(e, 0x3D4);
    }
    if ((FLD<u32>(this, 0x10) >> 28) & 1) {
        return FLD<f32>(e, 0x264);
    }
    if (FLD<f32>(this, 0x278) > FLD<f32>(e, 0x2D0)) {
        if (this->isStatusActive(0x1F)) {
            return 0.5f * FLD<f32>(e, 0x260);
        }
        if (FLD<u8>(this, 0x714) < 5) {
            return FLD<f32>(e, 0x268);
        }
        return FLD<f32>(e, 0x260);
    }
    return FLD<f32>(e, 0x25C);
}

// NONMATCHING: the reset/timer chain is approximated from the draft
void Mario::fn_8039A480() {
    if (FLD<u32>(this, 0x42C) == 8) {
        if (this->checkTrgZ()) {
            this->resetTornado();
            this->cancelTornadoJump();
            this->stopAnimation((const char*)nullptr, (const char*)nullptr);
            this->changeAnimationInterpoleFrame(1);
            FLD<u32>(this, 0x10) |= 0x800000;
            this->playSound("声小ジャンプ", -1);
            if (FLD<u16>(this, 0x42A) != 0) {
                this->playEffect("スピンライト消去");
            }
        } else {
            FLD<u32>(this, 0xC) &= 0xFFFFBFFF;
        }
        if (FLD<u16>(this, 0x42A) != 0) {
            u16 t = FLD<u16>(this, 0x402);
            if (t != 0) {
                FLD<u16>(this, 0x402) = t - 1;
            }
            if (FLD<u16>(this, 0x402) == 0) {
                this->cancelTornadoJump();
                this->playEffect("スピンライト消去");
                if (this->getPlayerMode() != 4) {
                    FLD<u16>(this, 0x402) = FLD<u16>(marioCurTableEntry(this), 0xD8);
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
    if (!((FLD<u32>(this, 0x10) >> 29) & 1)) {
        for (u32 i = 0; i < this->_578; i++) {
            Triangle* tri = this->_57C[i];
            TVec3f clipped;
            if (MR::vecKillElement(FLD<TVec3f>(this, 0x160), *tri->getNormal(0), &clipped) < 0.0f) {
                FLD<TVec3f>(this, 0x160) = clipped;
                if (!((FLD<u32>(this, 0x20) >> 18) & 1)) {
                    TVec3f projected;
                    f32 depth = MR::vecKillElement(FLD<TVec3f>(this, 0x2D4), *tri->getNormal(0), &projected);
                    if (depth <= 0.0f) {
                        TVec3f normal(*tri->getNormal(0));
                        normal *= depth;
                        TVec3f half(normal);
                        half *= 0.5f;
                        TVec3f result(projected);
                        result += half;
                        FLD<TVec3f>(this, 0x2D4) = result;
                    }
                } else if (MR::diffAngleAbs(*getGravityVec(), FLD<TVec3f>(this, 0x2D4)) < 0.10471976f) {
                    TVec3f tangent;
                    MR::vecKillElement(*tri->getNormal(0), *getGravityVec(), &tangent);
                    f32 length = PSVECMag(&FLD<TVec3f>(this, 0x2D4));
                    MR::normalizeOrZero(&tangent);
                    FLD<TVec3f>(this, 0x2D4) += tangent;
                    FLD<TVec3f>(this, 0x2D4).setLength(length);
                }
                if (rising) {
                    if (tri->getNormal(0)->dot(*getGravityVec()) > 0.707f) {
                        mActor->sendMsgUpperPunch(tri->mSensor);
                    }
                }
                if (isPlayerModeBee() && FLD<u8>(mActor, 0xECD) == 0) {
                    const char* code = MR::getWallCodeString(tri);
                    if (code && strcmp(code, "Fur") == 0) {
                        Triangle hitTriangle;
                        TVec3f negativeNormal(-*tri->getNormal(0));
                        TVec3f ray(negativeNormal);
                        ray *= 300.0f;
                        TVec3f hitPos;
                        const TVec3f& origin = FLD<TVec3f>(mActor, 0x5F4);
                        bool hit;
                        if (fn_803B2950()) {
                            hit = MR::fn_80037E80(&hitPos, &hitTriangle, origin, ray);
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
    if ((_1C_WORD >> 19) & 1) {
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
    if (!mMovementStates._8 && weight < 0.1f) {
        if (mMovementStates._1A) {
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
                const TVec3f& contact = mActor->mBinder->getPlane(bestIndex)->_64;
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
    if (!((FLD<u32>(this, 0xC) >> 9) & 1)) {
        if (this->checkDamage() == false) {
            if (this->isPlayerModeYoshi()) {
                this->mActor->fn_803B9810()->fn_8041DF40();
            }
            if (!this->isPlayerModeYoshi() || this->mActor->fn_803B9810()->fn_8041F8A0() == 0) {
                if (_97C != nullptr) {
                    this->sendStateMsg(2);
                } else if ((FLD<u32>(this, 0xC) >> 31) != 0) {
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
            if ((FLD<u32>(this, 0xC) >> 30) & 1) {
                FLD<u32>(this, 0x10) |= 0xC000;
                this->doFrontStep();
            }
            if (this->isStatusActive(0x28)) {
                this->mActor->fn_803B9A30();
                _10_HIGH_WORD &= 0xFFDFFFFF;
                this->mActor->fn_803B9830()->fn_8041BB90();
                return;
            }
            u32 t = FLD<u32>(this, 0xC);
            if (((t >> 30) & 1) || !((t >> 12) & 1)) {
                _10_HIGH_WORD &= 0xFFEFFFFF;
            }
        }
        this->fn_803A7540();
        if (this->getPlayerMode() != 6 && ((FLD<u32>(this, 0xC) >> 23) & 1) && FLD<Triangle*>(this, 0x4C8)->isValid() &&
            FLD<Triangle*>(this, 0x4C8)->getNormal(0)->dot(FLD<TVec3f>(this, 0x160)) < 0.0f) {
            MR::vecKillElement(FLD<TVec3f>(this, 0x160), *FLD<Triangle*>(this, 0x4C8)->getNormal(0), &FLD<TVec3f>(this, 0x160));
        }
        if (this->mActor->fn_803B9840() != 0) {
            this->mActor->fn_803B9830()->fn_8041BBB0();
        }
        if (this->isStickOn()) {
            if (this->isAnimationRun("水泳陸うちあげ")) {
                this->stopAnimation((const char*)nullptr, (const char*)nullptr);
            }
        }
        if (!((FLD<u32>(this, 0x1C) >> 27) & 1)) {
            FLD<u16>(this, 0x3FE) = 0;
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
        if (!((FLD<u32>(this, 0x1C) >> 9) & 1)) {
            this->retainMoveDir(mStickPos.x, mStickPos.y, nullptr);
        }
    }
}

// NONMATCHING: the two-stage neighbourhood test is approximated (returns the matched-triangle flag)

// NONMATCHING: independent loads and register scheduling; unused legacy argument.
void Mario::fn_8038C620(s32) {
    if (FLD<u8>(this, 0xA91) != 0) {
        MR::startLevelSoundPlayer("JumpFromYoshi", -1, -1);
    } else if (FLD<u8>(this, 0xA90) != 0) {
        if (this->isPlayerModeYoshi()) {
            if (FLD<s32>(this->mActor->fn_803B97F0(), 0x134) > 0) {
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
            s32 t5 = FLD<s32>(this, 0xA98);
            s32 d = v3 - t5;
            s32 var_r31;
            if ((d < 0 ? -d : d) > 150) {
                var_r31 = MR::clamp(v3, t5 - 66, t5 + 66);
            } else {
                var_r31 = MR::clamp(v3, t5 - 22, t5 + 22);
            }
            s32 temp_r29 = FLD<s32>(this->mActor->fn_803B97F0(), 0x134);
            MR::startActionSoundPlayer("DashLV", v28, var_r31);
            MR::startActionSoundPlayer("DashLVWind", v28, -1);
            MR::startActionSoundPlayer("DashLVFire", temp_r29, -1);
            this->fn_803B2950();
            FLD<s32>(this, 0xA98) = var_r31;
        } else {
            FLD<s32>(this, 0xA98) = 0x226;
        }
        if (PSVECMag(&FLD<TVec3f>(this, 0x160)) >= 2.0f && this->isAnimationRun("口に含む") && this->isAnimationRun("基本")) {
            if (this->getAnimator()->fn_803DF0E0() >= 2.5f) {
                MR::startActionSoundPlayer("RunMouthFull", -1, -1);
            }
        }
    }
    FLD<MarioFlags3>(this, 0xA90).fn_8038C890();
}

// NONMATCHING: the wall-contact push-out is approximated
void Mario::fn_8038E1A0(const Triangle* tri, s32 arg2) {
    if (!MR::isThroughPolygon(tri)) {
        if (this->isThroughWall(tri)) {
        }
        if (reinterpret_cast<const HitInfo*>(tri)->isCollisionAtEdge() || reinterpret_cast<const HitInfo*>(tri)->isCollisionAtCorner()) {
            FLD<u32>(this, 0xC) |= 0x400000;
        }
        const TVec3f* n = tri->getNormal(0);
        f32 d = n->dot(FLD<TVec3f>(this, 0x368));
        if (!MR::isNearZero(d, 0.3f) || arg2 != 0) {
            TVec3f sp2C(FLD<TVec3f>(this, 0x16C));
            MR::normalizeOrZero(&sp2C);
            TVec3f sp20;
            sp20 = -*n;
            if (!(sp2C.dot(sp20) < 0.707f)) {
                f32 d2 = FLD<TVec3f>(this, 0x16C).dot(*n);
                f32 lim = FLD<f32>(tri, 0x60);
                if (!((d2 < 0.0f ? -d2 : d2) < lim) && !MR::isNearZero(d2 - lim, 0.001f)) {
                    TVec3f sp14(FLD<TVec3f>(tri, 0x64));
                    sp14 -= this->mPosHolder->mPos;
                    if (!(sp14.dot(this->mHeadVec) < 0.0f)) {
                        f32 tf = lim - 1.0f;
                        TVec3f sp8(*n);
                        sp8 *= tf;
                        this->addTrans(sp8, nullptr);
                        u32 t = FLD<u32>(this, 0xC);
                        if (!((t >> 30) & 1) && (t >> 31) != 0 && !this->isRising() && n->dot(*this->getGravityVec()) < -0.99f) {
                            this->mTargetWalkSpeedIndex = this->mTargetWalkSpeedIndex;
                        }
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
    u16 code = FLD<u16>(this, 0x964);
    if (code == 0x19) {
        if (timer > 0x20) mSinkTimer = timer - 0x20;
        if (mSinkTimer < 0x40) mSinkTimer = 0x40;
        mMovementStates_HIGH_WORD &= 0xFFFFFF7F;
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
    if (FLD<u16>(this, 0x964) == 0x12) playSound("毒沼脱出");
    else playSound("砂脱出");
    playSound("声砂脱出");
    return true;
}

void Mario::fn_80399E60(TVec3f* stick) {
    _42A = 0;
    MarioConst* constants = mActor->mConst;
    if (_3CE > constants->mTable[constants->mCurrentTable]->mJumpConnectTime || !((mMovementStates_LOW_WORD >> 26) & 1)) _430 = 0;
    else _430++;
    if (_430 > 2) _430 = 0;
    if (_430 == 2) {
        constants = mActor->mConst;
        if (mWalkSpeed < constants->mTable[constants->mCurrentTable]->mJumpConnectSpeed) _430 = 0;
    }
    if (getPlayerMode() == 4 || getPlayerMode() == 6) _430 = 0;
    if (isPlayerModeYoshi() && _430 > 1) _430 = 0;
    u32 flags = _1C_WORD;
    if ((flags >> 20) & 1) {
        _430 = 2;
        MR::start2PJumpAssistJustSound();
    } else if ((flags >> 21) & 1) MR::start2PJumpAssistSound();
    if ((isPlayerModeInvincible() || FLD<u8>(this, 0xA79) != 0) && mWalkSpeed > 1.0f) {
        _430 = 1;
        _42A = 1;
    }
    if (!MR::isNearZero(*stick, 0.001f) && mSinkTimer != 0) {
        _430 = 0;
        mMovementStates_LOW_WORD &= 0xF7FFFFFF;
        _3D2 = 0;
    }
}

// NONMATCHING: the air-gravity effect placement is approximated from the draft
void Mario::fn_8039A2A0() {
    f32 d = this->calcDistToCeil(false);
    if (d != 160.0f) {
        f32 g = this->cutGravityElementFromJumpVec(true);
        MR::isNearZero(FLD<TVec3f>(this, 0x1E4), 0.001f);
        TVec3f sp20(FLD<TVec3f>(this, 0x1E4));
        sp20 *= d;
        TVec3f sp2C(this->mPosHolder->mPos);
        sp2C -= sp20;
        this->playEffectTrans("天井ヒット", sp2C);
        f32 f31 = (FLD<u32>(this, 0x42C) == 6 || FLD<u32>(this, 0x42C) == 4) ? 0.9f : 0.6f;
        TVec3f sp8(FLD<TVec3f>(this, 0x1E4));
        sp8 *= g;
        TVec3f sp14(sp8);
        sp14 *= f31;
        FLD<TVec3f>(this, 0x2D4) += sp14;
    }
}

// NONMATCHING: temporary allocation and load scheduling; verified virtual addVelocity calls.
void Mario::fn_8039A600() {
    u32 t = FLD<u32>(this, 0xC);
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
        this->addVelocity(FLD<TVec3f>(this, 0x2D4), f29);
        f32 f30 = this->fn_8039AA40();
        f32 f31 = this->fn_8039A810();
        TVec3f sp20(FLD<TVec3f>(this->mActor, 0x594));
        sp20 *= f31;
        TVec3f sp2C(sp20);
        sp2C *= f30;
        TVec3f sp38(sp2C);
        sp38 *= f29;
        FLD<TVec3f>(this, 0x2D4) += sp38;
        this->moveWallSlide(1.0f);
        return;
    }
    if (!((t >> 30) & 1)) {
        this->addVelocity(FLD<TVec3f>(this, 0x2D4));
        f32 f31 = this->fn_8039AA40();
        f32 f30 = this->fn_8039A810();
        TVec3f sp8(FLD<TVec3f>(this->mActor, 0x594));
        sp8 *= f30;
        TVec3f sp14(sp8);
        sp14 *= f31;
        FLD<TVec3f>(this, 0x2D4) += sp14;
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
        fn_8038A840(nullptr, 0.8f * FLD<f32>(this, 0x60), 0.0f, 0.8f * FLD<f32>(this, 0x68));
        if (FLD<s32>(this, 0x134) != 0) {
            FLD<s32>(this, 0x134)--;
        }
    } else {
        TVec3f previous(FLD<TVec3f>(this, 0x310));
        TVec3f up(FLD<TVec3f>(this, 0x1FC));
        TVec3f front(FLD<TVec3f>(this, 0x208));
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
                f32 limit = 0.1f * FLD<u16>(this, 0x40E);
                if (limit > 1.0471976f) {
                    limit = 1.0471976f;
                }
                f32 clamped = MR::clamp(angle, -limit, limit);
                fn_8038A840(nullptr, clamped, 0.0f, zero);
                FLD<s32>(this, 0x134) = 0x10;
            }
        }
    }
}

// NONMATCHING: register allocation and position-reference load scheduling.
bool Mario::fn_80391330(const HitInfo* hit, const TVec3f& normal, const TVec3f& direction) {
    f32 angle = 180.0f * (marioAcos(-getGravityVec()->dot(normal)) / 3.14159f);
    TVec3f fromBase(hit->_64);
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

// NONMATCHING: m2c draft's table/switch on the mode is approximated with the same weights
f32 Mario::fn_8039A810() {
    u16 t = FLD<u16>(this, 0x76C);
    if (t != 0) {
        void* e = marioCurTableEntry(this);
        f32 f31 = (f32)t / FLD<f32>(e, 0x670);
        return ((1.0f - f31) + f31 * FLD<f32>(e, 0x54)) * this->fn_8039ADC0();
    }
    if (FLD<u16>(this, 0x3CC) != 0) {
        return FLD<f32>(marioCurTableEntry(this), 0x54);
    }
    f32 var_f31 = 0.0f;
    if (FLD<u32>(this, 0x42C) == 0xB && (this->isRising() || FLD<u16>(this, 0x3BE) < FLD<u16>(marioCurTableEntry(this), 0x138))) {
        var_f31 = FLD<f32>(marioCurTableEntry(this), 0x24);
    } else {
        u32 m = FLD<u32>(this, 0x42C);
        if (m == 8 && FLD<u16>(this, 0x42A) == 1) {
            var_f31 = FLD<f32>(marioCurTableEntry(this), 0x430);
        } else if (m == 0xC) {
            var_f31 = 1.8f;
        } else if (m == 0xD) {
            var_f31 = FLD<f32>(marioCurTableEntry(this), 0x48);
        } else {
            var_f31 = FLD<f32>(marioCurTableEntry(this), 0x24);
        }
    }
    if (this->isPlayerModeCloud() && !this->isRising()) {
        var_f31 *= 0.5f;
    }
    if (FLD<u32>(this, 0x42C) == 8 && !this->isRising()) {
        var_f31 *= 0.25f;
    }
    return var_f31 * this->fn_8039ADC0();
}

// NONMATCHING: temporary register allocation; verified collision-sensor and velocity gates.
void Mario::fn_803A02F0(s32 keepSlip, s32 hard, s32 blend) {
    u32 mode = _430;
    if (mode - 4 <= 2 || mode == 8 || mode == 2 || mode == 11) keepSlip = 1;
    if (((_10_LOW_WORD >> 8) & 1) && _8D8 == _45C->mSensor) {
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
        if (!((_10_LOW_WORD >> 21) & 1)) recordTurnSlipAngle();
    } else fn_803A0490(hard, blend);
}

// NONMATCHING: the state dispatch chain is recovered from the draft; ordering of the sub-states is approximate
void Mario::fn_803A0BE0() {
    this->fn_803A2CC0();
    FLD<u32>(this, 0x10) = (FLD<u32>(this, 0x10) & ~0x400u) | ((this->isSkatableFloor() << 10) & 0x400u);
    this->fn_803A2CF0();
    s32 sp8 = this->fn_803A2D10();
    this->fn_803A2D50();
    if (this->fn_803A2D80() == 0 && this->fn_803A3030() == 0 && this->fn_803A3130() == 0) {
        this->checkTornado();
        s32 var_r31 = 0;
        f32 t31 = FLD<f32>(this, 0x1C4);
        TVec3f spC;
        if (MR::isNearZero(spC, 0.001f) && MR::isNearZero(t31, 0.001f)) {
            var_r31 = 1;
        }
        if (var_r31 != 0) {
            this->fn_803A0D90(&spC);
        } else if (this->fn_803A0E10()) {
            this->fn_803A0ED0(&spC, sp8);
        } else {
            this->fn_803A11A0(&spC);
        }
        this->calcShadowDir(this->mFrontVec, &FLD<TVec3f>(this, 0x214));
        this->fn_803A1330(&sp8);
        this->fn_803A1600(&sp8);
        this->fn_803A1800();
        if (!((FLD<u32>(this, 0xC) >> 27) & 1)) {
            this->fn_803A1FF0(0.0f, sp8);
        } else {
            this->fn_803A20E0(sp8);
        }
        this->fn_803A2180();
        this->fn_803A28D0(&spC);
    }
}

// NONMATCHING: temporary allocation and register scheduling; full velocity dispatch recovered.
void Mario::fn_803A2A70() {
    MarioConst* constants = mActor->mConst;
    f32 speed = constants->mTable[constants->mCurrentTable]->mWalkSpeed;
    if ((FLD<u32>(this, 0x10) >> 11) & 1) {
        TVec3f front(FLD<TVec3f>(this, 0x208));
        TVec3f direction(FLD<TVec3f>(this, 0x208));
        if (!MR::isNearZero(FLD<TVec3f>(this, 0x16C), 0.001f)) {
            direction = FLD<TVec3f>(this, 0x16C);
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
    if ((FLD<u32>(this, 0x1C) >> 26) & 1) {
        f32 walkSpeed = mWalkSpeed;
        TVec3f direction(FLD<TVec3f>(this, 0x22C));
        direction *= speed;
        TVec3f scaled(direction);
        scaled *= walkSpeed;
        addVelocity(scaled);
        return;
    }
    f32 walkSpeed = mWalkSpeed;
    TVec3f direction(FLD<TVec3f>(this, 0x208));
    direction *= speed;
    TVec3f scaled(direction);
    scaled *= walkSpeed;
    addVelocity(scaled);
}

// NONMATCHING: temporary/register scheduling; full landing switch and return structure recovered.
void Mario::fn_803A0490(s32 arg1, s32 arg2) {
    s32 var_r29 = arg1;
    s32 var_r28 = arg2;
    if (FLD<u32>(this, 0x42C) == 4) {
        TVec3f sp14(-FLD<TVec3f>(this, 0x220));
        this->setFrontVecKeepUp(sp14);
    }
    if (this->isAnimationRun("飛び込みジャンプ") || this->isAnimationRun("後方飛び込みジャンプ") || ((FLD<u32>(this, 0x14) >> 23) & 1)) {
        if (this->isAnimationRun("後方飛び込みジャンプ")) {
            TVec3f sp8(-this->mFrontVec);
            this->setFrontVecKeepUp(sp8);
        }
        if ((FLD<u32>(this, 0x14) >> 23) & 1) {
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
    if (!((FLD<u32>(this, 0x1C) >> 19) & 1)) {
        this->clearSlope();
        switch (FLD<u32>(this, 0x42C)) {
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
            if (FLD<u32>(this->mActor, 0x750) == 0) {
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
            FLD<u16>(this, 0x3CE) += 0x10;
        }
        if (FLD<u8>(this->mActor, 0xECD) != 0) {
            this->changeAnimation("ハチ匍匐着地", "基本");
        }
    } else {
        if (!this->isAnimationRun("坂すべり上向きうつぶせ", 2) && !this->isAnimationRun("坂すべり下向きあおむけ", 3)) {
            this->stopAnimation((const char*)nullptr, "基本");
        }
        this->changeAnimation("すべり着地", "基本");
        FLD<u8>(this, 0x716) = 0;
    }
}

// NONMATCHING: the wall-turn/brake gate chain is approximated; push/pop task calls are omitted
void Mario::fn_803A1330(s32* arg1) {
    f32 var_f31 = FLD<TVec3f>(this, 0x214).dot(FLD<TVec3f>(this, 0x22C));
    if (FLD<u16>(this, 0x3D2) != 0) {
        var_f31 = FLD<TVec3f>(this, 0x3E4).dot(FLD<TVec3f>(this, 0x22C));
    }
    FLD<TVec3f>(this, 0x214).dot(FLD<TVec3f>(this, 0x22C));
    if (this->fn_803A1E60(var_f31)) {
        this->recordTurnSlipAngle();
    } else {
        this->fn_803A1F60();
    }
    if (((FLD<u32>(this, 0xC) >> 16) & 1) || ((FLD<u32>(this, 0x10) >> 11) & 1)) {
        FLD<u16>(this, 0x3D2) = 0;
        FLD<u32>(this, 0xC) &= 0xFFFF7FFF;
    }
    if (FLD<u16>(this, 0x3D2) != 0) {
        this->fn_803A1F80();
        this->fn_803A1FC0();
        if (this->isAnimationRun("ブレーキ") || ((FLD<u32>(this, 0xC) >> 15) & 1)) {
            *arg1 = 1;
        }
    } else {
        u32 hi = FLD<u32>(this, 0x10);
        if (!((hi >> 11) & 1)) {
            u32 lo = FLD<u32>(this, 0xC);
            if (!((lo >> 21) & 1) && ((lo >> 15) & 1) && !((lo >> 16) & 1) && !((hi >> 28) & 1) && !this->isStatusActive(0x1F)) {
                if (var_f31 < JMACosRadian(FLD<f32>(marioCurTableEntry(this), 0x2C0)) && !((lo >> 27) & 1)) {
                    if ((hi >> 10) & 1) {
                        *arg1 = 1;
                    }
                    FLD<TVec3f>(this, 0x220) = FLD<TVec3f>(this, 0x3E4);
                    if (*arg1 == 0) {
                        TVec3f sp14;
                        sp14 = -FLD<TVec3f>(this, 0x220);
                        this->setFrontVecKeepUp(sp14);
                    }
                    if ((hi >> 10) & 1) {
                        FLD<s16>(this, 0x3D0) = FLD<s16>(marioCurTableEntry(this), 0x2D6);
                        FLD<u32>(this, 0xC) |= 0x08000000;
                        this->changeAnimation("ターンブレーキ滑り床", (const char*)nullptr);
                        FLD<TVec3f>(this, 0x2B8) = FLD<TVec3f>(this, 0x1D8);
                        this->stopWalk();
                        FLD<s32>(this, 0x754) = 10;
                    } else {
                        FLD<s16>(this, 0x3D0) = FLD<s16>(marioCurTableEntry(this), 0x2D4);
                        FLD<u32>(this, 0xC) |= 0x08000000;
                        this->mWalkSpeed = 0.0f;
                        this->changeAnimation("ターンブレーキ", (const char*)nullptr);
                        this->playEffect("共通ブレーキ");
                    }
                }
            }
        }
        FLD<u32>(this, 0xC) &= 0xFFFF7FFF;
    }
}

// NONMATCHING: brake-end timer and the slide-end task are approximated
void Mario::fn_803A1600(s32* arg1) {
    s32 t = FLD<s16>(this, 0x3D0);
    if (t != 0) {
        if (t == FLD<s16>(marioCurTableEntry(this), 0x2D4)) {
            this->startPadVib(2);
        }
        FLD<s16>(this, 0x3D0) = t - 1;
        if (!((FLD<u32>(this, 0x10) >> 10) & 1)) {
            this->mWalkSpeed = 0.0f;
        }
    }
    if (FLD<s16>(this, 0x3D0) == 0) {
        if ((FLD<u32>(this, 0xC) >> 27) & 1) {
            const TVec3f* g = this->getGravityVec();
            if (MR::diffAngleAbsHorizontal(this->getWorldPadDir(), this->mFrontVec, *g) > 1.5707964f) {
                this->stopAnimation("ブレーキ", (const char*)nullptr);
                TVec3f sp2C;
                sp2C = -this->mFrontVec;
                this->setFrontVecKeepUp(sp2C);
                FLD<u32>(this, 0xC) &= 0xF7FFFFFF;
            }
            if (this->isAnimationTerminate("ブレーキ")) {
                this->stopAnimation("ブレーキ", (const char*)nullptr);
                FLD<u32>(this, 0xC) &= 0xF7FFFFFF;
                if (FLD<f32>(this, 0x1C8) < 0.1f) {
                    this->stopWalk();
                }
            }
        }
        if (this->isAnimationRun("ターンブレーキ滑り床")) {
            TVec3f sp14;
            sp14 = -FLD<TVec3f>(this, 0x220);
            this->setFrontVecKeepUp(sp14);
            *arg1 = 1;
            this->stopAnimation("ターンブレーキ滑り床", (const char*)nullptr);
            FLD<f32>(this, 0x74C) = 0.0f;
            FLD<s32>(this, 0x754) = 0;
            this->mWalkSpeed = 0.0f;
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
    if (mActor->mVelocityBlendE24 != 0) {
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
    if (mMovementStates._1 && !mMovementStates.jumping) {
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
        _404 = FLD<u16>(constants->mTable[constants->mCurrentTable], 0x282);
    }
    u16 remaining = _404;
    if (remaining != 0) {
        constants = mActor->mConst;
        u16 duration = FLD<u16>(constants->mTable[constants->mCurrentTable], 0x282);
        _404 = remaining - 1;
        ramp = static_cast<f32>(duration - remaining) / static_cast<f32>(duration);
    }
    target *= ramp * ramp;
    if (((mMovementStates_LOW_WORD >> 16) & 1) || isStatusActive(17)) {
        constants = mActor->mConst;
        target *= constants->mTable[constants->mCurrentTable]->mTornadoMultiply;
    }
    s32 wasSquatting = (mMovementStates_LOW_WORD >> 21) & 1;
    if (wasSquatting && ((_1C_WORD >> 16) & 1)) {
        fn_803A6BE0();
    } else {
        mMovementStates_LOW_WORD &= 0xFFDFFFFF;
        if (_436 == 0 && _434 == 0 && checkSquat(false) && mSinkTimer <= 32 && !isStatusActive(31)) {
            if (!checkLockOnHoming()) mMovementStates_LOW_WORD |= 0x200000;
            if (!wasSquatting) {
                u32 flags = mMovementStates_LOW_WORD;
                if (((flags >> 21) & 1) && (((flags >> 23) & 1) || ((mMovementStates_HIGH_WORD >> 13) & 1))) {
                    mTargetWalkSpeedIndex = 0;
                    mWalkSpeed = 0.0f;
                }
            }
        }
        if (((_1C_WORD >> 16) & 1) && !((mMovementStates_LOW_WORD >> 21) & 1) && isAnimationRun("しゃがみ終了")) mMovementStates_LOW_WORD |= 0x200000;
        u32 flags = mMovementStates_LOW_WORD;
        if (!((flags >> 21) & 1) && wasSquatting) {
            mMovementStates_LOW_WORD = flags | 0x200000;
            cancelSquatMode();
            _71E = 0;
        }
    }
    if (_3D0 != 0) target = 0.0f;
    if ((mMovementStates_LOW_WORD >> 15) & 1) target = 0.0f;
    f32 inertia = decideInertia(target);
    if (sPreviousInertiaName != sInertiaName) sPreviousInertiaName = sInertiaName;
    if (mCometFireDanceArea && mTargetWalkSpeedIndex != 0) {
        inertia = 1.0f;
        target = inertia;
        mWalkSpeed = 1.5f;
    }
    if (!((mMovementStates_LOW_WORD >> 21) & 1) && (getPlayerMode() == 1 || mCometFireDanceArea)) {
        f32 speed = mWalkSpeed;
        if (speed >= 0.9999f) {
            constants = mActor->mConst;
            target *= FLD<f32>(constants->mTable[constants->mCurrentTable], 0x274);
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
            FLD<u8>(this, 0xA70) = 1;
        } else if (FLD<u8>(this, 0xA70) != 0) {
            getAnimator()->changeTrackAnim(0, "ヨッシー鈍行");
            getAnimator()->changeTrackAnim(1, "ヨッシー歩行");
            getAnimator()->changeTrackAnim(2, "ヨッシーラン");
            FLD<u8>(this, 0xA70) = 0;
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
        return FLD<f32>(c->mTable[c->mCurrentTable], 0x258);
    }
    sInertiaName = "床";
    u32 flags = mMovementStates_HIGH_WORD;
    if ((flags >> 11) & 1) return decideInertiaOnIce(speed);
    if ((flags >> 10) & 1) return decideInertiaOnSlip(speed);
    sInertiaName = "基本";
    MarioConst* c = mActor->mConst;
    MarioConstTable* table = c->mTable[c->mCurrentTable];
    f32 base = (1.0f - mWalkSpeed) * FLD<f32>(table, 0x228) + mWalkSpeed * FLD<f32>(table, 0x22C);
    f32 inertia = base * (1.0f - _3F4) + _3F4 * FLD<f32>(table, 0x254);
    if (base != inertia) sInertiaName = "空転";
    if (speed == 0.0f) {
        sInertiaName = "停止";
        c = mActor->mConst;
        inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x238);
        if ((mMovementStates_LOW_WORD >> 15) & 1) {
            sInertiaName = "ターン滑り";
            c = mActor->mConst;
            inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x240);
        }
        if (!((mMovementStates_LOW_WORD >> 15) & 1) && isAnimationRun("歩行制動ブレーキ")) {
            sInertiaName = "ターン後滑り";
            c = mActor->mConst;
            inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x244);
        }
        if ((mMovementStates_LOW_WORD >> 27) & 1) {
            sInertiaName = "ターン中";
            c = mActor->mConst;
            inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x248);
        }
        if (_3CE < 30) {
            sInertiaName = "ジャンプ終了";
            c = mActor->mConst;
            inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x24C);
        }
        if ((mMovementStates_LOW_WORD >> 21) & 1) {
            sInertiaName = "しゃがみ";
            c = mActor->mConst;
            inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x23C);
        }
    }
    if ((mMovementStates_LOW_WORD >> 16) & 1) {
        sInertiaName = "トルネード";
        c = mActor->mConst;
        if (mWalkSpeed >= speed) inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x3DC);
        else inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x3D8);
    }
    if (_3F8 != 0) {
        sInertiaName = "反射";
        _3F8--;
        c = mActor->mConst;
        inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x250);
    }
    if (mWalkSpeed < 0.08f && _3CE > 10 && speed > 0.5f && (((mMovementStates_LOW_WORD >> 21) & 1) || ((mMovementStates_LOW_WORD >> 19) & 1))) {
        c = mActor->mConst;
        _3FA = FLD<u16>(c->mTable[c->mCurrentTable], 0x27C);
        mWalkSpeed = 0.08f;
    }
    if (_3FA != 0) {
        sInertiaName = "停止急発進";
        _3FA--;
        c = mActor->mConst;
        inertia = FLD<f32>(c->mTable[c->mCurrentTable], 0x254);
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
    if (MR::fn_8001E6A0() && isStickOn()) {
        if (_60D != 1) _60D = 1;
        mDrawStates_WORD |= 0x40000;
        _10_LOW_WORD |= 0x200000;
        _40E = 0;
    }
    if (_3D0 != 0 || _3D2 != 0) mDrawStates_WORD &= 0xFFFBFFFF;
    if (!_60D && mActor->mAxisRotationDisabled6A0 != 0) mDrawStates_WORD &= 0xFFFBFFFF;
    if (MR::isInAreaObj("PlayerSideRetainOffArea", getTrans())) mDrawStates_WORD &= 0xFFFBFFFF;
    if ((isStatusActive(30) || isStatusActive(29)) && _60D) _60D = 0;
    TVec3f previousHead(mHeadVec);
    TVec3f up(-*getGravityVec());
    MR::normalize(&up);
    if (mHeadVec.dot(up) < -0.9f && (mMovementStates_LOW_WORD >> 31)) FLD<u8>(this, 0xA71) = 1;
    else if ((mMovementStates_LOW_WORD >> 30) & 1) FLD<u8>(this, 0xA71) = 0;
    f32 rate = fn_803B2CC0(&up);
    if (_60D && mTargetWalkSpeedIndex != 0 && (((mMovementStates_LOW_WORD >> 30) & 1) || !(mMovementStates_LOW_WORD >> 31))) {
        if (!((mDrawStates_WORD >> 18) & 1) || !fn_803B2BD0()) {
            fn_803B23E0(&up, &mSideVec, rate);
            fixFrontVecFromUpSide();
            _29C = groundNormal;
            _290 = mSideVec;
        }
    } else {
        if (FLD<u8>(this, 0xA71) != 0) {
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
        if (MR::diffAngleAbs(_43C, mFrontVec) < FLD<f32>(table, 0x79C)) setFrontVec(_43C);
        constants = mActor->mConst;
        table = constants->mTable[constants->mCurrentTable];
        if (MR::diffAngleAbs(mHeadVec, previousHead) < FLD<f32>(table, 0x798)) {
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
