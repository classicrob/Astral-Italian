// SMG2-only MarioActor functions (part B, fn_803CA000 and up), written from the asm. Included at the end of MarioActor.cpp (unity TU).


void MarioActor::fn_803CA1F0() {
    this->mWaterReflectionEF8 = 0;
    this->mSphereReflectionEF9 = 0;
    this->mGroundReflectionEFC = 0;
    this->mShadowEnabledEFA = 0;
    this->mShadowModeEFB = 0;
}

void MarioActor::fn_803CA270(TVec3f* out, s32 isStatic) {
    TVec3f g;
    MR::calcGravityVectorOrZero(this, this->mPosition, &g, mGravityInfo, 0);
    if (isStatic == 0) {
        MR::calcGravityVectorOrZero(this, this->_240, out, mGravityInfo, 0);
    } else {
        out->set(g);
    }
    TVec3f up((this->mMario)->mFrontVec);
    up.scale(100.0f);
    TVec3f p(this->mPosition);
    p += up;
    TVec3f g2;
    MR::calcGravityVectorOrZero(this, p, &g2, nullptr, 0);
    TVec3f d(g);
    d -= g2;
    mAxisRotationDisabled6A0 = MR::isNearZero(d, 0.001f);
    TVec3f hit;
    if (this->mMario->fn_803AA340(&this->mPosition, &hit)) {
        out->set(hit);
        g = hit;
        mAxisRotationDisabled6A0 = 0;
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
    if (((this->mMario->mMovementStates.mLowWord >> (0x14)) & 1) && MR::isNearZero(*out, 0.001f)) {
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
        (this->mMario)->mFlags14.mLowWord |= 0x20;
    } else {
        if ((this->mMario)->_430 == 0xB) {
            (this->mMario)->_430 = 0;
        }
        if (_934 == 0) {
            if (((this->mMario->mMovementStates.mLowWord >> (0x14)) & 1)) {
                this->mMario->changeAnimation("ショートジャンプ", "落下");
            } else {
                this->mMario->changeAnimation("ショートジャンプ", (const char*)nullptr);
            }
        }
        r = true;
    }
    (this->mMario)->mFlags14.mLowWord &= 0xFFDFFFFF;
    (this->mMario)->mMovementStates.mLowWord &= 0xFFEFFFFF;
    if (mSwimSurfaceState1010 != 0) {
        r = true;
    }
    this->_334 = 0xF;
    this->_legacy2AC = (this->mMario)->mHeadVec;
    if (!MR::isNearZero(b, 0.001f)) {
        if (ang >= 2.0943952f || !((this->mMario->mMovementStates.mHighWord >> (8)) & 1)) {
            this->mMario->cutGravityElementFromJumpVec(false);
            (this->mMario)->mJumpVec.scale(0.5f);
        } else {
            TVec3f& jv = (this->mMario)->mJumpVec;
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
    (this->mMario)->mFlags14.mLowWord |= 0x10000000;
    this->_3AA = 0xF;
    if (!((this->mMario->mMovementStates.mHighWord >> (8)) & 1)) {
        (this->mMario)->_2E0 = -(this->mMario)->_2E0;
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
            this->calcBaseFrontVec((this->mMario)->_1FC);
        }
    } else {
        TVec3f axis;
        f32 cosv;
        if (MR::makeAxisAndCosignVecToVec(&axis, &cosv, a, b)) {
            const f32 angle = JMAAcosRadian(cosv);
            Mtx m;
            PSMTXRotAxisRad(m, (Vec*)&axis, angle);
            PSMTXMultVecSR(m, (Vec*)&this->_258, (Vec*)&this->_258);
            TVec3f cr;
            PSVECCrossProduct((Vec*)&this->_258, (Vec*)&b, (Vec*)&cr);
            if (MR::normalizeOrZero(&cr)) {
            } else {
                PSVECCrossProduct((Vec*)&b, (Vec*)&cr, (Vec*)&this->_258);
            }
            MR::normalize(&this->_258);
        }
    }
}

void MarioActor::fn_803CA860(TVec3f& v) {
    if (!MR::isNearZero(v, 0.001f)) {
        MR::normalize(&v);
    } else {
        Mario* m = this->mMario;
        if (!((m->mMovementStates.mLowWord >> (0x1E)) & 1)) {
            v.set(mAppliedGravityVec);
        } else {
            TVec3f n = -(m)->_368;
            v.set(n);
        }
    }
    if (this->mMario->isPlayerModeYoshi() && (mYoshiHolder->mYoshi)->mDashTimer > 0 && mDashWallHit1053 == 0) {
        WaterInfo info;
        MR::getWaterAreaObj(&info, this->mPosition);
        if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1) && !info.isInWater()) {
            TVec3f n = -(this->mMario)->_368;
            v.set(n);
        }
    }
    mDashWallHit1053 = 0;
}

// NONMATCHING: saved registers, stack slots and equivalent reuse of the 70.0f scalar.
void MarioActor::fn_803CAA00() {
    f32 len = 70.0f;
    Mario* m = this->mMario;
    TVec3f blend;
    if (((m->mDrawStates.mWord >> (0x10)) & 1)) {
        MR::vecBlendSphere(&blend, (m)->mHeadVec, (m)->_380, 0.5f * this->_984);
    } else {
        MR::vecBlendSphere(&blend, (m)->mHeadVec, (m)->_368, this->_984);
    }
    TVec3f old(this->mVector60C);
    m = this->mMario;
    if (((m->mMovementStates.mLowWord >> (0x15)) & 1)) {
        len = 50.0f;
    }
    if (((m->mMovementStates.mLowWord >> (0x1E)) & 1)) {
        TVec3f t(blend);
        t.scale(len);
        this->mVector60C = t;
    } else if (((m->mFlags14.mHighWord >> (0x1C)) & 1) || _934 != 0) {
        TVec3f neg = -mAppliedGravityVec;
        TVec3f t(neg);
        t.scale(70.0f);
        if (this->_334 != 0 || mMario->calcPolygonAngleD(mMario->mGroundPolygon) >= 59.0f) {
            MR::vecBlend(&this->mVector60C, this->mVector60C, t, 0.1f);
        } else {
            MR::vecBlend(&this->mVector60C, this->mVector60C, t, 0.5f);
        }
    } else {
        TVec3f neg = -mAppliedGravityVec;
        TVec3f t(neg);
        t.scale(70.0f);
        MR::vecBlend(&this->mVector60C, this->mVector60C, t, 0.1f);
    }
    u16 c = this->_3AA;
    if (c != 0) {
        this->_3AA = c - 1;
        f32 r = (f32)(u16)(c - 1) / 15.0f;
        TVec3f a(old);
        a.scale(r);
        TVec3f b(this->mVector60C);
        b.scale(1.0f - r);
        TVec3f s(b);
        s += a;
        this->mVector60C = s;
        (this->mMario)->mFlags14.mLowWord |= 0x10000000;
    }
    if (this->_334 != 0) {
        Mario* m2 = this->mMario;
        if ((m2)->mVerticalSpeed > 160.0f) {
            TVec3f a((m2)->mHeadVec);
            a.scale(80.0f);
            TVec3f b(this->_legacy2AC);
            b.scale(80.0f);
            TVec3f c2(b);
            c2 -= a;
            f32 k = MR::vecKillElement(c2, mAppliedGravityVec, &c2);
            TVec3f push(mAppliedGravityVec);
            push.scale(k);
            this->mMario->push2(push);
            this->_legacy2AC = (this->mMario)->mHeadVec;
        }
        this->_334 = this->_334 - 1;
    }
}


void MarioActor::fn_803CAD10(const char* name, const char* b, const BckCtrlData* d) {
    const char* bck = b;
    if (this->_B91 == 0) {
        if (static_cast<s32>(_468) == 0) {
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
            this->mMarioAnim->closeCallback();
            this->mMarioAnim->entryCallback(name);
        }
    }
}

bool MarioActor::fn_803CB1A0(u16 mode) {
    if (mPlayerMode != mode) {
        return false;
    }
    if (mode == 1) {
        this->_3DC = mConst->getTable()->mMetalHoldTime;
        MR::startSubBGM("BGM_MUTEKI_A", false);
    } else if (mode == 9) {
        this->_3DC = mConst->getTable()->mTornadoHoldTime;
    } else if (mode == 3) {
        if (((this->mMario->mFlags14.mHighWord >> (0x16)) & 1)) {
            this->_3DC = 0;
        } else {
            this->_3DC = mConst->getTable()->mIceModeTime;
        }
        MR::startSubBGM("BGM_ICE_A", false);
    } else if (mode == 2) {
        this->_3DC = mConst->getTable()->mFireModeTime;
        MR::startPlayFireMorphBGM();
    } else if (mode == 7) {
        this->_3DC = 0xE10;
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
            this->_483 = 0;
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
        _9E4->kill();
        _9E8->kill();
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
        this->_A6E = 0;
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
            mConst->changeTable(1);
        } else {
            mConst->changeTable(0);
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
    this->_3D6 = mPlayerMode;
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
        this->_3DC = mConst->getTable()->mTornadoHoldTime;
        MR::startBtp(this, "TornadoElement");
        if (this->isJumping()) {
            this->mMario->changeAnimation("エレメントゲット", (const char*)nullptr);
        } else {
            this->mMario->changeAnimation("エレメントゲット接地中", (const char*)nullptr);
        }
        break;
    case 1:
        this->_3DC = mConst->getTable()->mMetalHoldTime;
        this->playEffect("無敵中");
        MR::startSubBGM("BGM_MUTEKI_A", false);
        MR::stopBrk(_9C0);
        MR::stopBrk(mHandModelL);
        MR::stopBrk(mHandModelR);
        MR::startBrk(_9C0, "InvincibleMario");
        MR::startBrk(mHandModelL, "InvincibleMarioHandL");
        MR::startBrk(mHandModelR, "InvincibleMarioHandR");
        MR::removeAllClingingKarikari();
        break;
    case 0:
        MR::startBtp(this, "ElementEnd");
        if (a) {
            u16 prev = this->_3D6;
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
        this->_3DC = 0;
        (this->mMario)->_76C = 0;
        if (this->mMario->isSwimming()) {
            if (static_cast<s32>(_468) == 0) {
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
        this->_483 = 1;
        this->rushDropThrowMemoSensor();
        this->mMario->startTeresaMode();
        (mTeresaModel->mActor)->calcAnim();
        MR::emitEffect(mTeresaModel->mActor, "Glow");
        MR::emitEffect(mTeresaModel->mActor, "Appear");
        break;
    case 2:
        MR::startPlayFireMorphBGM();
        this->_3DC = mConst->getTable()->mFireModeTime;
        break;
    case 3:
        this->playEffect("アイス中");
        MR::startSubBGM("BGM_ICE_A", false);
        if (((this->mMario->mFlags14.mHighWord >> (0x16)) & 1)) {
            this->_3DC = 0;
        } else {
            this->_3DC = mConst->getTable()->mIceModeTime;
        }
        break;
    case 4:
        MR::getGameSceneLayoutHolder()->changeLifeMeterModeBee();
        this->rushDropThrowMemoSensor();
        _9E4->appear();
        MR::showModel(_9E4);
        (this->mMario)->_402 = mConst->getTable()->mAirWalkTime;
        (this->mMario)->_42A = 0;
        _9E8->appear();
        this->_9F0 = 0;
        MR::startAction(_9E8, "Wait");
        MR::startBva(_9E8, "Wait");
        MR::stopBtk(_9E8);
        break;
    case 7:
        MR::startStageBGM("BGM_FLYING_A", false);
        this->_3DC = 3600;
        break;
    case 10: {
        mConst->changeTable(2);
        LiveActor* ya = mYoshiHolder->mYoshi;
        MR::setPlayerPos(ya->mPosition);
        MtxPtr bm = ya->getBaseMtx();
        TVec3f front(bm[0][2], bm[1][2], bm[2][2]);
        MR::setPlayerFrontVec(front, 1);
        mYoshiHolder->mTongueAction->fn_8041FCD0();
        mYoshiHolder->_10->mSensor = this->getSensor("Body");
        this->mMarioAnim->fn_803DF8E0();
        this->rushDropThrowMemoSensor();
        if (mYoshiHolder->mYoshi->mMountedBySpin != 0) {
            this->mMario->changeAnimation("ヨッシースピン乗車", "基本");
        } else {
            this->mMario->changeAnimation("ヨッシー乗車", "基本");
        }
        this->mMario->stopWalk();
        this->_3DC = 0;
        this->resetCondition();
        this->setPunchHitTimer(0);
        MR::startStarPointerModeCommandStream(this);
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
    if (this->mTransforming != 0) {
        MR::endDemo(this, "マリオ変身");
        this->mTransforming = 0;
    }
    if (a) {
        this->_3D8 = 0x1E;
        this->mMario->startPadVib("マリオ[変身解除]");
    } else {
        this->_3D8 = 2;
    }
    if (b) {
        this->playEffect("変身解除");
    }
}

// NONMATCHING: Typed flag stores use direct member addresses instead of a cached address.
void MarioActor::fn_803CBB30(bool b) {
    if (this->_3D8 == 0 || this->_3D6 == 0) {
        this->mPowerupCollected = 1;
    } else {
        if (mPlayerMode == 0xA) {
            this->_3D8 = 0x13;
            this->mMario->clearVelocity();
            (this->mMario)->mWalkSpeed = 0.0f;
        } else {
            this->_3D8 = 0x40;
            MR::stopAnimFrame(this);
            if (b) {
                this->playEffect("変身");
                this->mMario->startPadVib("マリオ[変身]");
            }
        }
        this->mPowerupCollected = 0;
    }
    if (!b) {
        this->_3D8 = 2;
        this->mPowerupCollected = 0;
    }
    this->_945 = 0;
    reinterpret_cast<s16&>(this->_946) = 0;
    this->mSpinRadius = 0.0f;
    this->mSpinHeight = 0.0f;
    for (int i = 0; i < 0x80; i++) {
        mSpinNoise[i] = 0.0f;
    }
    reinterpret_cast<s16&>(this->_948) = 0;
    reinterpret_cast<s16&>(this->_94C) = 0;
    this->updateFairyStar();
}

void MarioActor::fn_803CBC50() {
    if (mPlayerMode == 2) {
        MR::adjustSandyBossFireMorphBGM(this->_3DC, mConst->getTable()->mFireModeTime);
    }
    u16 t = this->_3DC;
    if (t != 0) {
        this->_3DC = t - 1;
        if (this->_3DC == 0) {
            this->setPlayerMode(0, true, true);
        }
    }
    if (MR::isInAreaObj("PlayerMorphLimitArea", this->mPosition)) {
        this->mWasInMorphLimitArea1065 = 1;
        return;
    }
    if (this->mWasInMorphLimitArea1065 != 0) {
        this->mWasInMorphLimitArea1065 = 0;
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803CCE60(HitSensor* a, s16 b) {
    this->mMarioAnim->fn_803DDD80(a);
    if (!this->mMario->isSwimming()) {
        if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1) && this->_B92 != -3) {
            reinterpret_cast<s16&>(this->_38C) = b;
        }
        mVelocity.zero();
        (this->mMario)->mWalkSpeed = 0.0f;
        if (((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1)) {
            this->mMario->stopJump();
            (this->mMario)->mVerticalSpeed = 0.0f;
        }
    }
    _424 = a;
    this->_480 = 1;
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
    if (_934 != 0) {
        return false;
    }
    if (this->mHealth == 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (!mActionTable1080->isActionOk("カメ持ち")) {
        return false;
    }
    if (_468 != 0) {
        return false;
    }
    return true;
}

bool MarioActor::fn_803CD0A0() {
    if (_934 != 0) {
        return false;
    }
    if (this->mHealth == 0) {
        return false;
    }
    if (this->_3AC != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (!mActionTable1080->isActionOk("カメ持ち")) {
        return false;
    }
    if (static_cast<s32>(_468) != 0) {
        return false;
    }
    if (((this->mMario->mMovementStates.mLowWord >> (0x14)) & 1) && this->fn_803CD180()) {
        return false;
    }
    return true;
}

bool MarioActor::fn_803CD180() {
    bool r = false;
    if (!this->mMario->isSwimming() && !((this->mMario->mMovementStates.mLowWord >> (0x1E)) & 1) && _424 == nullptr) {
        r = true;
    }
    return r;
}

f32 MarioActor::fn_803CD250(HitSensor* s) {
    TVec3f a((this->mMario)->mShadowPos);
    a -= (s)->mPosition;
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
    if (this->_3AC != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    if (!mActionTable1080->isActionOk("カメ持ち")) {
        return false;
    }
    if (static_cast<s32>(_468) != 0) {
        return false;
    }
    if (((this->mMario->mMovementStates.mLowWord >> (0x14)) & 1) && this->fn_803CD180()) {
        return false;
    }
    return true;
}

bool MarioActor::fn_803CD380() {
    if (this->mHealth == 0) {
        return false;
    }
    if (this->_3D8 != 0) {
        return false;
    }
    return (this->mMario)->mMorphResetTimer == 0;
}

bool MarioActor::fn_803CD3C0() {
    if (_934 != 0) {
        return false;
    }
    if (this->mHealth == 0) {
        return false;
    }
    if (this->_3AC != 0) {
        return false;
    }
    if (this->mMario->isDamaging()) {
        return false;
    }
    return static_cast<s32>(_468) == 0;
}

void MarioActor::fn_803CD450(HitSensor* s) {
    if (this->fn_803CD1F0(s) && this->fn_803CD180()) {
        this->doTrampleJump(s);
    }
}


bool MarioActor::fn_803CD4B0(HitSensor* s, f32 f0, f32 f1) {
    f32 r = (s)->mRadius;
    TVec3f d((s)->mPosition);
    d -= this->_240;
    LiveActor* h = s->mHost;
    if (h != nullptr && MR::isEqualString(h->mName, "ヨッシー")) {
        return this->fn_803CD8A0(&d, r, 40.0f, 10.0f);
    }
    return this->cylinderPushCheck(d, r, f0, f1);
}

void MarioActor::sendWallTouch(u32 msg, HitSensor* a, HitSensor* b) {
    if (a != nullptr) {
        if (!MR::isDead(MR::getSensorHost(a))) {
            a->receiveMessage(msg, b);
            if (((this->mMario->mMovementStates.mLowWord >> (0x10)) & 1)) {
                a->receiveMessage(0x3A, b);
            }
        }
    }
}

bool MarioActor::fn_803CD8A0(TVec3f* v, f32 f0, f32 f1, f32 f2) {
    TVec3f p(*v);
    if (((this->mMario->mFlags14.mLowWord >> (0x19)) & 1)) {
        f32 k = MR::vecKillElement(p, mCameraBasisZ, &p);
        if (k >= 250.0f || k <= -250.0f) {
            return false;
        }
    }
    TVec3f perp;
    f32 d = MR::vecKillElement(p, mBodyDownDirection, &perp);
    if (d > -f0) {
        if (d < f2 && PSVECMag((Vec*)&perp) < (f1 + f0)) {
            HitSensor* hs = this->getSensor("dummy");
            TVec3f t(mBodyDownDirection);
            t.scale(d);
            TVec3f q(this->_240);
            q += t;
            (hs)->mPosition.set(q);
            (this->getSensor("dummy"))->mRadius = f1;
            return true;
        }
        if (d >= f2) {
            TVec3f t(mBodyDownDirection);
            t.scale(f2);
            TVec3f e(p);
            e -= t;
            if (PSVECMag((Vec*)&e) < (f0 + f1)) {
                HitSensor* hs = this->getSensor("dummy");
                TVec3f t2(mBodyDownDirection);
                t2.scale(f2);
                TVec3f q(this->_240);
                q += t2;
                (hs)->mPosition.set(q);
                (this->getSensor("dummy"))->mRadius = f1;
                return true;
            }
        }
    }
    Mario* m = this->mMario;
    if (((m->mMovementStates.mLowWord >> (0x15)) & 1) && ((m->mMovementStates.mLowWord >> (0x1E)) & 1)) {
        return false;
    }
    f32 d2 = MR::vecKillElement(p, mBodyUpDirection, &perp);
    if (d2 > -f0) {
        if (d2 < f2 && PSVECMag((Vec*)&perp) < (f1 + f0)) {
            HitSensor* hs = this->getSensor("dummy");
            TVec3f t(mBodyUpDirection);
            t.scale(d2);
            TVec3f q(this->_240);
            q += t;
            (hs)->mPosition.set(q);
            (this->getSensor("dummy"))->mRadius = f1;
            return true;
        }
        if (d2 >= f2) {
            TVec3f t(mBodyUpDirection);
            t.scale(f2);
            TVec3f e(p);
            e -= t;
            if (PSVECMag((Vec*)&e) < (f0 + f1)) {
                HitSensor* hs = this->getSensor("dummy");
                TVec3f t2(mBodyUpDirection);
                t2.scale(f2);
                TVec3f q(this->_240);
                q += t2;
                (hs)->mPosition.set(q);
                (this->getSensor("dummy"))->mRadius = f1;
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
    if (this->_480 != 0) {
        if (s == nullptr) {
            this->_480 = 0;
            return;
        }
        bool drill = false;
        if (MR::getSensorHost(s) != nullptr && MR::isEqualString(MR::getSensorHost(s)->mName, "アイテムドリル")) {
            drill = true;
        }
        if (drill) {
            if (this->mMario->isAnimationTerminate(nullptr)) {
                this->_480 = 0;
                MR::explainDrillIfAtFirst();
            }
        } else {
            u32 t = (s)->mType;
            if (t - 0x12 > 1) {
                if (t == 0x1D && this->mMario->isAnimationTerminate("カブ抜き")) {
                    this->mMario->stopAnimation((const char*)nullptr, (const char*)nullptr);
                    this->_480 = 0;
                    this->mMario->changeAnimationUpper("カブウエイト", (const char*)nullptr);
                }
            } else if (this->mMario->isAnimationTerminate(nullptr)) {
                this->_480 = 0;
            }
        }
    }
}


void MarioActor::fn_803CE9D0(HitSensor* s) {
    if (this->mRushAnimationDelay1054 == 0) {
        _494->calc();
        TVec3f t, r;
        _494->mMtx.getTrans(t);
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
    if (this->mRushAnimationDelay1054 == 0) {
        Mtx tmp;
        PSMTXConcat(this->getBaseMtx(), mMainJointMatrix4A8.mMtx, tmp);
        PSMTXCopy(tmp, MR::getJ3DModel(mNullAnimation)->getBaseTRMtx());
        TVec3f pos, rot;
        if (mNullAnimation->getFramePos(t, &pos, &rot)) {
            this->_B92 = -3;
            mNullAnimation->kill();
            if (s == nullptr) {
                this->_480 = 0;
                this->_B92 = 0;
                mNullAnimation->kill();
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

// NONMATCHING: vector temporary stack slots and literal identities.
void MarioActor::fn_803CEBC0() {
    if (static_cast<s32>(_468) != 0) {
        HitSensor* cs = (HitSensor*)this->getCarrySensor();
        bool drill = false;
        if (MR::getSensorHost(cs) != nullptr && MR::isEqualString(MR::getSensorHost(cs)->mName, "アイテムドリル")) {
            drill = true;
        }
        if (drill) {
            if (_934 == 0 && this->mRushAnimationDelay1054 == 0) {
                _494->calc();
                TVec3f t;
                _494->mMtx.getTrans(t);
                LiveActor* host = MR::getSensorHost(cs);
                TVec3f p(t);
                p += mVelocity;
                host->mPosition = p;
                _494->copyRotate(&MR::getSensorHost(cs)->mRotation);
            }
        } else {
            if (cs->isType(0x48) || cs->isType(0x12) || cs->isType(0x13) || cs->isType(0x1D)) {
                _494->calc();
                TVec3f t;
                _494->mMtx.getTrans(t);
                LiveActor* host = MR::getSensorHost(cs);
                TVec3f p(t);
                p += mVelocity;
                host->mPosition = p;
                _494->copyRotate(&MR::getSensorHost(cs)->mRotation);
                return;
            }
            TVec3f lp;
            mNullAnimation->getLastPos(&lp);
            PSMTXMultVec(mMainJointMatrix4A8.mMtx, (Vec*)&lp, (Vec*)&lp);
            PSMTXMultVec(this->getBaseMtx(), (Vec*)&lp, (Vec*)&lp);
            f32 sp = (this->mMario)->mWalkSpeed;
            f32 amp = 10.0f * (sp * sp);
            amp *= JMASinRadian(mCarryWobblePhase778);
            mCarryWobblePhase778 += MR::getRandom(0.1f, 1.0f);
            lp.y += amp;
            MR::getSensorHost(cs)->mPosition = lp;
            LiveActor* host = MR::getSensorHost(cs);
            TVec3f r(mThrowMemoRotations[0]);
            r += mRotation;
            host->mRotation = r;
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

void MarioActor::updateBaseMtxTeresa(MtxPtr mtx) {
    TVec3f tmp;
    f32 k = MR::vecKillElement(mVelocity, *this->mMario->getGravityVec(), &tmp);
    f32 v = MR::clamp(k / 10.0f, -1.0f, 1.0f);
    if (!MR::isNearZero(this->mMario->getWorldPadDir(), 0.001f)) {
        Mario* m = this->mMario;
        const TVec3f* g = m->getGravityVec();
        if (MR::diffAngleAbsHorizontal(m->getWorldPadDir(), m->mFrontVec, *g) > 0.3926991f) {
            v = 0.0f;
        }
    }
    mTeresaModel->_8 = 0.02f * v + 0.98f * mTeresaModel->_8;
    f32 var = mConst->getTable()->mTeresaAngleDown;
    if (mTeresaModel->_8 > 0.0f) {
        var = mConst->getTable()->mTeresaAngleUp;
    }
    PSMTXConcat(mtx, MR::tmpMtxRotXRad(var * mTeresaModel->_8), mtx);
}

void MarioActor::updateFireSpin() {
    if (!this->mMario->isAnimationRun("ファイア投げ") && !this->mMario->isAnimationRun("ファイアスピン空中") &&
        (!this->mMario->isAnimationRun("ファイアスピン") || !this->fn_803CB010())) {
        if (((this->mMario->mMovementStates.mLowWord >> (0x17)) & 1)) {
            const Triangle* tri = this->mMario->getWallPolygon();
            LiveActor* h = MR::getSensorHost(tri->getSensor());
            if (!MR::isEqualString(h->mName, "木箱") && !MR::isEqualString(h->mName, "タイムアタック用木箱")) {
                HitSensor* ws = this->mMario->getWallPolygon()->getSensor();
                ws->receiveMessage(0xB, this->getSensor("Body"));
                this->mMario->changeAnimation("ファイアスピン", (const char*)nullptr);
                return;
            }
        }
        this->mFireBallDelay = 7;
        this->mMario->playSound("声投げ", -1);
        if (!this->isJumping()) {
            this->mMario->stopAnimation("ファイアスピン", (const char*)nullptr);
            this->mMario->stopAnimationUpper("ファイアスピン", (const char*)nullptr);
            this->mMario->changeAnimation("ファイアスピン", (const char*)nullptr);
            (this->mMario)->mWalkSpeed = 0.0f;
            return;
        }
        this->mMario->setFrontVecKeepUp((this->mMario)->mWorldPadDir);
        Mario* m = this->mMario;
        u16 c = (m)->_42C;
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
        (this->mMario)->_42C += 1;
        f32 k = this->mMario->cutGravityElementFromJumpVec(true);
        (this->mMario)->mJumpVec.scale(0.5f);
        TVec3f g(mAppliedGravityVec);
        g.scale(k);
        (this->mMario)->mJumpVec += g;
    }
}

void MarioActor::killAllFireBall() {
    int i = 0;
    do {
        if (!MR::isDead(mFireBalls[i])) {
            mFireBalls[i]->kill();
        }
        i++;
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
    if (d.dot((this->mMario)->mFrontVec) > c) {
        return false;
    }
    return true;
}




// NONMATCHING: sensor-loop register allocation and verified constant-pool addresses.
HitSensor* MarioActor::fn_803D0CE0(s32 a) {
    if (this->mHealth == 0 || this->_930 == 0) {
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
            while (i < this->_930) {
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
    if (((m->mMovementStates.mLowWord >> (9)) & 1)) {
        return false;
    }
    if (s == nullptr) {
        return false;
    }
    if (this->mHealth == 0) {
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
    mVelocity.zero();
    this->beginRush(s);
    return true;
}

bool MarioActor::fn_803D11C0() {
    Mario* m = this->mMario;
    if (((m->mMovementStates.mLowWord >> (9)) & 1)) {
        return true;
    }
    if (this->mHealth == 0) {
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
    b -= this->_240;
    f32 db = PSVECMag((Vec*)&b);
    TVec3f c(p);
    c -= this->mCapPosition;
    f32 dc = PSVECMag((Vec*)&c);
    if (da > db) {
        da = db;
    }
    if (da > dc) {
        da = dc;
    }
    return da;
}

// NONMATCHING: Typed sensor position uses direct member addresses; saved-register and frame scheduling differ.
void MarioActor::fn_803D1740(HitSensor* s) {
    if (this->mMario->isStatusActive(5)) {
        Mtx m;
        this->getRealMtx(m, "Spine1");
        MR::extractMtxTrans(m, &(s)->mPosition);
        (s)->mRadius = 60.0f;
        return;
    }
    if (this->mMario->isStatusActive(6)) {
        Mtx m;
        this->getRealMtx(m, "Spine1");
        MR::extractMtxTrans(m, &(s)->mPosition);
        (s)->mRadius = 70.0f;
        return;
    }
    (s)->mPosition.set(this->_240);
    Mario* m = this->mMario;
    u32 st = (m)->mMovementStates.mLowWord;
    if (((st >> 0x14) & 1) && !((st >> 0x1E) & 1)) {
        (s)->mPosition += (m)->mJumpVec;
    }
    (s)->mRadius = 100.0f;
}


void MarioActor::fn_803D1830(HitSensor* s) {
    (s)->mPosition.set(this->_240);
    (s)->mRadius = this->fn_803D18D0();
    this->fn_803D1940();
    bool r = false;
    if (s->mValidByHost != 0 && s->mValidBySystem != 0) {
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
    if (static_cast<s32>(_468) != 0) {
        return 2000.0f;
    }
    if (((this->mMario->mMovementStates.mLowWord >> (0x10)) & 1)) {
        return 1000.0f;
    }
    return 600.0f;
}

void MarioActor::fn_803D1940() {
    this->_3E5 = 0;
    this->_3E6 = 0;
    const char* cur = this->mMarioAnim->fn_803DF0D0();
    if (strcmp(cur, "spin2nd") == 0) {
        this->_3E5 = 1;
        this->_3E6 = 1;
        if (this->_944 <= 2) {
            this->setPunchHitTimer(2);
        }
    } else if (!this->mMario->isAnimationTerminate(nullptr) &&
               (((this->mMario)->_430 == 8 && this->isJumping()) || this->mMario->isAnimationRun("地上ひねり") ||
                this->mMario->isAnimationRun("雲スピン") || this->mMario->isAnimationRun("サマーソルト") ||
                this->mMario->isAnimationRun("水泳スピン") || this->mMario->isAnimationRun("水上スピン") ||
                this->mMario->isAnimationRun("しゃがみスピン") || this->mMario->isAnimationRun("フーファイタースピン") ||
                this->mMario->isAnimationRun("ハチスピン") || this->mMario->isAnimationRun("ハチスピン空中") ||
                this->mMario->isAnimationRun("アイスひねり空中") || this->mMario->isAnimationRun("ファイアスピン空中") ||
                this->mMario->isAnimationRun("ファイアスピン") || this->mMario->isAnimationRun("アイスひねり") ||
                this->mMario->isAnimationRun("アイスひねり移動") || this->mMario->isAnimationRun("アイスひねり静止") ||
                this->mMario->isAnimationRun("ハンマー投げリリース") ||
                (this->mMario->isSwimming() && this->mMario->mSwim->fn_803FE9F0()) ||
                (this->mMario->isStatusActive(0x18) && this->mMario->mFoo->_4C != 0))) {
        this->_3E5 = 1;
        this->_3E6 = 1;
    }
}

// NONMATCHING: boolean materialization, constant conversion and stack allocation.
void MarioActor::fn_803D26F0() {
    Mario* m = this->mMario;
    bool reflect = false;
    if (((m->mMovementStates.mLowWord >> 0x1D) & 1) && !m->fn_803AA2C0() && !mMario->isStatusActive(0x12) &&
        (mMario->_962 == 0xE || mMario->_962 == 0x21)) {
        const TVec3f* gravity = mMario->getGravityVec();
        TVec3f camera = MR::getCamZdir();
        reflect = gravity->dot(camera) >= 0.0f;
    }
    if (reflect) {
        this->fn_803CA1F0();
        this->mGroundReflectionEFC = 1;
        f32 v = (this->mMario)->mVerticalSpeed / 500.0f;
        if (v >= 0.75f) {
            v = 0.75f;
        }
        s32 target = (s32)((f32)0x40U * (1.0f - v));
        for (u32 i = 0; i < 4; ++i) {
            u8 c = this->mGroundReflectionAlphaEFD;
            if (c < (u8)target) {
                this->mGroundReflectionAlphaEFD = c + 1;
            } else if (c > (u8)target) {
                this->mGroundReflectionAlphaEFD = c - 1;
            }
        }
    } else {
        u8 c = this->mGroundReflectionAlphaEFD;
        if (c != 0) {
            if (c > 4) {
                this->mGroundReflectionAlphaEFD = c - 4;
            } else {
                this->mGroundReflectionAlphaEFD = 0;
            }
            if (this->mGroundReflectionAlphaEFD == 0) {
                this->fn_803CA1F0();
                this->mShadowEnabledEFA = 1;
            } else {
                this->fn_803CA1F0();
                this->mGroundReflectionEFC = 1;
            }
        } else {
            this->fn_803CA1F0();
            this->mShadowEnabledEFA = 1;
        }
    }
    if (this->mMario->isSwimming() && MR::isInWater(mCameraPosition)) {
        if (this->mFlagF19 != 0 || mCameraBasisZ.dot(-*this->mMario->getGravityVec()) > 0.0f) {
            this->fn_803CA1F0();
            this->mWaterReflectionEF8 = 1;
        }
        if (this->mFlagF18 != 0 ||
            (this->mMario->mSwim->getWaterEdgeDist() > 0.0f && this->mMario->mSwim->getWaterEdgeDist() < 1000.0f)) {
            this->fn_803CA1F0();
            this->mWaterReflectionEF8 = 1;
            this->mSphereReflectionEF9 = 1;
            this->mShadowEnabledEFA = 1;
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

// NONMATCHING: saved-register allocation and 0.1/0.9 blend load scheduling only.
void MarioActor::fn_803D32F0() {
    mWallShadow574->mActive = 0;
    if (this->mGroundReflectionEFC || this->mWaterReflectionEF8 || this->mSphereReflectionEF9) {
        return;
    }
    if (this->mBeeWallWalk) {
        return;
    }

    TVec3f axis;
    s32 len = 0;
    AreaObj* area = MR::getAreaObj("ExtraWallCheckArea", this->_240);
    if (area != nullptr) {
        MR::calcCubeAxisZ(area, &axis);
        len = MR::getAreaObjArg(area, 0);
        mExtraWallArea578 = area;
        mExtraWallAreaKind57C = 0;
    } else {
        area = MR::getAreaObj("ExtraWallCheckCylinder", this->_240);
        if (area != nullptr) {
            len = this->calcCylinderToCenter(area, &axis);
            mExtraWallArea578 = area;
            mExtraWallAreaKind57C = 1;
        }
    }

    bool fading = false;
    if (area == nullptr || (f32)len < 0.0f) {
        f32 f = mWallShadow574->mDistance;
        if (f >= 1000.0f) {
            return;
        }
        mWallShadow574->mDistance = 10.0f + f;
        if (mWallShadow574->mDistance >= 1000.0f) {
            mWallShadow574->mDistance = 1000.0f;
        }
        switch (mExtraWallAreaKind57C) {
        case 0:
            MR::calcCubeAxisZ(mExtraWallArea578, &axis);
            len = MR::getAreaObjArg(mExtraWallArea578, 0);
            break;
        case 1:
            len = this->calcCylinderToCenter(mExtraWallArea578, &axis);
            break;
        }
        fading = true;
    }

    Triangle tri;
    TVec3f hit;
    if (!this->mMario->fn_803B2770(&hit, &tri, this->_240, axis * (f32)len)) {
        return;
    }

    mWallShadow574->mPosition.set(hit);
    mWallShadow574->mNormal.set(*tri.getNormal(0));
    if (!fading) {
        mWallShadow574->mDistance = 0.9f * mWallShadow574->mDistance + 0.1f * (hit - this->_240).length();
    }
    this->updateRandomTexture(mWallShadow574->mDistance);
    TVec3f nrm(axis);
    MR::normalizeOrZero(&nrm);
    J3DModel* mdl = mAllModel->getJ3DModel();
    J3DModelX* mx = mSimpleModelF08;
    PSMTXCopy(mdl->getBaseTRMtx(), mx->getBaseTRMtx());
    TPos3f& rot = mx->mBaseTransformMtx;
    TVec3f x;
    rot.getXDir(x);
    MR::vecKillElement(x, nrm, &x);
    TVec3f y;
    rot.getYDir(y);
    MR::vecKillElement(y, nrm, &y);
    TVec3f z;
    rot.getZDir(z);
    MR::vecKillElement(z, nrm, &z);
    rot.setXDir(x);
    rot.setYDir(y);
    rot.setZDir(z);
    const TVec3f* gravity = this->mMario->getGravityVec();
    TVec3f perp;
    MR::vecKillElement(this->mPosition - hit, *gravity, &perp);
    f32 dist = perp.length();
    TVec3f trans = this->mPosition + nrm * (10.0f + dist);
    MR::setMtxTrans(rot.toMtxPtr(), trans.x, trans.y, trans.z);
    PSMTXConcat(rot.toMtxPtr(), mReflectionBaseMtx264.mMtx, rot.toMtxPtr());
    mx->viewCalcRef(3, mdl);
    mWallShadow574->mActive = 1;
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

// NONMATCHING: vector temporary stack slots and literal identities.
void MarioActor::fn_803D82C0(TVec2f* a, TVec2f* b) const {
    TVec3f thr;
    this->getThrowVec(&thr);
    TDDraw::project2D(a, this->mPosition);
    TDDraw::project2D(b, mPosition + thr * 10000.0f);
    TVec2f lo;
    TVec2f hi;
    if (a->x >= b->x) {
        lo.x = b->x;
        hi.x = a->x;
    } else {
        lo.x = a->x;
        hi.x = b->x;
    }
    if (a->y >= b->y) {
        lo.y = b->y;
        hi.y = a->y;
    } else {
        lo.y = a->y;
        hi.y = b->y;
    }
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

// NONMATCHING: Equivalent model/material saved-register allocation.
s32 MarioActor::fn_803D8490() const {
    TDDraw::setup(0, 1, 1);
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
    c.a = (u8)(s32)(f * (f32)mConst->getTable()->mSearchLightColor[3]);
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
    const MarioConstTable* tbl = mConst->getTable();
    u32 col = static_cast<u32>(tbl->mSearchLightColor[0]) << 24;
    col |= static_cast<u32>(tbl->mSearchLightColor[1]) << 16;
    col |= static_cast<u32>(tbl->mSearchLightColor[2]) << 8;
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
    c.a = (u8)(s32)(f * (f32)(mConst->getTable())->mSearchLightAlphaLevel);
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
            d.a = (u8)(s32)(f * (f32)mConst->getTable()->mSearchLightBlurAlpha[i]);
            GXSetTevColorS10(GX_TEVREG0, d);
            GXSetTevColorS10(GX_TEVREG1, c2);
            m->simpleDrawShape(mat);
            i++;
        } while (i < 3);
    }
}

void MarioActor::fn_803D8A30(s32 a, TVec3f& v) {
    MarioSearchLight* p = mSearchLight;
    if (p == nullptr) {
        return;
    }
    if (a != 0) {
        p->_A8 = 1;
        p->_AC = v;
        return;
    }
    p->_A8 = 0;
}

void MarioActor::fn_803D8A70(s32 mode) {
    u16 t = this->_3D8;
    if (t != 0 && mPlayerMode == 0) {
        if (mode != mPlayerMode && this->_3D6 == 6) {
            if (t > 0x1A) {
                (mTeresaModel->mActor)->appear();
            }
            this->_483 = 1;
            this->updateHand();
            this->updateFace();
            return;
        }
        if (this->_3D6 == 6) {
            (mTeresaModel->mActor)->kill();
            this->_483 = 0;
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
    if (PSVECMag((Vec*)&mVelocity) < ma) {
        if (a->dot(*this->mMario->getGravityVec()) < -0.0f) {
            TVec3f k;
            MR::vecKillElement(mVelocity, *mMario->getGravityVec(), &k);
            if (MR::isNearZero(k, 0.001f)) {
                MR::vecKillElement(*a, *this->mMario->getGravityVec(), &k);
            }
            k.setLength(PSVECMag((Vec*)a));
            this->mMario->push(k);
            if ((this->mMario)->_3BC <= 2) {
                f32 m2 = PSVECMag((Vec*)b);
                f32 v = m2;
                if (m2 > 10.0f) {
                    v = 10.0f;
                }
                TVec3f up = -*this->mMario->getGravityVec();
                up.scale(v);
                (this->mMario)->mJumpVec += up;
            }
        }
    } else {
        TVec3f n((this->mMario)->_8F8);
        MR::normalizeOrZero(&n);
        TVec3f tmp;
        f32 f1 = MR::vecKillElement(*a, n, &tmp);
        f32 f2 = MR::vecKillElement(mVelocity, n, &tmp);
        if (PSVECMag((Vec*)&mVelocity) > 20.0f && f1 < 0.5f * f2) {
            if (this->mMario->isAnimationRun("坂すべり下向きあおむけ")) {
                TVec3f p((this->mMario)->mFrontVec);
                p.scale(5.0f);
                this->mMario->push(p);
            } else if (this->mMario->isAnimationRun("坂すべり上向きうつぶせ")) {
                TVec3f p((this->mMario)->mFrontVec);
                p.scale(-5.0f);
                this->mMario->push(p);
            }
            (this->mMario)->mFlags14.mHighWord |= 0x20000000;
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
            if (!(this->mMario->getGravityVec()->dot(d) >= -5.0f)) {
                CollisionParts* h = tri->mParts;
                if (h != nullptr && h->mStationaryFrames == 0) {
                    if (((this->mMario)->mMovementStates.mHighWord & 3) == 1) {
                        return;
                    }
                    this->mPosition = hit;
                    this->mMario->mPosHolder->fn_80405F90(this->mPosition);
                    this->mMario->stopJump();
                    this->mMario->changeAnimation("基本", "落下");
                    this->mMario->updateGroundInfo();
                }
            }
        }
    } else if (v->dot(*this->mMario->getGravityVec()) > 40.0f) {
        Triangle* gt = (this->mMario)->_4C8;
        if (gt->isValid()) {
            if (MR::isSensorPressObj(gt->getSensor()) || MR::getFloorCodeIndex((this->mMario)->_4C8) == 0x1D ||
                MR::getFloorCodeIndex((this->mMario)->_4C8) == 0x27) {
                (this->mMario)->mDrawStates.mWord |= 2;
            }
        } else {
            u32 i = 0;
            u32 n = (this->mBinder)->mPlaneNum;
            while (i < n) {
                const HitInfo* p = this->mBinder->getPlane(i);
                if (MR::isSensorPressObj(p->mParentTriangle.getSensor())) {
                    (this->mMario)->mDrawStates.mWord |= 2;
                }
                i++;
            }
        }
        TVec3f d((this->mMario)->mShadowPos);
        d -= this->mPosition;
        if (d.dot(*this->mMario->getGravityVec()) < 0.0f && !this->fn_803D9370(this->mMario->_45C->mParts, v)) {
            this->mPosition = (this->mMario)->mShadowPos;
            (this->mMario)->mJumpVec.zero();
            (this->mMario)->_148.zero();
            Mario* m = this->mMario;
            if (m->_5FC == nullptr && (((m->mDrawStates.mWord >> (1)) & 1) || ((m->mPrevDrawStates.mWord >> (1)) & 1))) {
                TVec3f nv(*v);
                MR::normalizeOrZero(&nv);
                mPressNormal6D8 = nv;
                this->setPress(0, 0);
                this->_3B0 = 0.1f;
            }
        }
    }
}

void MarioActor::fn_803D9170(TVec3f* v) {
    u32 n = (this->mBinder)->mPlaneNum;
    s32 hit = 0;
    f32 zero = 0.0f;
    u32 i = 0;
    while (i < n) {
        Triangle* tri = (Triangle*)this->mBinder->getPlane(i);
        if (MR::isSensorPressObj(tri->getSensor())) {
            if ((this->mMario)->_368.dot(*tri->getNormal(0)) > zero) {
                if ((this->mMario)->_72C < 200.0f) {
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
                } else if (this->mMario->_5FC != nullptr) {
                    if (!MR::isWallCodeNoAction(tri) && !this->mMario->isOnimasuBinderPressSkip()) {
                        mPressNormal6D8 = (this->mMario)->_368;
                        (this->mMario)->mFlags14.mLowWord &= 0xFFFFFFFB;
                        *(this->mMario)->_4C8 = *tri;
                        this->setPress(2, 0);
                        this->_3B0 = 0.1f;
                    }
                    hit = 1;
                }
            }
        }
        i++;
    }
    if (hit != 0) {
        TVec3f tmp;
        f32 k = MR::vecKillElement(*v, (this->mMario)->_368, &tmp);
        TVec3f s((this->mMario)->_368);
        s.scale(k);
        this->mPosition -= s;
    }
}

bool MarioActor::fn_803D9370(void* tri, TVec3f* v) {
    CollisionParts* parts = static_cast<CollisionParts*>(tri);
    if (tri == nullptr) {
        return false;
    }
    if (parts->mStationaryFrames != 0) {
        return false;
    }
    TVec3f a, b;
    PSMTXMultVec(parts->mInvBaseMatrix.mMtx, (Vec*)&(this->mMario)->mShadowPos, (Vec*)&a);
    PSMTXMultVec(parts->mPrevBaseMatrix.mMtx, (Vec*)&a, (Vec*)&b);
    TVec3f d((this->mMario)->mShadowPos);
    d -= b;
    return v->dot(d) > 0.0f;
}

bool MarioActor::isSilhouetteVisible() const {
    if (this->isAllHidden()) {
        return false;
    }
    if (this->mMario->isStatusActive(0x1A)) {
        return false;
    }
    if (this->mMario->isStatusActive(0x1B)) {
        TVec3f a(this->mPosition);
        a -= mCameraPosition;
        if (!this->mMario->fn_803B2880(mCameraPosition, a)) {
            return false;
        }
        TVec3f b(mCapPosition);
        b -= mCameraPosition;
        if (!this->mMario->fn_803B2880(mCameraPosition, b)) {
            return false;
        }
    }
    u8 t = mMario->mSinkTimer;
    if (t > 0x40) {
        return false;
    }
    if (t != 0) {
        TVec3f c(mCapPosition);
        c -= mCameraPosition;
        if (!this->mMario->fn_803B2880(mCameraPosition, c)) {
            return false;
        }
    }
    if (mPlayerMode == 6) {
        return false;
    }
    if (this->mMario->isStatusActive(0x12) && mFirstPersonDrawHidden1A5 != 0) {
        return false;
    }
    if (_EA4 != 0) {
        return false;
    }
    bool r = false;
    if (mAnimationDelayE28 == 0 && (_934 == 0 || _7E2 == 1)) {
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
    reinterpret_cast<s16&>(this->_1A8) = 0;
    this->mBlinkRequested1067 = 1;
}

void MarioActor::fn_803D9730() {
    this->mBlinkRequested1067 = 0;
}

void MarioActor::fn_803D9740() {
    if (gIsLuigi) {
        if (this->mMaxHealth != 3 && this->mHealth <= 3) {
            this->mMaxHealth = 3;
        }
    } else if (this->mMaxHealth != 3 && this->mHealth <= 3) {
        this->mMaxHealth = 3;
    }
}

void MarioActor::decreaseLifeByOne() {
    if (mMario->isPlayerModeYoshi()) fn_803B9870();
    if (mHealth) --mHealth;
    this->mLifeDecreasedThisFrame = 1;
}
