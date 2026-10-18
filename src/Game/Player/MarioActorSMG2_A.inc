#include "Game/Util/PlayerUtil.hpp"
// SMG2-only MarioActor functions (part A), written from the asm. Included at the end of MarioActor.cpp (unity TU).
// Field access by raw offset (`F(T, off)`): the class header offsets are partly guesses.
#define F(T, off) (*(T*)((u8*)self + (off)))
#define FP(p, T, off) (*(T*)((u8*)(p) + (off)))

// generic object with N virtual slots (CW vtable: slot i at 8+4*i)
struct SMG2VObj {
    virtual void v0(); virtual void v1(); virtual void v2(); virtual void v3();
    virtual void v4(); virtual void v5(); virtual void v6(); virtual void v7();
    virtual void v8(); virtual void v9(); virtual void v10(); virtual void v11();
    virtual void v12(); virtual void v13(); virtual void v14(); virtual void v15();
    virtual void v16(); virtual void v17(); virtual void v18(); virtual void v19();
};

extern "C" void setPlayerMode__10MarioActorFUsbb(MarioActor*, u16, bool, bool);
extern "C" {
extern u32 lbl_807D4E50;
void fn_80393C40(Mario*);
void fn_8041B6F0(void*);
void fn_8040CF20(void*);
void fn_8041BAE0(void*);
void fn_8041BAF0(void*);
void fn_8041BB00(void*);
s32 fn_8041BB30(void*);
s32 fn_8041BB50(void*);
void fn_8041BBF0(void*, s32, s32, s32);
s32 fn_8041BD40();
void fn_8041FF70(void*);
s32 fn_803DCB80(void*);
void fn_803DF0F0(void*);
void fn_803DF1B0(void*);
void fn_803DF2D0(void*);
s32 fn_803CEEE0(MarioActor*, HitSensor*);
void fn_803D5A00(MarioActor*, u32);
}

extern "C" void fn_803B83F0(MarioActor* self) {
    if (MR::isFirstStep(self)) {
        MR::setCubeBgmChangeInvalid();
        MR::clearBgmQueue();
        if (MR::getPlayerLeft() == 0) {
            MR::startPlayerEvent("ゲームオーバー");
        } else {
            MR::startPlayerEvent("演出なし死");
            F(s32, 0x6B0) = 0;
        }
        MR::startStarPointerModeDemoMarioDeath(self);
    }
}

extern "C" const TVec3f& fn_803B8520(MarioActor* self) {
    return self->mMario->getAirGravityVec();
}

extern "C" u32 fn_803B89E0(MarioActor* self) {
    return (FP(self->mMario, u32, 0xC) >> 0x14) & 1;
}

extern "C" void fn_803B8A80(MarioActor* self) {
    fn_80393C40(self->mMario);
}

extern "C" s32 fn_803B8DE0(MarioActor* self) {
    return F(s32, 0x69C);
}

extern "C" u8* fn_803B8FE0(MarioActor* self) {
    return (u8*)self + 0x5B8;
}

extern "C" u8* fn_803B9020(MarioActor* self) {
    return (u8*)self + 0x63C;
}

extern "C" void fn_803B9270(MarioActor* self) {
    self->decLife(0);
}

extern "C" bool fn_803B93E0(MarioActor* self) {
    return MR::isNearZero(F(TVec3f, 0x5A0), 0.001f);
}

extern "C" void fn_803B94E0(MarioActor* self) {
    Mario* m = self->mMario;
    if (FP(m, u8, 0xA7B) == 0) {
        if ((FP(m, u32, 0xC) >> 0x10) & 1) {
            m->forceStopTornado();
        }
        if (FP(self, MarioActor*, 0x1080)->isActionOk("ダメージ解除")) {
            setPlayerMode__10MarioActorFUsbb(self, 0, true, true);
        }
    }
}

extern "C" void fn_803B9560(MarioActor* self) {
    if (FP(self, MarioActor*, 0x1080)->isActionOk("ノーダメージ解除")) {
        setPlayerMode__10MarioActorFUsbb(self, 0, true, true);
    }
}

extern "C" s32 fn_803B97F0(MarioActor* self) {
    return *F(s32*, 0xEE4);
}

extern "C" void fn_803B9800(MarioActor* self) {
    fn_8041B6F0(F(void*, 0xEE4));
}

extern "C" s32 fn_803B9810(MarioActor* self) {
    return FP(F(void*, 0xEE4), s32, 8);
}

extern "C" s32 fn_803B9820(MarioActor* self) {
    return FP(F(void*, 0xEE4), s32, 0xC);
}

extern "C" void* fn_803B9830(MarioActor* self) {
    return F(void*, 0xEE4);
}

extern "C" bool fn_803B9840(MarioActor* self) {
    return F(void*, 0xEE4) != nullptr;
}

extern "C" void fn_803B9850(MarioActor* self) {
    (*F(SMG2VObj**, 0xEE4))->v14();
}

extern "C" void fn_803B9870(MarioActor* self) {
    if (self->mMario->isPlayerModeYoshi()) {
        fn_8040CF20(*F(void**, 0xEE4));
    }
}

extern "C" void fn_803B98C0(MarioActor* self) {
    if (F(void*, 0xEE4) != nullptr) {
        FP(F(void*, 0xEE4), u8, 0x3C) = 1;
    }
}

extern "C" void fn_803B98E0(MarioActor* self) {
    if (F(void*, 0xEE4) != nullptr) {
        FP(F(void*, 0xEE4), u8, 0x3C) = 0;
    }
}

extern "C" u8 fn_803B9900(MarioActor* self) {
    void* p = F(void*, 0xEE4);
    if (p != nullptr) {
        return FP(p, u8, 0x3C);
    }
    return 0;
}

extern "C" bool fn_803B9920(MarioActor* self) {
    if (!self->mMario->isPlayerModeYoshi()) {
        return false;
    }
    if (F(u8, 0xE24) != 0 || F(u16, 0x6B8) != 0) {
        return false;
    }
    if (F(u8, 0xE2C) != 0) {
        return false;
    }
    Mario* m = self->mMario;
    u32 r = FP(m, u32, 0x18);
    if ((r >> 0x14) & 1) {
        return false;
    }
    if ((r >> 0x15) & 1) {
        return false;
    }
    if ((FP(m, u32, 0xC) >> 0x1F) == 1 && !m->isRising() && ((FP(self->mMario, u32, 0xC) >> 0xC) & 1)) {
        return true;
    }
    return self->checkButtonType(FP(F(void*, 0x107C), u16, 8), false);
}

extern "C" void fn_803B9A10(MarioActor* self) {
    if (*(void* volatile*)((u8*)self + 0xEE4) != nullptr) {
        fn_8041BAE0(F(void*, 0xEE4));
    }
}

extern "C" void fn_803B9A30(MarioActor* self) {
    if (*(void* volatile*)((u8*)self + 0xEE4) != nullptr) {
        fn_8041BAF0(F(void*, 0xEE4));
    }
}

extern "C" void fn_803B9A50(MarioActor* self) {
    if (*(void* volatile*)((u8*)self + 0xEE4) != nullptr) {
        fn_8041BB00(F(void*, 0xEE4));
    }
}

extern "C" s32 fn_803B9A70(MarioActor* self) {
    void* p = F(void*, 0xEE4);
    if (p != nullptr) {
        return fn_8041BB30(p);
    }
    return 0;
}

extern "C" s32 fn_803B9A90(MarioActor* self) {
    void* p = F(void*, 0xEE4);
    if (p != nullptr) {
        return fn_8041BB50(p);
    }
    return 0;
}

extern "C" bool fn_803B9AB0(MarioActor* self, LiveActor* a) {
    void* p = F(void*, 0xEE4);
    if (p == nullptr) {
        return false;
    }
    if (FP(p, LiveActor*, 0x18) == nullptr) {
        return false;
    }
    return !MR::isDead(FP(p, LiveActor*, 0x18));
}

extern "C" void fn_803B9F10(MarioActor* self) {
    self->tryReleaseWithMsg(0x2F);
    self->updateSearchLight();
    if (F(HitSensor*, 0x70C) != nullptr) {
        F(HitSensor*, 0x70C)->receiveMessage(0x2F, self->getSensor("Body"));
        F(HitSensor*, 0x70C) = nullptr;
    }
}

extern "C" void fn_803BA070(MarioActor* self) {
    if (F(s32, 0x750) == 0) {
        if (self->mMario->isAnimationRun("ひろいクイック")) {
            self->mMario->stopAnimationUpperForce();
            F(s8, 0xFBA) = 0;
            F(SMG2VObj*, 0xFB4)->v11();
            F(u8, 0x768) = 0;
        }
    } else {
        for (u32 i = 0; i < (u32)F(s32, 0x750); i++) {
            if (fn_803CEEE0(self, FP(self, HitSensor*, 0x710 + i * 4))) {
                FP(self, HitSensor*, 0x710 + i * 4)->receiveMessage(0x2D, self->getSensor("Body"));
                FP(self, HitSensor*, 0x710 + i * 4) = nullptr;
                F(s32, 0x750) -= 1;
            } else if (FP(self, HitSensor*, 0x710 + i * 4) != nullptr) {
                fn_803D5A00(self, i);
            }
        }
        F(s32, 0x75C) = 0;
        if (F(s32, 0x750) == 0) {
            self->mMario->stopAnimationUpperForce();
            FP(self->mMario, MarioSwim*, 0x888)->resetJet();
        }
        if (F(u8, 0x768) != 0) {
            self->mMario->stopAnimationUpperForce();
            F(u8, 0x768) = 0;
        }
        F(s8, 0xFBA) = 0;
        F(SMG2VObj*, 0xFB4)->v11();
        self->tryReleaseWithMsg(0x2D);
        self->updateSearchLight();
        if (F(HitSensor*, 0x70C) != nullptr) {
            F(HitSensor*, 0x70C)->receiveMessage(0x2C, self->getSensor("Body"));
            F(HitSensor*, 0x70C) = nullptr;
        }
    }
}

extern "C" void fn_803BA220(MarioActor* self) {
    F(u8, 0x768) = 0;
}

extern "C" void fn_803BAFE0(MarioActor* self) {
    if (self->isEnableNerveChange()) {
        self->setNerve(reinterpret_cast<const Nerve*>(&lbl_807D4E50));
    }
}

extern "C" void fn_803BB4F0(MarioActor* self) {
    if (F(u16, 0x6F8) == 6) {
        MR::requestMovementOn(*F(LiveActor**, 0xE98));
    }
}

extern "C" void fn_803BB510(MarioActor* self, const TVec3f& a, const TVec3f& b) {
    MR::emitEffectHit(*F(LiveActor**, 0xE98), a, b, "WallHit");
}

extern "C" void fn_803BE6D0(MarioActor* self) {
    if (F(void*, 0xF74) != nullptr) {
        F(FootPrint*, 0xF74)->clearForce();
    }
}

extern "C" void fn_803BE650(MarioActor* self, u32 a) {
    if (a == 0xD) {
        F(s16, 0xE42) = 0x13;
    }
}

extern "C" void fn_803BE670(MarioActor* self) {
    MR::requestMovementOn(self);
    if (self->mMario->isPlayerModeYoshi()) {
        MR::requestMovementOn(*F(LiveActor**, 0xEE4));
        fn_8041FF70(FP(F(void*, 0xEE4), void*, 8));
    }
}

extern "C" void fn_803BE6F0(MarioActor* self) {
    if (F(u8, 0xE24) != 0) {
        HitSensor* s = F(HitSensor*, 0xE14);
        if (s != nullptr && s->isType(0x65)) {
            F(u8, 0xFB9) = 1;
            F(u8, 0xFB8) = 1;
        }
        if (F(u8, 0xE24) != 0) {
            if (!F(HitSensor*, 0xE14)->receiveMessage(0x95, self->getSensor("Body"))) {
                F(HitSensor*, 0xE14)->receiveMessage(0x96, self->getSensor("Body"));
            }
            if (F(u8, 0xE24) != 0) {
                RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
                self->endRush(&info);
            }
        }
        F(u8, 0xE24) = 0;
        F(u8, 0xE25) = 0;
    }
}

extern "C" J3DModel* getJ3DModel__13MarioAllModelCFv(void*);

extern "C" bool fn_803BE800(MarioActor* self) {
    void* m = F(void*, 0xF00);
    J3DModel* c = FP(m, J3DModel*, 0xC);
    return getJ3DModel__13MarioAllModelCFv(m) == c;
}

extern "C" s32 fn_8040CF40(void*);
extern "C" void fn_803B1100(Mario*);
extern "C" void fn_8040CD20(void*, s32);
extern "C" void fn_8040CDD0(void*, s32);
extern "C" void fn_8040CE80(void*);
extern "C" void fn_8040CED0(void*);

extern "C" void fn_803BE840(MarioActor* self, s32 a) {
    if (self->mMario->isPlayerModeYoshi() && fn_8040CF40(*F(void**, 0xEE4))) {
        fn_803B1100(self->mMario);
        fn_8040CD20(*F(void**, 0xEE4), a);
        setPlayerMode__10MarioActorFUsbb(self, 0, true, true);
    }
}

extern "C" void fn_803BE8D0(MarioActor* self, s32 a) {
    if (self->mMario->isPlayerModeYoshi() && fn_8040CF40(*F(void**, 0xEE4))) {
        fn_803B1100(self->mMario);
        fn_8040CDD0(*F(void**, 0xEE4), a);
        setPlayerMode__10MarioActorFUsbb(self, 0, true, true);
    }
}

extern "C" void fn_803BE960(MarioActor* self) {
    if (self->mMario->isPlayerModeYoshi() && fn_8040CF40(*F(void**, 0xEE4))) {
        fn_803B1100(self->mMario);
        fn_8040CE80(*F(void**, 0xEE4));
        setPlayerMode__10MarioActorFUsbb(self, 0, true, true);
    }
}

extern "C" void fn_803BE9E0(MarioActor* self) {
    if (self->mMario->isPlayerModeYoshi() && fn_8040CF40(*F(void**, 0xEE4))) {
        fn_803B1100(self->mMario);
        fn_8040CED0(*F(void**, 0xEE4));
        setPlayerMode__10MarioActorFUsbb(self, 0, true, true);
    }
}

extern "C" void fn_803CAD10(MarioActor*, s32, s32, s32);

extern "C" void fn_803BEA60(MarioActor* self, s32 a, s32 b) {
    fn_803CAD10(self, a, b, 0);
}

extern "C" void fn_803BEA70(MarioActor* self, s32 a, s32 b) {
    fn_803CAD10(self, a, 0, b);
}

extern "C" void fn_803EC7F0(void*);
extern "C" void fn_803EC8A0(void*);
extern "C" void fn_803EC780(void*);

extern "C" void fn_803BEA80(MarioActor* self) {
    if (F(void*, 0x1090) != nullptr) {
        fn_803EC7F0(F(void*, 0x1090));
    }
}

extern "C" void fn_803BEAA0(MarioActor* self) {
    if (F(void*, 0x1090) != nullptr) {
        fn_803EC8A0(F(void*, 0x1090));
    }
}

extern "C" void fn_803BEAC0(MarioActor* self) {
    if (F(void*, 0x1090) != nullptr) {
        fn_803EC780(F(void*, 0x1090));
    }
    MR::startPlayerNormalBva();
}

extern "C" void fn_803BEAF0(MarioActor* self) {
    if (F(LiveActor*, 0x1090) != nullptr) {
        MR::requestMovementOn(F(LiveActor*, 0x1090));
    }
}

extern "C" s32 fn_803B2880(Mario*, TVec3f*, TVec3f*);

extern "C" void fn_803BEB10(MarioActor* self, s32 a) {
    TVec3f up(*self->mMario->getGravityVec());
    up.scale(150.0f);
    TVec3f from(F(TVec3f, 0x5F4));
    from -= up;
    TVec3f dir(*self->mMario->getGravityVec());
    dir.scale(100.0f);
    if (fn_803B2880(self->mMario, &from, &dir)) {
        F(TVec3f, 0x6D8) = -*self->mMario->getGravityVec();
        self->setPress(4, a);
    }
}

extern "C" bool fn_803BEBE0(MarioActor* self) {
    if (F(u8, 0xE24) == 0) {
        return false;
    }
    if (F(HitSensor*, 0xE14) == nullptr) {
        return false;
    }
    if (MR::getSensorHost(F(HitSensor*, 0xE14)) == nullptr) {
        return false;
    }
    const char* name = MR::getSensorHost(F(HitSensor*, 0xE14))->mName;
    if (MR::isEqualString(name, "スピンドライバ")) {
        return true;
    }
    if (MR::isEqualString(name, "スーパースピンドライバー")) {
        return true;
    }
    if (MR::isEqualString(name, "ピンクスーパースピンドライバー")) {
        return true;
    }
    return MR::isEqualString(name, "土管");
}

extern "C" void* fn_803BECC0(MarioActor* self) {
    if (self->mMario->isPlayerModeYoshi()) {
        return *F(void**, 0xEE4);
    }
    return self;
}

extern "C" bool fn_803BED10(MarioActor* self) {
    return self->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D4E50));
}

extern "C" void* fn_803E26E0(void*);
extern "C" void* fn_803DB5E0(void*);
extern "C" void* fn_803DBF80(void*);

extern "C" void fn_803BF6F0(MarioActor* self) {
    F(s32, 0x94) = 0;
    F(u8, 0x1A5) = 0;
    for (int i = 0; i < 0x40; i++) {
        FP(self, s32, 0x98 + i * 4) = 0;
    }
    F(s32, 0x198) = 0;
    F(s32, 0x19C) = 0;
    F(s32, 0x1A0) = 0;
    F(u8, 0x1A4) = 0;
    F(f32, 0x1A8) = 0.0f;
    F(s16, 0x552) = 0;
    F(s16, 0x554) = 0;
    F(f32, 0x558) = 0.0f;
    F(Color8, 0x55C).set(0, 0, 0, 0);
}

extern "C" void fn_803BF760(MarioActor* self, const JMapInfoIter& iter, s32* anim) {
    MR::getJMapInfoTrans(iter, &self->mPosition);
    MR::getJMapInfoRotate(iter, &self->mRotation);
    self->mScale.x = 1.0f;
    self->mScale.y = 1.0f;
    self->mScale.z = 1.0f;
    self->mRotation.x = MR::repeat(self->mRotation.x, 0.0f, 360.0f);
    self->mRotation.y = MR::repeat(self->mRotation.y, 0.0f, 360.0f);
    self->mRotation.z = MR::repeat(self->mRotation.z, 0.0f, 360.0f);
    if (MR::isExistJMapArg(iter)) {
        MR::getJMapInfoArg0NoInit(iter, anim);
    }
}

extern "C" void fn_803BF860(MarioActor* self) {
    gIsLuigi = false;
    if (MR::isPlayerLuigi()) {
        gIsLuigi = true;
    }
}

extern "C" void fn_803BF8A0(MarioActor* self) {
    self->mMario->setHeadAndFrontVecFromRotate(self->mRotation);
    self->mMario->_290 = self->mMario->mSideVec;
}

extern "C" void fn_803BF8F0(MarioActor* self) {
    void* p = ::operator new(0x10);
    if (p != nullptr) {
        p = fn_803E26E0(p);
    }
    F(void*, 0x590) = p;
    if (gIsLuigi) {
        FP(p, u32, 0xC) = 1;
    }
}

extern "C" void fn_803BF950(MarioActor* self) {
    F(GravityInfo*, 0x69C) = new GravityInfo();
    F(f32, 0x6A4) = 1.0f;
    F(f32, 0x594) = 0.0f;
    F(f32, 0x598) = -1.0f;
    F(f32, 0x59C) = 0.0f;
    F(TVec3f, 0x5A0) = F(TVec3f, 0x594);
    F(s16, 0x664) = 0;
}

extern "C" void fn_803BF9D0(MarioActor* self) {
    self->initBinder(60.0f, 0.0f, 8);
    F(f32, 0x60C) = 0.0f;
    F(f32, 0x610) = 70.0f;
    F(f32, 0x614) = 0.0f;
    self->mBinder->_1EC._0 = false;
    MR::setBinderOffsetVec(self, &F(TVec3f, 0x60C), false);
    self->mBinder->setTriangleFilter(MR::createTriangleFilterDelegator(self, &MarioActor::binderFilter));
    self->mBinder->_1EC._3 = true;
    F(u8, 0x51E) = 1;
    F(f32, 0xE78) = 0.0f;
    F(TVec3f, 0xE6C).zero();
    F(TVec3f, 0x5D0).zero();
    F(TVec3f, 0x5DC).zero();
}

extern "C" void fn_803BFAE0(MarioActor* self) {
    F(s8, 0x666) = 0;
    F(s16, 0x668) = 0;
    void* p = ::operator new(8);
    if (p != nullptr) {
        p = fn_803DB5E0(p);
    }
    F(void*, 0x1094) = p;
}

extern "C" void fn_803BFB30(MarioActor* self) {
    MR::declareGlobalEventCameraAbyss("奈落カメラ");
    MR::declareBlackHoleCamera("ブラックホール");
    MR::declareGlobalEventCameraDead("昇天カメラ", 0.35f, 120, 60);
    MR::declareGlobalEventCamera("水中フォロー");
    MR::declareGlobalEventCamera("水中プラネット");
    MR::declareGlobalEventCamera("水上フォロー");
    MR::declareGlobalEventCamera("引き戻し");
    MR::declareEventCameraProgrammable("変身初出カメラ");
}

extern "C" void fn_803BFBB0(MarioActor* self) {
    F(TVec3f, 0x648) = FP(self->mMario, TVec3f, 0x1F0);
    F(TVec3f, 0x618) = F(TVec3f, 0x648);
    F(s16, 0x660) = 0;
    F(s16, 0x662) = 0;
    MR::setGameCameraTargetToPlayer();
}

extern "C" void fn_803BFC10(MarioActor* self) {
    FootPrint* fp = new FootPrint("足跡", 0x100, -1);
    F(FootPrint*, 0xF74) = fp;
    F(FootPrint*, 0xF74)->setTexture(MR::getTexFromArc("Footprint.bti", self));
}

extern "C" void fn_803BFC90(MarioActor* self, s32 a) {
    switch (a) {
    case 1:
        self->mMario->changeAnimation("基本", (const char*)nullptr);
        break;
    case 2:
        self->mMario->changeAnimationNonStop("ウォークイン");
        break;
    default:
        self->mMario->changeAnimation("ステージインA", (const char*)nullptr);
        break;
    }
}

extern "C" void fn_803BFCE0(MarioActor* self) {
    F(FixedPosition*, 0x780) = new FixedPosition(self, "HandR", TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    F(FixedPosition*, 0x784) = new FixedPosition(self, "HandR", TVec3f(1.8f, 52.51f, 39.58f), TVec3f(76.33f, 15.69f, 88.99f));
    F(FixedPosition*, 0x788) = new FixedPosition(self, "HandR", TVec3f(5.59f, 10.85f, -23.16f), TVec3f(90.45f, 19.45f, 67.7f));
    F(s32, 0x77C) = 0;
}

extern "C" void fn_803BFE50(MarioActor* self) {
    F(f32, 0x7A0) = 35.0f;
    F(f32, 0x7A4) = 60.0f;
    F(f32, 0x7A8) = 0.0f;
    F(f32, 0x7AC) = 1.0f;
    F(f32, 0x7B0) = 0.0f;
    F(TVec3f, 0x7B4) = -F(TVec3f, 0x7A8);
}

extern "C" void fn_803C0670(DLchanger* self, J3DModelX* m) {
    m->setDynamicDL(self->mBuffers[self->mCurrentBuffer].mDL, self->mBuffers[self->mCurrentBuffer].mSize);
}

extern "C" bool fn_803D9430();

extern "C" void fn_803C0E60(MarioActor* self) {
    if (fn_803D9430()) {
        J3DModelX* m = F(J3DModelX*, 0xF08);
        m->setDrawView(1);
        m->mFlags.clear();
        m->mFlags._1C = true;
        m->directDraw(nullptr);
        m->mFlags._1C = false;
        GXSetAlphaUpdate(GX_TRUE);
        GXSetColorUpdate(GX_TRUE);
        GXSetDstAlpha(GX_TRUE, 0);
    }
}

#define BITAT(p, off, n) ((FP(p, u32, off) >> (n)) & 1)

extern "C" {
void fn_803D8BD0(MarioActor*, TVec3f*, TVec3f*);
void fn_803D8E30(MarioActor*, TVec3f*);
void fn_803D9170(MarioActor*, TVec3f*);
bool fn_803C1F30(MarioActor*);
void fn_803C1AF0(MarioActor*, TVec3f*, TVec3f*);
}

extern "C" void fn_803C0EF0(MarioActor* self) {
    if (!self->mMario->isPlayerModeYoshi()) {
        if (F(u8, 0x56E) == 0) {
            if (FP(F(void*, 0xF40), u8, 0) != 0) {
                f32 w = MR::getScreenWidth();
                f32 h = MR::getScreenHeight();
                TDDraw::setup(0, 1, 2);
                TVec3f pos(0.0f, 0.0f, 0.0f);
                TVec3f size(w, h, 0.0f);
                u16 a = FP(F(void*, 0xF40), u16, 0xC);
                u32 alpha = 0xE0;
                if (a <= 0x80) {
                    alpha = (a >> 1) + ((s32)a >> 2) + a;
                }
                TDDraw::drawFillBox(pos, size, alpha | 0x28280000 | 0x2800);
                MR::loadViewMtx();
                MR::loadProjectionMtx();
                MR::loadActorLight(self);
                self->drawMarioModel();
                MR::drawInitFor2DModel();
            }
        } else {
            MR::loadViewMtx();
            MR::loadProjectionMtx();
            MR::loadActorLight(self);
            self->drawMarioModel();
            MR::drawInitFor2DModel();
        }
    }
}

extern "C" void fn_803C1940(MarioActor* self) {
    TVec3f a(self->mPosition);
    a -= F(TVec3f, 0x5E8);
    TVec3f b(a);
    b -= F(TVec3f, 0x38);
    F(TVec3f, 0x5D0) = a;
    TVec3f c(F(TVec3f, 0x5DC));
    F(TVec3f, 0x5DC) = b;
    fn_803C1AF0(self, &F(TVec3f, 0x5DC), &c);
    if (PSVECMag((Vec*)&b) > 0.1f) {
        if (!BITAT(self->mMario, 0xC, 0x15) && !MR::isNearZero(F(TVec3f, 0x38), 0.001f)) {
            TVec3f d(F(TVec3f, 0x5E8));
            d -= F(TVec3f, 0x5C4);
            f32 m1 = PSVECMag((Vec*)&d);
            f32 m2 = PSVECMag((Vec*)&F(TVec3f, 0x38));
            if (PSVECMag((Vec*)&b) > 2.0f * (m1 + m2)) {
                self->mMario->stopWalk();
            }
        }
        if (BITAT(self->mMario, 0x10, 0x1C)) {
            fn_803D8BD0(self, &a, &b);
        }
        if ((FP(self->mMario, u32, 0xC) >> 0x1F) != 0 && F(u8, 0xECD) == 0) {
            fn_803D8E30(self, &b);
            return;
        }
        if (BITAT(self->mMario, 0xC, 0x1E)) {
            fn_803D9170(self, &a);
        }
    }
}

extern "C" void fn_803C1AF0(MarioActor* self, TVec3f* a, TVec3f* b) {
    if (MR::isOppositeDirection(*a, *b, 0.01f)) {
        f32 m1 = PSVECMag((Vec*)a);
        f32 m2 = PSVECMag((Vec*)b);
        if (!MR::isNearZero(m1, 0.001f) && !MR::isNearZero(m2, 0.001f) && MR::isNearZero(m1 - m2, 1.0f)) {
            TVec3f t(*a);
            t.scale(0.5f);
            self->mPosition -= t;
        }
    }
}

extern "C" bool fn_803C1C60(MarioActor* self) {
    if (F(u8, 0x514) == 0) {
        return false;
    }
    if (!MR::tryStartDemoWithoutCinemaFrame(self, "マリオスーパー化")) {
        return false;
    }
    F(u8, 0x514) = 0;
    self->changeMaxLife(6);
    MR::stopAnimFrame(self);
    if (self->mMario->isPlayerModeYoshi()) {
        MR::stopAnimFrame(*F(LiveActor**, 0xEE4));
    }
    MR::requestPowerUpHPMeter();
    self->mMario->startPadVib("マリオ[変身]");
    MR::startSystemSE("SE_SY_SUPER_KINOKO_GET", -1, -1);
    F(u16, 0x6FE) = 0x5A;
    return true;
}

extern "C" bool fn_803C1D30(MarioActor* self) {
    if (F(u8, 0x515) == 0) {
        return false;
    }
    if (!MR::tryStartDemoWithoutCinemaFrame(self, "マリオ変身")) {
        return false;
    }
    F(u8, 0x516) = 1;
    F(u8, 0x515) = 0;
    F(u16, 0x6FC) = 0x40;
    if (F(u16, 0x6F8) == 6) {
        fn_803BB4F0(self);
        self->changeTeresaAnimation("change", -1);
    }
    if (F(u16, 0x6F8) == 0xA) {
        MR::requestMovementOn(self);
        MR::requestMovementOn(*F(LiveActor**, 0xEE4));
        F(u16, 0x6FC) = 0x13;
        self->mMario->clearVelocity();
        FP(self->mMario, f32, 0x278) = 0.0f;
    }
    if (F(u16, 0x6F8) != 0xA) {
        MR::stopAnimFrame(self);
        self->playEffect("変身");
        self->mMario->startPadVib("マリオ[変身]");
    }
    return true;
}

extern "C" bool fn_803C1E40(MarioActor* self) {
    bool r = false;
    if ((F(u16, 0x6FC) != 0 && F(u16, 0x6F8) != 0) || F(u16, 0x6FE) != 0) {
        r = true;
    }
    return r;
}

extern "C" void fn_803C1E80(MarioActor* self) {
    FP(self->mMario, TVec3f, 0x350).zero();
    FP(self->mMario, TVec3f, 0x35C).zero();
    F(s16, 0xCCC) = 0;
    F(s32, 0xE20) = 0;
    F(TVec3f, 0x38).zero();
    F(TVec3f, 0x5B8).zero();
    F(TVec3f, 0x5C4) = self->mPosition;
    if (fn_803C1F30(self)) {
        fn_80405F90(self->mMario->mPosHolder, self->mPosition);
        self->mMario->checkEnforceMove();
        self->mPosition = self->mMario->mPosHolder->mPos;
    }
}

extern "C" bool fn_803C1F30(MarioActor* self) {
    Mario* m = self->mMario;
    bool r = false;
    if (BITAT(m, 0xC, 0x1E)) {
        MtxPtr prev = FP(m, Triangle*, 0x458)->getPrevBaseMtx()->mMtx;
        if (!MR::isSameMtx(FP(self->mMario, Triangle*, 0x458)->getBaseMtx()->mMtx, prev)) {
            r = true;
        }
    }
    return r;
}

extern "C" void fn_803C2010(MarioActor* self) {
    u16 t = F(u16, 0x260);
    if (t != 0) {
        F(u16, 0x260) = t - 1;
    }
}

extern "C" void fn_803C2030(MarioActor* self) {
    if (F(s8, 0x4E0) == 0) {
        if (F(s8, 0x51E) == 0) {
            MR::offBind(self);
        } else {
            MR::onBind(self);
        }
    }
    F(TVec3f, 0xE6C) = F(TVec3f, 0x38);
    if (BITAT(self->mMario, 0x1C, 0x14)) {
        self->mBinder->_1EC._1 = false;
        return;
    }
    self->mBinder->_1EC._1 = true;
}

extern "C" void fn_803DDF60(void*);

extern "C" void fn_803C2320(MarioActor* self) {
    F(s32, 0x6AC) = F(s32, 0x6AC) + 1;
    if (F(u16, 0xFF2) != 0) {
        F(u16, 0xFF2) -= 1;
    }
    if (F(u16, 0x6CE) != 0) {
        F(u16, 0x6CE) -= 1;
    }
    if (F(u16, 0x6D2) != 0) {
        F(u16, 0x6D2) -= 1;
    }
    if (F(s32, 0xE28) != 0) {
        F(s32, 0xE28) -= 1;
    }
    if (F(s32, 0x1054) != 0) {
        F(s32, 0x1054) -= 1;
        if (F(s32, 0x1054) == 0) {
            F(u8, 0x768) = 1;
            fn_803DDF60(F(void*, 0x588));
        }
    }
}

extern "C" void fn_803C23B0(MarioActor* self) {
    TVec3f d(self->mPosition);
    d -= F(TVec3f, 0x5C4);
    F(TVec3f, 0x5B8) = d;
    F(TVec3f, 0x5C4) = self->mPosition;
}

extern "C" void fn_803C2520(MarioActor* self) {
    if (F(u16, 0xFBC) != 0) {
        F(u16, 0xFBC) -= 1;
        if (F(u16, 0xFBC) == 0) {
            self->mMario->stopAnimationUpper("ハンマー投げ回転中", (const char*)nullptr);
            self->mMario->stopAnimation("ハンマー投げ回転中", (const char*)nullptr);
        }
    }
}

extern "C" void fn_803C2610(MarioActor*);
extern "C" void fn_803C26A0(MarioActor*);
extern "C" bool fn_803C2710(MarioActor*);

extern "C" void fn_803C2590(MarioActor* self) {
    if (F(u8, 0xE3C) != 0) {
        u8 t = F(u8, 0xE3D);
        F(u8, 0xE3C) = F(u8, 0xE3C) - 1;
        F(u8, 0xE3D) = t + 1;
        fn_803C2610(self);
        fn_803C26A0(self);
    }
    if (fn_803C2710(self)) {
        self->mMario->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
}

extern "C" void fn_803C2610(MarioActor* self) {
    if ((F(s16, 0xE3E) != 0 || BITAT(self->mMario, 0x10, 0x14)) && !self->mMario->isSwimming() && F(u8, 0xE3C) == 0 &&
        FP(self, MarioActor*, 0x1080)->selectAction("スピン回復エフェクト") == 1) {
        self->playEffect("スピンリング");
    }
}

extern "C" void fn_803C26A0(MarioActor* self) {
    if ((self->isInPunchTimerRange() || !self->mMario->isSwimming())) {
        u32 t = FP(self->mMario, u32, 0xC);
        if (((t >> 0x17) & 1) || ((t >> 5) & 1) || ((t >> 6) & 1)) {
            self->mMario->tryWallPunch();
        }
    }
}

extern "C" bool fn_803C2710(MarioActor* self) {
    bool a = false;
    bool b = false;
    bool c = false;
    bool d = false;
    if (self->mMario->isAnimationRun("ハンマー投げリリース") && BITAT(self->mMario, 0xC, 0x1E)) {
        d = true;
    }
    if (d && F(u16, 0x6B8) == 0) {
        c = true;
    }
    if (c && FP(self->mMario, u16, 0x41C) == 0) {
        b = true;
    }
    if (b && self->mMario->isStickOn()) {
        a = true;
    }
    return a;
}

extern "C" bool isPlayerModeCloud__11MarioModuleCFv(const MarioModule*);
extern "C" void selectHomingInSuperHipDrop__10MarioActorCFPCc(const MarioActor*, const char*);
extern "C" {
s32 fn_803DB380(void*, const char*, const char*);
void fn_803FD5E0(void*, s32);
s32 fn_8030B260();
void fn_8038C540(Mario*);
s32 fn_803B26C0(Mario*);
void fn_803C3E40(MarioActor*);
bool fn_803C3DF0(MarioActor*);
void fn_803C4020(MarioActor*);
void fn_803D9740(MarioActor*);
void fn_803D8A70(MarioActor*, s32);
void fn_803E7880(void*, s32);
void fn_803E7B80(void*);
extern u32 lbl_807D4E34;
}

extern "C" void fn_803C2900(MarioActor* self) {
    if (fn_803DB380(F(void*, 0x1080), MR::getSensorHost(F(HitSensor*, 0xE14))->mName, "水陸移動特殊処理")) {
        WaterInfo info;
        if (!MR::getWaterAreaObj(&info, self->mPosition)) {
            self->mMario->forceExitSwim();
        }
    } else if (!fn_803DB380(F(void*, 0x1080), MR::getSensorHost(F(HitSensor*, 0xE14))->mName, "水柱エフェクト")) {
        void* sw = FP(self->mMario, void*, 0x888);
        s32 before = FP(sw, s32, 0xFC);
        fn_803FD5E0(sw, 0);
        s32 after = FP(FP(self->mMario, void*, 0x888), s32, 0xFC);
        if (after != before) {
            void* sw2 = FP(self->mMario, void*, 0x888);
            if ((u32)after <= 1 && (u32)(before - 2) <= 1) {
                self->playEffectRT("水面ジャンプ水柱", FP(sw2, TVec3f, 0x11C), FP(sw2, TVec3f, 0x128));
                void* sw3 = FP(self->mMario, void*, 0x888);
                self->emitEffectWaterColumn(FP(sw3, TVec3f, 0x11C), FP(sw3, TVec3f, 0x128));
            } else if ((u32)before <= 1 && (u32)(after - 2) <= 1) {
                TVec3f neg = -FP(self->mMario, TVec3f, 0x328);
                self->playEffectRT("水面ジャンプ水柱", neg, FP(FP(self->mMario, void*, 0x888), TVec3f, 0x128));
                void* sw3 = FP(self->mMario, void*, 0x888);
                self->emitEffectWaterColumn(FP(sw3, TVec3f, 0x11C), FP(sw3, TVec3f, 0x128));
            }
            switch (after) {
            case 0:
                MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();
                self->mMario->forceExitSwim();
                return;
            case 2:
                MR::getGameSceneLayoutHolder()->changeLifeMeterModeWater();
                break;
            }
        }
    }
}

extern "C" void fn_803C2AA0(MarioActor* self) {
    if (self->mMario->isForceStopRush() && F(u8, 0xE24) != 0 && F(HitSensor*, 0xE14)->receiveMessage(0x97, self->getSensor("Body"))) {
        RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
        self->endRush(&info);
    }
}

extern "C" void fn_803C2B50(MarioActor* self) {
    if (F(u8, 0xE24) != 0 && fn_803DB380(F(void*, 0x1080), MR::getSensorHost(F(HitSensor*, 0xE14))->mName, "ハチ回復")) {
        u32 i = 0;
        do {
            self->mMario->incAirWalkTimer();
            i++;
        } while (i < 4);
    }
}

extern "C" bool fn_803C3260(MarioActor* self) {
    if (self->mMario->isAnimationRun("壁はじき")) {
        return false;
    }
    if (self->isJumping() && FP(self->mMario, s32, 0x424) != 0) {
        return false;
    }
    if (F(u16, 0x6F8) == 9) {
        return false;
    }
    if (self->mMario->getCurrentStatus() == 1) {
        return false;
    }
    if (self->mMario->isSwimming()) {
        return false;
    }
    if (self->mMario->isStatusActive(0x18)) {
        return false;
    }
    if (self->mMario->isStatusActive(0x13)) {
        return false;
    }
    if (F(s32, 0x750) != 0) {
        return false;
    }
    if (self->mMario->isStatusActive(2)) {
        return false;
    }
    if (F(u8, 0x6E4) != 0) {
        return false;
    }
    if (F(u8, 0x4E0) != 0) {
        return false;
    }
    Mario* m = self->mMario;
    if (BITAT(m, 0x1C, 0x13)) {
        return false;
    }
    return FP(m, s32, 0xA78) == 0;
}

extern "C" bool fn_803C33D0(MarioActor* self) {
    bool r = false;
    bool a = false;
    bool b = false;
    if (!BITAT(self->mMario, 0xC, 0x10) && self->isJumping()) {
        b = true;
    }
    if (b) {
        bool c = true;
        if (self->mMario->isDamaging()) {
            bool d = false;
            if (isPlayerModeCloud__11MarioModuleCFv(self->mMario) && fn_8030B260() == 0) {
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
    if (a && !BITAT(self->mMario, 0x10, 0x14)) {
        r = true;
    }
    return r;
}

extern "C" void fn_803C34F0(MarioActor* self, const char* name) {
    if (F(s32, 0x6B4) != 0) {
        F(s32, 0x6B4) -= 1;
        if (F(s32, 0x6B4) == 0) {
            selectHomingInSuperHipDrop__10MarioActorCFPCc(self, name);
            fn_803D9740(self);
        }
    }
    if ((u32)F(s32, 0x6B0) <= 1) {
        if (!MR::isDemoActive() && !MR::isPowerStarGetDemoActive() && !MR::isGalaxyDarkCometAppearInCurrentStage() && MR::isPermitSE() &&
            self->isEnableNerveChange()) {
            FP(self->mMario, u8, 0xA92) = 1;
        }
    }
    fn_8038C540(self->mMario);
}

extern "C" void fn_803C38D0(MarioActor* self) {
    if (fn_803C1F30(self)) {
        fn_80405F90(self->mMario->mPosHolder, self->mPosition);
        self->mMario->checkEnforceMove();
        self->mPosition = self->mMario->mPosHolder->mPos;
    }
}

extern "C" bool fn_803C3930(MarioActor* self) {
    Mario* m = self->mMario;
    bool r = false;
    if (BITAT(m, 0xC, 0x1E)) {
        MtxPtr prev = FP(m, Triangle*, 0x458)->getPrevBaseMtx()->mMtx;
        if (!MR::isSameMtx(FP(self->mMario, Triangle*, 0x458)->getBaseMtx()->mMtx, prev)) {
            r = true;
        }
    }
    return r;
}

extern "C" void fn_803C3C90(MarioActor* self) {
    if (F(u16, 0x6FE) != 0) {
        F(u16, 0x6FE) -= 1;
        if (F(u16, 0x6FE) == 0) {
            MR::endDemo(self, "マリオスーパー化");
            MR::releaseAnimFrame(self);
            if (self->mMario->isPlayerModeYoshi()) {
                MR::releaseAnimFrame(*F(LiveActor**, 0xEE4));
            }
            MR::explainLifeUpIfAtFirst();
        }
    }
}

extern "C" void fn_803C3D10(MarioActor* self) {
    if (F(u16, 0x6FC) != 0) {
        if (F(u16, 0x6F8) == 6) {
            void* r5 = F(void*, 0x590);
            if (FP(self->mMario, f32, 0x484) < FP(FP(r5, void*, FP(r5, u32, 0xC) * 4), f32, 0x710)) {
                self->mPosition -= *self->mMario->getGravityVec();
            }
        }
        if (!fn_803C3DF0(self) && !fn_803B26C0(self->mMario)) {
            fn_803C3E40(self);
        }
        F(u16, 0x6FC) -= 1;
        if (F(u16, 0x6FC) == 0 && !fn_803B26C0(self->mMario)) {
            fn_803C4020(self);
        }
        F(u8, 0x703) = 1;
        return;
    }
    self->mMario->updateMorphResetTimer();
}

extern "C" bool fn_803C3DF0(MarioActor* self) {
    switch (F(u16, 0x6F8)) {
    case 2:
        return MR::isOnGameEventFlagFireMarioAtFirst();
    case 3:
        return MR::isOnGameEventFlagIceMarioAtFirst();
    case 4:
        return MR::isOnGameEventFlagBeeMarioAtFirst();
    case 5:
        return MR::isOnGameEventFlagHopperMarioAtFirst();
    case 6:
        return MR::isOnGameEventFlagTeresaMarioAtFirst();
    case 7:
        return MR::isOnGameEventFlagFlyingMarioAtFirst();
    case 12:
        return MR::isOnGameEventFlagCloudMarioAtFirst();
    case 13:
        return MR::isOnGameEventFlagRockMarioAtFirst();
    }
    return true;
}

extern "C" void fn_803C3E40(MarioActor* self) {
    TVec3f front;
    if (F(void*, 0x790) != nullptr) {
        MR::calcFrontVec(&front, MR::getSensorHost(F(HitSensor*, 0x790)));
    } else if (F(void*, 0x794) != nullptr) {
        MR::calcFrontVec(&front, MR::getSensorHost(F(HitSensor*, 0x794)));
    } else {
        self->getFrontVec(&front);
    }
    self->mMario->setFrontVecKeepUp(front);
    if (F(s8, 0x666) == 0) {
        MR::zoomInTargetGameCamera();
        CameraTargetArg arg(self);
        MR::startGlobalEventCamera("変身初出カメラ", arg, 60);
        TVec3f up = -*self->mMario->getGravityVec();
        TVec3f base(F(TVec3f, 0x5F4));
        f32 dist = 500.0f;
        f32 height = 50.0f;
        if (F(u16, 0x6F8) == 6) {
            dist = 700.0f;
            height = 150.0f;
        }
        Mario* m = self->mMario;
        u32 st = FP(m, u32, 0xC);
        if (((st >> 0x14) & 1) && !((st >> 0x1E) & 1)) {
            base += FP(m, TVec3f, 0x2D4);
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
        TVec3f upv = -F(TVec3f, 0x594);
        MR::setProgrammableCameraParam("変身初出カメラ", eye, tgt, upv, true);
    }
    F(u8, 0x666) = 1;
}

extern "C" void fn_803C4020(MarioActor* self) {
    if (F(u8, 0x516) != 0) {
        MR::endDemo(self, "マリオ変身");
        F(u8, 0x516) = 0;
        MR::releaseAnimFrame(self);
    }
    F(u8, 0x702) = 1;
    switch (F(u16, 0x6F8)) {
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
    if (F(u8, 0x666) != 0) {
        F(u16, 0x668) = 10;
        F(u8, 0x666) = 0;
    }
}

extern "C" void fn_803C4100(MarioActor* self) {
    if (F(u16, 0x668) != 0) {
        F(u16, 0x668) -= 1;
        if (F(u16, 0x668) == 0) {
            MR::zoomOutTargetGameCamera();
            MR::endGlobalEventCamera("変身初出カメラ", 60, true);
        }
    }
}

extern "C" void calcAnimPlayerMode__10MarioActorFv(MarioActor* self) {
    if (F(u8, 0x703) == 0) {
        F(u8, 0x702) = 0;
        return;
    }
    u16 mode;
    u16 t = F(u16, 0x6FC);
    if ((s32)(t & 7) < ((s32)t >> 3)) {
        mode = F(u16, 0x6FA);
    } else {
        mode = F(u16, 0x6F8);
    }
    fn_803D8A70(self, mode);
    switch (mode) {
    case 1:
        fn_803E7880(F(void*, 0xF00), 4);
        F(SMG2VObj*, 0xEA4)->v6();
        if (F(u8, 0x702) != 0) {
            F(u8, 0x1B1) = 2;
        }
        break;
    case 4:
        fn_803E7880(F(void*, 0xF00), 2);
        break;
    case 5:
        F(u8, 0x703) = 0;
        fn_803E7880(F(void*, 0xF00), 5);
        break;
    case 3:
        fn_803E7880(F(void*, 0xF00), 3);
        break;
    case 12:
        fn_803E7880(F(void*, 0xF00), 7);
        break;
    case 13:
        fn_803E7880(F(void*, 0xF00), 8);
        break;
    default: {
        fn_803E7880(F(void*, 0xF00), 0);
        u32 frame = 0;
        switch (mode) {
        case 7:
            frame = 1;
            break;
        case 2:
            frame = 2;
            break;
        }
        MR::startBtp(self, "ColorChange");
        MR::setBtpFrameAndStop(self, (f32)frame);
        break;
    }
    }
    F(u8, 0x703) = 0;
    F(u8, 0x702) = 0;
}

extern "C" void calcAnimHorrorBone__10MarioActorFv(MarioActor* self) {
    if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
        Mario* m = self->mMario;
        if (FP(m, s32, 0x5F8) != 0) {
            if (FP(F(void*, 0xF00), s32, 4) == 6) {
                fn_803E7B80(F(void*, 0xF00));
                self->playEffect("ホラーマリオ解除");
            }
        } else if (MR::isPowerStarGetDemoActive()) {
            if (FP(F(void*, 0xF00), s32, 4) == 6) {
                fn_803E7B80(F(void*, 0xF00));
            }
        } else {
            void* p = F(void*, 0xF00);
            if (FP(p, s32, 4) != 6) {
                FP(p, s32, 8) = FP(p, s32, 4);
                self->playEffect("ホラーマリオ変身");
            }
            fn_803E7880(F(void*, 0xF00), 6);
        }
    } else {
        if (self->mMario->isStatusActive(0xB)) {
            void* p = F(void*, 0xF00);
            if (FP(p, s32, 4) != 1) {
                FP(p, s32, 8) = FP(p, s32, 4);
            }
            if ((F(s32, 0x6AC) & 3) && (self->isNerve(reinterpret_cast<const Nerve*>(&lbl_807D4E34)) || self->isNerve(&NrvMarioActor::MarioActorNrvNoRush::sInstance))) {
                fn_803E7880(F(void*, 0xF00), 1);
                return;
            }
            fn_803E7B80(F(void*, 0xF00));
            return;
        }
        if (FP(F(void*, 0xF00), s32, 4) == 1 && F(s32, 0x6B0) != 0) {
            fn_803E7B80(F(void*, 0xF00));
        }
    }
}

extern "C" bool fn_803C4F70(MarioActor* self) {
    bool r = false;
    if (F(u8, 0x4E1) == 0 && F(u8, 0x4E0) != 0 && F(u8, 0x4E2) != 0) {
        r = true;
    }
    return r;
}

extern "C" bool fn_803C4FB0(MarioActor* self) {
    bool r = false;
    bool a = false;
    if (F(u16, 0x6F8) == 4 && !self->mMario->isStatusActive(0x16) && !self->mMario->isStatusActive(0x15) && !self->mMario->isStatusActive(0x1B)) {
        Mario* m = self->mMario;
        if (!BITAT(m, 0x10, 0x1C) && !BITAT(m, 0xC, 0x15)) {
            a = true;
        }
    }
    if (a) {
        bool b = false;
        u32 st = FP(self->mMario, u32, 0xC);
        if (((st >> 0x14) & 1) && (st >> 0x1F) != 0) {
            b = true;
        }
        if (!b) {
            r = true;
        }
    }
    return r;
}

#define CT(off) FP(((void**)F(void*, 0x590))[FP(F(void*, 0x590), u32, 0xC)], f32, off)

extern "C" void fn_803C5090(MarioActor* self, MtxPtr mtx) {
    F(TVec3f, 0xED0) = FP(self->mMario, TVec3f, 0x1D8);
    f32 k = CT(0x69C);
    TVec3f grav = -*self->mMario->getGravityVec();
    TVec3f t1(grav);
    t1.scale(k);
    MtxPtr m1 = MR::tmpMtxTrans(-t1);
    f32 k2 = CT(0x69C);
    TVec3f e(F(TVec3f, 0xED0));
    e.scale(k2);
    TVec3f p(self->mPosition);
    p -= e;
    TVec3f d(F(TVec3f, 0x66C));
    f32 angle = 0.0f;
    d -= p;
    TVec3f axis;
    if (!MR::normalizeOrZero(&d)) {
        PSVECCrossProduct((Vec*)&FP(self->mMario, TVec3f, 0x208), (Vec*)&d, (Vec*)&axis);
        if (!MR::normalizeOrZero(&axis)) {
            f32 ang = MR::acosEx(F(TVec3f, 0xED0).dot(d));
            f32 lim = CT(0x688);
            if (BITAT(self->mMario, 0xC, 0x1E)) {
                lim = CT(0x68C);
            }
            angle = MR::clamp(ang, 0.0f, lim);
        }
    }
    Mtx rot;
    PSMTXRotAxisRad(rot, (Vec*)&axis, angle);
    PSMTXConcat(m1, mtx, mtx);
    PSMTXConcat(rot, mtx, mtx);
    PSMTXConcat(MR::tmpMtxTrans(t1), mtx, mtx);
    f32 k3 = CT(0x690);
    TVec3f s110(F(TVec3f, 0xED0));
    s110.scale(k3);
    MR::vecKillElement(s110, F(TVec3f, 0x690), &s110);
    TVec3f b0(F(TVec3f, 0x684));
    b0 += s110;
    F(TVec3f, 0x684) = b0;
    TVec3f a4(F(TVec3f, 0x66C));
    a4 += F(TVec3f, 0x684);
    F(TVec3f, 0x66C) = a4;
    f32 k4 = CT(0x69C);
    TVec3f s80(F(TVec3f, 0xED0));
    s80.scale(k4);
    TVec3f s8c(self->mPosition);
    s8c -= s80;
    TVec3f s98(F(TVec3f, 0x66C));
    s98 -= s8c;
    F(TVec3f, 0x690) = s98;
    TVec3f s104(F(TVec3f, 0x690));
    TVec3f sF8(F(TVec3f, 0x690));
    MR::normalizeOrZero(&sF8);
    f32 ang2 = MR::acosEx(F(TVec3f, 0xED0).dot(sF8));
    f32 lim2 = CT(0x694);
    if (BITAT(self->mMario, 0xC, 0x1E)) {
        lim2 = CT(0x698);
    }
    if (ang2 > lim2) {
        TVec3f sEC;
        PSVECCrossProduct((Vec*)&F(TVec3f, 0xED0), (Vec*)&sF8, (Vec*)&sEC);
        if (MR::normalizeOrZero(&sEC)) {
            F(TVec3f, 0x690) = F(TVec3f, 0xED0);
        } else {
            Mtx rot2;
            PSMTXRotAxisRad(rot2, (Vec*)&sEC, lim2);
            PSMTXMultVec(rot2, (Vec*)&F(TVec3f, 0xED0), (Vec*)&F(TVec3f, 0x690));
        }
    }
    F(TVec3f, 0x690).setLength(CT(0x69C));
    if (PSVECMag((Vec*)&s104) > CT(0x69C)) {
        TVec3f s68(s104);
        s68 -= F(TVec3f, 0x690);
        TVec3f n = -s68;
        F(TVec3f, 0x684) += n;
    }
    f32 k5 = CT(0x69C);
    TVec3f s44(F(TVec3f, 0xED0));
    s44.scale(k5);
    TVec3f s50(self->mPosition);
    s50 -= s44;
    TVec3f s5c(s50);
    s5c += F(TVec3f, 0x690);
    F(TVec3f, 0x66C) = s5c;
    MR::normalizeOrZero(&F(TVec3f, 0x690));
    if (PSVECMag((Vec*)&F(TVec3f, 0x684)) > CT(0x6A0)) {
        F(TVec3f, 0x684).setLength(CT(0x6A0));
    }
    f32 sc = CT(0x6A4);
    if (FP(self->mMario, s32, 0x714) == 0 && !self->isJumping()) {
        sc = CT(0x6AC);
    }
    F(TVec3f, 0x684).scale(sc);
    if (!MR::isSameDirection(F(TVec3f, 0x690), FP(self->mMario, TVec3f, 0x208), 0.01f)) {
        TVec3f neg = -F(TVec3f, 0x690);
        MR::makeMtxUpFront((TPos3f*)mtx, neg, FP(self->mMario, TVec3f, 0x208));
    } else if (!MR::isSameDirection(FP(self->mMario, TVec3f, 0x1FC), FP(self->mMario, TVec3f, 0x208), 0.01f)) {
        MR::makeMtxUpFront((TPos3f*)mtx, FP(self->mMario, TVec3f, 0x1FC), FP(self->mMario, TVec3f, 0x208));
    } else {
        MR::makeMtxUpFront((TPos3f*)mtx, FP(self->mMario, TVec3f, 0x1F0), FP(self->mMario, TVec3f, 0x208));
    }
    if (FP(self->mMario, u8, 0x714) > 2 || self->isJumping()) {
        f32 inv = 1.0f - CT(0x6B0);
        TVec3f s8(F(TVec3f, 0x66C));
        s8 -= self->mPosition;
        TVec3f s14(s8);
        s14.scale(inv);
        f32 k6 = CT(0x6B0);
        TVec3f s20(F(TVec3f, 0x678));
        s20.scale(k6);
        TVec3f s2c(s20);
        s2c += s14;
        F(TVec3f, 0x678) = s2c;
        MR::setMtxTrans(mtx, F(TVec3f, 0x678).x, F(TVec3f, 0x678).y, F(TVec3f, 0x678).z);
        return;
    }
    F(TVec3f, 0x678).scale(CT(0x6A8));
    MR::setMtxTrans(mtx, F(TVec3f, 0x678).x, F(TVec3f, 0x678).y, F(TVec3f, 0x678).z);
}

extern "C" void fn_803C56F0(MarioActor* self) {
    F(TVec3f, 0x684).zero();
    F(TVec3f, 0x678).zero();
    F(TVec3f, 0x66C) = self->mPosition;
    F(TVec3f, 0xED0) = *self->mMario->getGravityVec();
}

extern "C" void fn_803C58D0(MarioActor*);
extern "C" void fn_803C5920(MarioActor*) NO_INLINE;

extern "C" bool fn_803C5750(MarioActor* self) {
    bool r = false;
    if (F(u8, 0xE24) != 0) {
        r = F(HitSensor*, 0xE14)->receiveMessage(0xA1, self->getSensor("Body"));
    }
    if (F(u8, 0x4E1) != 0) {
        PSMTXCopy(F(Mtx, 0x4E4), self->getBaseMtx());
    }
    if (r || F(u8, 0x4E1) != 0) {
        fn_803C58D0(self);
        return true;
    }
    if (F(u8, 0xE24) != 0) {
        fn_803C5920(self);
    }
    return false;
}

extern "C" void fn_803C59C0(MarioActor*);

extern "C" void fn_803C5820(MarioActor* self) {
    TVec3f old(self->mPosition);
    MR::extractMtxTrans(self->getBaseMtx(), &self->mPosition);
    self->mMario->invalidateRelativePosition();
    TVec3f diff(self->mPosition);
    diff -= old;
    F(TVec3f, 0xE30) = diff;
    if (BITAT(self->mMario, 0x10, 8)) {
        FP(self->mMario, TVec3f, 0x67C) = self->mPosition;
    }
    self->updateGravityVec(true, true);
    self->mMario->updateGroundInfo();
}

extern "C" void fn_803C58D0(MarioActor* self) {
    fn_803C59C0(self);
    fn_803C5820(self);
    if (F(u8, 0x4E0) != 0) {
        F(u8, 0x4E2) = 1;
    }
}

extern "C" void fn_803C5920(MarioActor* self) {
    TVec3f cur;
    MR::extractMtxTrans(self->getBaseMtx(), &cur);
    TVec3f diff(self->mPosition);
    diff -= cur;
    F(TVec3f, 0xE30) = diff;
}

extern "C" bool fn_803C5990(MarioActor* self) {
    if (F(u8, 0x56C) == 0) {
        F(u8, 0x56D) = 1;
        return false;
    }
    F(u8, 0x56D) = 0;
    F(u8, 0x56C) = 0;
    return true;
}

extern "C" void fn_803C59C0(MarioActor* self) {
    TVec3f front, side, head;
    self->getBaseMtx();
    TPos3f* bm = (TPos3f*)self->getBaseMtx();
    bm->getZDir(front);
    self->mMario->setFrontVec(front);
    ((TPos3f*)self->getBaseMtx())->getXDir(side);
    self->mMario->setSideVec(side);
    ((TPos3f*)self->getBaseMtx())->getYDir(head);
    self->mMario->forceSetHeadVecKeepSide(head);
    FP(self->mMario, f32, 0x278) = 0.0f;
    FP(self->mMario, u8, 0x714) = 0;
    self->mMario->stopJump();
    FP(self->mMario, TVec3f, 0x334) = FP(self->mMario, TVec3f, 0x208);
    if (self->mMario->isSwimming()) {
        FP(self->mMario, MarioSwim*, 0x888)->resetAndFixPose();
    }
}

extern "C" void fn_803C5B30(MarioActor*, MtxPtr);
extern "C" void fn_803C5E20(MarioActor*, MtxPtr);

extern "C" void fn_803C5AB0(MarioActor* self, MtxPtr mtx) {
    if (F(s32, 0x6BC) != 0) {
        fn_803C5B30(self, mtx);
    } else if (F(s32, 0x6C0) != 0) {
        fn_803C5E20(self, mtx);
    } else if (!BITAT(self->mMario, 0x10, 3)) {
        F(f32, 0x6D4) = 1.0f;
    }
    s32 t = F(s32, 0x6C4);
    if (t != 0) {
        F(s32, 0x6C4) = t - 1;
    }
}

extern "C" void fn_803C5B30(MarioActor* self, MtxPtr mtx) {
    s32 mode = F(s32, 0x6C8);
    F(s32, 0x6C0) = 0x1E;
    switch (mode) {
    case 0:
    case 2: {
        u32 cnt = F(u32, 0x6BC);
        if (cnt > 0xF) {
            f32 a = self->mMario->calcDistToCeil(false);
            f32 b = self->mMario->calcDistToCeilOnPress();
            f32 v1 = MR::clamp(a / 150.0f, 0.2f, 1.0f);
            if (F(f32, 0x6D4) > v1) {
                F(f32, 0x6D4) = v1;
            } else {
                f32 v2 = MR::clamp(b / 150.0f, 0.2f, 1.0f);
                if (F(f32, 0x6D4) > v2) {
                    F(f32, 0x6D4) = v2;
                }
                MR::setMtxTrans(mtx, self->mPosition.x, self->mPosition.y, self->mPosition.z);
                if (FP(self->mMario, u16, 0x964) != 0x1B) {
                    F(u8, 0x573) = 1;
                }
            }
        } else {
            F(f32, 0x6D4) = 0.2f + ((0.8f * (f32)(0xF - cnt)) / 15.0f);
        }
        s32 t = F(s32, 0x6C4);
        if (t != 0) {
            F(f32, 0x6D4) = F(f32, 0x6D4) + 0.02f * (f32)t;
            return;
        }
        return;
    }
    case 1:
    case 3: {
        u32 cnt = F(u32, 0x6BC);
        if (cnt > 0xF) {
            f32 w = self->mMario->calcDistWidth();
            self->mPosition = self->mMario->mPosHolder->mPos;
            f32 v = MR::clamp(w / 80.0f, 0.2f, 1.0f);
            if (F(f32, 0x6D4) > v) {
                F(f32, 0x6D4) = v;
            } else {
                f32 t2 = F(f32, 0x6D4) - 0.01f;
                F(f32, 0x6D4) = t2;
                F(f32, 0x6D4) = MR::clamp(t2, 0.2f, 1.0f);
            }
        } else {
            F(f32, 0x6D4) = 0.2f + ((0.8f * (f32)(0xF - cnt)) / 15.0f);
        }
        s32 t = F(s32, 0x6C4);
        if (t != 0) {
            F(f32, 0x6D4) = F(f32, 0x6D4) + 0.02f * (f32)t;
            return;
        }
        break;
    }
    case 4:
        F(f32, 0x6D4) = 0.2f + ((0.8f * (160.0f * (1.0f - JMACosRadian(0.5f * (((f32)F(s32, 0x6BC) / 120.0f) * 3.1415927f))))) / 160.0f);
        break;
    }
}

extern "C" void fn_803C5E20(MarioActor* self, MtxPtr mtx) {
    u32 t = F(u32, 0x6C0) - 1;
    F(u32, 0x6C0) = t;
    F(f32, 0x6D4) = 1.0f + (((0.15f * (f32)F(u32, 0x6C0)) / 30.0f) * JMASinRadian((2.0f * ((f32)t * 3.1415927f)) / 15.0f));
}

extern "C" void fn_803C5ED0(MarioActor* self, MtxPtr m1, MtxPtr m2) {
    u16 t = F(u16, 0x4D8);
    if (t != 0) {
        f32 f = 1.0f - ((1.0f - F(f32, 0x4DC)) * ((f32)(t - 1) / (f32)t));
        F(f32, 0x4DC) = f;
        MR::blendMtx(m2, m1, f, m1);
        F(u16, 0x4D8) = F(u16, 0x4D8) - 1;
    }
}

extern "C" bool passRing__10MarioStateFPC9HitSensor(void*, const HitSensor*) {
    return false;
}

namespace MR {
    bool isCreatePlayerResourceFire();
    bool isCreatePlayerResourceYoshi();
    bool isCreatePlayerResourceCloud();
    bool isCreatePlayerResourceRock();
}
extern "C" void* __ct__11YoshiHolderFP5Mario(void*, Mario*);
extern "C" void* __ct__13CloudMarioHatFPCc(void*, const char*);
extern "C" bool isExistRes__8ResTableCFPCc(void*, const char*);
extern "C" void* getRes__8ResTableCFPCc(void*, const char*);
extern "C" void registerModel__13MarioAllModelFlP9LiveActorbb(void*, s32, LiveActor*, bool, bool);
extern "C" void addShapePacket__12J3DMatPacketFP14J3DShapePacket(void*, void*);
extern "C" void initFireBall__10MarioActorFv(MarioActor*);
extern "C" void fn_8041B700(MarioActor*);

extern "C" bool fn_803BB5F0(MarioActor* self) {
    if (MR::isActionEnd(F(LiveActor*, 0xEA0))) {
        F(LiveActor*, 0xEA0)->kill();
        MR::offCalcAnim(F(LiveActor*, 0xEA0));
        return false;
    }
    return true;
}

extern "C" void* fn_803BBA20(MarioActor* self) {
    return getJ3DModel__13MarioAllModelCFv(F(void*, 0xF00));
}

extern "C" void fn_803BBAD0(MarioActor* self) {
    FP(F(void*, 0xF40), u8, 0) = 1;
}

extern "C" void fn_803BBDD0(MarioActor* self, s32 a, s32 b, s32 c) {
    if (self->mMario->isPlayerModeYoshi()) {
        fn_8041BBF0(F(void*, 0xEE4), a, b, c);
    }
}

extern "C" void fn_803BBFD0(MarioActor* self, const char* name) {
    if (name == nullptr) {
        F(void*, 0xFA0) = nullptr;
        return;
    }
    char buf[0x80];
    const char* prefix = "";
    if (self->mMario->isPlayerModeYoshi()) {
        prefix = "Yoshi";
    }
    snprintf(buf, 0x80, "%s%s", prefix, name);
    if (isExistRes__8ResTableCFPCc(FP(MR::getResourceHolder(self), void*, 0x10), buf)) {
        void* r = getRes__8ResTableCFPCc(FP(MR::getResourceHolder(self), void*, 0x10), buf);
        F(void*, 0xFA0) = r;
        FP(r, f32, 8) = 0.0f;
        return;
    }
    F(void*, 0xFA0) = nullptr;
}

extern "C" void fn_803BC440(MarioActor* self, const char* name) {
    F(LiveActor*, 0xFB4)->appear();
    MR::startAction(F(LiveActor*, 0xFB4), name);
    F(s8, 0xFBA) = -2;
}

extern "C" void fn_803BC5C0(MarioActor* self) {
    Mario* m = self->mMario;
    if (!BITAT(m, 0xC, 0x15) && m->getCurrentStatus() == 0) {
        Mario* m2 = self->mMario;
        if (BITAT(m2, 0xC, 0x1E) && FP(m2, u16, 0x964) == 0x20 && fn_803DCB80(F(void*, 0x588))) {
            fn_803DF0F0(F(void*, 0x588));
            F(u8, 0xFBE) = 2;
        }
    } else {
        F(u8, 0xFBE) = 0;
    }
    switch (F(u16, 0x6F8)) {
    case 6:
        self->updateTeresaAnimation();
        break;
    case 4:
        if (F(u8, 0xECD) != 0 && !self->isJumping() && fn_803DCB80(F(void*, 0x588))) {
            fn_803DF1B0(F(void*, 0x588));
        }
        break;
    default:
        FP(self->mMario, s16, 0x414) = 0;
        break;
    }
    u8 t = F(u8, 0xFBE);
    if (t != 0) {
        u8 n = t - 1;
        F(u8, 0xFBE) = n;
        if (n == 0 && BITAT(self->mMario, 0xC, 0x1E) && fn_803DCB80(F(void*, 0x588))) {
            fn_803DF2D0(F(void*, 0x588));
        }
    }
}

extern "C" void setGlobalScale__12MultiEmitterFfl(MultiEmitter* self, f32 scale, s32 n) {
    TVec3f v(scale, scale, scale);
    self->setGlobalScale(v, n);
}

extern "C" void fn_803BDA20(MarioActor* self, MtxPtr dst, const char* joint) {
    PSMTXConcat(MR::getJointMtx(self, joint), F(Mtx, 0x294), dst);
}

extern "C" bool fn_803BDE10(MarioActor* self) {
    if (self->mMario->isPlayerModeYoshi() && fn_8041BD40() == 0) {
        return false;
    }
    if (F(u8, 0xE24) != 0 || F(u16, 0x6B8) != 0) {
        return false;
    }
    if (F(u8, 0xE2C) != 0) {
        return false;
    }
    return self->checkButtonType(*F(u16*, 0x107C), false);
}

extern "C" bool fn_803BE0C0() {
    return false;
}

extern "C" void fn_803BE0D0(MarioActor* self) {
    self->checkButtonType(5, false);
}

extern "C" bool fn_803BE0E0() {
    return false;
}

extern "C" void initFireMario__10MarioActorFv(MarioActor* self) {
    if (MR::isCreatePlayerResourceFire()) {
        self->initFireBall();
        F(u16, 0x6F6) |= 4;
    }
}

extern "C" void initYoshi__10MarioActorFv(MarioActor* self) {
    if (MR::isCreatePlayerResourceYoshi()) {
        void* p = ::operator new(0x50);
        if (p != nullptr) {
            p = __ct__11YoshiHolderFP5Mario(p, self->mMario);
        }
        F(void*, 0xEE4) = p;
        fn_8041B700(self);
        F(u16, 0x6F6) |= 0x400;
    }
}

extern "C" void initCloudMario__10MarioActorFv(MarioActor* self) {
    if (MR::isCreatePlayerResourceCloud()) {
        const char* name = "CloudMario";
        if (gIsLuigi) {
            name = "CloudLuigi";
        }
        ModelHolder* mh = new ModelHolder(name, true);
        F(ModelHolder*, 0xEEC) = mh;
        mh->initWithoutIter();
        registerModel__13MarioAllModelFlP9LiveActorbb(F(void*, 0xF00), 7, F(LiveActor*, 0xEEC), true, true);
        F(LiveActor*, 0xEEC)->kill();
        MR::hideModel(F(LiveActor*, 0xEEC));
        MR::hideJoint(F(LiveActor*, 0xEEC), "Cap0");
        MR::hideJoint(F(LiveActor*, 0xEEC), "Face0");
        void* hat = ::operator new(0xD4);
        if (hat != nullptr) {
            hat = __ct__13CloudMarioHatFPCc(hat, "雲マリオの帽子");
        }
        F(void*, 0xEF0) = hat;
        ((NameObj*)hat)->initWithoutIter();
        F(u16, 0x6F6) |= 0x1000;
    }
}

extern "C" void initRockMario__10MarioActorFv(MarioActor* self) {
    if (MR::isCreatePlayerResourceRock()) {
        const char* name = "RockMario";
        if (gIsLuigi) {
            name = "RockLuigi";
        }
        ModelHolder* mh = new ModelHolder(name, true);
        F(ModelHolder*, 0xEF4) = mh;
        mh->initWithoutIter();
        registerModel__13MarioAllModelFlP9LiveActorbb(F(void*, 0xF00), 8, F(LiveActor*, 0xEF4), true, true);
        F(LiveActor*, 0xEF4)->kill();
        MR::hideModel(F(LiveActor*, 0xEF4));
        MR::hideJoint(F(LiveActor*, 0xEF4), "Face0");
        F(u16, 0x6F6) |= 0x2000;
    }
}

extern "C" void fn_803C8AA0(MarioActor* self) {
    J3DModelX* m = (J3DModelX*)getJ3DModel__13MarioAllModelCFv(F(void*, 0xF00));
    if (F(u8, 0x73) == 0) {
        m->viewCalc3(0, nullptr);
        Mtx inv1, inv2;
        PSMTXInverse(F(Mtx, 0x294), inv1);
        PSMTXInverse(FP(getJ3DModel__13MarioAllModelCFv(F(void*, 0xF00)), Mtx, 0x24), inv2);
        PSMTXConcat(inv1, inv2, F(Mtx, 0x264));
    }
}

extern "C" void swapDrawBuffer__9J3DModelXFUl(J3DModelX* self, u32 a) {
    if (FP(self, u8, 0xDC) == a) {
        FP(self, u8, 0xDC) = 0;
        return;
    }
    FP(self, u8, 0xDC) = (u8)a;
}

extern "C" void fn_803C8E40(MarioActor* self, void* pk, u16 idx, s32 which) {
    void* data = FP(getJ3DModel__13MarioAllModelCFv(F(void*, 0xF00)), void*, 4);
    void* obj = FP(data, void**, 0x28)[idx];
    void* t3 = FP(obj, void*, 0x58);
    if (t3 != nullptr) {
        u16 lo, hi;
        if (which == -1) {
            lo = 0;
            hi = 7;
        } else {
            lo = (u16)which;
            hi = lo;
        }
        s32 off = FP(t3, u16, 0x14) * 4;
        while (lo <= hi) {
            void* mp = FP(pk, u8*, 0xC0) + (lo << 6);
            FP(mp, void*, 0x30) = *(void**)(FP(data, u8*, 0x60) + off);
            void* sp = FP(pk, u8*, 0xC4) + lo * 0x3C;
            FP(mp, void*, 0x28) = sp;
            addShapePacket__12J3DMatPacketFP14J3DShapePacket(mp, sp);
            lo++;
            FP(mp, void*, 0x38) = FP(data, void*, 0x6C);
            FP(mp, void*, 0x20) = FP(*(void**)(FP(data, u8*, 0x60) + off), void*, 0x48);
        }
    }
}

extern "C" bool fn_803B2770(Mario*, TVec3f*, Triangle*, TVec3f*, TVec3f*);

extern "C" bool fn_803C9300(MarioActor* self) {
    bool c = self->mMario->isCeiling();
    if ((c || BITAT(self->mMario, 0x1C, 0xA)) && F(u8, 0xECD) == 0 && F(u16, 0xECE) == 0) {
        Mario* m = self->mMario;
        bool r = false;
        if (BITAT(m, 0x1C, 0xA)) {
            r = true;
        } else if (MR::getWallCodeString(FP(m, Triangle*, 0x4C4)) != nullptr && strcmp(MR::getWallCodeString(FP(m, Triangle*, 0x4C4)), "Fur") == 0) {
            r = true;
        }
        if (r) {
            Triangle tri;
            Mario* m2 = self->mMario;
            TVec3f dir = -F(TVec3f, 0x594);
            dir.scale(200.0f);
            TVec3f hit;
            if (fn_803B2770(m2, &hit, &tri, &self->mPosition, &dir)) {
                self->entryWallWalkMode(hit, *tri.getNormal(0));
            }
        }
    }
    return false;
}

extern "C" void fn_803C9920(MarioActor* self, TVec3f& vec) {
    if (F(u16, 0x6F8) != 4) {
        F(u8, 0xECD) = 0;
        F(u16, 0xECE) = 0;
        return;
    }
    u16 timer = F(u16, 0xECE);
    u8 prev = F(u8, 0xECD);
    if (timer != 0) {
        F(u16, 0xECE) = timer - 1;
    }
    if (F(u16, 0xECE) != 0 || (!fn_803C9300(self) && !self->checkBeeWallStick(vec) && !self->checkBeeFloorStick(vec))) {
        self->updateBeeStickMode(vec);
    }
    u8 cur = F(u8, 0xECD);
    if (cur == 5) {
        MR::vecBlendSphere(&vec, F(TVec3f, 0x594), vec, 0.1f);
        return;
    }
    if (cur != 0) {
        MR::vecBlendSphere(&vec, F(TVec3f, 0x594), vec, 0.5f);
        return;
    }
    if (prev != 0) {
        F(u16, 0xECE) = 0x3C;
    }
}

extern "C" bool fn_803CB1A0(MarioActor*, u16);
extern "C" void fn_803CB2E0(MarioActor*, u16);
extern "C" void fn_803CB510(MarioActor*, u16, bool);
extern "C" void fn_803CBA90(MarioActor*, bool, bool);
extern "C" void fn_803CBB30(MarioActor*, bool);

extern "C" void setPlayerMode__10MarioActorFUsbb(MarioActor* self, u16 mode, bool a, bool b) {
    if (mode != 3) {
        FP(self->mMario, u32, 0x18) &= 0xFFBFFFFF;
    }
    if (!fn_803CB1A0(self, mode)) {
        if (self->mMario->isStatusActive(0x18)) {
            self->mMario->closeStatus(nullptr);
        }
        F(s16, 0x700) = 0;
        F(u8, 0x1B1) = 0;
        fn_803CB2E0(self, mode);
        fn_803CB510(self, mode, a);
        if (F(u16, 0x6F8) == 0) {
            fn_803CBA90(self, a, b);
        } else {
            fn_803CBB30(self, b);
        }
        self->updateHandAtMorph();
    }
}

void J3DModelX::setDynamicDL(u8* dl, u32 size) {
    if (dl == nullptr) {
        size = 0;
    }
    FP(this, u8*, 0x1B8) = dl;
    FP(this, u32, 0x1BC) = size;
}
