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
extern "C" void fn_8004F6D0();

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
    fn_8004F6D0();
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
