// SMG2-only MarioActor functions (part B, fn_803CA000 and up), written from the asm. Included at the end of MarioActor.cpp (unity TU).
// Uses the F()/FP()/BITAT()/CT() helpers defined in MarioActorSMG2_A.inc.


void MarioActor::fn_803CA1F0() {
    F(u8, 0xEF8) = 0;
    F(u8, 0xEF9) = 0;
    F(u8, 0xEFC) = 0;
    F(u8, 0xEFA) = 0;
    F(u8, 0xEFB) = 0;
}

void MarioActor::fn_803CA270(TVec3f* out, s32 isStatic) {
    TVec3f g;
    MR::calcGravityVectorOrZero(this, this->mPosition, &g, mGravityInfo, 0);
    if (isStatic == 0) {
        MR::calcGravityVectorOrZero(this, F(TVec3f, 0x5F4), out, mGravityInfo, 0);
    } else {
        out->set(g);
    }
    TVec3f up(FP(this->mMario, TVec3f, 0x208));
    up.scale(100.0f);
    TVec3f p(this->mPosition);
    p += up;
    TVec3f g2;
    MR::calcGravityVectorOrZero(this, p, &g2, nullptr, 0);
    TVec3f d(g);
    d -= g2;
    F(u8, 0x6A0) = MR::isNearZero(d, 0.001f);
    TVec3f hit;
    if (this->mMario->fn_803AA340(&this->mPosition, &hit)) {
        out->set(hit);
        g = hit;
        F(u8, 0x6A0) = 0;
    }
    MR::normalizeOrZero(out);
    MR::normalizeOrZero(&g);
    if (out->dot(g) < 0.9f) {
        f32 a = this->mMario->getAirGravityVec().dot(*out);
        f32 b = this->mMario->getAirGravityVec().dot(g);
        if (b > a) {
            out->set(g);
        }
    }
    if (BITAT(this->mMario, 0xC, 0x14) && MR::isNearZero(*out, 0.001f)) {
        out->set(this->mMario->getAirGravityVec());
    }
}

bool MarioActor::fn_803CA440(TVec3f& a, TVec3f& b) {
    bool r = false;
    f32 ang = MR::diffAngleAbs(a, b);
    if (MR::isNearZero(a, 0.001f) || MR::isNearZero(b, 0.001f)) {
        ang = 0.0f;
    }
    if (this->mMario->isPlayerModeBee()) {
        if (ang >= 0.5235988f) {
            this->setBlendMtxTimer(8);
        }
    }
    if (ang < 1.2566371f) {
        return false;
    }
    if (this->mMario->isStatusActive(0x27)) {
        return false;
    }
    if (!this->isJumping()) {
        FP(this->mMario, u32, 0x14) |= 0x20;
    } else {
        if (FP(this->mMario, u32, 0x42C) == 0xB) {
            FP(this->mMario, u32, 0x42C) = 0;
        }
        if (F(u8, 0xE24) == 0) {
            if (BITAT(this->mMario, 0xC, 0x14)) {
                this->mMario->changeAnimation("ショートジャンプ", "落下");
            } else {
                this->mMario->changeAnimation("ショートジャンプ", (const char*)nullptr);
            }
        }
        r = true;
    }
    FP(this->mMario, u32, 0x14) &= 0xFFDFFFFF;
    FP(this->mMario, u32, 0xC) &= 0xFFEFFFFF;
    if (F(u8, 0x1010) != 0) {
        r = true;
    }
    F(u16, 0x664) = 0xF;
    F(TVec3f, 0x654) = FP(this->mMario, TVec3f, 0x1F0);
    if (!MR::isNearZero(b, 0.001f)) {
        if (ang >= 2.0943952f || !BITAT(this->mMario, 0x10, 8)) {
            this->mMario->cutGravityElementFromJumpVec(false);
            FP(this->mMario, TVec3f, 0x2D4).scale(0.5f);
        } else {
            TVec3f& jv = FP(this->mMario, TVec3f, 0x2D4);
            f32 k = MR::vecKillElement(jv, *this->mMario->getGravityVec(), &jv);
            jv.scale(0.75f);
            TVec3f g(*this->mMario->getGravityVec());
            g.scale(k);
            TVec3f g2(g);
            g2.scale(0.75f);
            jv += g2;
        }
    }
    this->mMario->clear2DStick();
    FP(this->mMario, u32, 0x14) |= 0x10000000;
    F(u16, 0x6D0) = 0xF;
    if (!BITAT(this->mMario, 0x10, 8)) {
        FP(this->mMario, TVec3f, 0x2E0) = -FP(this->mMario, TVec3f, 0x2E0);
    }
    return r;
}

// NONMATCHING: vector register allocation, local stack slots, and verified literal addresses.
void MarioActor::fn_803CA710(TVec3f& a, TVec3f& b) {
    if (MR::isNearZero(b, 0.001f) || MR::isNearZero(a, 0.001f)) {
        if (!MR::isNearZero(b, 0.001f)) {
            TVec3f n = -b;
            this->calcBaseFrontVec(n);
            return;
        }
        if (MR::isNearZero(b, 0.001f) && MR::isNearZero(a, 0.001f)) {
            this->calcBaseFrontVec(FP(this->mMario, TVec3f, 0x1FC));
        }
    } else {
        TVec3f axis;
        f32 cosv;
        if (MR::makeAxisAndCosignVecToVec(&axis, &cosv, a, b)) {
            const f32 angle = JMAAcosRadian(cosv);
            Mtx m;
            PSMTXRotAxisRad(m, (Vec*)&axis, angle);
            PSMTXMultVecSR(m, (Vec*)&F(TVec3f, 0x5AC), (Vec*)&F(TVec3f, 0x5AC));
            TVec3f cr;
            PSVECCrossProduct((Vec*)&F(TVec3f, 0x5AC), (Vec*)&b, (Vec*)&cr);
            if (MR::normalizeOrZero(&cr)) {
            } else {
                PSVECCrossProduct((Vec*)&b, (Vec*)&cr, (Vec*)&F(TVec3f, 0x5AC));
            }
            MR::normalize(&F(TVec3f, 0x5AC));
        }
    }
}

void MarioActor::fn_803CA860(TVec3f& v) {
    if (!MR::isNearZero(v, 0.001f)) {
        MR::normalize(&v);
    } else {
        Mario* m = this->mMario;
        if (!BITAT(m, 0xC, 0x1E)) {
            v.set(mAppliedGravityVec);
        } else {
            TVec3f n = -FP(m, TVec3f, 0x368);
            v.set(n);
        }
    }
    if (this->mMario->isPlayerModeYoshi() && FP(mYoshiHolder->mYoshi, s32, 0x134) > 0 && F(u8, 0x1053) == 0) {
        WaterInfo info;
        MR::getWaterAreaObj(&info, this->mPosition);
        if (BITAT(this->mMario, 0xC, 0x1E) && !info.isInWater()) {
            TVec3f n = -FP(this->mMario, TVec3f, 0x368);
            v.set(n);
        }
    }
    F(u8, 0x1053) = 0;
}

// NONMATCHING: saved registers, stack slots and equivalent reuse of the 70.0f scalar.
void MarioActor::fn_803CAA00() {
    f32 len = 70.0f;
    Mario* m = this->mMario;
    TVec3f blend;
    if (BITAT(m, 0x1C, 0x10)) {
        MR::vecBlendSphere(&blend, FP(m, TVec3f, 0x1F0), FP(m, TVec3f, 0x380), 0.5f * F(f32, 0xE78));
    } else {
        MR::vecBlendSphere(&blend, FP(m, TVec3f, 0x1F0), FP(m, TVec3f, 0x368), F(f32, 0xE78));
    }
    TVec3f old(F(TVec3f, 0x60C));
    m = this->mMario;
    if (BITAT(m, 0xC, 0x15)) {
        len = 50.0f;
    }
    if (BITAT(m, 0xC, 0x1E)) {
        TVec3f t(blend);
        t.scale(len);
        F(TVec3f, 0x60C) = t;
    } else if (BITAT(m, 0x18, 0x1C) || F(u8, 0xE24) != 0) {
        TVec3f neg = -mAppliedGravityVec;
        TVec3f t(neg);
        t.scale(70.0f);
        if (F(u16, 0x664) != 0 || mMario->calcPolygonAngleD(mMario->mGroundPolygon) >= 59.0f) {
            MR::vecBlend(&F(TVec3f, 0x60C), F(TVec3f, 0x60C), t, 0.1f);
        } else {
            MR::vecBlend(&F(TVec3f, 0x60C), F(TVec3f, 0x60C), t, 0.5f);
        }
    } else {
        TVec3f neg = -mAppliedGravityVec;
        TVec3f t(neg);
        t.scale(70.0f);
        MR::vecBlend(&F(TVec3f, 0x60C), F(TVec3f, 0x60C), t, 0.1f);
    }
    u16 c = F(u16, 0x6D0);
    if (c != 0) {
        F(u16, 0x6D0) = c - 1;
        f32 r = (f32)(u16)(c - 1) / 15.0f;
        TVec3f a(old);
        a.scale(r);
        TVec3f b(F(TVec3f, 0x60C));
        b.scale(1.0f - r);
        TVec3f s(b);
        s += a;
        F(TVec3f, 0x60C) = s;
        FP(this->mMario, u32, 0x14) |= 0x10000000;
    }
    if (F(u16, 0x664) != 0) {
        Mario* m2 = this->mMario;
        if (FP(m2, f32, 0x484) > 160.0f) {
            TVec3f a(FP(m2, TVec3f, 0x1F0));
            a.scale(80.0f);
            TVec3f b(F(TVec3f, 0x654));
            b.scale(80.0f);
            TVec3f c2(b);
            c2 -= a;
            f32 k = MR::vecKillElement(c2, mAppliedGravityVec, &c2);
            TVec3f push(mAppliedGravityVec);
            push.scale(k);
            this->mMario->push2(push);
            F(TVec3f, 0x654) = FP(this->mMario, TVec3f, 0x1F0);
        }
        F(u16, 0x664) = F(u16, 0x664) - 1;
    }
}

#define CTU(off) FP(mConst->getTable(), u16, off)

void MarioActor::fn_803CAD10(const char* name, const char* b, const BckCtrlData* d) {
    const char* bck = b;
    if (F(u8, 0xFB9) == 0) {
        if (F(s32, 0x750) == 0) {
            this->mMario->stopAnimationUpperForce();
        }
        char buf[0x80];
        LiveActor* actor = this;
        if (this->mMario->isPlayerModeYoshi()) {
            actor = mYoshiHolder->mYoshi;
            snprintf(buf, 0x80, "Yoshi%s", name);
        } else if (this->mMario->isPlayerModeTeresa()) {
            actor = mTeresaModel->mActor;
            snprintf(buf, 0x80, "%s", name);
        } else {
            snprintf(buf, 0x80, "%s", name);
        }
        if (MR::getResourceHolder(actor)->mMotionResTable->isExistRes(buf)) {
            if (d == nullptr) {
                MR::startBck(actor, buf, bck);
            } else {
                MR::startBck(actor, buf, nullptr);
                MR::reflectBckCtrlData(actor, *d);
            }
            if (this->mMario->isPlayerModeYoshi()) {
                this->mMarioAnim->fn_803DFBF0()->changeSimpleBck(buf);
                if (d == nullptr) {
                    BckCtrl* c = this->mModelManager->getResourceHolder()->mBckCtrl;
                    if (c != nullptr) {
                        if (bck == nullptr) {
                            bck = buf;
                        }
                        c->changeBckSetting(bck, this->mMarioAnim->fn_803DFBF0());
                    }
                } else {
                    BckCtrlFunction::reflectBckCtrlData(*d, this->mMarioAnim->fn_803DFBF0());
                }
            }
            this->fn_803BBFD0(name);
            this->mMarioAnim->fn_803DC330();
            this->mMarioAnim->fn_803DC1B0(name);
        }
    }
}

bool MarioActor::fn_803CB1A0(u16 mode) {
    if (mPlayerMode != mode) {
        return false;
    }
    if (mode == 1) {
        F(u16, 0x700) = CTU(0x78C);
        MR::startSubBGM("BGM_MUTEKI_A", false);
    } else if (mode == 9) {
        F(u16, 0x700) = CTU(0x78A);
    } else if (mode == 3) {
        if (BITAT(this->mMario, 0x18, 0x16)) {
            F(u16, 0x700) = 0;
        } else {
            F(u16, 0x700) = CTU(0x790);
        }
        MR::startSubBGM("BGM_ICE_A", false);
    } else if (mode == 2) {
        F(u16, 0x700) = CTU(0x78E);
        MR::startPlayFireMorphBGM();
    } else if (mode == 7) {
        F(u16, 0x700) = 0xE10;
        MR::startStageBGM("BGM_FLYING_A", false);
    } else {
        this->playEffect("アイテム再ゲット");
    }
    return true;
}

void MarioActor::fn_803CB2E0(u16 newMode) {
    switch (mPlayerMode) {
    case 5:
        this->mMario->endRabbitMode();
        MR::startBtp(this, "ColorChange");
        MR::setBtpFrameAndStop(this, 0.0f);
        break;
    case 6:
        MR::deleteEffect(mTeresaModel->mActor, "Glow");
        if (newMode == 0) {
            MR::offCalcAnim(mTeresaModel->mActor);
        } else {
            (mTeresaModel->mActor)->kill();
            F(u8, 0x76B) = 0;
        }
        this->mMario->resetTeresaMode();
        break;
    case 2:
        MR::startBtp(this, "ColorChange");
        MR::setBtpFrameAndStop(this, 0.0f);
        MR::fn_8001C280();
        break;
    case 4:
        this->mMario->stopAnimation("ハチ飛行中", (const char*)nullptr);
        if (!this->mMario->isSwimming()) {
            MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();
        }
        F(LiveActor*, 0xEC0)->kill();
        F(LiveActor*, 0xEC4)->kill();
        break;
    case 3:
        this->stopEffect("アイス中");
        if (this->mMario->isStatusActive(0x1F)) {
            this->mMario->closeStatus(nullptr);
        }
        MR::stopSubBGM(30);
        break;
    case 1:
        this->stopEffect("無敵中");
        F(u8, 0x1B1) = 0;
        MR::stopSubBGM(30);
        break;
    case 7:
        MR::startCurrentStageBGM(false);
        break;
    case 10:
        if (this->mMario->isStatusActive(0x12)) {
            this->mMario->closeStatus(nullptr);
        }
        if (gIsLuigi) {
            FP(F(void*, 0x590), u32, 0xC) = 1;
        } else {
            FP(F(void*, 0x590), u32, 0xC) = 0;
        }
        MR::endStarPointerMode(this);
        if (!mYoshiHolder->mYoshi->fn_8040F0E0()) {
            this->mMario->fn_803B1180();
        }
        break;
    case 12:
        MR::resetSpinCloudBlocks();
        this->stopEffect("雲マリオ頭煙");
        break;
    }
    F(u16, 0x6FA) = mPlayerMode;
}

namespace MR {
    void removeAllClingingKarikari();
    bool isSandyBossBGMPlaying();
    void adjustSandyBossFireMorphBGM(s32, s32);
    void endStarPointerMode(void*);
    void setPlayerFrontVec(const TVec3f&, s32);
}

void MarioActor::fn_803CB510(u16 mode, bool a) {
    mPlayerMode = mode;
    this->fn_803BA070();
    HitSensor* s = _4A4;
    if (s != nullptr) {
        s->receiveMessage(0x8F, this->getSensor("Body"));
        _4A4 = nullptr;
    }
    mSensor794 = nullptr;
    switch (mode) {
    case 9:
        F(u16, 0x700) = CTU(0x78A);
        MR::startBtp(this, "TornadoElement");
        if (this->isJumping()) {
            this->mMario->changeAnimation("エレメントゲット", (const char*)nullptr);
        } else {
            this->mMario->changeAnimation("エレメントゲット接地中", (const char*)nullptr);
        }
        break;
    case 1:
        F(u16, 0x700) = CTU(0x78C);
        this->playEffect("無敵中");
        MR::startSubBGM("BGM_MUTEKI_A", false);
        MR::stopBrk(F(LiveActor*, 0xEA4));
        MR::stopBrk(F(LiveActor*, 0xF2C));
        MR::stopBrk(F(LiveActor*, 0xF30));
        MR::startBrk(F(LiveActor*, 0xEA4), "InvincibleMario");
        MR::startBrk(F(LiveActor*, 0xF2C), "InvincibleMarioHandL");
        MR::startBrk(F(LiveActor*, 0xF30), "InvincibleMarioHandR");
        MR::removeAllClingingKarikari();
        break;
    case 0:
        MR::startBtp(this, "ElementEnd");
        if (a) {
            u16 prev = F(u16, 0x6FA);
            if ((u16)(prev - 4) <= 2) {
                this->mMario->playSound("呪い解除", -1);
            } else if (prev == 10) {
            } else if (prev == 2) {
                if (MR::isSandyBossBGMPlaying()) {
                    this->mMario->playSound("変身解除ＢＧＭ維持", -1);
                } else {
                    this->mMario->playSound("変身解除", -1);
                }
            } else {
                this->mMario->playSound("変身解除", -1);
            }
        }
        F(u16, 0x700) = 0;
        FP(this->mMario, u16, 0x764) = 0;
        if (this->mMario->isSwimming()) {
            if (F(s32, 0x750) == 0) {
                this->mMario->changeAnimation((const char*)nullptr, "水泳基本");
            }
        } else if (this->isJumping()) {
            this->mMario->changeAnimation((const char*)nullptr, "落下");
        } else {
            this->mMario->changeAnimation((const char*)nullptr, "基本");
        }
        break;
    case 5:
        this->mMario->startRabbitMode();
        this->rushDropThrowMemoSensor();
        break;
    case 6:
        (mTeresaModel->mActor)->appear();
        MR::onCalcAnim(mTeresaModel->mActor);
        this->runTeresaBaseAnimation();
        MR::startBrk(mTeresaModel->mActor, "Erase");
        MR::setBrkFrameAndStop(mTeresaModel->mActor, 0.0f);
        mTeresaModel->_4 = 0.0f;
        mTeresaModel->_8 = 0.0f;
        F(u8, 0x76B) = 1;
        this->rushDropThrowMemoSensor();
        this->mMario->startTeresaMode();
        (mTeresaModel->mActor)->calcAnim();
        MR::emitEffect(mTeresaModel->mActor, "Glow");
        MR::emitEffect(mTeresaModel->mActor, "Appear");
        break;
    case 2:
        MR::startPlayFireMorphBGM();
        F(u16, 0x700) = CTU(0x78E);
        break;
    case 3:
        this->playEffect("アイス中");
        MR::startSubBGM("BGM_ICE_A", false);
        if (BITAT(this->mMario, 0x18, 0x16)) {
            F(u16, 0x700) = 0;
        } else {
            F(u16, 0x700) = CTU(0x790);
        }
        break;
    case 4:
        MR::getGameSceneLayoutHolder()->changeLifeMeterModeBee();
        this->rushDropThrowMemoSensor();
        F(LiveActor*, 0xEC0)->appear();
        MR::showModel(F(LiveActor*, 0xEC0));
        FP(this->mMario, u16, 0x402) = CTU(0xD8);
        FP(this->mMario, u16, 0x426) = 0;
        F(LiveActor*, 0xEC4)->appear();
        F(u8, 0xECC) = 0;
        MR::startAction(F(LiveActor*, 0xEC4), "Wait");
        MR::startBva(F(LiveActor*, 0xEC4), "Wait");
        MR::stopBtk(F(LiveActor*, 0xEC4));
        break;
    case 7:
        MR::startStageBGM("BGM_FLYING_A", false);
        F(u16, 0x700) = 3600;
        break;
    case 10: {
        FP(F(void*, 0x590), u32, 0xC) = 2;
        LiveActor* ya = mYoshiHolder->mYoshi;
        MR::setPlayerPos(ya->mPosition);
        MtxPtr bm = ya->getBaseMtx();
        TVec3f front(bm[0][2], bm[1][2], bm[2][2]);
        MR::setPlayerFrontVec(front, 1);
        mYoshiHolder->mTongueAction->fn_8041FCD0();
        FP(mYoshiHolder->_10, HitSensor*, 0x14) = this->getSensor("Body");
        this->mMarioAnim->fn_803DF8E0();
        this->rushDropThrowMemoSensor();
        if (FP(mYoshiHolder->mYoshi, u8, 0x131) != 0) {
            this->mMario->changeAnimation("ヨッシースピン乗車", "基本");
        } else {
            this->mMario->changeAnimation("ヨッシー乗車", "基本");
        }
        this->mMario->stopWalk();
        F(u16, 0x700) = 0;
        this->resetCondition();
        this->setPunchHitTimer(0);
        MR::fn_8005E430(this);
        break;
    }
    case 12:
        if (MR::isOnGameEventFlagCloudMarioAtFirst()) {
            MR::startSystemSE("SE_SY_POWER_UP", -1, -1);
        } else {
            MR::startSystemSE("SE_SY_POWER_UP_2", -1, -1);
        }
        this->playEffect("雲マリオ頭煙");
        break;
    }
}

void MarioActor::fn_803CBA90(bool a, bool b) {
    if (F(u8, 0x516) != 0) {
        MR::endDemo(this, "マリオ変身");
        F(u8, 0x516) = 0;
    }
    if (a) {
        F(u16, 0x6FC) = 0x1E;
        this->mMario->startPadVib("マリオ[変身解除]");
    } else {
        F(u16, 0x6FC) = 2;
    }
    if (b) {
        this->playEffect("変身解除");
    }
}

void MarioActor::fn_803CBB30(bool b) {
    if (F(u16, 0x6FC) == 0 || F(u16, 0x6FA) == 0) {
        F(u8, 0x515) = 1;
    } else {
        if (mPlayerMode == 0xA) {
            F(u16, 0x6FC) = 0x13;
            this->mMario->clearVelocity();
            FP(this->mMario, f32, 0x278) = 0.0f;
        } else {
            F(u16, 0x6FC) = 0x40;
            MR::stopAnimFrame(this);
            if (b) {
                this->playEffect("変身");
                this->mMario->startPadVib("マリオ[変身]");
            }
        }
        F(u8, 0x515) = 0;
    }
    if (!b) {
        F(u16, 0x6FC) = 2;
        F(u8, 0x515) = 0;
    }
    F(u8, 0xE3D) = 0;
    F(s16, 0xE3E) = 0;
    F(f32, 0x9C4) = 0.0f;
    F(f32, 0x9C8) = 0.0f;
    for (int i = 0; i < 0x80; i++) {
        FP(this, f32, 0x9CC + i * 4) = 0.0f;
    }
    F(s16, 0xE42) = 0;
    F(s16, 0xE44) = 0;
    this->updateFairyStar();
}

void MarioActor::fn_803CBC50() {
    if (mPlayerMode == 2) {
        MR::adjustSandyBossFireMorphBGM(F(u16, 0x700), CTU(0x78E));
    }
    u16 t = F(u16, 0x700);
    if (t != 0) {
        F(u16, 0x700) = t - 1;
        if (F(u16, 0x700) == 0) {
            this->setPlayerMode(0, true, true);
        }
    }
    if (MR::isInAreaObj("PlayerMorphLimitArea", this->mPosition)) {
        F(u8, 0x1065) = 1;
        return;
    }
    if (F(u8, 0x1065) != 0) {
        F(u8, 0x1065) = 0;
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803CCE60(HitSensor* a, s16 b) {
    this->mMarioAnim->fn_803DDD80(a);
    if (!this->mMario->isSwimming()) {
        if (BITAT(this->mMario, 0xC, 0x1E) && F(s8, 0xFBA) != -3) {
            F(s16, 0x6B8) = b;
        }
        F(TVec3f, 0x38).zero();
        FP(this->mMario, f32, 0x278) = 0.0f;
        if (BITAT(this->mMario, 0xC, 0x1E)) {
            this->mMario->stopJump();
            FP(this->mMario, f32, 0x484) = 0.0f;
        }
    }
    _424 = a;
    F(u8, 0x768) = 1;
}

void MarioActor::fn_803CCF20(HitSensor* s) {
    HitSensor* old = _4A4;
    if (old != nullptr) {
        old->receiveMessage(0x8F, this->getSensor("Body"));
    }
    _4A4 = s;
    mSensor794 = nullptr;
}

void MarioActor::fn_803CCF90(HitSensor* sensor) {
    HitSensor* old = _4A4;
    if (old != nullptr) {
        old->receiveMessage(0x8F, this->getSensor("Body"));
    }
    _4A4 = nullptr;
    mSensor794 = sensor;
}


bool MarioActor::fn_803CD000() {
    if (F(u8, 0xE24) != 0) {
        return false;
    }
    if (F(s32, 0x6B0) == 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (mActionTable1080->isActionOk("カメ持ち")) {
        return F(s32, 0x750) == 0;
    }
    return false;
}

bool MarioActor::fn_803CD0A0() {
    if (F(u8, 0xE24) != 0) {
        return false;
    }
    if (F(s32, 0x6B0) == 0) {
        return false;
    }
    if (F(u16, 0x6D2) != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (!mActionTable1080->isActionOk("カメ持ち")) {
        return false;
    }
    if (F(s32, 0x750) != 0) {
        return false;
    }
    if (BITAT(this->mMario, 0xC, 0x14) && this->fn_803CD180()) {
        return false;
    }
    return true;
}

bool MarioActor::fn_803CD180() {
    bool r = false;
    if (!this->mMario->isSwimming() && !BITAT(this->mMario, 0xC, 0x1E) && _424 == nullptr) {
        r = true;
    }
    return r;
}

f32 MarioActor::fn_803CD250(HitSensor* s) {
    TVec3f a(FP(this->mMario, TVec3f, 0x31C));
    a -= FP(s, TVec3f, 4);
    TVec3f b;
    return MR::vecKillElement(a, *this->mMario->getGravityVec(), &b);
}

bool MarioActor::fn_803CD1F0(HitSensor* s) {
    if (this->fn_803CD250(s) > 100.0f) {
        return false;
    }
    return MR::isNearZero(MR::getSensorHost(s)->mVelocity, 0.001f);
}

bool MarioActor::fn_803CD2C0() {
    if (F(u16, 0x6D2) != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (!mActionTable1080->isActionOk("カメ持ち")) {
        return false;
    }
    if (F(s32, 0x750) != 0) {
        return false;
    }
    if (BITAT(this->mMario, 0xC, 0x14) && this->fn_803CD180()) {
        return false;
    }
    return true;
}

bool MarioActor::fn_803CD380() {
    if (F(s32, 0x6B0) == 0) {
        return false;
    }
    if (F(u16, 0x6FC) != 0) {
        return false;
    }
    return FP(this->mMario, u16, 0x412) == 0;
}

bool MarioActor::fn_803CD3C0() {
    if (F(u8, 0xE24) != 0) {
        return false;
    }
    if (F(s32, 0x6B0) == 0) {
        return false;
    }
    if (F(u16, 0x6D2) != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    return F(s32, 0x750) == 0;
}

void MarioActor::fn_803CD450(HitSensor* s) {
    if (this->fn_803CD1F0(s) && this->fn_803CD180()) {
        this->doTrampleJump(s);
    }
}


bool MarioActor::fn_803CD4B0(HitSensor* s, f32 f0, f32 f1) {
    f32 r = FP(s, f32, 0x10);
    TVec3f d(FP(s, TVec3f, 4));
    d -= F(TVec3f, 0x5F4);
    void* h = FP(s, void*, 0x24);
    if (h != nullptr && MR::isEqualString(FP(h, const char*, 4), "ヨッシー")) {
        return this->fn_803CD8A0(&d, r, 40.0f, 10.0f);
    }
    return this->cylinderPushCheck(d, r, f0, f1);
}

void MarioActor::sendWallTouch(u32 msg, HitSensor* a, HitSensor* b) {
    if (a != nullptr) {
        if (!MR::isDead(MR::getSensorHost(a))) {
            a->receiveMessage(msg, b);
            if (BITAT(this->mMario, 0xC, 0x10)) {
                a->receiveMessage(0x3A, b);
            }
        }
    }
}

bool MarioActor::fn_803CD8A0(TVec3f* v, f32 f0, f32 f1, f32 f2) {
    TVec3f p(*v);
    if (BITAT(this->mMario, 0x14, 0x19)) {
        f32 k = MR::vecKillElement(p, F(TVec3f, 0x1038), &p);
        if (k == 250.0f || k == -250.0f) {
            return false;
        }
    }
    TVec3f perp;
    f32 d = MR::vecKillElement(p, F(TVec3f, 0x7B4), &perp);
    if (d > -f0) {
        if (d < f2 && PSVECMag((Vec*)&perp) < (f1 + f0)) {
            HitSensor* hs = this->getSensor("dummy");
            TVec3f t(F(TVec3f, 0x7B4));
            t.scale(d);
            TVec3f q(F(TVec3f, 0x5F4));
            q += t;
            FP(hs, TVec3f, 4).set(q);
            FP(this->getSensor("dummy"), f32, 0x10) = f1;
            return true;
        }
        if (d == f2) {
            TVec3f t(F(TVec3f, 0x7B4));
            t.scale(f2);
            TVec3f e(p);
            e -= t;
            if (PSVECMag((Vec*)&e) < (f0 + f1)) {
                HitSensor* hs = this->getSensor("dummy");
                TVec3f t2(F(TVec3f, 0x7B4));
                t2.scale(f2);
                TVec3f q(F(TVec3f, 0x5F4));
                q += t2;
                FP(hs, TVec3f, 4).set(q);
                FP(this->getSensor("dummy"), f32, 0x10) = f1;
                return true;
            }
        }
    }
    Mario* m = this->mMario;
    if (BITAT(m, 0xC, 0x15) && BITAT(m, 0xC, 0x1E)) {
        return false;
    }
    f32 d2 = MR::vecKillElement(p, F(TVec3f, 0x7A8), &perp);
    if (d2 > -f0) {
        if (d2 < f2 && PSVECMag((Vec*)&perp) < (f1 + f0)) {
            HitSensor* hs = this->getSensor("dummy");
            TVec3f t(F(TVec3f, 0x7A8));
            t.scale(d2);
            TVec3f q(F(TVec3f, 0x5F4));
            q += t;
            FP(hs, TVec3f, 4).set(q);
            FP(this->getSensor("dummy"), f32, 0x10) = f1;
            return true;
        }
        if (d2 == f2) {
            TVec3f t(F(TVec3f, 0x7A8));
            t.scale(f2);
            TVec3f e(p);
            e -= t;
            if (PSVECMag((Vec*)&e) < (f0 + f1)) {
                HitSensor* hs = this->getSensor("dummy");
                TVec3f t2(F(TVec3f, 0x7A8));
                t2.scale(f2);
                TVec3f q(F(TVec3f, 0x5F4));
                q += t2;
                FP(hs, TVec3f, 4).set(q);
                FP(this->getSensor("dummy"), f32, 0x10) = f1;
                return true;
            }
        }
    }
    return false;
}

namespace MR {
    void explainDrillIfAtFirst();
}

void MarioActor::fn_803CE8A0(HitSensor* s) {
    if (F(u8, 0x768) != 0) {
        if (s == nullptr) {
            F(u8, 0x768) = 0;
            return;
        }
        bool drill = false;
        if (MR::getSensorHost(s) != nullptr && MR::isEqualString(MR::getSensorHost(s)->mName, "アイテムドリル")) {
            drill = true;
        }
        if (drill) {
            if (this->mMario->isAnimationTerminate(nullptr)) {
                F(u8, 0x768) = 0;
                MR::explainDrillIfAtFirst();
            }
        } else {
            u32 t = FP(s, u32, 0);
            if (t - 0x12 > 1) {
                if (t == 0x1D && this->mMario->isAnimationTerminate("カブ抜き")) {
                    this->mMario->stopAnimation((const char*)nullptr, (const char*)nullptr);
                    F(u8, 0x768) = 0;
                    this->mMario->changeAnimationUpper("カブウエイト", (const char*)nullptr);
                }
            } else if (this->mMario->isAnimationTerminate(nullptr)) {
                F(u8, 0x768) = 0;
            }
        }
    }
}


void MarioActor::fn_803CE9D0(HitSensor* s) {
    if (F(s32, 0x1054) == 0) {
        _494->calc();
        TVec3f t, r;
        FP(_494, TPos3f, 0x1C).getTrans(t);
        _494->copyRotate(&r);
        if (s != nullptr) {
            if (this->fn_803CEE50()) {
                mMarioAnim->updateTakingAnimation(s);
            }
            MR::getSensorHost(s)->mPosition = t;
            MR::getSensorHost(s)->mRotation = r;
        }
    }
}

void MarioActor::fn_803CEA80(HitSensor* s, f32 t) {
    if (F(s32, 0x1054) == 0) {
        Mtx tmp;
        PSMTXConcat(this->getBaseMtx(), F(Mtx, 0x4A8), tmp);
        PSMTXCopy(tmp, FP(MR::getJ3DModel(F(LiveActor*, 0xFB4)), Mtx, 0x24));
        TVec3f pos, rot;
        if (mNullAnimation->getFramePos(t, &pos, &rot)) {
            F(s8, 0xFBA) = -3;
            F(LiveActor*, 0xFB4)->kill();
            if (s == nullptr) {
                F(u8, 0x768) = 0;
                F(s8, 0xFBA) = 0;
                F(LiveActor*, 0xFB4)->kill();
                return;
            }
            MR::sendArbitraryMsg(50, s, this->getSensor("Body"));
        }
        if (s != nullptr) {
            MR::getSensorHost(s)->mPosition = pos;
            MR::getSensorHost(s)->mRotation = rot;
        }
    }
}

void MarioActor::fn_803CEBC0() {
    if (F(s32, 0x750) != 0) {
        HitSensor* cs = (HitSensor*)this->getCarrySensor();
        bool drill = false;
        if (MR::getSensorHost(cs) != nullptr && MR::isEqualString(MR::getSensorHost(cs)->mName, "アイテムドリル")) {
            drill = true;
        }
        if (drill) {
            if (F(u8, 0xE24) == 0 && F(s32, 0x1054) == 0) {
                _494->calc();
                TVec3f t;
                FP(_494, TPos3f, 0x1C).getTrans(t);
                TVec3f p(t);
                p += F(TVec3f, 0x38);
                MR::getSensorHost(cs)->mPosition = p;
                _494->copyRotate(&MR::getSensorHost(cs)->mRotation);
            }
        } else {
            if (cs->isType(0x48) || cs->isType(0x12) || cs->isType(0x13) || cs->isType(0x1D)) {
                _494->calc();
                TVec3f t;
                FP(_494, TPos3f, 0x1C).getTrans(t);
                TVec3f p(t);
                p += F(TVec3f, 0x38);
                MR::getSensorHost(cs)->mPosition = p;
                _494->copyRotate(&MR::getSensorHost(cs)->mRotation);
                return;
            }
            TVec3f lp;
            mNullAnimation->getLastPos(&lp);
            PSMTXMultVec(F(Mtx, 0x4A8), (Vec*)&lp, (Vec*)&lp);
            PSMTXMultVec(this->getBaseMtx(), (Vec*)&lp, (Vec*)&lp);
            f32 sp = FP(this->mMario, f32, 0x278);
            f32 amp = 10.0f * (sp * sp);
            f32 sn = JMASinRadian(F(f32, 0x778));
            F(f32, 0x778) += MR::getRandom(0.1f, 1.0f);
            lp.y += amp * sn;
            MR::getSensorHost(cs)->mPosition = lp;
            TVec3f r(F(TVec3f, 0x720));
            r += F(TVec3f, 0x20);
            MR::getSensorHost(cs)->mRotation = r;
        }
    }
}

bool MarioActor::fn_803CEE50() {
    bool r = true;
    bool a = true;
    if (!this->mMario->isAnimationTerminate(nullptr) && this->mMario->isAnimationRun(nullptr)) {
        a = false;
    }
    if (!a && !this->mMario->fn_8038CCF0()) {
        r = false;
    }
    return r;
}

s32 MarioActor::fn_803CEEE0(HitSensor* s) {
    bool r = false;
    if (MR::getSensorHost(s) != nullptr && MR::isEqualString(MR::getSensorHost(s)->mName, "アイテムドリル")) {
        r = true;
    }
    return r;
}

void MarioActor::fn_803CEF50(LiveActor* a) {
    mSensorSlots108C->fn_803DAB10(a);
}

void MarioActor::fn_803CF130(MtxPtr mtx) {
    TVec3f tmp;
    f32 k = MR::vecKillElement(F(TVec3f, 0x38), *this->mMario->getGravityVec(), &tmp);
    f32 v = MR::clamp(k / 10.0f, -1.0f, 1.0f);
    if (!MR::isNearZero(this->mMario->getWorldPadDir(), 0.001f)) {
        Mario* m = this->mMario;
        const TVec3f* g = m->getGravityVec();
        if (MR::diffAngleAbsHorizontal(m->getWorldPadDir(), FP(m, TVec3f, 0x208), *g) > 0.3926991f) {
            v = 0.0f;
        }
    }
    mTeresaModel->_8 = 0.02f * v + 0.98f * mTeresaModel->_8;
    f32 var = CT(0x700);
    if (mTeresaModel->_8 > 0.0f) {
        var = CT(0x6FC);
    }
    PSMTXConcat(mtx, MR::tmpMtxRotXRad(var * mTeresaModel->_8), mtx);
}

void MarioActor::updateFireSpin() {
    if (!this->mMario->isAnimationRun("ファイア投げ") && !this->mMario->isAnimationRun("ファイアスピン空中") &&
        (!this->mMario->isAnimationRun("ファイアスピン") || !this->fn_803CB010())) {
        if (BITAT(this->mMario, 0xC, 0x17)) {
            const Triangle* tri = this->mMario->getWallPolygon();
            LiveActor* h = MR::getSensorHost(FP(tri, HitSensor*, 8));
            if (!MR::isEqualString(h->mName, "木箱") && !MR::isEqualString(h->mName, "タイムアタック用木箱")) {
                HitSensor* ws = FP(this->mMario->getWallPolygon(), HitSensor*, 8);
                ws->receiveMessage(0xB, this->getSensor("Body"));
                this->mMario->changeAnimation("ファイアスピン", (const char*)nullptr);
                return;
            }
        }
        F(s32, 0xF94) = 7;
        this->mMario->playSound("声投げ", -1);
        if (!this->isJumping()) {
            this->mMario->stopAnimation("ファイアスピン", (const char*)nullptr);
            this->mMario->stopAnimationUpper("ファイアスピン", (const char*)nullptr);
            this->mMario->changeAnimation("ファイアスピン", (const char*)nullptr);
            FP(this->mMario, f32, 0x278) = 0.0f;
            return;
        }
        this->mMario->setFrontVecKeepUp(FP(this->mMario, TVec3f, 0x1CC));
        Mario* m = this->mMario;
        u16 c = FP(m, u16, 0x428);
        if (c >= 3) {
            m->changeAnimation("ファイア投げ", (const char*)nullptr);
            return;
        }
        if (c == 0) {
            m->changeAnimation("ファイアスピン空中", (const char*)nullptr);
        } else {
            m->changeAnimation("ファイア投げ", (const char*)nullptr);
        }
        this->jumpHop();
        FP(this->mMario, u16, 0x428) += 1;
        f32 k = this->mMario->cutGravityElementFromJumpVec(true);
        FP(this->mMario, TVec3f, 0x2D4).scale(0.5f);
        TVec3f g(mAppliedGravityVec);
        g.scale(k);
        FP(this->mMario, TVec3f, 0x2D4) += g;
    }
}

void MarioActor::killAllFireBall() {
    int i = 0;
    int off = 0;
    do {
        if (!MR::isDead(FP(this, LiveActor*, 0xF80 + off))) {
            FP(this, LiveActor*, 0xF80 + off)->kill();
        }
        i++;
        off += 4;
    } while (i < 5);
}


bool MarioActor::fn_803CFCA0(HitSensor* s) {
    if (!this->mMario->isPlayerModeYoshi()) {
        return false;
    }
    TVec3f d(*MR::getSensorPos(s));
    d -= this->mPosition;
    const TVec3f* g = &this->mMario->getAirGravityVec();
    TVec3f dd;
    JMAVECScaleAdd((Vec*)g, (Vec*)&d, (Vec*)&d, -g->dot(d));
    if (MR::normalizeOrZero(&d)) {
        return false;
    }
    f32 c = JMath::sSinCosTable.cosDegree(YoshiParam::fn_8041BDC0());
    if (d.dot(FP(this->mMario, TVec3f, 0x208)) > c) {
        return false;
    }
    return true;
}




// NONMATCHING: sensor-loop register allocation and verified constant-pool addresses.
HitSensor* MarioActor::fn_803D0CE0(s32 a) {
    if (F(s32, 0x6B0) == 0 || F(s32, 0xE20) == 0) {
        return nullptr;
    }
    f32 best = 100000.0f;
    HitSensor* res = nullptr;
    for (u32 i = 0; i < _930; ++i) {
        HitSensor* cur = _7E4[i];
        if (isFixJumpRushSensor(cur) || (a != 0 && !MR::isSensorAutoRush(cur))) {
            continue;
        }
        if (getCarrySensor() != nullptr) {
            LiveActor* host = MR::getSensorHost(cur);
            if (MR::getSensorHost(getCarrySensor()) == host) {
                continue;
            }
        }
        const f32 distance = fn_803D1470(cur);
        if ((mRushTouchingDD4[i] || a == 0 || !(20.0f + cur->mRadius < distance)) && distance < best) {
            res = cur;
            best = distance;
        }
    }
    return res;
}

HitSensor* MarioActor::fn_803D0E40(s32 a) {
    HitSensor* t = (HitSensor*)this->getHighPriorityTarget();
    if (t == nullptr) {
        t = this->getRescueTarget();
    }
    if (t != nullptr) {
        if (a != 0) {
            if (MR::getSensorRadius(t) < this->fn_803D1470(t)) {
                return nullptr;
            }
        }
        return t;
    }
    if (!this->mMario->isStateHanging()) {
        if (!this->mMario->isDamaging()) {
            u32 i = 0;
            s32 off = 0;
            while (i < (u32)F(s32, 0xE20)) {
                HitSensor* cur = _7E4[i];
                if (mActionTable1080->testEffect(MR::getSensorHost(cur)->mName, "HD中ラッシュ")) {
                    return cur;
                }
                i++;
                off += 4;
            }
        }
    }
    return nullptr;
}

bool MarioActor::fn_803D0F60(HitSensor* s) {
    Mario* m = this->mMario;
    if (BITAT(m, 0xC, 9)) {
        return false;
    }
    if (s == nullptr) {
        return false;
    }
    if (F(s32, 0x6B0) == 0) {
        return false;
    }
    if (m->isStatusActive(0x13)) {
        return false;
    }
    if (this->mMario->isStatusActive(0x1B)) {
        return false;
    }
    if (!s->receiveMessage(0x94, this->getSensor("Body"))) {
        return false;
    }
    F(TVec3f, 0x38).zero();
    this->beginRush(s);
    return true;
}

bool MarioActor::fn_803D11C0() {
    Mario* m = this->mMario;
    if (BITAT(m, 0xC, 9)) {
        return true;
    }
    if (F(s32, 0x6B0) == 0) {
        return true;
    }
    if (m->fn_8038CCF0()) {
        return true;
    }
    if (this->mMario->isStatusActive(0x13)) {
        return true;
    }
    if (this->mMario->isStatusActive(0x1B)) {
        return true;
    }
    return !this->fn_803CB0B0();
}

f32 MarioActor::fn_803D1470(HitSensor* s) {
    TVec3f p(*MR::getSensorPos(s));
    TVec3f a(p);
    a -= this->mPosition;
    f32 da = PSVECMag((Vec*)&a);
    TVec3f b(p);
    b -= F(TVec3f, 0x5F4);
    f32 db = PSVECMag((Vec*)&b);
    TVec3f c(p);
    c -= F(TVec3f, 0x600);
    f32 dc = PSVECMag((Vec*)&c);
    if (da > db) {
        da = db;
    }
    if (da > dc) {
        da = dc;
    }
    return da;
}

void MarioActor::fn_803D1740(HitSensor* s) {
    if (this->mMario->isStatusActive(5)) {
        Mtx m;
        this->getRealMtx(m, "Spine1");
        MR::extractMtxTrans(m, &FP(s, TVec3f, 4));
        FP(s, f32, 0x10) = 60.0f;
        return;
    }
    if (this->mMario->isStatusActive(6)) {
        Mtx m;
        this->getRealMtx(m, "Spine1");
        MR::extractMtxTrans(m, &FP(s, TVec3f, 4));
        FP(s, f32, 0x10) = 70.0f;
        return;
    }
    FP(s, TVec3f, 4).set(F(TVec3f, 0x5F4));
    Mario* m = this->mMario;
    u32 st = FP(m, u32, 0xC);
    if (((st >> 0x14) & 1) && !((st >> 0x1E) & 1)) {
        FP(s, TVec3f, 4) += FP(m, TVec3f, 0x2D4);
    }
    FP(s, f32, 0x10) = 100.0f;
}


void MarioActor::fn_803D1830(HitSensor* s) {
    FP(s, TVec3f, 4).set(F(TVec3f, 0x5F4));
    FP(s, f32, 0x10) = this->fn_803D18D0();
    this->fn_803D1940();
    bool r = false;
    if (FP(s, u8, 0x21) != 0 && FP(s, u8, 0x20) != 0) {
        r = true;
    }
    if (r) {
        this->attackOrPushPolygons();
    }
    HitSensor* t = _424;
    if (t != nullptr) {
        this->tryTornadoPull(t);
    }
}

f32 MarioActor::fn_803D18D0() {
    if (this->mMario->isSwimming()) {
        return 2000.0f;
    }
    if (F(s32, 0x750) != 0) {
        return 2000.0f;
    }
    if (BITAT(this->mMario, 0xC, 0x10)) {
        return 1000.0f;
    }
    return 600.0f;
}

void MarioActor::fn_803D1940() {
    F(u8, 0x704) = 0;
    F(u8, 0x705) = 0;
    const char* cur = this->mMarioAnim->fn_803DF0D0();
    if (strcmp(cur, "spin2nd") == 0) {
        F(u8, 0x704) = 1;
        F(u8, 0x705) = 1;
        if (F(u8, 0xE3C) <= 2) {
            this->setPunchHitTimer(2);
        }
    } else if (!this->mMario->isAnimationTerminate(nullptr) &&
               ((FP(this->mMario, u32, 0x42C) == 8 && this->isJumping()) || this->mMario->isAnimationRun("地上ひねり") ||
                this->mMario->isAnimationRun("雲スピン") || this->mMario->isAnimationRun("サマーソルト") ||
                this->mMario->isAnimationRun("水泳スピン") || this->mMario->isAnimationRun("水上スピン") ||
                this->mMario->isAnimationRun("しゃがみスピン") || this->mMario->isAnimationRun("フーファイタースピン") ||
                this->mMario->isAnimationRun("ハチスピン") || this->mMario->isAnimationRun("ハチスピン空中") ||
                this->mMario->isAnimationRun("アイスひねり空中") || this->mMario->isAnimationRun("ファイアスピン空中") ||
                this->mMario->isAnimationRun("ファイアスピン") || this->mMario->isAnimationRun("アイスひねり") ||
                this->mMario->isAnimationRun("アイスひねり移動") || this->mMario->isAnimationRun("アイスひねり静止") ||
                this->mMario->isAnimationRun("ハンマー投げリリース") ||
                (this->mMario->isSwimming() && this->mMario->mSwim->fn_803FE9F0()) ||
                (this->mMario->isStatusActive(0x18) && FP(FP(this->mMario, void*, 0x774), s32, 0x50) != 0))) {
        F(u8, 0x704) = 1;
        F(u8, 0x705) = 1;
    }
}

// NONMATCHING: boolean materialization, equivalent constant conversion and loop register allocation.
void MarioActor::fn_803D26F0() {
    Mario* m = this->mMario;
    bool reflect = BITAT(m, 0xC, 0x1D) && !m->fn_803AA2C0() && !mMario->isStatusActive(0x12) &&
        (FP(mMario, u16, 0x966) == 0xE || FP(mMario, u16, 0x966) == 0x21);
    if (reflect) {
        const TVec3f* gravity = mMario->getGravityVec();
        TVec3f camera = MR::getCamZdir();
        reflect = !(gravity->dot(camera) >= 0.0f);
    }
    if (reflect) {
        this->fn_803CA1F0();
        F(u8, 0xEFC) = 1;
        f32 v = FP(this->mMario, f32, 0x484) / 500.0f;
        if (!(v >= 0.75f)) {
            v = 0.75f;
        }
        s32 target = (s32)((f32)0x40U * (1.0f - v));
        int n = 4;
        do {
            u8 c = F(u8, 0xEFD);
            if (c < (u8)target) {
                F(u8, 0xEFD) = c + 1;
            } else if (c > (u8)target) {
                F(u8, 0xEFD) = c - 1;
            }
            n--;
        } while (n != 0);
    } else {
        u8 c = F(u8, 0xEFD);
        if (c != 0) {
            if (c > 4) {
                F(u8, 0xEFD) = c - 4;
            } else {
                F(u8, 0xEFD) = 0;
            }
            if (F(u8, 0xEFD) == 0) {
                this->fn_803CA1F0();
                F(u8, 0xEFA) = 1;
            } else {
                this->fn_803CA1F0();
                F(u8, 0xEFC) = 1;
            }
        } else {
            this->fn_803CA1F0();
            F(u8, 0xEFA) = 1;
        }
    }
    if (this->mMario->isSwimming() && MR::isInWater(F(TVec3f, 0x1014))) {
        if (F(u8, 0xF19) != 0 || F(TVec3f, 0x1038).dot(-*this->mMario->getGravityVec()) > 0.0f) {
            this->fn_803CA1F0();
            F(u8, 0xEF8) = 1;
        }
        if (F(u8, 0xF18) != 0 ||
            (this->mMario->mSwim->getWaterEdgeDist() > 0.0f && this->mMario->mSwim->getWaterEdgeDist() < 1000.0f)) {
            this->fn_803CA1F0();
            F(u8, 0xEF8) = 1;
            F(u8, 0xEF9) = 1;
            F(u8, 0xEFA) = 1;
        }
    }
    mCollisionShadow580->setUpdateFlag();
}


f32 MarioState::getBlurOffset() const {
    return 0.0f;
}

// NONMATCHING: equivalent counted-loop lowering uses a general register instead of CTR.
void MarioActor::fn_803D5A00(u32 idx) {
    if (idx != 0) {
        u32 i = 0;
        u32 n = idx;
        if (n > 0) {
            do {
                if (_428[i] == nullptr) {
                    _428[i] = _428[idx];
                    _428[idx] = nullptr;
                    return;
                }
                ++i;
                n--;
            } while (n != 0);
        }
    }
}

void MarioActor::fn_803D32F0() {
    mWallShadow574->mActive = 0;
    if (F(u8, 0xEFC) == 0 && F(u8, 0xEF8) == 0) {
        if (F(u8, 0xEF9) != 0) {
            return;
        }
        if (F(u8, 0xECD) == 0) {
            s32 len = 0;
            TVec3f axis;
            AreaObj* area = MR::getAreaObj("ExtraWallCheckArea", F(TVec3f, 0x5F4));
            if (area != nullptr) {
                MR::calcCubeAxisZ(area, &axis);
                s32 arg = MR::getAreaObjArg(area, 0);
                F(AreaObj*, 0x578) = area;
                len = arg;
                F(u8, 0x57C) = 0;
            } else {
                area = MR::getAreaObj("ExtraWallCheckCylinder", F(TVec3f, 0x5F4));
                if (area != nullptr) {
                    f32 r = this->calcCylinderToCenter(area, &axis);
                    F(AreaObj*, 0x578) = area;
                    F(u8, 0x57C) = 1;
                    len = (s32)r;
                }
            }
            bool useTimer = false;
            bool run = false;
            if (area == nullptr || (f32)len < 0.0f) {
                f32 f = mWallShadow574->mDistance;
                if (f < 1000.0f) {
                    mWallShadow574->mDistance = 10.0f + f;
                    if (mWallShadow574->mDistance >= 1000.0f) {
                        mWallShadow574->mDistance = 1000.0f;
                    }
                    switch (F(u8, 0x57C)) {
                    case 0:
                        MR::calcCubeAxisZ(F(AreaObj*, 0x578), &axis);
                        len = MR::getAreaObjArg(F(AreaObj*, 0x578), 0);
                        break;
                    case 1:
                        len = (s32)this->calcCylinderToCenter(F(AreaObj*, 0x578), &axis);
                        break;
                    }
                    useTimer = true;
                    run = true;
                }
            } else {
                run = true;
            }
            if (run) {
                Triangle tri;
                TVec3f scaled(axis);
                scaled.scale((f32)len);
                TVec3f hit;
                if (this->mMario->fn_803B2770(&hit, &tri, F(TVec3f, 0x5F4), scaled)) {
                    mWallShadow574->mPosition.set(hit);
                    mWallShadow574->mNormal.set(*tri.getNormal(0));
                    if (!useTimer) {
                        TVec3f d(hit);
                        d -= F(TVec3f, 0x5F4);
                        MarioWallShadow* c = mWallShadow574;
                        c->mDistance = 0.1f * PSVECMag((Vec*)&d) + 0.9f * c->mDistance;
                    }
                    this->updateRandomTexture(mWallShadow574->mDistance);
                    TVec3f nrm(axis);
                    MR::normalizeOrZero(&nrm);
                    J3DModel* mdl = mAllModel->getJ3DModel();
                    J3DModelX* mx = mSimpleModelF08;
                    PSMTXCopy(FP(mdl, Mtx, 0x24), FP(mx, Mtx, 0x24));
                    TPos3f* rot = (TPos3f*)((u8*)mx + 0x24);
                    TVec3f x, y, z;
                    rot->getXDir(x);
                    MR::vecKillElement(x, nrm, &x);
                    rot->getYDir(y);
                    MR::vecKillElement(y, nrm, &y);
                    rot->getZDir(z);
                    MR::vecKillElement(z, nrm, &z);
                    rot->setXDir(x);
                    rot->setYDir(y);
                    rot->setZDir(z);
                    const TVec3f* g = this->mMario->getGravityVec();
                    TVec3f off(this->mPosition);
                    off -= hit;
                    TVec3f perp;
                    MR::vecKillElement(off, *g, &perp);
                    f32 m = PSVECMag((Vec*)&perp);
                    TVec3f t(nrm);
                    t.scale(10.0f + m);
                    TVec3f p(this->mPosition);
                    p += t;
                    MR::setMtxTrans(rot->mMtx, p.x, p.y, p.z);
                    PSMTXConcat(rot->mMtx, F(Mtx, 0x264), rot->mMtx);
                    mx->viewCalcRef(3, mdl);
                    mWallShadow574->mActive = 1;
                }
            }
        }
    }
}


void MarioActor::drawSearchLight() const {
    TVec2f a, b;
    if (mSearchLight != nullptr && this->mMario->isStatusActive(6)) {
        if (!this->mMario->mSwim->fn_80400910()) {
            return;
        }
        if (!MR::isSystemTalking() && !this->mMario->mSwim->fn_80400930()) {
            this->fn_803D82C0(&a, &b);
            this->fn_803D8400(&a, &b);
            if (this->fn_803D8490()) {
                this->fn_803D86B0(&a, &b);
            }
        }
    }
}

void MarioActor::fn_803D82C0(TVec2f* a, TVec2f* b) const {
    TVec3f thr;
    this->getThrowVec(&thr);
    TDDraw::project2D(a, this->mPosition);
    TVec3f s(thr);
    s.scale(10000.0f);
    TVec3f p(this->mPosition);
    p += s;
    TDDraw::project2D(b, p);
    f32 minx, maxx, miny, maxy;
    if (a->x >= b->x) {
        minx = b->x;
        maxx = a->x;
    } else {
        minx = a->x;
        maxx = b->x;
    }
    if (a->y >= b->y) {
        miny = b->y;
        maxy = a->y;
    } else {
        miny = a->y;
        maxy = b->y;
    }
    TVec2f lo(minx, miny);
    TVec2f hi(maxx, maxy);
    TVec2f m1(-10.0f, -10.0f);
    *a = lo + m1;
    TVec2f m2(10.0f, 10.0f);
    *b = hi + m2;
}

void MarioActor::fn_803D8400(TVec2f* a, TVec2f* b) const {
    TDDraw::setup(0, 0, 2);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetColorUpdate(GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetDstAlpha(GX_TRUE, 0);
    TDDraw::drawFillBox(*a, *b, 0);
}

s32 MarioActor::fn_803D8490() const {
    TDDraw::setup(1, 1, 0);
    J3DModelX* m = static_cast<J3DModelX*>(MR::getJ3DModel(mSearchLight));
    J3DMaterial* mat = m->getModelData()->getMaterialNodePointer(0);
    if (!m->simpleDrawSetup(mat)) {
        return 0;
    }
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetColorUpdate(GX_FALSE);
    GXSetDstAlpha(GX_FALSE, 0);
    u32 t = mSearchLightTimer;
    f32 f = 1.0f;
    if (t < 0x1E) {
        f = 0.0f;
    } else if (t < 0xB4) {
        f = (f32)(t - 0x1E) / 150.0f;
    }
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_A0);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    static const GXColorS10 white = {255, 255, 255, 0};
    GXColorS10 c = white;
    c.a = (u8)(s32)(f * (f32)FP(mConst->getTable(), u8, 0x77B));
    GXSetTevColorS10(GX_TEVREG0, c);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
    GXSetCullMode(GX_CULL_BACK);
    m->setDrawView(0);
    m->simpleDrawShape(mat);
    GXSetCullMode(GX_CULL_FRONT);
    GXSetBlendMode(GX_BM_SUBTRACT, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
    m->simpleDrawShape(mat);
    return 1;
}

// NONMATCHING: registers, pooled color/bias addressing, equivalent color packing and redundant target branches.
void MarioActor::fn_803D86B0(TVec2f* a, TVec2f* b) const {
    J3DModelX* m = static_cast<J3DModelX*>(MR::getJ3DModel(mSearchLight));
    u32 t = mSearchLightTimer;
    f32 f = 1.0f;
    J3DMaterial* mat = m->getModelData()->getMaterialNodePointer(0);
    if (t < 0x1E) {
        f = 0.0f;
    } else if (t < 0xB4) {
        f = (f32)(t - 0x1E) / 150.0f;
    }
    TDDraw::setup(0, 1, 2);
    GXSetCullMode(GX_CULL_NONE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetColorUpdate(GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_DSTALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetDstAlpha(GX_TRUE, 0);
    void* tbl = mConst->getTable();
    u8 ca = FP(tbl, u8, 0x77A);
    u32 col = (((FP(tbl, u8, 0x778) << 0x18) & ~0xFF0000) | ((FP(tbl, u8, 0x779) << 0x10) & 0xFF0000));
    col = (col & ~0xFF00) | ((ca << 8) & 0xFF00);
    TDDraw::drawFillBox(*a, *b, col);
    GXSetDstAlpha(GX_FALSE, 0);
    GXSetAlphaUpdate(GX_FALSE);
    TDDraw::setup(0, 1, 1);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetAlphaCompare(GX_GREATER, 4, GX_AOP_AND, GX_ALWAYS, 0);
    bool ok = m->simpleDrawSetup(mat);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    static const GXColorS10 c0 = {255, 255, 255, 0};
    GXColorS10 c = c0;
    c.a = (u8)(s32)(f * (f32)FP(mConst->getTable(), u8, 0x765));
    GXSetTevColorS10(GX_TEVREG0, c);
    GXSetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_1);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_KONST, GX_CA_ZERO, GX_CA_A0, GX_CA_RASA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    if (ok) {
        m->simpleDrawShape(mat);
        GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_A1, GX_CA_A0, GX_CA_RASA, GX_CA_ZERO);
        GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_DIVIDE_2, GX_TRUE, GX_TEVPREV);
        static const GXColorS10 c1 = {255, 255, 255, 0};
        static const GXColorS10 c2 = {255, 255, 255, 4};
        u32 i = 0;
        do {
            m->setDrawView(i + 1);
            GXColorS10 d = c1;
            d.a = (u8)(s32)(f * (f32)FP(mConst->getTable(), u8, 0x766 + i));
            GXSetTevColorS10(GX_TEVREG0, d);
            GXSetTevColorS10(GX_TEVREG1, c2);
            m->simpleDrawShape(mat);
            i++;
        } while (i < 3);
    }
}

void MarioActor::fn_803D8A30(s32 a, TVec3f& v) {
    void* p = F(void*, 0xE8C);
    if (p == nullptr) {
        return;
    }
    if (a != 0) {
        FP(p, u8, 0xA8) = 1;
        FP(p, TVec3f, 0xAC) = v;
        return;
    }
    FP(p, u8, 0xA8) = 0;
}

void MarioActor::fn_803D8A70(s32 mode) {
    u16 t = F(u16, 0x6FC);
    if (t != 0 && mPlayerMode == 0) {
        if (mode != mPlayerMode && F(u16, 0x6FA) == 6) {
            if (t > 0x1A) {
                (mTeresaModel->mActor)->appear();
            }
            F(u8, 0x76B) = 1;
            this->updateHand();
            this->updateFace();
            return;
        }
        if (F(u16, 0x6FA) == 6) {
            (mTeresaModel->mActor)->kill();
            F(u8, 0x76B) = 0;
        }
    }
}

bool MarioActor::fn_803D8B30(HitSensor* s) {
    HitSensor* hs = _924;
    if (hs == nullptr) {
        return true;
    }
    LiveActor* h = MR::getSensorHost(hs);
    if (h == nullptr) {
        return true;
    }
    if (MR::getSensorHost(s) == nullptr) {
        return true;
    }
    if (MR::isEqualString(h->mName, "たまころ")) {
        return s->isType(0x61);
    }
    return true;
}

// NONMATCHING: local vector stack slots, registers, and elided value copy before scaling.
void MarioActor::fn_803D8BD0(TVec3f* a, TVec3f* b) {
    f32 ma = PSVECMag((Vec*)a);
    if (PSVECMag((Vec*)&F(TVec3f, 0x38)) < ma) {
        if (a->dot(*this->mMario->getGravityVec()) < -0.0f) {
            TVec3f k;
            MR::vecKillElement(mVelocity, *mMario->getGravityVec(), &k);
            if (MR::isNearZero(k, 0.001f)) {
                MR::vecKillElement(*a, *this->mMario->getGravityVec(), &k);
            }
            k.setLength(PSVECMag((Vec*)a));
            this->mMario->push(k);
            if (FP(this->mMario, u16, 0x3BC) <= 2) {
                f32 m2 = PSVECMag((Vec*)b);
                f32 v = m2;
                if (m2 > 10.0f) {
                    v = 10.0f;
                }
                TVec3f up = -*this->mMario->getGravityVec();
                up.scale(v);
                FP(this->mMario, TVec3f, 0x2D4) += up;
            }
        }
    } else {
        TVec3f n(FP(this->mMario, TVec3f, 0x8FC));
        MR::normalizeOrZero(&n);
        TVec3f tmp;
        f32 f1 = MR::vecKillElement(*a, n, &tmp);
        f32 f2 = MR::vecKillElement(F(TVec3f, 0x38), n, &tmp);
        if (PSVECMag((Vec*)&F(TVec3f, 0x38)) > 20.0f && f1 < 0.5f * f2) {
            if (this->mMario->isAnimationRun("坂すべり下向きあおむけ")) {
                TVec3f p(FP(this->mMario, TVec3f, 0x208));
                p.scale(5.0f);
                this->mMario->push(p);
            } else if (this->mMario->isAnimationRun("坂すべり上向きうつぶせ")) {
                TVec3f p(FP(this->mMario, TVec3f, 0x208));
                p.scale(-5.0f);
                this->mMario->push(p);
            }
            FP(this->mMario, u32, 0x18) |= 0x20000000;
        }
    }
}


void MarioActor::fn_803D8E30(TVec3f* v) {
    if (v->dot(*this->mMario->getGravityVec()) < -40.0f) {
        TVec3f up(*this->mMario->getGravityVec());
        up.scale(100.0f);
        TVec3f start(this->mPosition);
        start -= up;
        TVec3f dir(*this->mMario->getGravityVec());
        dir.scale(200.0f);
        Triangle* tri = this->mMario->fn_803EFA20();
        TVec3f hit;
        if (this->mMario->fn_803B2770(&hit, tri, start, dir)) {
            TVec3f d(hit);
            d -= this->mPosition;
            if (this->mMario->getGravityVec()->dot(d) != -5.0f) {
                void* h = *(void**)tri;
                if (h != nullptr && FP(h, s32, 0xD4) == 0) {
                    if ((FP(this->mMario, u32, 0x10) & 3) == 1) {
                        return;
                    }
                    this->mPosition = hit;
                    this->mMario->stopJump();
                    this->mMario->changeAnimation("基本", "落下");
                    this->mMario->updateGroundInfo();
                }
            }
        }
    } else if (v->dot(*this->mMario->getGravityVec()) > 40.0f) {
        Triangle* gt = FP(this->mMario, Triangle*, 0x4C4);
        if (gt->isValid()) {
            if (MR::isSensorPressObj(FP(gt, HitSensor*, 8)) || MR::getFloorCodeIndex(FP(this->mMario, Triangle*, 0x4C4)) == 0x1D ||
                MR::getFloorCodeIndex(FP(this->mMario, Triangle*, 0x4C4)) == 0x27) {
                FP(this->mMario, u32, 0x1C) |= 2;
            }
        } else {
            u32 i = 0;
            u32 n = FP(this->mBinder, u32, 0x28);
            while (i < n) {
                const HitInfo* p = this->mBinder->getPlane(i);
                if (MR::isSensorPressObj(FP(p, HitSensor*, 8))) {
                    FP(this->mMario, u32, 0x1C) |= 2;
                }
                i++;
            }
        }
        TVec3f d(FP(this->mMario, TVec3f, 0x31C));
        d -= this->mPosition;
        if (d.dot(*this->mMario->getGravityVec()) < 0.0f && !this->fn_803D9370(*FP(this->mMario, void**, 0x458), v)) {
            this->mPosition = FP(this->mMario, TVec3f, 0x31C);
            FP(this->mMario, TVec3f, 0x2D4).zero();
            FP(this->mMario, TVec3f, 0x148).zero();
            Mario* m = this->mMario;
            if (FP(m, s32, 0x5FC) == 0 && (BITAT(m, 0x1C, 1) || BITAT(m, 0x34, 1))) {
                TVec3f nv(*v);
                MR::normalizeOrZero(&nv);
                F(TVec3f, 0x6D8) = nv;
                this->setPress(0, 0);
                F(f32, 0x6D4) = 0.1f;
            }
        }
    }
}

void MarioActor::fn_803D9170(TVec3f* v) {
    u32 n = FP(this->mBinder, u32, 0x28);
    s32 hit = 0;
    f32 zero = 0.0f;
    u32 i = 0;
    while (i < n) {
        Triangle* tri = (Triangle*)this->mBinder->getPlane(i);
        if (MR::isSensorPressObj(FP(tri, HitSensor*, 8))) {
            if (FP(this->mMario, TVec3f, 0x368).dot(*tri->getNormal(0)) > zero) {
                if (FP(this->mMario, f32, 0x724) < 200.0f) {
                    hit = 1;
                } else if (tri->getNormal(0)->dot(*v) < zero) {
                    hit = 1;
                } else {
                    hit = 0;
                }
            } else {
                f32 d = tri->getNormal(0)->dot(*v);
                if (d < zero) {
                    hit = 1;
                } else if (FP(this->mMario, s32, 0x5FC) != 0) {
                    if (!MR::isWallCodeNoAction(tri) && !this->mMario->isOnimasuBinderPressSkip()) {
                        F(TVec3f, 0x6D8) = FP(this->mMario, TVec3f, 0x368);
                        FP(this->mMario, u32, 0x14) &= 0xFFFFFFFB;
                        *FP(this->mMario, Triangle*, 0x4C4) = *tri;
                        this->setPress(2, 0);
                        F(f32, 0x6D4) = 0.1f;
                    }
                    hit = 1;
                }
            }
        }
        i++;
    }
    if (hit != 0) {
        TVec3f tmp;
        f32 k = MR::vecKillElement(*v, FP(this->mMario, TVec3f, 0x368), &tmp);
        TVec3f s(FP(this->mMario, TVec3f, 0x368));
        s.scale(k);
        this->mPosition -= s;
    }
}

bool MarioActor::fn_803D9370(void* tri, TVec3f* v) {
    if (tri == nullptr) {
        return false;
    }
    if (FP(tri, s32, 0xD4) != 0) {
        return false;
    }
    TVec3f a, b;
    PSMTXMultVec(FP(tri, Mtx, 0x64), (Vec*)&FP(this->mMario, TVec3f, 0x31C), (Vec*)&a);
    PSMTXMultVec(FP(tri, Mtx, 0x94), (Vec*)&a, (Vec*)&b);
    TVec3f d(FP(this->mMario, TVec3f, 0x31C));
    d -= b;
    return v->dot(d) > 0.0f;
}

bool MarioActor::fn_803D9430() {
    if (this->isAllHidden()) {
        return false;
    }
    if (this->mMario->isStatusActive(0x1A)) {
        return false;
    }
    if (this->mMario->isStatusActive(0x1B)) {
        TVec3f a(this->mPosition);
        a -= F(TVec3f, 0x1014);
        if (!this->mMario->fn_803B2880(F(TVec3f, 0x1014), a)) {
            return false;
        }
        TVec3f b(F(TVec3f, 0x600));
        b -= F(TVec3f, 0x1014);
        if (!this->mMario->fn_803B2880(F(TVec3f, 0x1014), b)) {
            return false;
        }
    }
    u8 t = FP(this->mMario, u8, 0x72D);
    if (t > 0x40) {
        return false;
    }
    if (t != 0) {
        TVec3f c(F(TVec3f, 0x600));
        c -= F(TVec3f, 0x1014);
        if (!this->mMario->fn_803B2880(F(TVec3f, 0x1014), c)) {
            return false;
        }
    }
    if (mPlayerMode == 6) {
        return false;
    }
    if (this->mMario->isStatusActive(0x12) && F(u8, 0x1A5) != 0) {
        return false;
    }
    if (F(u8, 0x4E0) != 0) {
        return false;
    }
    bool r = false;
    if (F(s32, 0xE28) == 0 && (F(u8, 0xE24) == 0 || F(u8, 0xE25) == 1)) {
        r = true;
    }
    return r;
}

namespace MR {
    bool isCreatePlayerResourceHairAndHead();
}

void MarioActor::fn_803D95E0() {
    if (MR::isCreatePlayerResourceHairAndHead()) {
        mHairAndHat1090 = new MarioHairAndHatModel();
        mHairAndHat1090->fn_803EC760(MR::getJointMtx(this, "CapPosition"));
        mHairAndHat1090->fn_803EC770(this->getBaseMtx());
    }
}

void MarioActor::startMovement() {
    if (this->mMario->isPlayerModeYoshi()) {
        MR::requestMovementOn(mYoshiHolder->mYoshi);
    }
}

void MarioActor::endMovement() {
    if (this->mMario->isPlayerModeYoshi()) {
        MR::requestMovementOff(mYoshiHolder->mYoshi);
    }
}

void MarioActor::fn_803D9710() {
    F(s16, 0x552) = 0;
    F(u8, 0x1067) = 1;
}

void MarioActor::fn_803D9730() {
    F(u8, 0x1067) = 0;
}

void MarioActor::fn_803D9740() {
    if (gIsLuigi) {
        if (F(u32, 0x1AC) != 3 && (u32)F(s32, 0x6B0) <= 3) {
            F(u32, 0x1AC) = 3;
        }
    } else if (F(u32, 0x1AC) != 3 && (u32)F(s32, 0x6B0) <= 3) {
        F(u32, 0x1AC) = 3;
    }
}

void MarioActor::decreaseLifeByOne() {
    if (mMario->isPlayerModeYoshi()) fn_803B9870();
    if (mHealth) --mHealth;
    F(u8, 0x1070) = 1;
}
