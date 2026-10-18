// SMG2-only Mario:: functions (fn_ range A), written from the asm. Included at the end of Mario.cpp (unity TU).
// FLD<T>(self, off): typed access to a Mario field by its SMG2 byte offset (used where the header name is unknown).
template <typename T, typename P>
inline T& FLD(P* p, u32 off) { return *reinterpret_cast<T*>(reinterpret_cast<u8*>(const_cast<void*>(static_cast<const void*>(p))) + off); }

extern "C" void fn_80406040(MarioPosHolder*);
extern "C" void fn_80406080(MarioPosHolder*);
extern "C" void fn_803F7C20(void*, const TVec3f*);
extern "C" void* fn_803EF670(const void*);
extern "C" s32 fn_80056780();
extern "C" void fn_803B3010();
extern "C" s32 fn_803B3040(void*, void*, void*);
extern "C" void fn_803B3280(void*, void*, void*);
extern "C" void fn_803B94E0(void*);
extern "C" void fn_803B9870(void*);
extern "C" void fn_803BE840(void*, const TVec3f*);

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
    fn_803B3010();
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
    if (self->isAnimationRun("" /* TODO string lbl_806C3BBC */) != 0) {
    }
    if (self->isAnimationRun("" /* TODO string lbl_806C3BC8 */) != 0) {
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
    self->playSound("" /* TODO string lbl_806C4484 */, -1);
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

extern "C" void* fn_803BE0E0(void*);
extern "C" void* fn_803BE0D0(void*);
extern "C" void* fn_803BE0C0(void*);

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
    if (d >= 0.0f && !self->isAnimationRun("" /* TODO string lbl_806C43DC */)) {
        FLD<f32>(self, 0x744) = 3.1415927f;
        FLD<u32>(self, 0x748) = 10;
    }
}

extern "C" void fn_803A2860(Mario* self) {
    if (!self->isAnimationRun("" /* TODO string lbl_806C4414 */)) {
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
            self->playSound("" /* TODO string lbl_806C3ADC */, -1);
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
