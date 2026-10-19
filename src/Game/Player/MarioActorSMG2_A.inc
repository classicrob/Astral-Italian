#include "Game/Util/PlayerUtil.hpp"
// SMG2-only MarioActor functions (part A), written from the asm. Included at the end of MarioActor.cpp (unity TU).
// PLACEHOLDER(enemies): field access by raw offset (`F(T, off)`) because the MarioActor header offsets are partly guesses; resolve when the class layout is recovered and the fields are named
#define F(T, off) (*(T*)((u8*)this + (off)))
#define FP(p, T, off) (*(T*)((u8*)(p) + (off)))


void MarioActor::exeGameOverNoDemo() {
    if (MR::isFirstStep(this)) {
        MR::setCubeBgmChangeInvalid();
        MR::clearBgmQueue();
        if (MR::getPlayerLeft() == 0) {
            MR::startPlayerEvent("ゲームオーバー");
        } else {
            MR::startPlayerEvent("演出なし死");
            F(s32, 0x6B0) = 0;
        }
        MR::startStarPointerModeDemoMarioDeath(this);
    }
}

const TVec3f& MarioActor::fn_803B8520() {
    return this->mMario->getAirGravityVec();
}

u32 MarioActor::fn_803B89E0() {
    return (FP(this->mMario, u32, 0xC) >> 0x14) & 1;
}

bool MarioActor::fn_803B8A80() {
    return this->mMario->fn_80393C40();
}

void MarioActor::fn_803B9270() {
    this->decLife(0);
}

bool MarioActor::fn_803B93E0() {
    return MR::isNearZero(mPreviousGravityVec, 0.001f);
}

void MarioActor::fn_803B94E0() {
    Mario* m = this->mMario;
    if (FP(m, u8, 0xA7B) == 0) {
        if ((FP(m, u32, 0xC) >> 0x10) & 1) {
            m->forceStopTornado();
        }
        if (mActionTable1080->isActionOk("ダメージ解除")) {
            this->setPlayerMode(0, true, true);
        }
    }
}

void MarioActor::fn_803B9560() {
    if (mActionTable1080->isActionOk("ノーダメージ解除")) {
        this->setPlayerMode(0, true, true);
    }
}

Yoshi* MarioActor::fn_803B97F0() {
    return mYoshiHolder->mYoshi;
}

void* MarioActor::fn_803B9800() {
    return mYoshiHolder->fn_8041B6F0();
}

YoshiTongueAction* MarioActor::fn_803B9810() {
    return mYoshiHolder->mTongueAction;
}

void* MarioActor::fn_803B9820() {
    return mYoshiHolder->_C;
}

YoshiHolder* MarioActor::fn_803B9830() {
    return mYoshiHolder;
}

bool MarioActor::fn_803B9840() {
    return mYoshiHolder != nullptr;
}

MtxPtr MarioActor::fn_803B9850() {
    return mYoshiHolder->mYoshi->getBaseMtx();
}

void MarioActor::fn_803B9870() {
    if (this->mMario->isPlayerModeYoshi()) {
        mYoshiHolder->mYoshi->fn_8040CF20();
    }
}

void MarioActor::fn_803B98C0() {
    if (*(void* volatile*)((u8*)this + 0xEE4) != nullptr) {
        FP(mYoshiHolder, u8, 0x3C) = 1;
    }
}

void MarioActor::fn_803B98E0() {
    if (*(void* volatile*)((u8*)this + 0xEE4) != nullptr) {
        FP(mYoshiHolder, u8, 0x3C) = 0;
    }
}

s32 MarioActor::fn_803B9900() {
    YoshiHolder* p = mYoshiHolder;
    if (p != nullptr) {
        return FP(p, u8, 0x3C);
    }
    return 0;
}

bool MarioActor::fn_803B9920() {
    if (!this->mMario->isPlayerModeYoshi()) {
        return false;
    }
    if (F(u8, 0xE24) != 0 || F(u16, 0x6B8) != 0) {
        return false;
    }
    if (F(u8, 0xE2C) != 0) {
        return false;
    }
    Mario* m = this->mMario;
    u32 r = FP(m, u32, 0x18);
    if ((r >> 0x14) & 1) {
        return false;
    }
    if ((r >> 0x15) & 1) {
        return false;
    }
    if ((FP(m, u32, 0xC) >> 0x1F) == 1 && !m->isRising() && ((FP(this->mMario, u32, 0xC) >> 0xC) & 1)) {
        return true;
    }
    return this->checkButtonType(mInputConfig->_8, false);
}

void MarioActor::fn_803B9A10() {
    if (*(void* volatile*)((u8*)this + 0xEE4) != nullptr) {
        mYoshiHolder->fn_8041BAE0();
    }
}

void MarioActor::fn_803B9A30() {
    if (*(void* volatile*)((u8*)this + 0xEE4) != nullptr) {
        mYoshiHolder->fn_8041BAF0();
    }
}

void MarioActor::fn_803B9A50() {
    if (*(void* volatile*)((u8*)this + 0xEE4) != nullptr) {
        mYoshiHolder->fn_8041BB00();
    }
}

s32 MarioActor::fn_803B9A70() {
    YoshiHolder* p = mYoshiHolder;
    if (p != nullptr) {
        return p->fn_8041BB30();
    }
    return 0;
}

f32 MarioActor::fn_803B9A90() {
    YoshiHolder* p = mYoshiHolder;
    if (p != nullptr) {
        return p->fn_8041BB50();
    }
    return 0.0f;
}

bool MarioActor::fn_803B9AB0() {
    YoshiHolder* p = mYoshiHolder;
    if (p == nullptr) {
        return false;
    }
    if (FP(p, LiveActor*, 0x18) == nullptr) {
        return false;
    }
    return !MR::isDead(FP(p, LiveActor*, 0x18));
}

void MarioActor::fn_803B9F10() {
    this->tryReleaseWithMsg(0x2F);
    this->updateSearchLight();
    if (_424 != nullptr) {
        _424->receiveMessage(0x2F, this->getSensor("Body"));
        _424 = nullptr;
    }
}

void MarioActor::fn_803BA070() {
    if (static_cast<s32>(_468) == 0) {
        if (this->mMario->isAnimationRun("ひろいクイック")) {
            this->mMario->stopAnimationUpperForce();
            F(s8, 0xFBA) = 0;
            mNullAnimation->kill();
            F(u8, 0x768) = 0;
        }
    } else {
        for (u32 i = 0; i < (u32)static_cast<s32>(_468); i++) {
            if (this->fn_803CEEE0(_428[i])) {
                _428[i]->receiveMessage(0x2D, this->getSensor("Body"));
                _428[i] = nullptr;
                _468 -= 1;
            } else if (_428[i] != nullptr) {
                this->fn_803D5A00(i);
            }
        }
        _474 = nullptr;
        if (static_cast<s32>(_468) == 0) {
            this->mMario->stopAnimationUpperForce();
            this->mMario->mSwim->resetJet();
        }
        if (F(u8, 0x768) != 0) {
            this->mMario->stopAnimationUpperForce();
            F(u8, 0x768) = 0;
        }
        F(s8, 0xFBA) = 0;
        mNullAnimation->kill();
        this->tryReleaseWithMsg(0x2D);
        this->updateSearchLight();
        if (_424 != nullptr) {
            _424->receiveMessage(0x2C, this->getSensor("Body"));
            _424 = nullptr;
        }
    }
}

void MarioActor::fn_803BA220() {
    F(u8, 0x768) = 0;
}

void MarioActor::fn_803BAFE0() {
    if (this->isEnableNerveChange()) {
        this->setNerve(&NrvMarioActor::MarioActorNrvGameOverSink::sInstance);
    }
}

void MarioActor::fn_803BB4F0() {
    if (mPlayerMode == 6) {
        MR::requestMovementOn(mTeresaModel->mActor);
    }
}

void MarioActor::fn_803BB510(const TVec3f& a, const TVec3f& b) {
    MR::emitEffectHit(mTeresaModel->mActor, a, b, "WallHit");
}

void MarioActor::fn_803BE6D0() {
    if (F(void*, 0xF74) != nullptr) {
        F(FootPrint*, 0xF74)->clearForce();
    }
}

void MarioActor::fn_803BE650(u32 a) {
    if (a == 0xD) {
        F(s16, 0xE42) = 0x13;
    }
}

void MarioActor::fn_803BE670() {
    MR::requestMovementOn(this);
    if (this->mMario->isPlayerModeYoshi()) {
        MR::requestMovementOn(mYoshiHolder->mYoshi);
        mYoshiHolder->mTongueAction->fn_8041FF70();
    }
}

void MarioActor::fn_803BE6F0() {
    if (F(u8, 0xE24) != 0) {
        HitSensor* s = _924;
        if (s != nullptr && s->isType(0x65)) {
            F(u8, 0xFB9) = 1;
            F(u8, 0xFB8) = 1;
        }
        if (F(u8, 0xE24) != 0) {
            if (!_924->receiveMessage(0x95, this->getSensor("Body"))) {
                _924->receiveMessage(0x96, this->getSensor("Body"));
            }
            if (F(u8, 0xE24) != 0) {
                RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
                this->endRush(&info);
            }
        }
        F(u8, 0xE24) = 0;
        F(u8, 0xE25) = 0;
    }
}


bool MarioActor::fn_803BE800() {
    MarioAllModel* m = mAllModel;
    J3DModel* c = FP(m, J3DModel*, 0xC);
    return m->getJ3DModel() == c;
}


void MarioActor::fn_803BE840(const TVec3f& a) {
    if (this->mMario->isPlayerModeYoshi() && mYoshiHolder->mYoshi->fn_8040CF40()) {
        this->mMario->fn_803B1100();
        mYoshiHolder->mYoshi->fn_8040CD20(a);
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803BE8D0(const TVec3f& a) {
    if (this->mMario->isPlayerModeYoshi() && mYoshiHolder->mYoshi->fn_8040CF40()) {
        this->mMario->fn_803B1100();
        mYoshiHolder->mYoshi->fn_8040CDD0(a);
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803BE960() {
    if (this->mMario->isPlayerModeYoshi() && mYoshiHolder->mYoshi->fn_8040CF40()) {
        this->mMario->fn_803B1100();
        mYoshiHolder->mYoshi->fn_8040CE80();
        this->setPlayerMode(0, true, true);
    }
}

void MarioActor::fn_803BE9E0() {
    if (this->mMario->isPlayerModeYoshi() && mYoshiHolder->mYoshi->fn_8040CF40()) {
        this->mMario->fn_803B1100();
        mYoshiHolder->mYoshi->fn_8040CED0();
        this->setPlayerMode(0, true, true);
    }
}


void MarioActor::fn_803BEA60(const char* a, const char* b) {
    this->fn_803CAD10(a, b, nullptr);
}

void MarioActor::fn_803BEA70(const char* a, const BckCtrlData* b) {
    this->fn_803CAD10(a, nullptr, b);
}


bool MarioActor::fn_803BEA80(const char* pName) {
    if (mHairAndHat1090 != nullptr) {
        return mHairAndHat1090->fn_803EC7F0(pName);
    }
    return false;
}

bool MarioActor::fn_803BEAA0(const char* pName, const BckCtrlData& rData) {
    if (mHairAndHat1090 != nullptr) {
        return mHairAndHat1090->fn_803EC8A0(pName, rData);
    }
    return false;
}

void MarioActor::fn_803BEAC0() {
    if (mHairAndHat1090 != nullptr) {
        mHairAndHat1090->fn_803EC780();
    }
    MR::startPlayerNormalBva();
}

void MarioActor::fn_803BEAF0() {
    if (F(LiveActor*, 0x1090) != nullptr) {
        MR::requestMovementOn(F(LiveActor*, 0x1090));
    }
}


void MarioActor::fn_803BEB10(s32 a) {
    TVec3f up(*this->mMario->getGravityVec());
    up.scale(150.0f);
    TVec3f from(F(TVec3f, 0x5F4));
    from -= up;
    TVec3f dir(*this->mMario->getGravityVec());
    dir.scale(100.0f);
    if (this->mMario->fn_803B2880(from, dir)) {
        mPressNormal6D8 = -*this->mMario->getGravityVec();
        this->setPress(4, a);
    }
}

bool MarioActor::fn_803BEBE0() {
    if (F(u8, 0xE24) == 0) {
        return false;
    }
    if (_924 == nullptr) {
        return false;
    }
    if (MR::getSensorHost(_924) == nullptr) {
        return false;
    }
    const char* name = MR::getSensorHost(_924)->mName;
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

LiveActor* MarioActor::fn_803BECC0() {
    if (this->mMario->isPlayerModeYoshi()) {
        return mYoshiHolder->mYoshi;
    }
    return this;
}

bool MarioActor::fn_803BED10() {
    return this->isNerve(&NrvMarioActor::MarioActorNrvGameOverSink::sInstance);
}


// NONMATCHING: typed count/pointer clears replace redundant vector type-pun; loop registers and scheduling differ.
void MarioActor::initMember() {
    F(u8, 0x90) = 0;
    fn_803BF6F0();
    F(u8, 0x560) = 0;
    F(u8, 0x561) = 0;
    _1B8 = nullptr;
    mMessenger568 = nullptr;
    F(u8, 0x56C) = 0;
    F(u8, 0x56D) = 0;
    F(u8, 0x56E) = 0;
    F(u16, 0x570) = 0;
    mActionHash = nullptr;
    mScreenBlur1074 = new MarioScreenBlur();
    mScreenStrip1084 = new MarioScreenStrip();
    mConditionGroup = new MarioActorConditionGroup(this);
    mSensorSlots108C = new MarioSensorSlots();
    mHairAndHat1090 = nullptr;
    F(u8, 0x573) = 0;
    F(u8, 0x572) = 0;
    mWallShadow574 = nullptr;
    F(u32, 0x578) = 0;
    F(u8, 0x57C) = 0;
    mCollisionShadow580 = nullptr;
    fn_803CA1F0();
    mMario = nullptr;
    mMarioAnim = nullptr;
    mMarioEffect = nullptr;
    mConst = nullptr;
    gIsLuigi = false;
    F(f32, 0x594) = 0.0f;
    F(f32, 0x598) = 1.0f;
    F(f32, 0x59C) = 0.0f;
    F(f32, 0x5A0) = 0.0f;
    F(f32, 0x5A4) = 1.0f;
    F(f32, 0x5A8) = 0.0f;
    mGravityInfo = nullptr;
    F(u8, 0x6A0) = 0;
    F(f32, 0x6A4) = 0.0f;
    F(u16, 0x664) = 0;
    F(TVec3f, 0x654).zero();
    F(f32, 0x5AC) = 0.0f;
    F(f32, 0x5B0) = 0.0f;
    F(f32, 0x5B4) = 1.0f;
    mLastMove.zero();
    F(TVec3f, 0x5C4).zero();
    mBinderMotion5D0.zero();
    mBinderDirection5DC.zero();
    mPreviousPosition.zero();
    F(TVec3f, 0x5F4).zero();
    F(TVec3f, 0x600).zero();
    mVector60C.zero();
    F(u16, 0x6D0) = 0;
    F(TVec3f, 0x618).zero();
    F(TVec3f, 0x624).zero();
    F(TVec3f, 0x630).zero();
    mCameraTrans.zero();
    F(TVec3f, 0x648).zero();
    mCounter660 = 0;
    mWaterSurfaceCameraTimer662 = 0;
    F(u8, 0x6E5) = 0;
    F(u8, 0x666) = 0;
    F(u16, 0x668) = 0;
    F(TVec3f, 0x66C).zero();
    F(TVec3f, 0x678).zero();
    F(TVec3f, 0x684).zero();
    F(TVec3f, 0x690).zero();
    F(u16, 0x6A8) = 0;
    F(u32, 0x6AC) = 0;
    F(u32, 0x1AC) = 0;
    F(u32, 0x6B0) = 0;
    F(u32, 0x6B4) = 0;
    F(u8, 0x1B0) = 0;
    F(u8, 0x6CC) = 0;
    F(u16, 0x6B8) = 0;
    F(u32, 0x6BC) = 0;
    F(u32, 0x6C0) = 0;
    F(u32, 0x6C4) = 0;
    mCounter6C8 = 0;
    F(f32, 0x6D4) = 0.0f;
    mPressNormal6D8.zero();
    F(u16, 0x6CE) = 0;
    F(u16, 0x6D2) = 0;
    F(u8, 0x6E4) = 0;
    F(TVec3f, 0x6E8).zero();
    F(u16, 0x6F4) = 0;
    F(u16, 0x6F6) = 0;
    mPlayerMode = 0;
    F(u16, 0x6FA) = 0;
    F(u16, 0x6FC) = 0;
    F(u16, 0x6FE) = 0;
    F(u16, 0x700) = 0;
    F(u8, 0x702) = 0;
    F(u8, 0x703) = 0;
    _4A4 = nullptr;
    mSensor794 = nullptr;
    F(u8, 0x1065) = 0;
    F(u8, 0x704) = 0;
    F(u8, 0x705) = 0;
    F(u8, 0x706) = 0;
    F(u8, 0x707) = 0;
    _424 = nullptr;
    _468 = 0;
    for (s32 i = 0; i < 4; i++) {
        _428[i] = nullptr;
    }
    _46C = nullptr;
    _470 = nullptr;
    _474 = nullptr;
    F(f32, 0x760) = 0.0f;
    F(u32, 0x764) = 0;
    F(u8, 0x768) = 0;
    mThrowProbePosition76C.zero();
    mCarryWobblePhase778 = 0.0f;
    _494 = nullptr;
    mCarryFixedPosition780 = nullptr;
    mTurnipFixedPosition784 = nullptr;
    mDrillFixedPosition788 = nullptr;
    F(u32, 0x1054) = 0;
    F(TVec3f, 0x1058).zero();
    F(u8, 0x769) = 0;
    F(u8, 0x76A) = 0;
    F(u8, 0x76B) = 0;
    mSearchLight = nullptr;
    mSearchLightTimer = 0;
    mSearchLightThrowPos = nullptr;
    mNearestUnderSensor798 = nullptr;
    mNearestUnderAngle79C = 0.0f;
    mBodyRadius7A0 = 0.0f;
    mBodyHeight7A4 = 0.0f;
    mBodyUpDirection.zero();
    mBodyDownDirection.zero();
    for (s32 i = 0; i < 128; i++) {
        mPulledSensors7C0[i] = nullptr;
    }
    F(u8, 0x9C0) = 0;
    mSpinRadius = 0.0f;
    mSpinHeight = 0.0f;
    for (s32 i = 0; i < 128; i++) {
        mSpinNoise[i] = 0.0f;
    }
    for (s32 i = 0; i < 64; i++) {
        mTornadoSensors[i] = nullptr;
    }
    F(u16, 0xCCC) = 0;
    F(u16, 0xCCE) = 0;
    F(u16, 0xCD0) = 0;
    for (s32 i = 0; i < 64; i++) {
        _7E4[i] = nullptr;
    }
    for (s32 i = 0; i < 64; i++) {
        FP(this, u8, 0xDD4 + i * 1) = 0;
    }
    _924 = nullptr;
    _928 = nullptr;
    mLastRushEndFrameE1C = 0;
    F(u32, 0xE20) = 0;
    F(u8, 0xE24) = 0;
    F(u8, 0xE25) = 0;
    F(u32, 0xE28) = 0;
    F(u8, 0xE2C) = 0;
    F(TVec3f, 0xE30).zero();
    F(u8, 0xE3C) = 0;
    F(u8, 0xE3D) = 0;
    F(u16, 0xE3E) = 0;
    F(u16, 0xE40) = 0;
    F(u16, 0xE42) = 0;
    F(u16, 0xE44) = 0;
    F(u8, 0xE46) = 0;
    for (s32 i = 0; i < 8; i++) {
        _954[i] = nullptr;
    }
    _974 = 0;
    F(TVec3f, 0xE6C).zero();
    F(f32, 0xE78) = 0.0f;
    F(u8, 0xE7C) = 0;
    F(u8, 0xE7D) = 0;
    mTornadoMario = nullptr;
    F(u8, 0xE84) = 0;
    _994 = nullptr;
    mJetTurtleShadowE94 = nullptr;
    mTeresaModel = nullptr;
    mModelE9C = nullptr;
    _9C4 = nullptr;
    _9C0 = nullptr;
    F(f32, 0xEA8) = 0.0f;
    F(f32, 0xEAC) = 0.0f;
    _9D4 = nullptr;
    F(TVec3f, 0xEB4).zero();
    _9E4 = nullptr;
    _9E8 = nullptr;
    _9EC = nullptr;
    F(u8, 0xECC) = 0;
    F(u8, 0xECD) = 0;
    F(u16, 0xECE) = 0;
    mGravityReferenceED0.zero();
    mModelEDC = nullptr;
    mModelEE0 = nullptr;
    mModelEE8 = nullptr;
    mModelEEC = nullptr;
    mModelEF4 = nullptr;
    F(u8, 0xEFD) = 0;
    mAllModel = new MarioAllModel(this);
    mSimpleModelActorF04 = nullptr;
    mSimpleModelF08 = nullptr;
    F(TVec3f, 0xF0C).zero();
    F(u8, 0xF18) = 0;
    F(u8, 0xF19) = 0;
    mModelF1C = nullptr;
    _A10 = nullptr;
    mModelF24 = nullptr;
    mModelF28 = nullptr;
    mHandModelL = nullptr;
    mHandModelR = nullptr;
    F(u8, 0xF34) = 0;
    F(u8, 0xF35) = 0;
    F(u8, 0xF36) = 0;
    F(u8, 0xF37) = 0;
    mModelF38 = nullptr;
    F(u8, 0xF3C) = 0;
    mActorEffectState = new MarioActorEffectState();
    F(u8, 0x1B1) = 0;
    for (s32 i = 0; i < 8; i++) {
        mBlurMatrices[0][i] = nullptr;
        mBlurMatrices[1][i] = nullptr;
    }
    PSMTXIdentity(mPreviousViewMatrix.toMtxPtr());
    PSMTXIdentity(mCurrentViewMatrix.toMtxPtr());
    mBlurBufferIndex = 0;
    mBlurHistoryIndex = 0;
    mBlurJointCount = 0;
    F(TVec3f, 0xF44).zero();
    F(f32, 0xF54) = 0.0f;
    F(f32, 0xF50) = 0.0f;
    F(f32, 0xF5C) = 0.0f;
    F(f32, 0xF58) = 0.0f;
    F(f32, 0xF64) = 0.0f;
    F(f32, 0xF60) = 0.0f;
    F(f32, 0xF6C) = 0.0f;
    F(f32, 0xF68) = 0.0f;
    mScreenBuffer = nullptr;
    _B48 = nullptr;
    _B4C = nullptr;
    F(u16, 0xF7C) = 0;
    for (s32 i = 0; i < 5; i++) {
        FP(this, u32, 0xF80 + i * 4) = 0;
    }
    F(u32, 0xF94) = 0;
    F(u8, 0xF98) = 0;
    F(u16, 0xF9A) = 0;
    F(u8, 0xF9C) = 0;
    F(u16, 0xF9E) = 0;
    mEyeRes = nullptr;
    _B7C = nullptr;
    mMaskTextures[0] = nullptr;
    mMaskTextures[1] = nullptr;
    mRandomTextureIndex = 0;
    mNullAnimation = nullptr;
    F(u8, 0xFB8) = 0;
    F(u8, 0xFB9) = 0;
    F(u8, 0xFBA) = 0;
    F(u16, 0xFBC) = 0;
    F(u8, 0xFBE) = 0;
    F(u16, 0xFC0) = 0;
    F(u16, 0xFC2) = 0;
    mTimedCommonEffects = nullptr;
    mMaterialEffectsFC8 = nullptr;
    mWaterColumn = nullptr;
    mEffectTranslationFD0.zero();
    mEffectRotationFDC.zero();
    F(u16, 0x260) = 0;
    PSMTXIdentity(mReflectionBaseMtx264.toMtxPtr());
    PSMTXIdentity(mJointTransform294.toMtxPtr());
    mJointMatrices2C4 = nullptr;
    PSMTXIdentity(mHandLMatrix.toMtxPtr());
    PSMTXIdentity(mHandRMatrix.toMtxPtr());
    PSMTXIdentity(mHandLJointMatrix.toMtxPtr());
    PSMTXIdentity(mHandRJointMatrix.toMtxPtr());
    PSMTXIdentity(mFaceJointMatrix.toMtxPtr());
    PSMTXIdentity(mFootRMatrix.toMtxPtr());
    PSMTXIdentity(mSpine1Matrix.toMtxPtr());
    PSMTXIdentity(mCapMatrix.toMtxPtr());
    PSMTXIdentity(mHipMatrix.toMtxPtr());
    PSMTXIdentity(mSpine2Matrix.toMtxPtr());
    PSMTXIdentity(mMainJointMatrix4A8.toMtxPtr());
    F(u16, 0x4D8) = 0;
    F(f32, 0x4DC) = 0.0f;
    F(u8, 0x4E0) = 0;
    F(u8, 0x4E1) = 0;
    F(u8, 0x4E2) = 0;
    PSMTXIdentity(F(TMtx34f, 0x4E4).toMtxPtr());
    F(u8, 0x514) = 0;
    F(u8, 0x515) = 0;
    F(u8, 0x516) = 0;
    F(u8, 0xFE8) = 0;
    mActionButtonHeld = 0;
    mActionTriggered = 0;
    mJumpButtonHeld = 0;
    F(u16, 0xFEC) = 0;
    F(u16, 0xFEE) = 0;
    F(u16, 0xFF0) = 0;
    F(u16, 0xFF2) = 0;
    F(u32, 0xFF4) = 0;
    F(u32, 0xFF8) = 0;
    F(u8, 0xFFC) = 0;
    F(u32, 0x1000) = 0;
    F(u32, 0x1004) = 0;
    F(u8, 0x1008) = 0;
    F(u8, 0x1009) = 0;
    mInputConfig = nullptr;
    F(u16, 0x100A) = 0;
    F(u16, 0x100C) = 0;
    mFlag100E = 0;
    mFlag100F = 0;
    _F24 = nullptr;
    F(u16, 0x51C) = 0;
    mFrontHistory = nullptr;
    F(u8, 0x51E) = 0;
    _F48 = nullptr;
    mBlackHole = nullptr;
    mBlackHolePosition528.zero();
    mBlackHoleRotateAxis.zero();
    mPosRelativeToBlackHole.zero();
    mSwimSurfaceState1010 = 0;
    mCameraPosition.zero();
    mCameraBasisX.zero();
    mCameraBasisY.zero();
    mCameraBasisZ.zero();
    mPostureReference1044.zero();
    _FB4 = nullptr;
    F(u16, 0x550) = 0;
    mActionTable1080 = nullptr;
    F(u8, 0x1050) = 0;
    F(u8, 0x1051) = 0;
    mFlag1052 = 0;
    mPunchTriangle106C = nullptr;
    mLifeDecreasedThisFrame = 0;
    F(u8, 0x1064) = 0;
    F(u8, 0x1066) = 0;
    F(u8, 0x1067) = 0;
}

void MarioActor::fn_803BF6F0() {
    mDLchanger = nullptr;
    F(u8, 0x1A5) = 0;
    for (int i = 0; i < 0x40; i++) {
        mBlurDisplayLists[i] = nullptr;
    }
    mDL[0] = nullptr;
    mDL[1] = nullptr;
    F(s32, 0x1A0) = 0;
    F(u8, 0x1A4) = 0;
    F(f32, 0x1A8) = 0.0f;
    F(s16, 0x552) = 0;
    F(s16, 0x554) = 0;
    F(f32, 0x558) = 0.0f;
    F(Color8, 0x55C).set(0, 0, 0, 0);
}

void MarioActor::fn_803BF760(const JMapInfoIter& iter, s32* anim) {
    MR::getJMapInfoTrans(iter, &this->mPosition);
    MR::getJMapInfoRotate(iter, &this->mRotation);
    this->mScale.x = 1.0f;
    this->mScale.y = 1.0f;
    this->mScale.z = 1.0f;
    this->mRotation.x = MR::repeat(this->mRotation.x, 0.0f, 360.0f);
    this->mRotation.y = MR::repeat(this->mRotation.y, 0.0f, 360.0f);
    this->mRotation.z = MR::repeat(this->mRotation.z, 0.0f, 360.0f);
    if (MR::isExistJMapInfoArg0(iter)) {
        MR::getJMapInfoArg0NoInit(iter, anim);
    }
}

void MarioActor::fn_803BF860() {
    gIsLuigi = false;
    if (MR::isPlayerLuigi()) {
        gIsLuigi = true;
    }
}

void MarioActor::fn_803BF8A0() {
    this->mMario->setHeadAndFrontVecFromRotate(this->mRotation);
    this->mMario->_290 = this->mMario->mSideVec;
}

void MarioActor::fn_803BF8F0() {
    mConst = new MarioConst();
    if (gIsLuigi) {
        mConst->changeTable(1);
    }
}

void MarioActor::fn_803BF950() {
    mGravityInfo = new GravityInfo();
    F(f32, 0x6A4) = 1.0f;
    F(f32, 0x594) = 0.0f;
    F(f32, 0x598) = -1.0f;
    F(f32, 0x59C) = 0.0f;
    mPreviousGravityVec = mAppliedGravityVec;
    F(s16, 0x664) = 0;
}

void MarioActor::fn_803BF9D0() {
    this->initBinder(60.0f, 0.0f, 8);
    F(f32, 0x60C) = 0.0f;
    F(f32, 0x610) = 70.0f;
    F(f32, 0x614) = 0.0f;
    this->mBinder->_1EC._0 = false;
    MR::setBinderOffsetVec(this, &F(TVec3f, 0x60C), false);
    this->mBinder->setTriangleFilter(MR::createTriangleFilterDelegator(this, &MarioActor::binderFilter));
    this->mBinder->_1EC._3 = true;
    F(u8, 0x51E) = 1;
    F(f32, 0xE78) = 0.0f;
    F(TVec3f, 0xE6C).zero();
    mBinderMotion5D0.zero();
    mBinderDirection5DC.zero();
}

void MarioActor::fn_803BFAE0() {
    F(s8, 0x666) = 0;
    F(s16, 0x668) = 0;
    mActionHash = new MarioActionHash();
}

void MarioActor::fn_803BFB30() {
    MR::declareGlobalEventCameraAbyss("奈落カメラ");
    MR::declareBlackHoleCamera("ブラックホール");
    MR::declareGlobalEventCameraDead("昇天カメラ", 0.35f, 120, 60);
    MR::declareGlobalEventCamera("水中フォロー");
    MR::declareGlobalEventCamera("水中プラネット");
    MR::declareGlobalEventCamera("水上フォロー");
    MR::declareGlobalEventCamera("引き戻し");
    MR::declareEventCameraProgrammable("変身初出カメラ");
}

void MarioActor::fn_803BFBB0() {
    F(TVec3f, 0x648) = FP(this->mMario, TVec3f, 0x1F0);
    F(TVec3f, 0x618) = F(TVec3f, 0x648);
    mCounter660 = 0;
    mWaterSurfaceCameraTimer662 = 0;
    MR::setGameCameraTargetToPlayer();
}

void MarioActor::fn_803BFC10() {
    FootPrint* fp = new FootPrint("足跡", 0x100, -1);
    F(FootPrint*, 0xF74) = fp;
    F(FootPrint*, 0xF74)->setTexture(MR::getTexFromArc("Footprint.bti", this));
}

void MarioActor::fn_803BFC90(s32 a) {
    switch (a) {
    case 1:
        this->mMario->changeAnimation("基本", (const char*)nullptr);
        break;
    case 2:
        this->mMario->changeAnimationNonStop("ウォークイン");
        break;
    default:
        this->mMario->changeAnimation("ステージインA", (const char*)nullptr);
        break;
    }
}

void MarioActor::fn_803BFCE0() {
    mCarryFixedPosition780 = new FixedPosition(this, "HandR", TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f));
    mTurnipFixedPosition784 = new FixedPosition(this, "HandR", TVec3f(1.8f, 52.51f, 39.58f), TVec3f(76.33f, 15.69f, 88.99f));
    mDrillFixedPosition788 = new FixedPosition(this, "HandR", TVec3f(5.59f, 10.85f, -23.16f), TVec3f(90.45f, 19.45f, 67.7f));
    _494 = nullptr;
}

void MarioActor::fn_803BFE50() {
    mBodyRadius7A0 = 35.0f;
    mBodyHeight7A4 = 60.0f;
    mBodyUpDirection.x = 0.0f;
    mBodyUpDirection.y = 1.0f;
    mBodyUpDirection.z = 0.0f;
    mBodyDownDirection = -mBodyUpDirection;
}

void DLchanger::addDL(J3DModelX* m) {
    m->setDynamicDL(mBuffers[mCurrentBuffer].mDL, mBuffers[mCurrentBuffer].mSize);
}


void MarioActor::drawSilhouette() const {
    if (this->isSilhouetteVisible()) {
        J3DModelX* m = mSimpleModelF08;
        m->setDrawView(1);
        m->mDisplayListMask = 0;
        m->mDisplayListMask |= (1U << 3);
        m->directDraw(nullptr);
        m->mDisplayListMask &= ~(1U << 3);
        GXSetAlphaUpdate(GX_TRUE);
        GXSetColorUpdate(GX_TRUE);
        GXSetDstAlpha(GX_TRUE, 0);
    }
}

#define BITAT(p, off, n) ((FP(p, u32, off) >> (n)) & 1)


// NONMATCHING: equivalent nonnegative timer shifts and argument-address scheduling.
void MarioActor::drawPreWipe() const {
    if (mMario->isPlayerModeYoshi()) {
        return;
    }
    if (!_1C3) {
        if (!mActorEffectState->mActive) {
            return;
        }
        f32 width = MR::getScreenWidth();
        f32 height = MR::getScreenHeight();
        TDDraw::setup(0, 1, 2);
        TVec3f position(0.0f, 0.0f, 0.0f);
        TVec3f size(width, height, 0.0f);
        u16 timer = mActorEffectState->mTimer;
        u32 alpha = 0xE0;
        if (timer <= 0x80) {
            alpha = (timer >> 1) + (static_cast<s32>(timer) >> 2) + timer;
        }
        TDDraw::drawFillBox(position, size, alpha | 0x28282800);
    }
    MR::loadViewMtx();
    MR::loadProjectionMtx();
    MR::loadActorLight(this);
    drawMarioModel();
    MR::drawInitFor2DModel();
}

void MarioActor::fn_803C1940() {
    TVec3f a(this->mPosition);
    a -= F(TVec3f, 0x5E8);
    TVec3f b(a);
    b -= F(TVec3f, 0x38);
    mBinderMotion5D0 = a;
    TVec3f c(mBinderDirection5DC);
    mBinderDirection5DC = b;
    this->fn_803C1AF0(&mBinderDirection5DC, &c);
    if (PSVECMag((Vec*)&b) > 0.1f) {
        if (!BITAT(this->mMario, 0xC, 0x15) && !MR::isNearZero(F(TVec3f, 0x38), 0.001f)) {
            TVec3f d(F(TVec3f, 0x5E8));
            d -= F(TVec3f, 0x5C4);
            f32 m1 = PSVECMag((Vec*)&d);
            f32 m2 = PSVECMag((Vec*)&F(TVec3f, 0x38));
            if (PSVECMag((Vec*)&b) > 2.0f * (m1 + m2)) {
                this->mMario->stopWalk();
            }
        }
        if (BITAT(this->mMario, 0x10, 0x1C)) {
            this->fn_803D8BD0(&a, &b);
        }
        if ((FP(this->mMario, u32, 0xC) >> 0x1F) != 0 && F(u8, 0xECD) == 0) {
            this->fn_803D8E30(&b);
            return;
        }
        if (BITAT(this->mMario, 0xC, 0x1E)) {
            this->fn_803D9170(&a);
        }
    }
}

void MarioActor::fn_803C1AF0(TVec3f* a, TVec3f* b) {
    if (MR::isOppositeDirection(*a, *b, 0.01f)) {
        f32 m1 = PSVECMag((Vec*)a);
        f32 m2 = PSVECMag((Vec*)b);
        if (!MR::isNearZero(m1, 0.001f) && !MR::isNearZero(m2, 0.001f) && MR::isNearZero(m1 - m2, 1.0f)) {
            TVec3f t(*a);
            t.scale(0.5f);
            this->mPosition -= t;
        }
    }
}

bool MarioActor::fn_803C1C60() {
    if (F(u8, 0x514) == 0) {
        return false;
    }
    if (!MR::tryStartDemoWithoutCinemaFrame(this, "マリオスーパー化")) {
        return false;
    }
    F(u8, 0x514) = 0;
    this->changeMaxLife(6);
    MR::stopAnimFrame(this);
    if (this->mMario->isPlayerModeYoshi()) {
        MR::stopAnimFrame(mYoshiHolder->mYoshi);
    }
    MR::requestPowerUpHPMeter();
    this->mMario->startPadVib("マリオ[変身]");
    MR::startSystemSE("SE_SY_SUPER_KINOKO_GET", -1, -1);
    F(u16, 0x6FE) = 0x5A;
    return true;
}

bool MarioActor::fn_803C1D30() {
    if (F(u8, 0x515) == 0) {
        return false;
    }
    if (!MR::tryStartDemoWithoutCinemaFrame(this, "マリオ変身")) {
        return false;
    }
    F(u8, 0x516) = 1;
    F(u8, 0x515) = 0;
    F(u16, 0x6FC) = 0x40;
    if (mPlayerMode == 6) {
        this->fn_803BB4F0();
        this->changeTeresaAnimation("change", -1);
    }
    if (mPlayerMode == 0xA) {
        MR::requestMovementOn(this);
        MR::requestMovementOn(mYoshiHolder->mYoshi);
        F(u16, 0x6FC) = 0x13;
        this->mMario->clearVelocity();
        FP(this->mMario, f32, 0x278) = 0.0f;
    }
    if (mPlayerMode != 0xA) {
        MR::stopAnimFrame(this);
        this->playEffect("変身");
        this->mMario->startPadVib("マリオ[変身]");
    }
    return true;
}

bool MarioActor::fn_803C1E40() {
    bool r = false;
    if ((F(u16, 0x6FC) != 0 && mPlayerMode != 0) || F(u16, 0x6FE) != 0) {
        r = true;
    }
    return r;
}

void MarioActor::fn_803C1E80() {
    FP(this->mMario, TVec3f, 0x350).zero();
    FP(this->mMario, TVec3f, 0x35C).zero();
    F(s16, 0xCCC) = 0;
    F(s32, 0xE20) = 0;
    F(TVec3f, 0x38).zero();
    mLastMove.zero();
    F(TVec3f, 0x5C4) = this->mPosition;
    if (this->fn_803C1F30()) {
        this->mMario->mPosHolder->fn_80405F90(this->mPosition);
        this->mMario->checkEnforceMove();
        this->mPosition = this->mMario->mPosHolder->mPos;
    }
}

bool MarioActor::fn_803C1F30() {
    Mario* m = this->mMario;
    bool r = false;
    if (BITAT(m, 0xC, 0x1E)) {
        MtxPtr prev = FP(m, Triangle*, 0x458)->getPrevBaseMtx()->mMtx;
        if (!MR::isSameMtx(FP(this->mMario, Triangle*, 0x458)->getBaseMtx()->mMtx, prev)) {
            r = true;
        }
    }
    return r;
}

void MarioActor::fn_803C2010() {
    u16 t = F(u16, 0x260);
    if (t != 0) {
        F(u16, 0x260) = t - 1;
    }
}

void MarioActor::fn_803C2030() {
    if (F(s8, 0x4E0) == 0) {
        if (F(s8, 0x51E) == 0) {
            MR::offBind(this);
        } else {
            MR::onBind(this);
        }
    }
    F(TVec3f, 0xE6C) = F(TVec3f, 0x38);
    if (BITAT(this->mMario, 0x1C, 0x14)) {
        this->mBinder->_1EC._1 = false;
        return;
    }
    this->mBinder->_1EC._1 = true;
}


void MarioActor::fn_803C2320() {
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
            this->mMarioAnim->fn_803DDF60();
        }
    }
}

void MarioActor::fn_803C23B0() {
    TVec3f d(this->mPosition);
    d -= F(TVec3f, 0x5C4);
    mLastMove = d;
    F(TVec3f, 0x5C4) = this->mPosition;
}

void MarioActor::fn_803C2520() {
    if (F(u16, 0xFBC) != 0) {
        F(u16, 0xFBC) -= 1;
        if (F(u16, 0xFBC) == 0) {
            this->mMario->stopAnimationUpper("ハンマー投げ回転中", (const char*)nullptr);
            this->mMario->stopAnimation("ハンマー投げ回転中", (const char*)nullptr);
        }
    }
}


void MarioActor::fn_803C2590() {
    if (F(u8, 0xE3C) != 0) {
        u8 t = F(u8, 0xE3D);
        F(u8, 0xE3C) = F(u8, 0xE3C) - 1;
        F(u8, 0xE3D) = t + 1;
        this->fn_803C2610();
        this->fn_803C26A0();
    }
    if (this->fn_803C2710()) {
        this->mMario->stopAnimation((const char*)nullptr, (const char*)nullptr);
    }
}

void MarioActor::fn_803C2610() {
    if ((F(s16, 0xE3E) != 0 || BITAT(this->mMario, 0x10, 0x14)) && !this->mMario->isSwimming() && F(u8, 0xE3C) == 0 &&
        mActionTable1080->selectAction("スピン回復エフェクト") == 1) {
        this->playEffect("スピンリング");
    }
}

void MarioActor::fn_803C26A0() {
    if ((this->isInPunchTimerRange() || !this->mMario->isSwimming())) {
        u32 t = FP(this->mMario, u32, 0xC);
        if (((t >> 0x17) & 1) || ((t >> 5) & 1) || ((t >> 6) & 1)) {
            this->mMario->tryWallPunch();
        }
    }
}

bool MarioActor::fn_803C2710() {
    s32 a = 0;
    s32 b = 0;
    s32 c = 0;
    s32 d = 0;
    if (this->mMario->isAnimationRun("ハンマー投げリリース") && BITAT(this->mMario, 0xC, 0x1E)) {
        d = 1;
    }
    if (d != 0 && F(u16, 0x6B8) == 0) {
        c = 1;
    }
    if (c != 0 && FP(this->mMario, u16, 0x41C) == 0) {
        b = 1;
    }
    if (b != 0 && this->mMario->isStickOn()) {
        a = 1;
    }
    return a;
}


void MarioActor::fn_803C2900() {
    if (mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, "水陸移動特殊処理")) {
        WaterInfo info;
        if (!MR::getWaterAreaObj(&info, this->mPosition)) {
            this->mMario->forceExitSwim();
        }
    } else if (!mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, "水柱エフェクト")) {
        MarioSwim* sw = this->mMario->mSwim;
        s32 before = FP(sw, s32, 0xFC);
        sw->fn_803FD5E0(0);
        s32 after = FP(this->mMario->mSwim, s32, 0xFC);
        if (after != before) {
            MarioSwim* sw2 = this->mMario->mSwim;
            if ((u32)after <= 1 && (u32)(before - 2) <= 1) {
                this->playEffectRT("水面ジャンプ水柱", FP(sw2, TVec3f, 0x11C), FP(sw2, TVec3f, 0x128));
                MarioSwim* sw3 = this->mMario->mSwim;
                this->emitEffectWaterColumn(FP(sw3, TVec3f, 0x11C), FP(sw3, TVec3f, 0x128));
            } else if ((u32)before <= 1 && (u32)(after - 2) <= 1) {
                TVec3f neg = -FP(this->mMario, TVec3f, 0x328);
                this->playEffectRT("水面ジャンプ水柱", neg, FP(this->mMario->mSwim, TVec3f, 0x128));
                MarioSwim* sw3 = this->mMario->mSwim;
                this->emitEffectWaterColumn(FP(sw3, TVec3f, 0x11C), FP(sw3, TVec3f, 0x128));
            }
            switch (after) {
            case 0:
                MR::getGameSceneLayoutHolder()->changeLifeMeterModeGround();
                this->mMario->forceExitSwim();
                return;
            case 2:
                MR::getGameSceneLayoutHolder()->changeLifeMeterModeWater();
                break;
            }
        }
    }
}

void MarioActor::fn_803C2AA0() {
    if (this->mMario->isForceStopRush() && F(u8, 0xE24) != 0 && _924->receiveMessage(0x97, this->getSensor("Body"))) {
        RushEndInfo info(nullptr, 4, TVec3f(0.0f, 0.0f, 0.0f), false, 0);
        this->endRush(&info);
    }
}

void MarioActor::fn_803C2B50() {
    if (F(u8, 0xE24) != 0 && mActionTable1080->testEffect(MR::getSensorHost(_924)->mName, "ハチ回復")) {
        u32 i = 0;
        do {
            this->mMario->incAirWalkTimer();
            i++;
        } while (i < 4);
    }
}

bool MarioActor::fn_803C3260() {
    if (this->mMario->isAnimationRun("壁はじき")) {
        return false;
    }
    if (this->isJumping() && FP(this->mMario, s32, 0x424) != 0) {
        return false;
    }
    if (mPlayerMode == 9) {
        return false;
    }
    if (this->mMario->getCurrentStatus() == 1) {
        return false;
    }
    if (this->mMario->isSwimming()) {
        return false;
    }
    if (this->mMario->isStatusActive(0x18)) {
        return false;
    }
    if (this->mMario->isStatusActive(0x13)) {
        return false;
    }
    if (static_cast<s32>(_468) != 0) {
        return false;
    }
    if (this->mMario->isStatusActive(2)) {
        return false;
    }
    if (F(u8, 0x6E4) != 0) {
        return false;
    }
    if (F(u8, 0x4E0) != 0) {
        return false;
    }
    Mario* m = this->mMario;
    if (BITAT(m, 0x1C, 0x13)) {
        return false;
    }
    return FP(m, s32, 0xA78) == 0;
}

bool MarioActor::fn_803C33D0() {
    bool r = false;
    bool a = false;
    bool b = false;
    if (!BITAT(this->mMario, 0xC, 0x10) && this->isJumping()) {
        b = true;
    }
    if (b) {
        bool c = true;
        if (this->mMario->isDamaging()) {
            bool d = false;
            if (this->mMario->isPlayerModeCloud() && MR::isSpinCloudBlockGroupEmpty() == 0) {
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
    if (a && !BITAT(this->mMario, 0x10, 0x14)) {
        r = true;
    }
    return r;
}

void MarioActor::fn_803C34F0() {
    if (F(s32, 0x6B4) != 0) {
        F(s32, 0x6B4) -= 1;
        if (F(s32, 0x6B4) == 0) {
            this->decreaseLifeByOne();
            this->fn_803D9740();
        }
    }
    if ((u32)F(s32, 0x6B0) <= 1) {
        if (!MR::isDemoActive() && !MR::isPowerStarGetDemoActive() && !MR::isGalaxyDarkCometAppearInCurrentStage() && MR::isPermitSE() &&
            this->isEnableNerveChange()) {
            this->mMario->mStateFlags._2 = true;
        }
    }
    this->mMario->fn_8038C540();
}

void MarioActor::fn_803C38D0() {
    if (this->fn_803C1F30()) {
        this->mMario->mPosHolder->fn_80405F90(this->mPosition);
        this->mMario->checkEnforceMove();
        this->mPosition = this->mMario->mPosHolder->mPos;
    }
}

bool MarioActor::fn_803C3930() {
    Mario* m = this->mMario;
    bool r = false;
    if (BITAT(m, 0xC, 0x1E)) {
        MtxPtr prev = FP(m, Triangle*, 0x458)->getPrevBaseMtx()->mMtx;
        if (!MR::isSameMtx(FP(this->mMario, Triangle*, 0x458)->getBaseMtx()->mMtx, prev)) {
            r = true;
        }
    }
    return r;
}

void MarioActor::fn_803C3C90() {
    if (F(u16, 0x6FE) != 0) {
        F(u16, 0x6FE) -= 1;
        if (F(u16, 0x6FE) == 0) {
            MR::endDemo(this, "マリオスーパー化");
            MR::releaseAnimFrame(this);
            if (this->mMario->isPlayerModeYoshi()) {
                MR::releaseAnimFrame(mYoshiHolder->mYoshi);
            }
            MR::explainLifeUpIfAtFirst();
        }
    }
}

void MarioActor::fn_803C3D10() {
    if (F(u16, 0x6FC) != 0) {
        if (mPlayerMode == 6) {
            void* r5 = F(void*, 0x590);
            if (FP(this->mMario, f32, 0x484) < FP(FP(r5, void*, FP(r5, u32, 0xC) * 4), f32, 0x710)) {
                this->mPosition -= *this->mMario->getGravityVec();
            }
        }
        if (!this->fn_803C3DF0() && !this->mMario->fn_803B26C0()) {
            this->fn_803C3E40();
        }
        F(u16, 0x6FC) -= 1;
        if (F(u16, 0x6FC) == 0 && !this->mMario->fn_803B26C0()) {
            this->fn_803C4020();
        }
        F(u8, 0x703) = 1;
        return;
    }
    this->mMario->updateMorphResetTimer();
}

bool MarioActor::fn_803C3DF0() {
    switch (mPlayerMode) {
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

void MarioActor::fn_803C3E40() {
    TVec3f front;
    if (_4A4 != nullptr) {
        MR::calcFrontVec(&front, MR::getSensorHost(_4A4));
    } else if (mSensor794 != nullptr) {
        MR::calcFrontVec(&front, MR::getSensorHost(mSensor794));
    } else {
        this->getFrontVec(&front);
    }
    this->mMario->setFrontVecKeepUp(front);
    if (F(s8, 0x666) == 0) {
        MR::zoomInTargetGameCamera();
        CameraTargetArg arg(this);
        MR::startGlobalEventCamera("変身初出カメラ", arg, 60);
        TVec3f up = -*this->mMario->getGravityVec();
        TVec3f base(F(TVec3f, 0x5F4));
        f32 dist = 500.0f;
        f32 height = 50.0f;
        if (mPlayerMode == 6) {
            dist = 700.0f;
            height = 150.0f;
        }
        Mario* m = this->mMario;
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
        TVec3f upv = -mAppliedGravityVec;
        MR::setProgrammableCameraParam("変身初出カメラ", eye, tgt, upv, true);
    }
    F(u8, 0x666) = 1;
}

void MarioActor::fn_803C4020() {
    if (F(u8, 0x516) != 0) {
        MR::endDemo(this, "マリオ変身");
        F(u8, 0x516) = 0;
        MR::releaseAnimFrame(this);
    }
    F(u8, 0x702) = 1;
    switch (mPlayerMode) {
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

void MarioActor::fn_803C4100() {
    if (F(u16, 0x668) != 0) {
        F(u16, 0x668) -= 1;
        if (F(u16, 0x668) == 0) {
            MR::zoomOutTargetGameCamera();
            MR::endGlobalEventCamera("変身初出カメラ", 60, true);
        }
    }
}

void MarioActor::calcAnimPlayerMode() {
    if (F(u8, 0x703) == 0) {
        F(u8, 0x702) = 0;
        return;
    }
    u16 mode;
    u16 t = F(u16, 0x6FC);
    if ((s32)(t & 7) < ((s32)t >> 3)) {
        mode = F(u16, 0x6FA);
    } else {
        mode = mPlayerMode;
    }
    this->fn_803D8A70(mode);
    switch (mode) {
    case 1:
        mAllModel->fn_803E7880(4);
        _9C0->calcAnim();
        if (F(u8, 0x702) != 0) {
            F(u8, 0x1B1) = 2;
        }
        break;
    case 4:
        mAllModel->fn_803E7880(2);
        break;
    case 5:
        F(u8, 0x703) = 0;
        mAllModel->fn_803E7880(5);
        break;
    case 3:
        mAllModel->fn_803E7880(3);
        break;
    case 12:
        mAllModel->fn_803E7880(7);
        break;
    case 13:
        mAllModel->fn_803E7880(8);
        break;
    default: {
        mAllModel->fn_803E7880(0);
        u32 frame = 0;
        switch (mode) {
        case 7:
            frame = 1;
            break;
        case 2:
            frame = 2;
            break;
        }
        MR::startBtp(this, "ColorChange");
        MR::setBtpFrameAndStop(this, (f32)frame);
        break;
    }
    }
    F(u8, 0x703) = 0;
    F(u8, 0x702) = 0;
}

void MarioActor::calcAnimHorrorBone() {
    if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
        Mario* m = this->mMario;
        if (FP(m, s32, 0x5F8) != 0) {
            if (FP(mAllModel, s32, 4) == 6) {
                mAllModel->fn_803E7B80();
                this->playEffect("ホラーマリオ解除");
            }
        } else if (MR::isPowerStarGetDemoActive()) {
            if (FP(mAllModel, s32, 4) == 6) {
                mAllModel->fn_803E7B80();
            }
        } else {
            void* p = mAllModel;
            if (FP(p, s32, 4) != 6) {
                FP(p, s32, 8) = FP(p, s32, 4);
                this->playEffect("ホラーマリオ変身");
            }
            mAllModel->fn_803E7880(6);
        }
    } else {
        if (this->mMario->isStatusActive(0xB)) {
            void* p = mAllModel;
            if (FP(p, s32, 4) != 1) {
                FP(p, s32, 8) = FP(p, s32, 4);
            }
            if ((F(s32, 0x6AC) & 3) && (this->isNerve(&NrvMarioActor::MarioActorNrvWait::sInstance) || this->isNerve(&NrvMarioActor::MarioActorNrvNoRush::sInstance))) {
                mAllModel->fn_803E7880(1);
                return;
            }
            mAllModel->fn_803E7B80();
            return;
        }
        if (FP(mAllModel, s32, 4) == 1 && F(s32, 0x6B0) != 0) {
            mAllModel->fn_803E7B80();
        }
    }
}

bool MarioActor::fn_803C4F70() {
    bool r = false;
    if (F(u8, 0x4E1) == 0 && F(u8, 0x4E0) != 0 && F(u8, 0x4E2) != 0) {
        r = true;
    }
    return r;
}

bool MarioActor::fn_803C4FB0() {
    bool r = false;
    bool a = false;
    if (mPlayerMode == 4 && !this->mMario->isStatusActive(0x16) && !this->mMario->isStatusActive(0x15) && !this->mMario->isStatusActive(0x1B)) {
        Mario* m = this->mMario;
        if (!BITAT(m, 0x10, 0x1C) && !BITAT(m, 0xC, 0x15)) {
            a = true;
        }
    }
    if (a) {
        bool b = false;
        u32 st = FP(this->mMario, u32, 0xC);
        if (((st >> 0x14) & 1) && (st >> 0x1F) != 0) {
            b = true;
        }
        if (!b) {
            r = true;
        }
    }
    return r;
}

#define CT(off) FP(mConst->getTable(), f32, off)

// NONMATCHING: saved-register allocation, stack slots and equivalent scheduling.
void MarioActor::fn_803C5090(MtxPtr mtx) {
    mGravityReferenceED0 = FP(this->mMario, TVec3f, 0x1D8);
    f32 k = CT(0x69C);
    TVec3f grav = -*this->mMario->getGravityVec();
    TVec3f t1(grav);
    t1.scale(k);
    MtxPtr m1 = MR::tmpMtxTrans(-t1);
    f32 k2 = CT(0x69C);
    TVec3f e(mGravityReferenceED0);
    e.scale(k2);
    TVec3f p(this->mPosition);
    p -= e;
    TVec3f d(F(TVec3f, 0x66C));
    f32 angle = 0.0f;
    d -= p;
    TVec3f axis;
    if (!MR::normalizeOrZero(&d)) {
        PSVECCrossProduct((Vec*)&FP(this->mMario, TVec3f, 0x208), (Vec*)&d, (Vec*)&axis);
        if (!MR::normalizeOrZero(&axis)) {
            f32 ang = MR::acosEx(mGravityReferenceED0.dot(d));
            f32 lim = CT(0x688);
            if (BITAT(this->mMario, 0xC, 0x1E)) {
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
    TVec3f s110(mGravityReferenceED0);
    s110.scale(k3);
    MR::vecKillElement(s110, F(TVec3f, 0x690), &s110);
    TVec3f b0(F(TVec3f, 0x684));
    b0 += s110;
    F(TVec3f, 0x684) = b0;
    TVec3f a4(F(TVec3f, 0x66C));
    a4 += F(TVec3f, 0x684);
    F(TVec3f, 0x66C) = a4;
    f32 k4 = CT(0x69C);
    TVec3f s80(mGravityReferenceED0);
    s80.scale(k4);
    TVec3f s8c(this->mPosition);
    s8c -= s80;
    TVec3f s98(F(TVec3f, 0x66C));
    s98 -= s8c;
    F(TVec3f, 0x690) = s98;
    TVec3f s104(F(TVec3f, 0x690));
    TVec3f sF8(F(TVec3f, 0x690));
    MR::normalizeOrZero(&sF8);
    f32 ang2 = MR::acosEx(mGravityReferenceED0.dot(sF8));
    f32 lim2 = CT(0x694);
    if (BITAT(this->mMario, 0xC, 0x1E)) {
        lim2 = CT(0x698);
    }
    if (ang2 > lim2) {
        TVec3f sEC;
        PSVECCrossProduct((Vec*)&mGravityReferenceED0, (Vec*)&sF8, (Vec*)&sEC);
        if (MR::normalizeOrZero(&sEC)) {
            F(TVec3f, 0x690) = mGravityReferenceED0;
        } else {
            Mtx rot2;
            PSMTXRotAxisRad(rot2, (Vec*)&sEC, lim2);
            PSMTXMultVec(rot2, (Vec*)&mGravityReferenceED0, (Vec*)&F(TVec3f, 0x690));
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
    TVec3f s44(mGravityReferenceED0);
    s44.scale(k5);
    TVec3f s50(this->mPosition);
    s50 -= s44;
    TVec3f s5c(s50);
    s5c += F(TVec3f, 0x690);
    F(TVec3f, 0x66C) = s5c;
    MR::normalizeOrZero(&F(TVec3f, 0x690));
    if (PSVECMag((Vec*)&F(TVec3f, 0x684)) > CT(0x6A0)) {
        F(TVec3f, 0x684).setLength(CT(0x6A0));
    }
    f32 sc = CT(0x6A4);
    if (mMario->mTargetWalkSpeedIndex == 0 && !this->isJumping()) {
        sc = CT(0x6AC);
    }
    F(TVec3f, 0x684).scale(sc);
    if (!MR::isSameDirection(F(TVec3f, 0x690), FP(this->mMario, TVec3f, 0x208), 0.01f)) {
        TVec3f neg = -F(TVec3f, 0x690);
        MR::makeMtxUpFront((TPos3f*)mtx, neg, FP(this->mMario, TVec3f, 0x208));
    } else if (!MR::isSameDirection(FP(this->mMario, TVec3f, 0x1FC), FP(this->mMario, TVec3f, 0x208), 0.01f)) {
        MR::makeMtxUpFront((TPos3f*)mtx, FP(this->mMario, TVec3f, 0x1FC), FP(this->mMario, TVec3f, 0x208));
    } else {
        MR::makeMtxUpFront((TPos3f*)mtx, FP(this->mMario, TVec3f, 0x1F0), FP(this->mMario, TVec3f, 0x208));
    }
    if (mMario->mTargetWalkSpeedIndex > 2 || this->isJumping()) {
        f32 inv = 1.0f - CT(0x6B0);
        TVec3f s8(F(TVec3f, 0x66C));
        s8 -= this->mPosition;
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

void MarioActor::fn_803C56F0() {
    F(TVec3f, 0x684).zero();
    F(TVec3f, 0x678).zero();
    F(TVec3f, 0x66C) = this->mPosition;
    mGravityReferenceED0 = *this->mMario->getGravityVec();
}

bool MarioActor::fn_803C5750() {
    bool r = false;
    if (F(u8, 0xE24) != 0) {
        r = _924->receiveMessage(0xA1, this->getSensor("Body"));
    }
    if (F(u8, 0x4E1) != 0) {
        PSMTXCopy(F(Mtx, 0x4E4), this->getBaseMtx());
    }
    if (r || F(u8, 0x4E1) != 0) {
        this->fn_803C58D0();
        return true;
    }
    if (F(u8, 0xE24) != 0) {
        this->fn_803C5920();
    }
    return false;
}


void MarioActor::fn_803C5820() {
    TVec3f old(this->mPosition);
    MR::extractMtxTrans(this->getBaseMtx(), &this->mPosition);
    this->mMario->invalidateRelativePosition();
    TVec3f diff(this->mPosition);
    diff -= old;
    F(TVec3f, 0xE30) = diff;
    if (BITAT(this->mMario, 0x10, 8)) {
        FP(this->mMario, TVec3f, 0x67C) = this->mPosition;
    }
    this->updateGravityVec(true, true);
    this->mMario->updateGroundInfo();
}

void MarioActor::fn_803C58D0() {
    this->fn_803C59C0();
    this->fn_803C5820();
    if (F(u8, 0x4E0) != 0) {
        F(u8, 0x4E2) = 1;
    }
}

void MarioActor::fn_803C5920() {
    TVec3f cur;
    MR::extractMtxTrans(this->getBaseMtx(), &cur);
    TVec3f diff(this->mPosition);
    diff -= cur;
    F(TVec3f, 0xE30) = diff;
}

bool MarioActor::fn_803C5990() {
    if (F(u8, 0x56C) == 0) {
        F(u8, 0x56D) = 1;
        return false;
    }
    F(u8, 0x56D) = 0;
    F(u8, 0x56C) = 0;
    return true;
}

void MarioActor::fn_803C59C0() {
    TVec3f front, side, head;
    this->getBaseMtx();
    TPos3f* bm = (TPos3f*)this->getBaseMtx();
    bm->getZDir(front);
    this->mMario->setFrontVec(front);
    ((TPos3f*)this->getBaseMtx())->getXDir(side);
    this->mMario->setSideVec(side);
    ((TPos3f*)this->getBaseMtx())->getYDir(head);
    this->mMario->forceSetHeadVecKeepSide(head);
    FP(this->mMario, f32, 0x278) = 0.0f;
    FP(this->mMario, u8, 0x714) = 0;
    this->mMario->stopJump();
    FP(this->mMario, TVec3f, 0x334) = FP(this->mMario, TVec3f, 0x208);
    if (this->mMario->isSwimming()) {
        this->mMario->mSwim->resetAndFixPose();
    }
}


void MarioActor::fn_803C5AB0(MtxPtr mtx) {
    if (F(s32, 0x6BC) != 0) {
        this->fn_803C5B30(mtx);
    } else if (F(s32, 0x6C0) != 0) {
        this->fn_803C5E20(mtx);
    } else if (!BITAT(this->mMario, 0x10, 3)) {
        F(f32, 0x6D4) = 1.0f;
    }
    s32 t = F(s32, 0x6C4);
    if (t != 0) {
        F(s32, 0x6C4) = t - 1;
    }
}

void MarioActor::fn_803C5B30(MtxPtr mtx) {
    s32 mode = static_cast<s32>(mCounter6C8);
    F(s32, 0x6C0) = 0x1E;
    switch (mode) {
    case 0:
    case 2: {
        u32 cnt = F(u32, 0x6BC);
        if (cnt > 0xF) {
            f32 a = this->mMario->calcDistToCeil(false);
            f32 b = this->mMario->calcDistToCeilOnPress();
            f32 v1 = MR::clamp(a / 150.0f, 0.2f, 1.0f);
            if (F(f32, 0x6D4) > v1) {
                F(f32, 0x6D4) = v1;
            } else {
                f32 v2 = MR::clamp(b / 150.0f, 0.2f, 1.0f);
                if (F(f32, 0x6D4) > v2) {
                    F(f32, 0x6D4) = v2;
                }
                MR::setMtxTrans(mtx, this->mPosition.x, this->mPosition.y, this->mPosition.z);
                if (FP(this->mMario, u16, 0x964) != 0x1B) {
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
            f32 w = this->mMario->calcDistWidth();
            this->mPosition = this->mMario->mPosHolder->mPos;
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

void MarioActor::fn_803C5E20(MtxPtr mtx) {
    u32 t = F(u32, 0x6C0) - 1;
    F(u32, 0x6C0) = t;
    F(f32, 0x6D4) = 1.0f + (((0.15f * (f32)F(u32, 0x6C0)) / 30.0f) * JMASinRadian((2.0f * ((f32)t * 3.1415927f)) / 15.0f));
}

void MarioActor::fn_803C5ED0(MtxPtr m1, MtxPtr m2) {
    u16 t = F(u16, 0x4D8);
    if (t != 0) {
        f32 f = 1.0f - ((1.0f - F(f32, 0x4DC)) * ((f32)(t - 1) / (f32)t));
        F(f32, 0x4DC) = f;
        MR::blendMtx(m2, m1, f, m1);
        F(u16, 0x4D8) = F(u16, 0x4D8) - 1;
    }
}

bool MarioState::passRing(const HitSensor*) {
    return false;
}

namespace MR {
    bool isCreatePlayerResourceFire();
    bool isCreatePlayerResourceYoshi();
    bool isCreatePlayerResourceCloud();
    bool isCreatePlayerResourceRock();
}

bool MarioActor::fn_803BB5F0() {
    if (MR::isActionEnd(F(LiveActor*, 0xEA0))) {
        F(LiveActor*, 0xEA0)->kill();
        MR::offCalcAnim(F(LiveActor*, 0xEA0));
        return false;
    }
    return true;
}

void* MarioActor::fn_803BBA20() {
    return mAllModel->getJ3DModel();
}

void MarioActor::fn_803BBAD0() {
    mActorEffectState->mActive = 1;
}

void MarioActor::fn_803BBDD0(const TVec3f& a, const TVec3f& b, const TVec3f& c) {
    if (this->mMario->isPlayerModeYoshi()) {
        mYoshiHolder->fn_8041BBF0(a, b, c);
    }
}

void MarioActor::fn_803BBFD0(const char* name) {
    if (name == nullptr) {
        F(void*, 0xFA0) = nullptr;
        return;
    }
    char buf[0x80];
    const char* prefix = "";
    if (this->mMario->isPlayerModeYoshi()) {
        prefix = "Yoshi";
    }
    snprintf(buf, 0x80, "%s%s", prefix, name);
    if (MR::getResourceHolder(this)->mBtpResTable->isExistRes(buf)) {
        void* r = MR::getResourceHolder(this)->mBtpResTable->getRes(buf);
        F(void*, 0xFA0) = r;
        FP(r, f32, 8) = 0.0f;
        return;
    }
    F(void*, 0xFA0) = nullptr;
}

void MarioActor::fn_803BC440(const char* name) {
    F(LiveActor*, 0xFB4)->appear();
    MR::startAction(F(LiveActor*, 0xFB4), name);
    F(s8, 0xFBA) = -2;
}

void MarioActor::fn_803BC5C0() {
    Mario* m = this->mMario;
    if (!BITAT(m, 0xC, 0x15) && m->getCurrentStatus() == 0) {
        Mario* m2 = this->mMario;
        if (BITAT(m2, 0xC, 0x1E) && FP(m2, u16, 0x964) == 0x20 && this->mMarioAnim->fn_803DCB80()) {
            this->mMarioAnim->fn_803DF0F0();
            F(u8, 0xFBE) = 2;
        }
    } else {
        F(u8, 0xFBE) = 0;
    }
    switch (mPlayerMode) {
    case 6:
        this->updateTeresaAnimation();
        break;
    case 4:
        if (F(u8, 0xECD) != 0 && !this->isJumping() && this->mMarioAnim->fn_803DCB80()) {
            this->mMarioAnim->fn_803DF1B0();
        }
        break;
    default:
        FP(this->mMario, s16, 0x414) = 0;
        break;
    }
    u8 t = F(u8, 0xFBE);
    if (t != 0) {
        u8 n = t - 1;
        F(u8, 0xFBE) = n;
        if (n == 0 && BITAT(this->mMario, 0xC, 0x1E) && this->mMarioAnim->fn_803DCB80()) {
            this->mMarioAnim->fn_803DF2D0();
        }
    }
}

void MultiEmitter::setGlobalScale(f32 scale, s32 n) {
    TVec3f v(scale, scale, scale);
    this->setGlobalScale(v, n);
}

void MarioActor::fn_803BDA20(MtxPtr dst, const char* joint) {
    PSMTXConcat(MR::getJointMtx(this, joint), mJointTransform294.mMtx, dst);
}

bool MarioActor::fn_803BDE10() {
    if (this->mMario->isPlayerModeYoshi() && YoshiParam::fn_8041BD40() == 0) {
        return false;
    }
    if (F(u8, 0xE24) != 0 || F(u16, 0x6B8) != 0) {
        return false;
    }
    if (F(u8, 0xE2C) != 0) {
        return false;
    }
    return this->checkButtonType(mInputConfig->_0, false);
}

void* MarioActor::fn_803BE0C0() const {
    return nullptr;
}

bool MarioActor::fn_803BE0D0() {
    return this->checkButtonType(5, false);
}

void* MarioActor::fn_803BE0E0() const {
    return nullptr;
}

void MarioActor::initFireMario() {
    if (MR::isCreatePlayerResourceFire()) {
        this->initFireBall();
        F(u16, 0x6F6) |= 4;
    }
}

void MarioActor::initYoshi() {
    if (MR::isCreatePlayerResourceYoshi()) {
        mYoshiHolder = new YoshiHolder(this->mMario);
        mYoshiHolder->fn_8041B700(this);
        F(u16, 0x6F6) |= 0x400;
    }
}

void MarioActor::initCloudMario() {
    if (MR::isCreatePlayerResourceCloud()) {
        const char* name = "CloudMario";
        if (gIsLuigi) {
            name = "CloudLuigi";
        }
        ModelHolder* mh = new ModelHolder(name, true);
        F(ModelHolder*, 0xEEC) = mh;
        mh->initWithoutIter();
        mAllModel->registerModel(7, F(LiveActor*, 0xEEC), true, true);
        F(LiveActor*, 0xEEC)->kill();
        MR::hideModel(F(LiveActor*, 0xEEC));
        MR::hideJoint(F(LiveActor*, 0xEEC), "Cap0");
        MR::hideJoint(F(LiveActor*, 0xEEC), "Face0");
        CloudMarioHat* hat = new CloudMarioHat("雲マリオの帽子");
        F(CloudMarioHat*, 0xEF0) = hat;
        hat->initWithoutIter();
        F(u16, 0x6F6) |= 0x1000;
    }
}

void MarioActor::initRockMario() {
    if (MR::isCreatePlayerResourceRock()) {
        const char* name = "RockMario";
        if (gIsLuigi) {
            name = "RockLuigi";
        }
        ModelHolder* mh = new ModelHolder(name, true);
        F(ModelHolder*, 0xEF4) = mh;
        mh->initWithoutIter();
        mAllModel->registerModel(8, F(LiveActor*, 0xEF4), true, true);
        F(LiveActor*, 0xEF4)->kill();
        MR::hideModel(F(LiveActor*, 0xEF4));
        MR::hideJoint(F(LiveActor*, 0xEF4), "Face0");
        F(u16, 0x6F6) |= 0x2000;
    }
}

void MarioActor::fn_803C8AA0() {
    J3DModelX* m = (J3DModelX*)mAllModel->getJ3DModel();
    if (F(u8, 0x73) == 0) {
        m->viewCalc3(0, nullptr);
        Mtx inv1, inv2;
        PSMTXInverse(mJointTransform294.mMtx, inv1);
        PSMTXInverse(FP(mAllModel->getJ3DModel(), Mtx, 0x24), inv2);
        PSMTXConcat(inv1, inv2, mReflectionBaseMtx264.mMtx);
    }
}

void J3DModelX::swapDrawBuffer(u32 a) {
    if (FP(this, u8, 0xDC) == a) {
        FP(this, u8, 0xDC) = 0;
        return;
    }
    FP(this, u8, 0xDC) = (u8)a;
}




bool MarioActor::fn_803C9300() {
    bool c = this->mMario->isCeiling();
    if ((c || BITAT(this->mMario, 0x1C, 0xA)) && F(u8, 0xECD) == 0 && F(u16, 0xECE) == 0) {
        Mario* m = this->mMario;
        bool r = false;
        if (BITAT(m, 0x1C, 0xA)) {
            r = true;
        } else if (MR::getWallCodeString(FP(m, Triangle*, 0x4C4)) != nullptr && strcmp(MR::getWallCodeString(FP(m, Triangle*, 0x4C4)), "Fur") == 0) {
            r = true;
        }
        if (r) {
            Triangle tri;
            Mario* m2 = this->mMario;
            TVec3f dir = -mAppliedGravityVec;
            dir.scale(200.0f);
            TVec3f hit;
            if (m2->fn_803B2770(&hit, &tri, this->mPosition, dir)) {
                this->entryWallWalkMode(hit, *tri.getNormal(0));
            }
        }
    }
    return false;
}

void MarioActor::fn_803C9920(TVec3f& vec) {
    if (mPlayerMode != 4) {
        F(u8, 0xECD) = 0;
        F(u16, 0xECE) = 0;
        return;
    }
    u16 timer = F(u16, 0xECE);
    u8 prev = F(u8, 0xECD);
    if (timer != 0) {
        F(u16, 0xECE) = timer - 1;
    }
    if (F(u16, 0xECE) != 0 || (!this->fn_803C9300() && !this->checkBeeWallStick(vec) && !this->checkBeeFloorStick(vec))) {
        this->updateBeeStickMode(vec);
    }
    u8 cur = F(u8, 0xECD);
    if (cur == 5) {
        MR::vecBlendSphere(&vec, mAppliedGravityVec, vec, 0.1f);
        return;
    }
    if (cur != 0) {
        MR::vecBlendSphere(&vec, mAppliedGravityVec, vec, 0.3f);
        return;
    }
    if (prev != 0) {
        F(u16, 0xECE) = 0x3C;
    }
}


void MarioActor::setPlayerMode(u16 mode, bool a, bool b) {
    if (mode != 3) {
        FP(this->mMario, u32, 0x18) &= 0xFFBFFFFF;
    }
    if (!this->fn_803CB1A0(mode)) {
        if (this->mMario->isStatusActive(0x18)) {
            this->mMario->closeStatus(nullptr);
        }
        F(s16, 0x700) = 0;
        F(u8, 0x1B1) = 0;
        this->fn_803CB2E0(mode);
        this->fn_803CB510(mode, a);
        if (mPlayerMode == 0) {
            this->fn_803CBA90(a, b);
        } else {
            this->fn_803CBB30(b);
        }
        this->updateHandAtMorph();
    }
}

void J3DModelX::setDynamicDL(u8* dl, u32 size) {
    if (dl == nullptr) {
        size = 0;
    }
    FP(this, u8*, 0x1B8) = dl;
    FP(this, u32, 0x1BC) = size;
}

// NONMATCHING: Matrix-address reuse, equivalent bit tests, and boolean-store scheduling.
void MarioActor::movement() {
    _46C = nullptr;
    ++_378;
    _1E1 = false;
    mLifeDecreasedThisFrame = 0;
    mNearestUnderAngle79C = 0.7853981852531433f;
    mNearestUnderSensor798 = nullptr;
    PSMTXCopy(mCurrentViewMatrix.toMtxPtr(), mPreviousViewMatrix.toMtxPtr());
    PSMTXCopy(MR::getCameraViewMtx(), mCurrentViewMatrix.toMtxPtr());
    updateCameraInfo();
    mMarioAnim->fn_803DFAC0();
    mMarioAnim->fn_803DFB70();
    LiveActor::movement();
    fn_803C1940();
    if (mMario->mDrawStates.mWord & 0x20) {
        mMario->mDrawStates.mWord &= ~0x20;
        mMarioAnim->update();
    }
    mMario->recordRelativePosition();
    updateTransForCamera();
    calcAnimInMovement();
    _935 = false;
    mMario->_2D0 = 0.0f;
    mFrontHistory->update();
    if (mMario->isPlayerModeYoshi()) mYoshiHolder->mYoshi->mRideModelVisible = isSilhouetteVisible() != 0;
    mSensorSlots108C->fn_803DAB90();
}
