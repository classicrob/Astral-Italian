// Retained helper and legacy definitions; SMG2 MarioActor methods are ordered in MarioActor.cpp.


f32 MarioState::getBlurOffset() const {
    return 0.0f;
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


