// SMG2-only Mario:: functions (fn_ range A), written from the asm. Included at the end of Mario.cpp (unity TU).
// FLD<T>(self, off): typed access to a Mario field by its SMG2 byte offset (used where the header name is unknown).
template <typename T, typename P>
inline T& FLD(P* p, u32 off) { return *reinterpret_cast<T*>(reinterpret_cast<u8*>(const_cast<void*>(static_cast<const void*>(p))) + off); }

extern "C" void fn_80406040(MarioPosHolder*);
extern "C" void fn_80406080(MarioPosHolder*);
extern "C" void fn_803F7C20(void*, const TVec3f*);
extern "C" void* fn_803EF670(const void*);
extern "C" s32 fn_80056780();
extern "C" void fn_803B3010(Mario* self);
extern "C" s32 fn_803B3040(Mario*, void*, void*);
extern "C" void fn_803B3280(Mario*, void*, void*);
extern "C" void fn_803B94E0(MarioActor*);
extern "C" void fn_803B9870(MarioActor*);
extern "C" void fn_803BE840(MarioActor*, const TVec3f*);

extern "C" void fn_803880D0(Mario* self) {
    if ((FLD<u32>(self, 0x20) >> 15) & 1) {
        fn_80406040(self->mPosHolder);
        self->addTrans(FLD<TVec3f>(self, 0x160), nullptr);
        if (self->calcDistToCeil(false) < 200.0f) {
            FLD<TVec3f>(self, 0x160).zero();
            self->stopWalk();
        }
        fn_80406080(self->mPosHolder);
    }
}

extern "C" void fn_80388150(Mario* self) {
    if (((FLD<u32>(self, 0x10) >> 8) & 1) || ((FLD<u32>(self, 0x14) >> 10) & 1)) {
        TVec3f sp38(self->mPosHolder->mPos);
        sp38 -= FLD<TVec3f>(self, 0x67C);
        MR::vecKillElement(sp38, FLD<TVec3f>(self, 0x694), &sp38);
        TVec3f* temp_r31 = &self->mPosHolder->mPos;
        TVec3f sp14(FLD<TVec3f>(self, 0x67C));
        sp14 += sp38;
        TVec3f sp20(sp14);
        sp20 -= *temp_r31;
        fn_80406010(self->mPosHolder, sp20);
        MR::vecKillElement(FLD<TVec3f>(self, 0x160), FLD<TVec3f>(self, 0x694), &FLD<TVec3f>(self, 0x160));
        TVec3f sp8(FLD<TVec3f>(self, 0x688));
        sp8 -= FLD<TVec3f>(self, 0x67C);
        TVec3f sp2C;
        f32 temp_f1 = MR::vecKillElement(sp8, FLD<TVec3f>(self, 0x694), &sp2C);
        f32 var_f31 = temp_f1;
        if (temp_f1 > 1.0f) {
            var_f31 = 1.0f;
        } else if (temp_f1 < -1.0f) {
            var_f31 = -1.0f;
        }
        sp2C = FLD<TVec3f>(self, 0x694);
        sp2C.setLength(var_f31);
        FLD<TVec3f>(self, 0x67C) += sp2C;
    }
}

extern "C" void fn_803885F0(Mario* self) {
    if (!((FLD<u32>(self, 0x10) >> 29) & 1)) {
        f32 temp_f31 = 0.0f;
        u32 var_r29 = 0U;
        s32 var_r31 = 0;
        while (var_r29 < (u32)FLD<u32>(self, 0x570)) {
            Triangle* tri = FLD<Triangle*>(self, 0x574 + var_r31);
            TVec3f sp8;
            if (MR::vecKillElement(FLD<TVec3f>(self, 0x160), *tri->getNormal(0), &sp8) < temp_f31) {
                FLD<TVec3f>(self, 0x160) = sp8;
                fn_803F7C20(self->mSwim, tri->getNormal(0));
            }
            var_r29 += 1;
            var_r31 += 4;
        }
    }
}

extern "C" bool fn_803890B0(Mario* self) {
    if (self->isStatusActive(6)) {
        return false;
    }
    return !self->isStatusActive(0x27);
}

extern "C" void fn_803896E0(Mario* self) {
    FLD<u8>(self, 0x89C) = 0;
    if (!((FLD<u32>(self, 0xC) >> 9) & 1) && MR::testFpViewStartTrigger()) {
        if (MR::isPossibleToShiftToFirstPersonCamera() == false) {
            if (!MR::isDemoActive()) {
                if (!MR::isStageStoryBook() && !MR::isStageFileSelect() && !fn_80056780() && !MR::isStageStaffRoll()) {
                    MR::startSystemSE("SE_SY_CAMERA_NG", -1, -1);
                }
                FLD<u8>(self, 0x89C) = 1;
            }
        } else {
            self->tryFpViewMode();
        }
    }
}

extern "C" void fn_803897A0(Mario* self) {
    u32 temp_r4 = FLD<u32>(self, 0x10);
    if ((temp_r4 >> 28) & 1) {
        if (!((FLD<u32>(self, 0xC) >> 30) & 1)) {
        }
        if ((temp_r4 >> 27) & 1) {
            if (fn_803EF670(FLD<Triangle*>(self, 0x460)) == nullptr) {
            }
            const TVec3f* nrm = FLD<Triangle*>(self, 0x460)->getNormal(0);
            f32 temp_f31 = MR::vecKillElement(FLD<TVec3f>(self, 0x160), *nrm, &FLD<TVec3f>(self, 0x160));
            nrm = FLD<Triangle*>(self, 0x460)->getNormal(0);
            TVec3f sp14(FLD<TVec3f>(self, 0x368));
            sp14 -= *nrm;
            TVec3f sp20(sp14);
            sp20 *= 0.5f;
            f32 temp_f31_2 = temp_f31 < 0.0f ? -temp_f31 : temp_f31;
            TVec3f sp8(sp20);
            sp8 *= temp_f31_2;
            FLD<TVec3f>(self, 0x160) += sp8;
        }
    }
}

extern "C" void fn_80389D90(void*, f32 (*m)[4], const TVec3f* col2, const TVec3f* col1, const TVec3f* col0) {
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

extern "C" void fn_8038A600(Mario* self);

extern "C" void fn_8038A560(Mario* self, f32 (*arg1)[4], s32 arg2) {
    if (!self->isNonFixHeadVec() && arg2 == 0 && !((FLD<u32>(self, 0xC) >> 9) & 1)) {
        self->fixHeadFrontVecByGravity();
    }
    if (!self->updatePosture(arg1)) {
        self->createDirectionMtx(arg1);
        PSMTXConcat(FLD<TMtx34f>(self, 0xC4), arg1, arg1);
        fn_8038A600(self);
    }
}

extern "C" void fn_8038A840(Mario* self, void*, f32 x, f32 y, f32 z) {
    FLD<TVec3f>(self, 0x60).set(x, y, z);
    MR::orderRotateMtx(0, FLD<TVec3f>(self, 0x60), FLD<TMtx34f>(self, 0x6C));
    MR::orderRotateMtx(0, -FLD<TVec3f>(self, 0x60), FLD<TMtx34f>(self, 0x9C));
}

extern "C" void fn_8038A9D0(Mario* self) {
    TMtx34f sp18;
    PSMTXRotAxisRad(sp18, &self->mHeadVec, FLD<f32>(self, 0x74C));
    TVec3f sp8;
    PSMTXMultVec(sp18, &self->mFrontVec, &sp8);
    self->setFrontVecKeepUp(sp8);
    FLD<f32>(self, 0x74C) = 0.0f;
}

extern "C" void fn_8038AB10(Mario* self) {
    s32 temp_r4 = FLD<s32>(self, 0x754);
    if (temp_r4 != 0) {
        f32 temp_f3 = FLD<f32>(self, 0x74C);
        FLD<s32>(self, 0x754) = temp_r4 - 1;
        FLD<s32>(self, 0x750) = 0;
        FLD<f32>(self, 0x74C) = temp_f3 + ((3.1415927f - temp_f3) / (f32)temp_r4);
    }
}

extern "C" void fn_8038AB70(Mario* self) {
    s32 temp_r6 = FLD<s32>(self, 0x750);
    if (temp_r6 != 0) {
        s32 temp_r5 = temp_r6 - 1;
        FLD<s32>(self, 0x750) = temp_r5;
        FLD<f32>(self, 0x74C) = (FLD<f32>(self, 0x74C) * (f32)temp_r5) / (f32)temp_r6;
        if (temp_r5 == 0) {
            self->setFrontVecKeepUp(self->mFrontVec);
        }
    }
}

extern "C" void fn_8038C890(u8* self) {
    self[0] = 0;
    self[1] = 0;
    self[2] = 0;
}

extern "C" void fn_803929A0(Mario* self) {
    fn_803B3010(self);
    Triangle sp18;
    TVec3f sp8;
    if (fn_803B3040(self, &sp8, &sp18) != 0) {
        fn_803B3280(self, &sp8, &sp18);
    }
}

extern "C" bool fn_80393C40(Mario* self) {
    if (((u32)FLD<u32>(self, 0x14) >> 31) != 0) {
    }
    return (u32)(self->getCurrentStatus() - 8) <= 2U;
}

extern "C" bool fn_803949E0(Mario* self) {
    if (self->isEnableAddDamage() == false) {
    }
    if (self->isStatusActive(6)) {
        FLD<u32>(self, 0xC) |= 0x10;
        FLD<TVec3f>(self, 0x7C4).zero();
        return self->checkWaterDamage();
    }
    return self->doFireDanceWithInitialDamage(1);
}

extern "C" bool fn_80395310(Mario* self) {
    if (FLD<u32>(self, 0xC) & 1) {
    }
    if (self->isInvincible() != 0) {
    }
    u32 temp_r3 = FLD<u32>(self, 0xC);
    if (((temp_r3 >> 31) != 0) || !((temp_r3 >> 30) & 1)) {
    }
    if (self->getCurrentStatus() == 0xE) {
    }
    if (self->getCurrentStatus() == 0xF) {
    }
    if (self->getCurrentStatus() == 8) {
    }
    if ((FLD<u32>(self, 0xC) >> 4) & 1) {
    }
    fn_803B94E0(self->mActor);
    self->stopJump();
    self->stopWalk();
    self->changeStatus(self->mStun);
    return true;
}

extern "C" bool fn_803972D0(Mario* self, const TVec3f* arg1) {
    if (self->isStatusActive(0x19) != 0) {
    }
    if (self->isAnimationRun("空中ひねり") != 0) {
    }
    if (self->isAnimationRun("雲スピン空中") != 0) {
    }
    if (self->isPlayerModeYoshi() != 0) {
        fn_803B9870(self->mActor);
        fn_803BE840(self->mActor, arg1);
    }
    self->changeStatus(self->mFlow);
    return true;
}

extern "C" void fn_8039CFD0(Mario* self) {
    FLD<TVec3f>(self, 0x4B0) = self->mPosHolder->mPos;
}

extern "C" void fn_803A1E40(Mario* self) {
    u16 temp_r4 = FLD<u16>(self, 0x3D4);
    if (temp_r4 != 0) {
        FLD<u16>(self, 0x3D4) = temp_r4 - 1;
    }
}

extern "C" void fn_803A1F60(Mario* self) {
    u16 temp_r4 = FLD<u16>(self, 0x3D2);
    if (temp_r4 != 0) {
        FLD<u16>(self, 0x3D2) = temp_r4 - 1;
    }
}

extern "C" void fn_803A2CF0(Mario* self) {
    if (FLD<u16>(self, 0x964) == 35) {
        FLD<u32>(self, 0x10) |= 0x400;
    }
}

extern "C" void fn_803A2420(Mario* self) {
    FLD<u32>(self, 0x1C) &= 0xFFFBFFFF;
    FLD<u16>(self, 0x40C) = 0;
    FLD<u32>(self, 0x14) &= 0xFFF7FFFF;
}

extern "C" void fn_803A1FC0(Mario* self) {
    if (FLD<f32>(self, 0x278) >= 0.1f) {
        FLD<f32>(self, 0x278) = 0.0f;
        FLD<u16>(self, 0x3D2) = 0;
    }
}

extern "C" void fn_803A2690(Mario* self, f32* p) {
    if (FLD<u8>(self->mActor, 0xECD) != 0) {
        if (*p > 0.1f) {
            *p = 0.1f;
        }
    }
}

extern "C" bool fn_8039A430(Mario* self) {
    if (!((FLD<u32>(self, 0xC) >> 8) & 1)) {
        return false;
    }
    FLD<u32>(self, 0xC) &= 0xFFFFFEFF;
    return FLD<MarioWall*>(self, 0x750)->startJump();
}

extern "C" s32 fn_803A2D10(Mario* self) {
    s32 r = 0;
    if (!((FLD<u32>(self, 0x1C) >> 26) & 1) && !((FLD<u32>(self, 0x10) >> 28) & 1)) {
        if (FLD<s32>(self, 0x74C) != 0) {
            r = 1;
        }
    }
    return r;
}

extern "C" void fn_803A1F80(Mario* self) {
    f32 temp_f1 = FLD<f32>(self, 0x1C8);
    void* table = FLD<void*>(self->mActor, 0x590);
    u32 n = FLD<u32>(table, 12);
    void* ent = *reinterpret_cast<void**>(reinterpret_cast<u8*>(table) + n * 4);
    if (temp_f1 >= FLD<f32>(ent, 0x2C8)) {
        return;
    }
    FLD<u32>(self, 0xC) |= 0x8000;
}

extern "C" void fn_803A2CC0(Mario* self) {
    if (!((FLD<u32>(self, 0x10) >> 28) & 1)) {
        return;
    }
    FLD<u8>(self, 0x714) = 0;
    FLD<f32>(self, 0x278) = 0.0f;
    self->playSound("坂滑り", -1);
}

extern "C" void fn_803A1A20(Mario* self, f32* out) {
    self->mActor->getStickValue(&out[0], &out[1]);
    out[2] = 0.0f;
}

extern "C" void fn_803A2530(Mario* self, TVec3f* out) {
    if (FLD<f32>(self, 0x520) == FLD<f32>(self, 0x524)) {
        *out = FLD<TVec3f>(self, 0x22C);
    } else {
        *out = FLD<TVec3f>(self, 0x208);
    }
}

extern "C" void fn_803A23F0(Mario* self, TVec3f* out) {
    if ((FLD<u32>(self, 0x10) >> 28) & 1) {
        *out = FLD<TVec3f>(self, 0x208);
    } else {
        *out = FLD<TVec3f>(self, 0x22C);
    }
}

extern "C" void fn_8039A460(Mario* self) {
    if (FLD<u16>(self, 0x540) <= 5) {
    }
}

// mActor->table(+0x590)[count(+0xC)]: the current entry used by several SMG2 helpers below
static inline void* marioCurTableEntry(Mario* self) {
    void* table = FLD<void*>(self->mActor, 0x590);
    u32 n = FLD<u32>(table, 12);
    return *reinterpret_cast<void**>(reinterpret_cast<u8*>(table) + n * 4);
}

extern "C" void fn_803A1A60(Mario* self, TVec3f* out);
extern "C" bool fn_80391330(Mario* self, Triangle* tri);

extern "C" void fn_8039A5B0(Mario* self) {
    if (self->beeMarioOnAir()) {
        FLD<u16>(self, 0x764) = FLD<u16>(marioCurTableEntry(self), 0x670);
    }
}

extern "C" void fn_803A1D60(Mario* self) {
    PSVECCrossProduct(&FLD<TVec3f>(self, 0x214), &FLD<TVec3f>(self, 0x22C), &FLD<TVec3f>(self, 0x3D8));
    fn_803A1A60(self, &FLD<TVec3f>(self, 0x244));
    FLD<TVec3f>(self, 0x238) = FLD<TVec3f>(self, 0x22C);
}

extern "C" bool fn_803902B0(Mario* self, HitSensor* arg1) {
    bool r = false;
    const HitSensor* temp_r3 = FLD<const HitSensor*>(FLD<void*>(self, 0x460), 8);
    if (temp_r3 != nullptr && MR::isSensorPressObj(temp_r3)) {
        r = true;
    }
    return r;
}

extern "C" bool fn_80391260(Mario* self, Triangle* arg1) {
    if (!((FLD<u32>(self, 0x10) >> 8) & 1)) {
        return false;
    }
    f32 d = arg1->getNormal(0)->dot(FLD<TVec3f>(self, 0x694));
    d = d < 0.0f ? -d : d;
    return d > 0.707f;
}

extern "C" void fn_803912D0(Mario* self, Triangle* arg1) {
    if (fn_80391330(self, arg1)) {
        *FLD<Triangle*>(self, 0x4D8) = *arg1;
        FLD<TVec3f>(self, 0x514) = FLD<TVec3f>(arg1, 0x64);
    }
}

extern "C" bool fn_8039A3D0(Mario* self) {
    bool r = false;
    if (FLD<u16>(self, 0x3BC) > 10 && self->checkTrgA()) {
        r = true;
    }
    return r;
}

extern "C" bool fn_803A0FF0(Mario* self) {
    u16 v = FLD<u16>(self, 0x40A);
    if (v == 0) {
        return false;
    }
    if (FLD<f32>(self, 0x1C8) < 0.75f) {
        FLD<u16>(self, 0x40A) = 0;
        return false;
    }
    FLD<u16>(self, 0x40A) = v - 1;
    FLD<f32>(self, 0x1C0) = 0.0f;
    FLD<f32>(self, 0x1C4) = 0.0f;
    FLD<f32>(self, 0x1C8) = 0.0f;
    return true;
}

extern "C" bool fn_803A2270(Mario* self) {
    TVec3f sp8 = -FLD<TVec3f>(self, 0x208);
    f32 d = MR::diffAngleAbsHorizontal(FLD<TVec3f>(self, 0x8FC), sp8, *self->getGravityVec());
    return d < 0.7853982f;
}

extern "C" void* fn_803BE0E0(MarioActor*);
extern "C" void* fn_803BE0D0(MarioActor*);
extern "C" void* fn_803BE0C0(MarioActor*);

extern "C" void fn_8039A020(Mario* self) {
    if (FLD<u32>(self, 0x14) & 1) {
        if (fn_803BE0E0(self->mActor) == nullptr) {
            FLD<u32>(self, 0xC) &= 0xFFFFEFFF;
        }
    } else {
        if (fn_803BE0D0(self->mActor) == nullptr) {
            FLD<u32>(self, 0xC) &= 0xFFFFEFFF;
        }
    }
}

extern "C" void fn_8039A090(Mario* self) {
    if (!self->checkTrgA()) {
        if (fn_803BE0C0(self->mActor) != nullptr) {
            FLD<u32>(self, 0x554) = FLD<u32>(self->mActor, 0x6AC);
            if (self->isEnableFutureJump()) {
                FLD<u32>(self, 0x10) |= 0x80;
            }
        }
    }
}

extern "C" f32 fn_803A26C0(Mario* self, f32 x) {
    void* e = marioCurTableEntry(self);
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

extern "C" void fn_803A24C0(Mario* self, TVec3f* arg) {
    if (FLD<u8>(self, 0x274) == 0) {
        TMtx34f sp8;
        PSMTXRotAxisRad(sp8, &FLD<TVec3f>(self, 0x1F0), -0.12f);
        PSMTXMultVecSR(sp8, &FLD<TVec3f>(self, 0x214), arg);
    }
}

extern "C" void fn_803A2450(Mario* self, const TVec3f* arg) {
    f32 d = FLD<TVec3f>(self, 0x208).dot(*arg);
    if (d >= 0.0f && !self->isAnimationRun("ブレーキ")) {
        FLD<f32>(self, 0x744) = 3.1415927f;
        FLD<u32>(self, 0x748) = 10;
    }
}

extern "C" void fn_803A2860(Mario* self) {
    if (!self->isAnimationRun("その場足踏み")) {
        if (FLD<TVec3f>(self, 0x214).dot(FLD<TVec3f>(self, 0x22C)) > 0.99f) {
            FLD<u32>(self, 0xC) &= 0xEFFFFFFF;
        } else {
            FLD<u32>(self, 0xC) |= 0x10000000;
        }
    }
}

extern "C" void fn_803F7C00(void*);
extern "C" void fn_803A1240(Mario* self, f32* a, f32* b);

extern "C" void fn_803A2720(Mario* self) {
    if ((FLD<u32>(self->getPlayer(), 0x14) >> 13) & 1) {
        f32 d = MR::diffAngleAbs(FLD<TVec3f>(self, 0x208), FLD<TVec3f>(self, 0x22C));
        f32 c = MR::clamp(d / 3.1415927f, 0.0f, 1.0f);
        FLD<f32>(self, 0x278) = FLD<f32>(self, 0x278) * (1.0f - 0.1f * c);
    }
}

extern "C" void fn_803A2C40(Mario* self, const TVec3f* arg) {
    FLD<TVec3f>(self, 0x328) = *arg;
    if ((FLD<u32>(self, 0xC) >> 20) & 1) {
        if (FLD<u8>(self, 0x714) != 0) {
            FLD<TVec3f>(self, 0x334) = *arg;
        }
    } else {
        if (FLD<u16>(self, 0x3D2) == 0) {
            FLD<TVec3f>(self, 0x334) = *arg;
        }
    }
}

// NONMATCHING: final arithmetic not recovered from the m2c draft (rough body)
extern "C" f32 fn_803A1DB0(Mario* self) {
    u16 v = FLD<u16>(self, 0x3D4);
    u16 lim = FLD<u16>(marioCurTableEntry(self), 0x2E6);
    if (v >= lim || FLD<u8>(self, 0x714) <= 4) {
        return 1.0f;
    }
    return 1.0f;
}

extern "C" void fn_803A11A0(Mario* self, TVec3f* arg) {
    f32 sp12 = FLD<f32>(self, 0x1C0);
    f32 sp8 = FLD<f32>(self, 0x1C4);
    if (!(FLD<u32>(self->getPlayer(), 0x14) & 0x4000)) {
        fn_803A1240(self, &sp12, &sp8);
        self->calcMoveDir(sp12, sp8, arg, true);
        self->calcShadowDir(*arg, &FLD<TVec3f>(self, 0x22C));
    }
}

extern "C" bool fn_80394550(Mario* self, const TVec3f* arg) {
    if (self->damage(*arg)) {
        if (self->isStatusActive(6)) {
            fn_803F7C00(self->mSwim);
        } else {
            void* e = marioCurTableEntry(self);
            FLD<MarioDamage*>(self, 0x7C4)->setVecSize(FLD<f32>(e, 0x170), FLD<f32>(e, 0x16C));
            self->playSound("投げられ", -1);
        }
        return true;
    }
    return false;
}

extern "C" bool fn_80390210(Mario* self, HitSensor* arg) {
    if (MR::isSensorPressObj(reinterpret_cast<const HitSensor*>(FLD<void*>(arg, 8)))) {
        return true;
    }
    FloorCode* fc = FLD<FloorCode*>(self, 0x960);
    if (fc->getCode(reinterpret_cast<const Triangle*>(arg)) == 0x1D) {
        return true;
    }
    if (fc->getCode(reinterpret_cast<const Triangle*>(arg)) == 0x27) {
        return true;
    }
    if (fn_803902B0(self, arg)) {
        return true;
    }
    u16 v = FLD<u16>(self, 0x964);
    return v == 0x1D || v == 0x27;
}

extern "C" void fn_803A1A60(Mario* self, TVec3f* arg) {
    TVec3f s;
    self->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    PSVECCrossProduct(&s, arg, &FLD<TVec3f>(self, 0x250));
}

extern "C" bool fn_803A19C0(Mario* self) {
    TVec3f s;
    self->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    return s.dot(FLD<TVec3f>(self, 0x244)) > 0.0f;
}

extern "C" bool fn_803A1050(Mario* self) {
    if ((FLD<u32>(self, 0x10) >> 8) & 1) {
        f32 d = MR::diffAngleAbsHorizontal(FLD<TVec3f>(self, 0x214), FLD<TVec3f>(self, 0x22C), FLD<TVec3f>(self, 0x694));
        return d > 1.5707964f;
    }
    f32 d2 = MR::diffAngleAbsHorizontal(FLD<TVec3f>(self, 0x214), FLD<TVec3f>(self, 0x22C), *self->getGravityVec());
    return d2 > 0.17453293f;
}

extern "C" void fn_803A0D90(Mario* self, TVec3f* arg) {
    if (MR::isNearZero(FLD<TVec3f>(self, 0x328), 0.001f)) {
        FLD<TVec3f>(self, 0x328) = FLD<TVec3f>(self, 0x208);
        *arg = FLD<TVec3f>(self, 0x328);
        self->calcShadowDir(FLD<TVec3f>(self, 0x328), &FLD<TVec3f>(self, 0x22C));
        FLD<u16>(self, 0x40A) = 10;
    }
}

extern "C" f32 fn_8039AC10(Mario* self);
extern "C" void fn_803EEC20(Mario* self, const char*, u32);

extern "C" bool fn_803A0E10(Mario* self) {
    if (FLD<u8>(self, 0x714) == 0) {
        return false;
    }
    if (FLD<u8>(self, 0x715) != 0) {
        return false;
    }
    if (self->isAnimationRun("ブレーキ")) {
        return false;
    }
    if (self->isAnimationRun("ターンブレーキ")) {
        return false;
    }
    if ((FLD<u32>(self, 0xC) >> 15) & 1) {
        return false;
    }
    if ((FLD<u32>(self, 0xC) >> 16) & 1) {
        return false;
    }
    if (FLD<f32>(self, 0x278) >= 0.05f) {
        return false;
    }
    if ((FLD<u32>(self, 0x1C) >> 26) & 1) {
        return false;
    }
    return true;
}

extern "C" void fn_8039AB50(Mario* self) {
    if (FLD<u16>(self, 0x42C) == 13 && !((FLD<u32>(self, 0x10) >> 31) & 1)) {
        TVec3f sp32;
        f32 old = MR::vecKillElement(FLD<TVec3f>(self, 0x2D4), *self->getGravityVec(), &sp32);
        f32 d = fn_8039AC10(self);
        if (!(old >= d)) {
            TVec3f sp8(*self->getGravityVec());
            sp8 *= d;
            TVec3f sp20(sp8);
            sp20 += sp32;
            FLD<TVec3f>(self, 0x2D4) = sp20;
        }
    }
}

extern "C" f32 fn_8039ADC0(Mario* self) {
    if (!self->mActor->isEnableNerveChange()) {
        return 0.0f;
    }
    if (MR::getKarikariClingNum() == 0) {
        return 0.0f;
    }
    if (self->getPlayerMode() == 4) {
        return 0.0f;
    }
    f32 n = (f32)MR::getKarikariClingNum();
    return (n + 2.0f) * 0.5f;
}

extern "C" void fn_803A27A0(Mario* self, TVec3f* arg) {
    if (MR::diffAngleAbs(FLD<TVec3f>(self, 0x208), FLD<TVec3f>(self, 0x22C)) < 2.5f) {
        return;
    }
    if (FLD<u16>(self, 0x3D4) != FLD<u16>(marioCurTableEntry(self), 0x2E6)) {
        return;
    }
    if ((FLD<u32>(self, 0x1C) >> 18) & 1) {
        return;
    }
    TVec3f sp8;
    PSVECCrossProduct(&FLD<TVec3f>(self, 0x214), arg, &sp8);
    f32 d = sp8.dot(FLD<TVec3f>(self, 0x1F0));
    FLD<u8>(self, 0x274) = d > 0.0f;
    FLD<u32>(self, 0x14) |= 0x80000;
}

extern "C" f32 fn_8039AC10(Mario* self) {
    void* e = marioCurTableEntry(self);
    if (self->getPlayerMode() == 4 && !self->isRising()) {
        return FLD<f32>(e, 0x8C);
    }
    if (FLD<u16>(self, 0x42C) == 11 && !self->isRising()) {
        if (FLD<u16>(self, 0x3BE) >= FLD<u16>(e, 0x138)) {
            return FLD<f32>(e, 0x88);
        }
    }
    void* e2 = marioCurTableEntry(self);
    u8 lvl = self->mActor->getGravityLevel();
    return FLD<f32>(e2, 0x7C + ((lvl << 2) & 0x3C));
}

extern "C" bool fn_8039A100(Mario* self) {
    if (FLD<u32>(self, 0x42C) == 8 && FLD<u16>(self, 0x426) == 0) {
        s16 v = FLD<s16>(self, 0x422);
        if (v != 0) {
            return v > (s16)(FLD<u16>(marioCurTableEntry(self), 0x3F0) - 16);
        }
    }
    if ((FLD<u32>(self, 0xC) >> 12) & 1) {
        return false;
    }
    if ((FLD<u32>(self, 0xC) >> 17) & 1) {
        return false;
    }
    if (!self->isRising()) {
        return false;
    }
    return FLD<u32>(self, 0x42C) == 2;
}

extern "C" void fn_803A10D0(Mario* self, s32 arg) {
    void* e = marioCurTableEntry(self);
    u32 dur = FLD<u8>(self->mActor, 0xECD) == 0 ? 30 : FLD<u16>(e, 0x2E4);
    if (arg == 0) {
        self->setFrontVecKeepUp(FLD<TVec3f>(self, 0x22C), dur);
        if (self->isEnableTurn()) {
            self->changeAnimation("その場足踏み", (const char*)nullptr);
            if (!self->isAnimationRun("カリカリ限界") && !self->isPlayerModeYoshi()) {
                fn_803EEC20(self, "その場足踏み上半身", 0);
            }
        }
    }
}

extern "C" void fn_803A1C90(Mario* self) {
    TVec3f sp8;
    PSVECCrossProduct(&FLD<TVec3f>(self, 0x214), &FLD<TVec3f>(self, 0x22C), &sp8);
    if (sp8.dot(FLD<TVec3f>(self, 0x3D8)) < 0.0f) {
        if (!fn_803A19C0(self)) {
            if (!(FLD<TVec3f>(self, 0x214).dot(FLD<TVec3f>(self, 0x22C)) < 0.0f)) {
                FLD<u16>(self, 0x3D4) = 0;
            }
        }
    } else {
        u16 v = FLD<u16>(self, 0x3D4);
        if (v < FLD<u16>(marioCurTableEntry(self), 0x2E6)) {
            FLD<u16>(self, 0x3D4) = v + 1;
            fn_803A1D60(self);
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

extern "C" bool fn_803A1920(Mario* self) {
    TVec3f s;
    self->mActor->getStickValue(&s.x, &s.y);
    s.z = 0.0f;
    bool eq0 = marioEpsEq(FLD<f32>(self, 0x250), s.x, 0.0001f);
    bool eq1 = eq0 && marioEpsEq(FLD<f32>(self, 0x254), s.y, 0.0001f);
    bool eq2 = marioEpsEq(FLD<f32>(self, 0x258), s.z, 0.0001f);
    return eq1 && !eq2;
}

extern "C" void fn_803A20E0(Mario* self, s32 arg) {
    if (FLD<u16>(self, 0x3D0) == 0) {
        TVec3f sp8;
        if (!MR::vecBlendSphere(&sp8, FLD<TVec3f>(self, 0x214), FLD<TVec3f>(self, 0x22C), 0.0f)) {
            MR::vecRotAxis(FLD<TVec3f>(self, 0x214), FLD<TVec3f>(self, 0x22C), FLD<TVec3f>(self, 0x1F0), &sp8, 0.3926991f);
            if (arg == 0) {
                self->setFrontVecKeepUp(sp8);
            }
        }
    }
    FLD<u32>(self, 0x14) &= 0xFFF7FFFF;
    FLD<u32>(self, 0xC) &= 0xEFFFFFFF;
}

extern "C" void fn_803A1BB0(Mario* self) {
    if (FLD<u16>(self, 0x3D0) == 0) {
        return;
    }
    FLD<u32>(self, 0x14) &= 0xFFF7FFFF;
    FLD<u32>(self, 0xC) &= 0xEFFFFFFF;
    f32 d = MR::diffAngleAbs(FLD<TVec3f>(self, 0x214), FLD<TVec3f>(self, 0x22C));
    if (MR::isInRange(d, 0.0f, 0.707f)) {
        TVec3f s;
        PSVECCrossProduct(&FLD<TVec3f>(self, 0x214), &FLD<TVec3f>(self, 0x22C), &s);
        if (FLD<u8>(self, 0x274) == 0) {
            if (s.dot(FLD<TVec3f>(self, 0x1F0)) < 0.0f) {
                FLD<u32>(self, 0x14) &= 0xFFF7FFFF;
            }
        } else {
            if (s.dot(FLD<TVec3f>(self, 0x1F0)) > 0.0f) {
                FLD<u32>(self, 0x14) &= 0xFFF7FFFF;
            }
        }
    }
}

extern "C" void fn_803A1240(Mario* self, f32* a, f32* b) {
    void* e = marioCurTableEntry(self);
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

extern "C" void fn_803A1AC0(Mario* self);
extern "C" void fn_803A1E40(Mario* self);
extern "C" void fn_803A22D0(Mario* self, f32 x, s32 arg);

extern "C" s32 fn_803A1E60(Mario* self, f32 f) {
    void* e = marioCurTableEntry(self);
    f32 v = JMACosRadian(FLD<f32>(e, 0x2C0));
    s32 r31 = 0;
    if (f < v && !((FLD<u32>(self, 0xC) >> 26) & 1)) {
        if (FLD<f32>(self, 0x278) > FLD<f32>(e, 0x2C8)) {
            if (FLD<f32>(self, 0x3C) > FLD<f32>(self, 0x1C8)) {
                if (!((FLD<u32>(self, 0x1C) >> 18) & 1)) {
                    r31 = 1;
                }
            }
        }
    }
    return r31;
}

// NONMATCHING: branch shape of the 0x3D4 == 0 block approximated
extern "C" void fn_803A1FF0(Mario* self, f32 x, s32 arg) {
    if ((FLD<u32>(self, 0x10) >> 28) & 1) {
        fn_803A1AC0(self);
        fn_803A1C90(self);
        x *= fn_803A1DB0(self);
    } else {
        fn_803A1E40(self);
        if (FLD<u16>(self, 0x3D4) == 0) {
            FLD<TVec3f>(self, 0x244).zero();
            PSVECCrossProduct(&FLD<TVec3f>(self, 0x214), &FLD<TVec3f>(self, 0x22C), &FLD<TVec3f>(self, 0x3D8));
            x *= 0.01f;
            FLD<TVec3f>(self, 0x238) = FLD<TVec3f>(self, 0x22C);
            f32 k = 0.3f;
            if ((FLD<u32>(self, 0x1C) >> 18) & 1) {
                if (fn_803A2270(self)) {
                    k = 0.1f;
                }
            }
            x *= k;
        }
    }
    fn_803A22D0(self, x, arg);
}

extern "C" void fn_803A2180(Mario* self) {
    if (FLD<u32>(self, 0x748) != 0 && FLD<u8>(self, 0x714) == 0 && self->isEnableTurn()) {
        self->changeAnimation("その場足踏み", (const char*)nullptr);
        if (!self->isAnimationRun("カリカリ限界") && !self->isPlayerModeYoshi()) {
            fn_803EEC20(self, "その場足踏み上半身", 0);
        }
    } else {
        if (FLD<u8>(self, 0x714) != 0) {
            self->stopAnimation("その場足踏み", (const char*)nullptr);
            if (self->isAnimationRun("その場足踏み上半身")) {
                self->stopAnimationUpper((const char*)nullptr, (const char*)nullptr);
            }
        }
    }
}

extern "C" void fn_803A2570(Mario* self, f32 x, TVec3f* arg);
extern "C" void fn_803A2690(Mario* self, f32* p);
extern "C" void fn_803A2A70(Mario* self);
extern "C" void* fn_803B97F0(MarioActor*);
extern "C" void fn_803A28D0(Mario* self, TVec3f* arg);

extern "C" void fn_803A22D0(Mario* self, f32 x, s32 arg) {
    TVec3f sp8;
    if ((FLD<u32>(self, 0x14) >> 12) & 1) {
        fn_803A2420(self);
        if ((FLD<u32>(self, 0x10) >> 8) & 1) {
            fn_803A23F0(self, &sp8);
            fn_803A2420(self);
            fn_803A2450(self, &sp8);
        }
    } else if ((FLD<u32>(self, 0x14) >> 13) & 1) {
        fn_803A24C0(self, &sp8);
    } else {
        f32 d = MR::diffAngleAbs(FLD<TVec3f>(self, 0x208), FLD<TVec3f>(self, 0x22C));
        if (d < 0.1f) {
            fn_803A2530(self, &sp8);
        } else {
            fn_803A2570(self, x, &sp8);
            fn_803A2720(self);
            fn_803A27A0(self, &sp8);
            fn_803A2860(self);
        }
    }
    if (arg == 0) {
        self->setFrontVecKeepUp(sp8);
    }
}

extern "C" void fn_803A2570(Mario* self, f32 x, TVec3f* arg) {
    f32 d = MR::diffAngleAbs(FLD<TVec3f>(self, 0x208), FLD<TVec3f>(self, 0x22C));
    f32 s = MR::clamp(x / d, 0.0f, 1.0f);
    fn_803A2690(self, &s);
    if ((FLD<u32>(self->getPlayer(), 0x14) >> 13) & 1) {
        s *= fn_803A26C0(self, d);
    }
    if (self->isPlayerModeYoshi()) {
        if (FLD<s32>(fn_803B97F0(self->mActor), 0x134) > 0) {
            s *= 0.5f;
        }
    }
    if (!MR::vecBlendSphere(arg, FLD<TVec3f>(self, 0x208), FLD<TVec3f>(self, 0x22C), s)) {
        FLD<u16>(self, 0x3D4) = 0;
        MR::vecRotAxis(FLD<TVec3f>(self, 0x214), FLD<TVec3f>(self, 0x22C), FLD<TVec3f>(self, 0x1F0), arg, 0.3926991f);
    }
}

// NONMATCHING: the wall-normal slide tail is approximated from the asm
extern "C" void fn_803A28D0(Mario* self, TVec3f* arg) {
    fn_803A2A70(self);
    fn_803A2C40(self, arg);
    if ((FLD<u32>(self, 0x10) >> 12) & 1) {
        if (FLD<f32>(self, 0x4DC) > 120.0f) {
            const TVec3f* n = FLD<Triangle*>(self, 0x4C8)->getNormal(0);
            if (FLD<TVec3f>(self, 0x160).dot(*n) < 0.0f) {
                TVec3f sp20(self->mPosHolder->mPos);
                sp20 -= FLD<TVec3f>(self, 0x4E4);
                TVec3f sp56;
                MR::vecKillElement(sp20, *n, &sp56);
                MR::vecKillElement(sp56, *self->getGravityVec(), &sp56);
                MR::vecKillElement(sp56, FLD<TVec3f>(self, 0x208), &sp56);
                MR::normalizeOrZero(&sp56);
                if (!MR::isNearZero(sp56, 0.001f)) {
                    f32 m = PSVECMag(&FLD<TVec3f>(self, 0x160));
                    TVec3f a(sp56);
                    a.scale(m);
                    FLD<TVec3f>(self, 0x160) = a;
                    TVec3f sp44(FLD<TVec3f>(self, 0x160));
                    f32 m2 = PSVECMag(&sp44);
                    MR::normalizeOrZero(&sp44);
                    TVec3f sp32;
                    PSVECCrossProduct(&FLD<TVec3f>(self, 0x368), &sp44, &sp32);
                    MR::normalizeOrZero(&sp32);
                    PSVECCrossProduct(&sp32, &FLD<TVec3f>(self, 0x368), &sp44);
                    sp44.setLength(m2);
                    FLD<TVec3f>(self, 0x160) = sp44;
                }
            }
        }
    }
}

extern "C" void fn_803A1BB0(Mario* self);
extern "C" void fn_803A1A20(Mario* self, f32* out);
extern "C" bool fn_803B93E0(MarioActor*);

// NONMATCHING: the nested gate structure is approximated from the asm
extern "C" void fn_803A1AC0(Mario* self) {
    if (MR::isNearZero(FLD<f32>(self, 0x1C8), 0.001f)) {
        FLD<u32>(self, 0x14) &= 0xFFF7FFFF;
        FLD<u32>(self, 0xC) &= 0xEFFFFFFF;
        return;
    }
    if (!MR::isNearZero(FLD<TVec3f>(self, 0x244), 0.001f)) {
        if (!((FLD<u32>(self, 0x10) >> 24) & 1) && fn_803A1920(self)) {
            FLD<u32>(self, 0xC) |= 0x10000000;
        }
    } else {
        fn_803A1BB0(self);
    }
    fn_803A1A20(self, &FLD<f32>(self, 0x250));
}

// NONMATCHING: the second gate and the press-normal copy are approximated
extern "C" void fn_8039ACE0(Mario* self) {
    if (self->calcDistToCeil(false) < 80.0f) {
        return;
    }
    bool a = false;
    if (FLD<void*>(self, 0x728) != nullptr && MR::isSensorPressObj(reinterpret_cast<const HitSensor*>(FLD<void*>(self, 0x728)))) {
        a = true;
    }
    bool b = false;
    Triangle* g = FLD<Triangle*>(self, 0x460);
    void* gs = FLD<void*>(g, 8);
    if (gs != nullptr && MR::isSensorPressObj(reinterpret_cast<const HitSensor*>(gs))) {
        b = true;
    }
    if (FLD<void*>(self, 0x728) == gs) {
        return;
    }
    if (!a && b) {
        self->mActor->setPress(0, 0);
        FLD<TVec3f>(self->mActor, 0x6D8) = *g->getNormal(0);
    }
}

// NONMATCHING: gravity scaling chain approximated from the asm
extern "C" f32 fn_8039AA40(Mario* self) {
    if (self->getPlayerMode() == 6) {
        return 0.0f;
    }
    f32 f31 = 1.0f;
    if (FLD<u32>(self, 0x42C) == 13) {
        if (!((FLD<u32>(self, 0xC) >> 17) & 1) && !self->isRising()) {
            f31 = 0.0f;
            return f31;
        }
    }
    if (fn_8039A100(self)) {
        f31 = FLD<f32>(marioCurTableEntry(self), 0x58);
        f31 *= self->mActor->getGravityRatio();
        if (FLD<u16>(self->mActor, 0x66C) != 0) {
            f31 *= 0.3f;
        }
        if (!fn_803B93E0(self->mActor) && FLD<f32>(self, 0x484) < 300.0f) {
            if (FLD<f32>(self, 0x484) < 200.0f) {
                f31 = 0.0f;
            }
        }
        self->cutGravityElementFromJumpVec(true);
        f31 = -0.1f;
    }
    return f31;
}

// ---- rough bodies (NONMATCHING) for the remaining range-A functions ----
extern "C" s32 fn_803A0BE0(...);
extern "C" s32 fn_803A7540(...);
extern "C" s32 fn_803B0E60(...);
extern "C" s32 fn_803B3CE0(...);
extern "C" s32 fn_803B4280(...);
extern "C" s32 fn_803B9810(...);
extern "C" void fn_803B9830(MarioActor*);
extern "C" s32 fn_803B9840(...);
extern "C" void fn_803B9A30(MarioActor*);
extern "C" void fn_8041BB90();
extern "C" s32 fn_8041BBB0(...);
extern "C" s32 fn_8041DF40(...);
extern "C" s32 fn_8041F8A0(...);

extern "C" bool fn_8038AA30(Mario* self);
extern "C" void fn_803EEA20(Mario* self, s32);
extern "C" void* fn_8005BC60(s32);
extern "C" void fn_80085D30(void*, s32);
extern "C" void fn_80085D40();
extern "C" void fn_803B2950(Mario*);
extern "C" f32 fn_803B57A0();
extern "C" bool fn_804225C0();
extern "C" f32 fn_803DF0E0();

// NONMATCHING: follows the m2c call sequence; the isNearZero/identity branches are approximated
extern "C" s32 fn_8038A8A0(Mario* self, f32 (*arg1)[4], TVec3f* arg2) {
    *arg2 = TVec3f(0.0f, 0.0f, 0.0f);
    if (self->isNonFixHeadVec()) {
        PSMTXIdentity(arg1);
    }
    if (MR::isNearZero(FLD<f32>(self, 0x74C), 0.001f) && FLD<s32>(self, 0x754) == 0) {
        PSMTXIdentity(arg1);
        FLD<s32>(self, 0x750) = 0;
        FLD<f32>(self, 0x74C) = 0.0f;
    }
    if (FLD<s32>(self, 0x750) == 0 && FLD<s32>(self, 0x754) == 0) {
        fn_8038A9D0(self);
    }
    TVec3f sp8(0.0f, 1.0f, 0.0f);
    if (fn_8038AA30(self)) {
        PSMTXRotAxisRad(arg1, &sp8, -FLD<f32>(self, 0x74C));
    } else {
        PSMTXRotAxisRad(arg1, &sp8, FLD<f32>(self, 0x74C));
    }
    fn_8038AB10(self);
    fn_8038AB70(self);
    return 0;
}

// NONMATCHING: branch structure of the two sign tests is approximated
extern "C" bool fn_8038AA30(Mario* self) {
    f32 d = self->mFrontVec.dot(self->getCamDirX());
    if (FLD<TVec3f>(self, 0x1FC).dot(self->getCamDirY()) < 0.0f) {
        d = -d;
    }
    f32 t = FLD<f32>(self, 0x74C);
    return (d < 0.0f && t < 0.0f) || (d > 0.0f && t > 0.0f);
}

// NONMATCHING: sound-slot bookkeeping approximated from the m2c draft
extern "C" void fn_8038C540(Mario* self) {
    if (FLD<u32>(self, 0xA92) != 0) {
        void* p = FLD<void*>(self, 0xA94);
        if (p == nullptr || FLD<u8>(p, 9) == 0) {
            FLD<void*>(self, 0xA94) = fn_8005BC60(0x258);
        }
        p = FLD<void*>(self, 0xA94);
        fn_80085D30(p, 1);
        s32 v = MR::clamp(0x1A4 - FLD<s32>(p, 0), 0, 0x1A4);
        if (FLD<s32>(p, 0) % 74 == 5) {
            MR::startActionSound(self->mActor, "LifeAlertTrg", v, -1, -1);
        }
    } else {
        void* p = FLD<void*>(self, 0xA94);
        if (p != nullptr && FLD<u8>(p, 9) != 0) {
            fn_80085D40();
        }
    }
}

// NONMATCHING: only the animation-run gate chain is recovered; the flags are approximated
extern "C" bool fn_8038CCF0(Mario* self) {
    if (self->isPlayerModeTeresa()) {
    }
    if (self->isAnimationRun("着地") || self->isAnimationRun("着地B") || self->isAnimationRun("着地C") ||
        self->isAnimationRun("着地ターン") || self->isAnimationRun("着地幅とび")) {
    }
    if (self->isAnimationRun("すべり着地")) {
    }
    if (self->isAnimationRun(FLD<const char*>(self, 0x728))) {
    }
    if (self->isAnimationRun("ハード着地")) {
    }
    if (self->isAnimationRun("逆着地")) {
    }
    if (self->isAnimationRun("ショート着地")) {
    }
    return MR::isBckPlaying(self->mActor, "pulloutgraspend");
}

// NONMATCHING: timer and gate logic approximated
extern "C" void fn_8039A1C0(Mario* self) {
    if (self->isAnimationRun("ジャンプB")) {
        self->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
    if (FLD<u8>(FLD<void*>(self, 0x888), 0x16E) == 0) {
        if (self->isAnimationRun("水泳スピン移動")) {
            self->stopAnimation((const char*)nullptr, (const char*)nullptr);
        }
        if (self->isAnimationRun("水泳スピンジャンプ")) {
            self->stopAnimation((const char*)nullptr, (const char*)nullptr);
        }
        if (self->isAnimationRun("水泳ジェット")) {
            self->changeAnimation("空中一回転", (const char*)nullptr);
        }
    }
    FLD<u16>(self, 0x3BE) += 1;
}

// NONMATCHING: the target-speed and sound branches are approximated
extern "C" void fn_803A0ED0(Mario* self, TVec3f* arg1, s32 arg2) {
    if (fn_803A0FF0(self)) {
        *arg1 = FLD<TVec3f>(self, 0x328);
    }
    self->calcMoveDir(self->mStickPos.x, FLD<f32>(self, 0x1C4), arg1, true);
    self->calcShadowDir(*arg1, &FLD<TVec3f>(self, 0x22C));
    self->calcShadowDir(self->mFrontVec, &FLD<TVec3f>(self, 0x214));
    if (fn_803A1050(self)) {
        fn_803A10D0(self, arg2);
    }
    if (arg2 == 0) {
        self->setFrontVecKeepUp(FLD<TVec3f>(self, 0x22C));
    }
    if (self->isAnimationRun("食べる") || self->isAnimationRun("噛む") || self->isAnimationRun("飲み下す")) {
        self->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
}

// NONMATCHING: table lookups follow the draft; the speed-band selection is approximated
extern "C" f32 fn_803A1800(Mario* self) {
    void* e = marioCurTableEntry(self);
    if ((FLD<u32>(self, 0xC) >> 16) & 1) {
        return FLD<f32>(e, 0x3D4);
    }
    if ((FLD<u32>(self, 0x10) >> 28) & 1) {
        return FLD<f32>(e, 0x264);
    }
    if (FLD<f32>(self, 0x278) > FLD<f32>(e, 0x2D0)) {
        if (self->isStatusActive(0x1F)) {
            return 0.5f * FLD<f32>(e, 0x260);
        }
        if (FLD<u8>(self, 0x714) < 5) {
            return FLD<f32>(e, 0x268);
        }
        return FLD<f32>(e, 0x260);
    }
    return FLD<f32>(e, 0x25C);
}

// NONMATCHING: the reset/timer chain is approximated from the draft
extern "C" void fn_8039A480(Mario* self) {
    if (FLD<u32>(self, 0x42C) == 8) {
        if (self->checkTrgZ()) {
            self->resetTornado();
            self->cancelTornadoJump();
            self->stopAnimation((const char*)nullptr, (const char*)nullptr);
            fn_803EEA20(self, 1);
            FLD<u32>(self, 0x10) |= 0x800000;
            self->playSound("声小ジャンプ", -1);
            if (FLD<u16>(self, 0x42A) != 0) {
                self->playEffect("スピンライト消去");
            }
        } else {
            FLD<u32>(self, 0xC) &= 0xFFFFBFFF;
        }
        if (FLD<u16>(self, 0x42A) != 0) {
            u16 t = FLD<u16>(self, 0x402);
            if (t != 0) {
                FLD<u16>(self, 0x402) = t - 1;
            }
            if (FLD<u16>(self, 0x402) == 0) {
                self->cancelTornadoJump();
                self->playEffect("スピンライト消去");
                if (self->getPlayerMode() != 4) {
                    FLD<u16>(self, 0x402) = FLD<u16>(marioCurTableEntry(self), 0xD8);
                }
            }
        }
    }
}

// NONMATCHING: rough, follows the m2c call sequence (turn-by-normal bookkeeping)
extern "C" s32 fn_803882A0(Mario* self) {
    s32 var_r31 = 0;
    if (self->isRising()) {
        var_r31 = 1;
    }
    if (FLD<s32>(self->mActor, 0x8C) != 0) {
        FLD<s32>(self, 0x570) = 0;
    }
    if (!((FLD<u32>(self, 0x10) >> 29) & 1)) {
        u32 n = FLD<u32>(self, 0x570);
        for (u32 i = 0; i < n; i++) {
            Triangle* tri = FLD<Triangle*>(self, 0x574 + i * 4);
            TVec3f sp68;
            if (MR::vecKillElement(FLD<TVec3f>(self, 0x160), *tri->getNormal(0), &sp68) < 0.9f) {
                FLD<TVec3f>(self, 0x160) = sp68;
                if (!((FLD<u32>(self, 0x20) >> 18) & 1)) {
                    TVec3f sp5C;
                    f32 t = MR::vecKillElement(FLD<TVec3f>(self, 0x2D4), *tri->getNormal(0), &sp5C);
                    if (t == 0.9f) {
                        TVec3f sp20(*tri->getNormal(0));
                        sp20 *= t;
                        TVec3f sp2C(sp20);
                        sp2C *= 0.5f;
                        TVec3f sp38(sp5C);
                        sp38 += sp2C;
                        FLD<TVec3f>(self, 0x2D4) = sp38;
                    }
                }
                if (var_r31 != 0) {
                    if (tri->getNormal(0)->dot(*self->getGravityVec()) > 0.707f) {
                        self->mActor->sendMsgUpperPunch(nullptr);
                    }
                }
            }
        }
    }
    return var_r31;
}

// NONMATCHING: 0x274 bookkeeping approximated
extern "C" void fn_803886B0(Mario* self) {
    if (!((FLD<u32>(self, 0x1C) >> 19) & 1)) {
        TVec3f* a = &FLD<TVec3f>(self->mActor, 0x5DC);
        f32 d = a->dot(FLD<TVec3f>(self, 0x368));
        if (MR::isNearZero(*a, d)) {
        }
        TVec3f sp38(*a);
        MR::normalizeOrZero(&sp38);
        TVec3f sp2C;
        f32 t1 = MR::vecKillElement(FLD<TVec3f>(self, 0x160), sp38, &sp2C);
        if (t1 != 0.0f) {
            TVec3f n(FLD<TVec3f>(self, 0x160));
            MR::normalizeOrZero(&n);
            f32 dd = n.dot(self->mFrontVec);
            f32 w = 1.0f - MR::clamp(1.4285715f * (-dd - 0.3f), 0.0f, 1.0f);
            if (!((FLD<u32>(self, 0xC) >> 23) & 1) && w < 0.1f) {
                if ((FLD<u32>(self, 0xC) >> 5) & 1) {
                    TVec3f sp14(self->getSideWallNorm());
                    sp14 *= -t1;
                    FLD<TVec3f>(self, 0x160) += sp14;
                } else {
                }
            }
            self->mTargetWalkSpeedIndex = 0;
            FLD<f32>(self, 0x278) *= w;
        }
    }
}

// NONMATCHING: gate chain from the draft, state transitions approximated
extern "C" void fn_80388DB0(Mario* self) {
    if (!((FLD<u32>(self, 0xC) >> 9) & 1)) {
        if (self->checkDamage() == false) {
            if (self->isPlayerModeYoshi()) {
                fn_803B9810(self->mActor);
                fn_8041DF40();
            }
            if (!self->isPlayerModeYoshi() || (fn_803B9810(self->mActor), fn_8041F8A0() == 0)) {
                if (FLD<void*>(self, 0x97C) != nullptr) {
                    self->sendStateMsg(2);
                } else if ((FLD<u32>(self, 0xC) >> 31) != 0) {
                    self->procJump(false);
                    self->checkTornado();
                    self->checkHang();
                    self->checkWallStick();
                    fn_803B0E60(self);
                } else {
                    fn_803A0BE0(self);
                    self->updateWalkSpeed();
                    fn_803B0E60(self);
                }
            }
            fn_803B3CE0(self);
            if ((FLD<u32>(self, 0xC) >> 30) & 1) {
                FLD<u32>(self, 0x10) |= 0xC000;
                self->doFrontStep();
            }
            if (self->isStatusActive(0x28)) {
                fn_803B9A30(self->mActor);
                FLD<u32>(self, 0x14) &= 0xFFDFFFFF;
                fn_803B9830(self->mActor);
                fn_8041BB90();
            }
            u32 t = FLD<u32>(self, 0xC);
            if (((t >> 30) & 1) || !((t >> 12) & 1)) {
                FLD<u32>(self, 0x14) &= 0xFFEFFFFF;
            }
        }
        fn_803A7540(self);
        if (self->getPlayerMode() != 6 && ((FLD<u32>(self, 0xC) >> 23) & 1) && FLD<Triangle*>(self, 0x4C8)->isValid() &&
            FLD<Triangle*>(self, 0x4C8)->getNormal(0)->dot(FLD<TVec3f>(self, 0x160)) < 0.0f) {
            MR::vecKillElement(FLD<TVec3f>(self, 0x160), *FLD<Triangle*>(self, 0x4C8)->getNormal(0), &FLD<TVec3f>(self, 0x160));
        }
        if (fn_803B9840(self->mActor) != 0) {
            fn_803B9830(self->mActor);
            fn_8041BBB0();
        }
        if (self->isStickOn()) {
            if (self->isAnimationRun("ブレーキ")) {
                self->stopAnimation((const char*)nullptr, (const char*)nullptr);
            }
        }
        if (!((FLD<u32>(self, 0x1C) >> 27) & 1)) {
            FLD<u16>(self, 0x3FE) = 0;
        }
        FLD<u32>(self, 0x5F4) = FLD<u32>(self, 0x5F8);
        self->callExtraTasks(0);
        self->beeMarioOnGround();
        self->checkSpecialWaitAnimation();
        fn_803B4280(self);
        if (!((FLD<u32>(self, 0x1C) >> 9) & 1)) {
            self->retainMoveDir(0.0f, 0.0f, nullptr);
        }
    }
}

// NONMATCHING: the two-stage neighbourhood test is approximated (returns the matched-triangle flag)
