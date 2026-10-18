// SMG2-only Mario:: functions (fn_ range A), written from the asm. Included at the end of Mario.cpp (unity TU).
// PLACEHOLDER(enemies): FLD<T>(self, off) typed access to a Mario field by its SMG2 byte offset where the header name is unknown; resolve when the Mario layout is recovered and the fields are named
template <typename T, typename P>
inline T& FLD(P* p, u32 off) { return *reinterpret_cast<T*>(reinterpret_cast<u8*>(const_cast<void*>(static_cast<const void*>(p))) + off); }


void Mario::fn_803880D0() {
    if ((FLD<u32>(this, 0x20) >> 15) & 1) {
        this->mPosHolder->fn_80406040();
        this->addTrans(FLD<TVec3f>(this, 0x160), nullptr);
        if (this->calcDistToCeil(false) < 200.0f) {
            FLD<TVec3f>(this, 0x160).zero();
            this->stopWalk();
        }
        this->mPosHolder->fn_80406080();
    }
}

void Mario::fn_80388150() {
    if (((FLD<u32>(this, 0x10) >> 8) & 1) || ((FLD<u32>(this, 0x14) >> 10) & 1)) {
        TVec3f sp38(this->mPosHolder->mPos);
        sp38 -= FLD<TVec3f>(this, 0x67C);
        MR::vecKillElement(sp38, FLD<TVec3f>(this, 0x694), &sp38);
        TVec3f* temp_r31 = &this->mPosHolder->mPos;
        TVec3f sp14(FLD<TVec3f>(this, 0x67C));
        sp14 += sp38;
        TVec3f sp20(sp14);
        sp20 -= *temp_r31;
        this->mPosHolder->fn_80406010(sp20);
        MR::vecKillElement(FLD<TVec3f>(this, 0x160), FLD<TVec3f>(this, 0x694), &FLD<TVec3f>(this, 0x160));
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
    if (!((FLD<u32>(this, 0x10) >> 29) & 1)) {
        f32 temp_f31 = 0.0f;
        u32 var_r29 = 0U;
        s32 var_r31 = 0;
        while (var_r29 < (u32)FLD<u32>(this, 0x570)) {
            Triangle* tri = FLD<Triangle*>(this, 0x574 + var_r31);
            TVec3f sp8;
            if (MR::vecKillElement(FLD<TVec3f>(this, 0x160), *tri->getNormal(0), &sp8) < temp_f31) {
                FLD<TVec3f>(this, 0x160) = sp8;
                this->mSwim->fn_803F7C20(tri->getNormal(0));
            }
            var_r29 += 1;
            var_r31 += 4;
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
    if (!((FLD<u32>(this, 0xC) >> 9) & 1) && MR::testFpViewStartTrigger()) {
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
    u32 temp_r4 = FLD<u32>(this, 0x10);
    if ((temp_r4 >> 28) & 1) {
        if (!((FLD<u32>(this, 0xC) >> 30) & 1)) {
        }
        if ((temp_r4 >> 27) & 1) {
            if (!this->isSlipPolygon(FLD<Triangle*>(this, 0x460))) {
            }
            const TVec3f* nrm = FLD<Triangle*>(this, 0x460)->getNormal(0);
            f32 temp_f31 = MR::vecKillElement(FLD<TVec3f>(this, 0x160), *nrm, &FLD<TVec3f>(this, 0x160));
            nrm = FLD<Triangle*>(this, 0x460)->getNormal(0);
            TVec3f sp14(FLD<TVec3f>(this, 0x368));
            sp14 -= *nrm;
            TVec3f sp20(sp14);
            sp20 *= 0.5f;
            f32 temp_f31_2 = temp_f31 < 0.0f ? -temp_f31 : temp_f31;
            TVec3f sp8(sp20);
            sp8 *= temp_f31_2;
            FLD<TVec3f>(this, 0x160) += sp8;
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
    if (!this->isNonFixHeadVec() && arg2 == 0 && !((FLD<u32>(this, 0xC) >> 9) & 1)) {
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
    s32 temp_r4 = FLD<s32>(this, 0x754);
    if (temp_r4 != 0) {
        f32 temp_f3 = FLD<f32>(this, 0x74C);
        FLD<s32>(this, 0x754) = temp_r4 - 1;
        FLD<s32>(this, 0x750) = 0;
        FLD<f32>(this, 0x74C) = temp_f3 + ((3.1415927f - temp_f3) / (f32)temp_r4);
    }
}

void Mario::fn_8038AB70() {
    s32 temp_r6 = FLD<s32>(this, 0x750);
    if (temp_r6 != 0) {
        s32 temp_r5 = temp_r6 - 1;
        FLD<s32>(this, 0x750) = temp_r5;
        FLD<f32>(this, 0x74C) = (FLD<f32>(this, 0x74C) * (f32)temp_r5) / (f32)temp_r6;
        if (temp_r5 == 0) {
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
    if (((u32)FLD<u32>(this, 0x14) >> 31) != 0) {
    }
    return (u32)(this->getCurrentStatus() - 8) <= 2U;
}

bool Mario::fn_803949E0() {
    if (this->isEnableAddDamage() == false) {
    }
    if (this->isStatusActive(6)) {
        FLD<u32>(this, 0xC) |= 0x10;
        FLD<TVec3f>(this, 0x7C4).zero();
        return this->checkWaterDamage();
    }
    return this->doFireDanceWithInitialDamage(1);
}

bool Mario::fn_80395310() {
    if (FLD<u32>(this, 0xC) & 1) {
    }
    if (this->isInvincible() != 0) {
    }
    u32 temp_r3 = FLD<u32>(this, 0xC);
    if (((temp_r3 >> 31) != 0) || !((temp_r3 >> 30) & 1)) {
    }
    if (this->getCurrentStatus() == 0xE) {
    }
    if (this->getCurrentStatus() == 0xF) {
    }
    if (this->getCurrentStatus() == 8) {
    }
    if ((FLD<u32>(this, 0xC) >> 4) & 1) {
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
        FLD<u32>(this, 0x10) |= 0x400;
    }
}

void Mario::fn_803A2420() {
    FLD<u32>(this, 0x1C) &= 0xFFFBFFFF;
    FLD<u16>(this, 0x40C) = 0;
    FLD<u32>(this, 0x14) &= 0xFFF7FFFF;
}

void Mario::fn_803A1FC0() {
    if (!(FLD<f32>(this, 0x278) >= 0.1f)) {
        FLD<f32>(this, 0x278) = 0.0f;
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
    if (!((FLD<u32>(this, 0xC) >> 8) & 1)) {
        return false;
    }
    FLD<u32>(this, 0xC) &= 0xFFFFFEFF;
    return FLD<MarioWall*>(this, 0x750)->startJump();
}

s32 Mario::fn_803A2D10() {
    s32 r = 0;
    if (!((FLD<u32>(this, 0x1C) >> 26) & 1) && !((FLD<u32>(this, 0x10) >> 28) & 1)) {
        if (FLD<s32>(this, 0x74C) != 0) {
            r = 1;
        }
    }
    return r;
}

void Mario::fn_803A1F80() {
    f32 temp_f1 = FLD<f32>(this, 0x1C8);
    void* table = FLD<void*>(this->mActor, 0x590);
    u32 n = FLD<u32>(table, 12);
    void* ent = *reinterpret_cast<void**>(reinterpret_cast<u8*>(table) + n * 4);
    if (temp_f1 >= FLD<f32>(ent, 0x2C8)) {
        return;
    }
    FLD<u32>(this, 0xC) |= 0x8000;
}

void Mario::fn_803A2CC0() {
    if (!((FLD<u32>(this, 0x10) >> 28) & 1)) {
        return;
    }
    FLD<u8>(this, 0x714) = 0;
    FLD<f32>(this, 0x278) = 0.0f;
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
        *out = FLD<TVec3f>(this, 0x208);
    }
}

void Mario::fn_803A23F0(TVec3f* out) {
    if ((FLD<u32>(this, 0x10) >> 28) & 1) {
        *out = FLD<TVec3f>(this, 0x208);
    } else {
        *out = FLD<TVec3f>(this, 0x22C);
    }
}

void Mario::fn_8039A460() {
    if (FLD<u16>(this, 0x540) <= 5) {
    }
}

// mActor->table(+0x590)[count(+0xC)]: the current entry used by several SMG2 helpers below
static inline void* marioCurTableEntry(Mario* self) {
    void* table = FLD<void*>(self->mActor, 0x590);
    u32 n = FLD<u32>(table, 12);
    return *reinterpret_cast<void**>(reinterpret_cast<u8*>(table) + n * 4);
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
    TVec3f sp8 = -FLD<TVec3f>(this, 0x208);
    f32 d = MR::diffAngleAbsHorizontal(FLD<TVec3f>(this, 0x8FC), sp8, *this->getGravityVec());
    return d < 0.7853982f;
}


void Mario::fn_8039A020() {
    if (FLD<u32>(this, 0x14) & 1) {
        if (this->mActor->fn_803BE0E0() == nullptr) {
            FLD<u32>(this, 0xC) &= 0xFFFFEFFF;
        }
    } else {
        if (this->mActor->fn_803BE0D0() == nullptr) {
            FLD<u32>(this, 0xC) &= 0xFFFFEFFF;
        }
    }
}

void Mario::fn_8039A090() {
    if (!this->checkTrgA()) {
        if (this->mActor->fn_803BE0C0() != nullptr) {
            FLD<u32>(this, 0x554) = FLD<u32>(this->mActor, 0x6AC);
            if (this->isEnableFutureJump()) {
                FLD<u32>(this, 0x10) |= 0x80;
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
        PSMTXRotAxisRad(sp8, &FLD<TVec3f>(this, 0x1F0), -0.12f);
        PSMTXMultVecSR(sp8, &FLD<TVec3f>(this, 0x214), arg);
    }
}

void Mario::fn_803A2450(const TVec3f* arg) {
    f32 d = FLD<TVec3f>(this, 0x208).dot(*arg);
    if (!(d >= 0.0f) && !this->isAnimationRun("ブレーキ")) {
        FLD<f32>(this, 0x744) = 3.1415927f;
        FLD<u32>(this, 0x748) = 10;
    }
}

void Mario::fn_803A2860() {
    if (!this->isAnimationRun("その場足踏み")) {
        if (FLD<TVec3f>(this, 0x214).dot(FLD<TVec3f>(this, 0x22C)) > 0.99f) {
            FLD<u32>(this, 0xC) &= 0xEFFFFFFF;
        } else {
            FLD<u32>(this, 0xC) |= 0x10000000;
        }
    }
}


void Mario::fn_803A2720() {
    if ((FLD<u32>(this->getPlayer(), 0x14) >> 13) & 1) {
        f32 d = MR::diffAngleAbs(FLD<TVec3f>(this, 0x208), FLD<TVec3f>(this, 0x22C));
        f32 c = MR::clamp(d / 3.1415927f, 0.0f, 1.0f);
        FLD<f32>(this, 0x278) = FLD<f32>(this, 0x278) * (1.0f - 0.1f * c);
    }
}

void Mario::fn_803A2C40(const TVec3f* arg) {
    FLD<TVec3f>(this, 0x328) = *arg;
    if ((FLD<u32>(this, 0xC) >> 21) & 1) {
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
    if ((FLD<u32>(this, 0x10) >> 8) & 1) {
        f32 d = MR::diffAngleAbsHorizontal(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), FLD<TVec3f>(this, 0x694));
        return d > 1.5707964f;
    }
    f32 d2 = MR::diffAngleAbsHorizontal(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), *this->getGravityVec());
    return d2 > 0.17453293f;
}

void Mario::fn_803A0D90(TVec3f* arg) {
    if (MR::isNearZero(FLD<TVec3f>(this, 0x328), 0.001f)) {
        FLD<TVec3f>(this, 0x328) = FLD<TVec3f>(this, 0x208);
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
    if ((FLD<u32>(this, 0xC) >> 15) & 1) {
        return false;
    }
    if ((FLD<u32>(this, 0xC) >> 16) & 1) {
        return false;
    }
    if (FLD<f32>(this, 0x278) >= 0.05f) {
        return false;
    }
    if ((FLD<u32>(this, 0x1C) >> 26) & 1) {
        return false;
    }
    return true;
}

void Mario::fn_8039AB50() {
    if (FLD<u16>(this, 0x42C) == 13 && !((FLD<u32>(this, 0x10) >> 31) & 1)) {
        TVec3f sp32;
        f32 old = MR::vecKillElement(FLD<TVec3f>(this, 0x2D4), *this->getGravityVec(), &sp32);
        f32 d = this->fn_8039AC10();
        if (!(old >= d)) {
            TVec3f sp8(*this->getGravityVec());
            sp8 *= d;
            TVec3f sp20(sp8);
            sp20 += sp32;
            FLD<TVec3f>(this, 0x2D4) = sp20;
        }
    }
}

f32 Mario::fn_8039ADC0() {
    if (!this->mActor->isEnableNerveChange()) {
        return 0.0f;
    }
    if (MR::getKarikariClingNum() == 0) {
        return 0.0f;
    }
    if (this->getPlayerMode() == 4) {
        return 0.0f;
    }
    f32 n = (f32)MR::getKarikariClingNum();
    return (n + 2.0f) * 0.5f;
}

void Mario::fn_803A27A0(TVec3f* arg) {
    if (MR::diffAngleAbs(FLD<TVec3f>(this, 0x208), FLD<TVec3f>(this, 0x22C)) < 2.5f) {
        return;
    }
    if (FLD<u16>(this, 0x3D4) != FLD<u16>(marioCurTableEntry(this), 0x2E6)) {
        return;
    }
    if ((FLD<u32>(this, 0x1C) >> 18) & 1) {
        return;
    }
    TVec3f sp8;
    PSVECCrossProduct(&FLD<TVec3f>(this, 0x214), arg, &sp8);
    f32 d = sp8.dot(FLD<TVec3f>(this, 0x1F0));
    FLD<u8>(this, 0x274) = d > 0.0f;
    FLD<u32>(this, 0x14) |= 0x80000;
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
    if ((FLD<u32>(this, 0xC) >> 12) & 1) {
        return false;
    }
    if ((FLD<u32>(this, 0xC) >> 17) & 1) {
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

// JGeometry::TUtil<f>::epsilonEquals(a, b, eps) stand-in: |a - b| <= eps
static inline bool marioEpsEq(f32 a, f32 b, f32 e) {
    f32 d = a - b;
    if (d < 0.0f) {
        d = -d;
    }
    return d <= e;
}

bool Mario::fn_803A1920() {
    TVec3f s;
    this->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    bool eq0 = marioEpsEq(FLD<f32>(this, 0x250), s.x, 0.0001f);
    bool eq1 = eq0 && marioEpsEq(FLD<f32>(this, 0x254), s.y, 0.0001f);
    bool eq2 = marioEpsEq(FLD<f32>(this, 0x258), s.z, 0.0001f);
    return eq1 && !eq2;
}

void Mario::fn_803A20E0(s32 arg) {
    if (FLD<u16>(this, 0x3D0) == 0) {
        TVec3f sp8;
        if (!MR::vecBlendSphere(&sp8, FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), 0.0f)) {
            MR::vecRotAxis(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), FLD<TVec3f>(this, 0x1F0), &sp8, 0.3926991f);
            if (arg == 0) {
                this->setFrontVecKeepUp(sp8);
            }
        }
    }
    FLD<u32>(this, 0x14) &= 0xFFF7FFFF;
    FLD<u32>(this, 0xC) &= 0xEFFFFFFF;
}

void Mario::fn_803A1BB0() {
    if (FLD<u16>(this, 0x3D0) == 0) {
        return;
    }
    FLD<u32>(this, 0x14) &= 0xFFF7FFFF;
    FLD<u32>(this, 0xC) &= 0xEFFFFFFF;
    f32 d = MR::diffAngleAbs(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C));
    if (MR::isInRange(d, 0.0f, 0.707f)) {
        TVec3f s;
        PSVECCrossProduct(&FLD<TVec3f>(this, 0x214), &FLD<TVec3f>(this, 0x22C), &s);
        if (FLD<u8>(this, 0x274) == 0) {
            if (s.dot(FLD<TVec3f>(this, 0x1F0)) < 0.0f) {
                FLD<u32>(this, 0x14) &= 0xFFF7FFFF;
            }
        } else {
            if (s.dot(FLD<TVec3f>(this, 0x1F0)) > 0.0f) {
                FLD<u32>(this, 0x14) &= 0xFFF7FFFF;
            }
        }
    }
}

void Mario::fn_803A1240(f32* a, f32* b) {
    void* e = marioCurTableEntry(this);
    f32 f1 = *b;
    if ((f1 < 0.0f ? -f1 : f1) > FLD<f32>(e, 0x36C)) {
        f32 f2 = FLD<f32>(e, 0x364);
        f32 av = *a;
        if ((av < 0.0f ? -av : av) < f2) {
            *a = 0.0f;
            return;
        }
        if (av > 0.0f) {
            *a = (av - f2) / (1.0f - f2);
        } else {
            *a = (av + f2) / (1.0f - f2);
        }
    } else {
        f32 av = *a;
        if ((av < 0.0f ? -av : av) <= FLD<f32>(e, 0x370)) {
            return;
        }
        f32 f2 = FLD<f32>(e, 0x368);
        if ((f1 < 0.0f ? -f1 : f1) < f2) {
            *b = 0.0f;
            return;
        }
        if (f1 > 0.0f) {
            *b = (f1 - f2) / (1.0f - f2);
        } else {
            *b = (f1 + f2) / (1.0f - f2);
        }
    }
}


s32 Mario::fn_803A1E60(f32 f) {
    void* e = marioCurTableEntry(this);
    f32 v = JMACosRadian(FLD<f32>(e, 0x2C0));
    s32 r31 = 0;
    if (f < v && !((FLD<u32>(this, 0xC) >> 26) & 1)) {
        if (FLD<f32>(this, 0x278) > FLD<f32>(e, 0x2C8)) {
            if (FLD<f32>(this, 0x3C) > FLD<f32>(this, 0x1C8)) {
                if (!((FLD<u32>(this, 0x1C) >> 18) & 1)) {
                    r31 = 1;
                }
            }
        }
    }
    return r31;
}

// NONMATCHING: branch shape of the 0x3D4 == 0 block approximated
void Mario::fn_803A1FF0(f32 x, s32 arg) {
    if ((FLD<u32>(this, 0x10) >> 28) & 1) {
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
            if ((FLD<u32>(this, 0x1C) >> 18) & 1) {
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
    if ((FLD<u32>(this, 0x14) >> 12) & 1) {
        this->fn_803A2420();
        if ((FLD<u32>(this, 0x10) >> 8) & 1) {
            this->fn_803A23F0(&sp8);
            this->fn_803A2420();
            this->fn_803A2450(&sp8);
        }
    } else if ((FLD<u32>(this, 0x14) >> 13) & 1) {
        this->fn_803A24C0(&sp8);
    } else {
        f32 d = MR::diffAngleAbs(FLD<TVec3f>(this, 0x208), FLD<TVec3f>(this, 0x22C));
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
    f32 d = MR::diffAngleAbs(FLD<TVec3f>(this, 0x208), FLD<TVec3f>(this, 0x22C));
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
    if (!MR::vecBlendSphere(arg, FLD<TVec3f>(this, 0x208), FLD<TVec3f>(this, 0x22C), s)) {
        FLD<u16>(this, 0x3D4) = 0;
        MR::vecRotAxis(FLD<TVec3f>(this, 0x214), FLD<TVec3f>(this, 0x22C), FLD<TVec3f>(this, 0x1F0), arg, 0.3926991f);
    }
}

// NONMATCHING: the wall-normal slide tail is approximated from the asm
void Mario::fn_803A28D0(TVec3f* arg) {
    this->fn_803A2A70();
    this->fn_803A2C40(arg);
    if ((FLD<u32>(this, 0x10) >> 12) & 1) {
        if (FLD<f32>(this, 0x4DC) > 120.0f) {
            const TVec3f* n = FLD<Triangle*>(this, 0x4C8)->getNormal(0);
            if (FLD<TVec3f>(this, 0x160).dot(*n) < 0.0f) {
                TVec3f sp20(this->mPosHolder->mPos);
                sp20 -= FLD<TVec3f>(this, 0x4E4);
                TVec3f sp56;
                MR::vecKillElement(sp20, *n, &sp56);
                MR::vecKillElement(sp56, *this->getGravityVec(), &sp56);
                MR::vecKillElement(sp56, FLD<TVec3f>(this, 0x208), &sp56);
                MR::normalizeOrZero(&sp56);
                if (!MR::isNearZero(sp56, 0.001f)) {
                    f32 m = PSVECMag(&FLD<TVec3f>(this, 0x160));
                    TVec3f a(sp56);
                    a.scale(m);
                    FLD<TVec3f>(this, 0x160) = a;
                    TVec3f sp44(FLD<TVec3f>(this, 0x160));
                    f32 m2 = PSVECMag(&sp44);
                    MR::normalizeOrZero(&sp44);
                    TVec3f sp32;
                    PSVECCrossProduct(&FLD<TVec3f>(this, 0x368), &sp44, &sp32);
                    MR::normalizeOrZero(&sp32);
                    PSVECCrossProduct(&sp32, &FLD<TVec3f>(this, 0x368), &sp44);
                    sp44.setLength(m2);
                    FLD<TVec3f>(this, 0x160) = sp44;
                }
            }
        }
    }
}


// NONMATCHING: the nested gate structure is approximated from the asm
void Mario::fn_803A1AC0() {
    if (MR::isNearZero(FLD<f32>(this, 0x1C8), 0.001f)) {
        FLD<u32>(this, 0x14) &= 0xFFF7FFFF;
        FLD<u32>(this, 0xC) &= 0xEFFFFFFF;
        return;
    }
    if (!MR::isNearZero(FLD<TVec3f>(this, 0x244), 0.001f)) {
        if (!((FLD<u32>(this, 0x10) >> 24) & 1) && this->fn_803A1920()) {
            FLD<u32>(this, 0xC) |= 0x10000000;
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

// NONMATCHING: gravity scaling chain approximated from the asm
f32 Mario::fn_8039AA40() {
    if (this->getPlayerMode() == 6) {
        return 0.0f;
    }
    f32 f31 = 1.0f;
    if (FLD<u32>(this, 0x42C) == 13) {
        if (!((FLD<u32>(this, 0xC) >> 17) & 1) && !this->isRising()) {
            f31 = 0.0f;
            return f31;
        }
    }
    if (this->fn_8039A100()) {
        f31 = FLD<f32>(marioCurTableEntry(this), 0x58);
        f31 *= this->mActor->getGravityRatio();
        if (FLD<u16>(this->mActor, 0x66C) != 0) {
            f31 *= 0.3f;
        }
        if (!this->mActor->fn_803B93E0() && FLD<f32>(this, 0x484) < 300.0f) {
            if (FLD<f32>(this, 0x484) < 200.0f) {
                f31 = 0.0f;
            }
        }
        this->cutGravityElementFromJumpVec(true);
        f31 = -0.1f;
    }
    return f31;
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
        FLD<u32>(this, 0x570) = 0;
    }
    if (!((FLD<u32>(this, 0x10) >> 29) & 1)) {
        for (u32 i = 0; i < FLD<u32>(this, 0x570); i++) {
            Triangle* tri = FLD<Triangle*>(this, 0x574 + i * 4);
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
void Mario::fn_803886B0() {
    if (!((FLD<u32>(this, 0x1C) >> 19) & 1)) {
        TVec3f* a = &FLD<TVec3f>(this->mActor, 0x5DC);
        f32 d = a->dot(FLD<TVec3f>(this, 0x368));
        if (MR::isNearZero(*a, d)) {
        }
        TVec3f sp38(*a);
        MR::normalizeOrZero(&sp38);
        TVec3f sp2C;
        f32 t1 = MR::vecKillElement(FLD<TVec3f>(this, 0x160), sp38, &sp2C);
        if (t1 != 0.0f) {
            TVec3f n(FLD<TVec3f>(this, 0x160));
            MR::normalizeOrZero(&n);
            f32 dd = n.dot(this->mFrontVec);
            f32 w = 1.0f - MR::clamp(1.4285715f * (-dd - 0.3f), 0.0f, 1.0f);
            if (!((FLD<u32>(this, 0xC) >> 23) & 1) && w < 0.1f) {
                if ((FLD<u32>(this, 0xC) >> 5) & 1) {
                    TVec3f sp14(this->getSideWallNorm());
                    sp14 *= -t1;
                    FLD<TVec3f>(this, 0x160) += sp14;
                } else {
                }
            }
            this->mTargetWalkSpeedIndex = 0;
            FLD<f32>(this, 0x278) *= w;
        }
    }
}

// NONMATCHING: gate chain from the draft, state transitions approximated
void Mario::fn_80388DB0() {
    if (!((FLD<u32>(this, 0xC) >> 9) & 1)) {
        if (this->checkDamage() == false) {
            if (this->isPlayerModeYoshi()) {
                this->mActor->fn_803B9810()->fn_8041DF40();
            }
            if (!this->isPlayerModeYoshi() || this->mActor->fn_803B9810()->fn_8041F8A0() == 0) {
                if (FLD<void*>(this, 0x97C) != nullptr) {
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
                FLD<u32>(this, 0x14) &= 0xFFDFFFFF;
                this->mActor->fn_803B9830()->fn_8041BB90();
            }
            u32 t = FLD<u32>(this, 0xC);
            if (((t >> 30) & 1) || !((t >> 12) & 1)) {
                FLD<u32>(this, 0x14) &= 0xFFEFFFFF;
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
            if (this->isAnimationRun("ブレーキ")) {
                this->stopAnimation((const char*)nullptr, (const char*)nullptr);
            }
        }
        if (!((FLD<u32>(this, 0x1C) >> 27) & 1)) {
            FLD<u16>(this, 0x3FE) = 0;
        }
        mPreviousCometRestrictions = mCometRestrictions;
        this->callExtraTasks(0);
        this->beeMarioOnGround();
        this->checkSpecialWaitAnimation();
        this->fn_803B4280();
        if (!((FLD<u32>(this, 0x1C) >> 9) & 1)) {
            this->retainMoveDir(0.0f, 0.0f, nullptr);
        }
    }
}

// NONMATCHING: the two-stage neighbourhood test is approximated (returns the matched-triangle flag)

// NONMATCHING: sound selection by yoshi/state approximated from the m2c draft
void Mario::fn_8038C620(s32 arg3) {
    if (FLD<u32>(this, 0xA91) != 0) {
        MR::startLevelSoundPlayer("JumpFromYoshi", -1, arg3);
    } else if (FLD<u32>(this, 0xA90) != 0) {
        if (this->isPlayerModeYoshi()) {
            if (FLD<s32>(this->mActor->fn_803B97F0(), 0x134) > 0) {
                MR::startActionSound(reinterpret_cast<const LiveActor*>(this->mActor->fn_803B97F0()), "DashJump", -1, -1, -1);
            } else {
                MR::startActionSound(reinterpret_cast<const LiveActor*>(this->mActor->fn_803B97F0()), "JumpS", -1, -1, -1);
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
        if (PSVECMag(&FLD<TVec3f>(this, 0x160)) == 2.0f && this->isAnimationRun("口に含む") && this->isAnimationRun("基本")) {
            if (this->getAnimator()->fn_803DF0E0() == 2.5f) {
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
void Mario::fn_80399D30() {
    u32 temp_r4 = this->mSinkTimer;
    if (temp_r4 == 0) {
    }
    s32 mode = FLD<u16>(this, 0x964);
    if (mode == 0x19) {
        if (temp_r4 > 0x20) {
            this->mSinkTimer = temp_r4 - 0x20;
        }
        if (this->mSinkTimer < 0x40) {
            this->mSinkTimer = 0x40;
        }
        FLD<u32>(this, 0x10) &= 0xFFFFFF7F;
    }
    if (mode == 0x1F) {
    }
    if (temp_r4 <= 100) {
    }
    if (temp_r4 > 200) {
        this->mSinkTimer = 0x80;
        this->changeAnimation("毒沼脱出", (const char*)nullptr);
    } else {
        this->mSinkTimer = 0x20;
        this->changeAnimation("埋まりジャンプB", (const char*)nullptr);
    }
    if (FLD<u16>(this, 0x964) == 0x12) {
        this->playSound("砂脱出", -1);
    } else {
        this->playSound("声砂脱出", -1);
    }
    this->playSound("埋まりジャンプA", -1);
}

// NONMATCHING: the timer-driven jump-stage bookkeeping is approximated
void Mario::fn_80399E60(TVec3f* arg1) {
    FLD<u16>(this, 0x42A) = 0;
    void* e = marioCurTableEntry(this);
    if (FLD<s16>(this, 0x3CE) > FLD<s16>(e, 0x68) || !((FLD<u32>(this, 0xC) >> 26) & 1)) {
        FLD<u32>(this, 0x42C) = 0;
    } else {
        FLD<u32>(this, 0x42C) += 1;
    }
    if (FLD<u32>(this, 0x42C) > 2) {
        FLD<u32>(this, 0x42C) = 0;
    }
    if (FLD<u32>(this, 0x42C) == 2) {
        if (FLD<f32>(this, 0x278) < FLD<f32>(e, 0x6C)) {
            FLD<u32>(this, 0x42C) = 0;
        }
    }
    if (this->getPlayerMode() == 4 || this->getPlayerMode() == 6) {
        FLD<u32>(this, 0x42C) = 0;
    }
    if (this->isPlayerModeYoshi() && FLD<u32>(this, 0x42C) > 1) {
        FLD<u32>(this, 0x42C) = 0;
    }
    u32 t = FLD<u32>(this, 0x1C);
    if ((t >> 20) & 1) {
        FLD<u32>(this, 0x42C) = 2;
        MR::start2PJumpAssistJustSound();
    } else if ((t >> 21) & 1) {
        MR::start2PJumpAssistSound();
    }
    if ((this->isPlayerModeInvincible() || FLD<u32>(this, 0xA79) != 0) && FLD<f32>(this, 0x278) > 1.0f) {
        FLD<u32>(this, 0x42C) = 1;
        FLD<u16>(this, 0x42A) = 1;
    }
    if (!MR::isNearZero(*arg1, 0.001f) && FLD<s32>(this, 0x72C) != 0) {
        FLD<u32>(this, 0x42C) = 0;
        FLD<u32>(this, 0xC) &= 0xF7FFFFFF;
        FLD<u16>(this, 0x3D2) = 0;
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

// NONMATCHING: the wall-slide/ground-dive branches are approximated
void Mario::fn_803A02F0(s32 arg1, s32 arg2, s32 arg3) {
    u32 m = FLD<u32>(this, 0x42C);
    s32 var_r31 = arg1;
    if (m - 4 <= 2 || m == 8 || m == 2 || m == 0xB) {
        var_r31 = 1;
    }
    if (((FLD<u32>(this, 0x10) >> 8) & 1) && FLD<u32>(this, 0x8D8) == FLD<u32>(this, 0x8D8)) {
        TVec3f sp8(FLD<TVec3f>(this, 0x2D4));
        sp8 -= FLD<TVec3f>(this, 0x8DC);
        if (sp8.dot(FLD<TVec3f>(this, 0x2D4)) < 0.0f) {
            FLD<TVec3f>(this, 0x2D4).zero();
        } else {
            FLD<TVec3f>(this, 0x2D4) = sp8;
        }
    }
    if ((this->mFrontVec.dot(FLD<TVec3f>(this, 0x2D4)) > 6.0f && var_r31 == 0) ||
        (this->isPlayerModeYoshi() && FLD<s32>(this->mActor->fn_803B97F0(), 0x134) > 0)) {
        f32 t31 = this->fn_803B1CA0();
        f32 dt = this->mFrontVec.dot(FLD<TVec3f>(this, 0x2D4));
        this->mWalkSpeed = MR::clamp(dt / t31, 0.0f, 2.0f);
        this->stopAnimation((const char*)nullptr, "基本");
        FLD<u16>(this, 0x3FA) = 0;
        FLD<u16>(this, 0x71E) = 0;
        if (!((FLD<u32>(this, 0x10) >> 21) & 1)) {
            this->recordTurnSlipAngle();
        }
    } else {
        this->fn_803A0490(arg2, arg3);
    }
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

// NONMATCHING: the animation switch on the mode is approximated (fallthrough cases kept, sounds/effects follow the draft)
void Mario::fn_803A0490(s32 arg1, s32 arg2) {
    s32 var_r29 = arg1;
    s32 var_r28 = arg2;
    if (FLD<u32>(this, 0x42C) == 4) {
        TVec3f sp14;
        sp14 = -FLD<TVec3f>(this, 0x220);
        this->setFrontVecKeepUp(sp14);
    }
    if (this->isAnimationRun("飛び込みジャンプ") || this->isAnimationRun("後方飛び込みジャンプ") || ((FLD<u32>(this, 0x10) >> 23) & 1)) {
        if (this->isAnimationRun("後方飛び込みジャンプ")) {
            TVec3f sp8;
            sp8 = -this->mFrontVec;
            this->setFrontVecKeepUp(sp8);
        }
        if ((FLD<u32>(this, 0x10) >> 23) & 1) {
            var_r29 = 1;
        }
        this->stopWalk();
        if (var_r29 != 0 || this->isAnimationRun("後方飛び込みジャンプ")) {
            if (var_r29 != 0) {
                this->changeAnimation("飛び込み失敗回転着地", "基本");
                this->startSlidingTask(8, 5.0f, 25);
                this->startCamVib(0);
            }
            this->changeAnimation("飛び込み失敗着地", "基本");
        }
        this->changeAnimation("飛び込み失敗着地", "基本");
    }
    if (!((FLD<u32>(this, 0x14) >> 19) & 1)) {
        this->clearSlope();
        switch (FLD<u32>(this, 0x42C)) {
        case 0:
        case 7:
            this->changeAnimation("着地", "基本");
            // fallthrough
        case 1:
            this->changeAnimation("着地B", "基本");
            // fallthrough
        case 2:
            this->changeAnimation("着地C", "基本");
            // fallthrough
        case 4:
            this->changeAnimation("着地ターン", "基本");
            // fallthrough
        case 5:
            if (!this->checkSquat(false)) {
                this->cancelSquatMode();
            }
            if (FLD<u32>(this->mActor, 0x750) == 0) {
                this->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
            }
            this->changeAnimation("着地幅とび", "基本");
            // fallthrough
        case 6:
            this->changeAnimation("しゃがみジャンプ着地", "基本");
            break;
        case 3:
        case 10:
            this->stopAnimation((const char*)nullptr, "基本");
            // fallthrough
        case 14:
            this->playSound("倒れ", -1);
            this->playSound("着地", -1);
            this->playSound("声小ダメージ", -1);
            this->playEffect("共通引き戻し着地");
            this->changeAnimation("引き戻し着地", "基本");
            var_r29 = 0;
            var_r28 = 0;
            break;
        default:
            if (this->isAnimationRun("水上ダメージ中")) {
                this->changeAnimation("中ダメージ着地", "基本");
            } else {
                this->changeAnimation("着地", "基本");
            }
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
                MR::startActionSound(reinterpret_cast<const LiveActor*>(this->mActor->fn_803B97F0()), "Land", -1, -1, -1);
            } else {
                this->playSound("声着地停止", -1);
            }
            this->startCamVib(4);
            FLD<s16>(this, 0x3CE) += 0x10;
        }
        if (FLD<u32>(this->mActor, 0xECD) != 0) {
            this->changeAnimation("ハチ匍匐着地", "基本");
        }
    } else {
        if (!this->isAnimationRun("坂すべり上向きうつぶせ", 2) && !this->isAnimationRun("坂すべり下向きあおむけ", 3)) {
            this->stopAnimation((const char*)nullptr, "基本");
        }
        this->changeAnimation("すべり着地", "基本");
        FLD<u16>(this, 0x71E) = 0;
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

// NONMATCHING: velocity blend/sink/shadow-danger terms follow the draft; vecBlend is written as a lerp
void Mario::fn_8038B9F0() {
    TVec3f spEC;
    spEC.zero();
    if (!this->isStatusActive(4)) {
        if (FLD<u32>(this->mActor, 0xE24) != 0) {
            TVec3f blend(FLD<TVec3f>(this, 0x13C));
            FLD<TVec3f>(this, 0x13C) = blend + (spEC - blend) * 0.3f;
        }
        void* st = FLD<void*>(this, 0x97C);
        if (st == nullptr) {
            if (this->isAnimationRun("LifeAlertTrg")) {
                TVec3f spB0(FLD<TVec3f>(this, 0x4E8));
                spB0 -= this->mPosHolder->mPos;
                TVec3f spE0;
                MR::vecKillElement(spB0, this->mHeadVec, &spE0);
                f32 mag = PSVECMag(&spE0);
                TVec3f spA4(*this->mFrontWallTriangle->getNormal(0));
                spA4 *= mag - 35.0f;
                spEC = spA4;
            }
        } else if (FLD<u32>(st, 0xC) == 6) {
            f32 t31 = FLD<f32>(marioCurTableEntry(this), 0x48);
            TVec3f sp98(-*this->getGravityVec());
            sp98 *= t31;
            spEC += sp98;
        }
        u32 hi = FLD<u32>(this, 0xC);
        if (((hi >> 30) & 1) && ((hi >> 31) == 0)) {
            f32 vs = this->mVerticalSpeed;
            if (vs > 5.0f && vs < 65.0f && !this->isCurrentShadowFloorDangerAction()) {
                TVec3f spD4(this->mGroundPos);
                spD4 -= this->mShadowPos;
                MR::normalizeOrZero(&spD4);
                f32 d = spD4.dot(FLD<TVec3f>(this, 0x368));
                if (d < 0.1f) {
                    TVec3f sp80(FLD<TVec3f>(this, 0x1D8));
                    sp80 *= vs;
                    spEC += sp80;
                } else {
                    TVec3f sp74(this->mGroundPos);
                    sp74 -= this->mPosHolder->mPos;
                    f32 t = MR::vecKillElement(sp74, *this->getGravityVec(), &spEC);
                    TVec3f sp68(*this->getGravityVec());
                    sp68 *= t;
                    spEC += sp68;
                }
            }
        }
        TVec3f spBC;
        spBC.zero();
        if (this->mSinkTimer != 0) {
            TVec3f sp44(*this->getGravityVec());
            sp44 *= (f32)this->mSinkTimer;
            TVec3f sp50(sp44);
            sp50 *= 0.00390625f;
            TVec3f sp5C(sp50);
            sp5C *= 120.0f;
            spBC = sp5C;
        }
        f32 var_f31 = 0.2f;
        if (!this->isUseSimpleGroundCheck()) {
            if (this->fn_8038CCF0()) {
                var_f31 = 0.01f;
            } else if (this->mTargetWalkSpeedIndex == 0 && this->mWalkSpeed < 0.2f && this->isBlendWaitGround()) {
                f32 m = PSVECMag(&spEC);
                if (PSVECMag(&FLD<TVec3f>(this, 0x148)) > m) {
                    var_f31 = 0.5f;
                } else {
                    var_f31 = 0.02f * (f32)FLD<s16>(this, 0x3CE);
                }
            }
        }
        TVec3f& v148 = FLD<TVec3f>(this, 0x148);
        v148 = v148 + (spEC - v148) * var_f31;
        TVec3f& v154 = FLD<TVec3f>(this, 0x154);
        v154 = v154 + (spBC - v154) * 0.2f;
        TVec3f sp8(FLD<TVec3f>(this, 0x148));
        sp8 += FLD<TVec3f>(this, 0x154);
        FLD<TVec3f>(this, 0x13C) = sp8;
    }
}

void Mario::fn_80395400(u32 arg) {
    this->pushTask(&Mario::taskOnEffectCheck, arg);
}
