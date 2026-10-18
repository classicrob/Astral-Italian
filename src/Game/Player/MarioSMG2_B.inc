// SMG2-only Mario:: functions (fn_ range B), written from the asm. Included at the end of Mario.cpp (unity TU).

extern "C" void fn_8005A7F0(MarioActor*);
extern "C" void fn_803BB5F0(MarioActor*);

extern "C" void fn_803A2D50(Mario* self) {
    if (self->_436 == 0) {
        self->_436 = 0;
        return;
    }
    self->_436--;
}

extern "C" bool fn_803A3130(Mario* self) {
    if (self->_420 == 0) {
        return false;
    }
    self->_420--;
    self->_10_HIGH_WORD |= 0x800000;
    return true;
}

extern "C" void fn_803A96D0(Mario* self) {
    fn_8005A7F0(self->mActor);
}

extern "C" bool fn_803AA2C0(Mario* self) {
    return false;
}

extern "C" void fn_803AC550(Mario* self) {
    fn_803BB5F0(self->mActor);
}

extern "C" void fn_803AC810(Mario* self) {
    self->mTeresa->isTeresaAccel();
}

extern "C" void fn_803B1710(Mario* self) {
    self->_10_HIGH_WORD &= 0xFFCFFFFF;
}

// column setters of a 3x4 matrix (X / Y / Z axis)
extern "C" void fn_803B2170(TMtx34f* pMtx, const TVec3f& rVec) {
    pMtx->mMtx[0][0] = rVec.x;
    pMtx->mMtx[1][0] = rVec.y;
    pMtx->mMtx[2][0] = rVec.z;
}

extern "C" void fn_803B2190(TMtx34f* pMtx, const TVec3f& rVec) {
    pMtx->mMtx[0][1] = rVec.x;
    pMtx->mMtx[1][1] = rVec.y;
    pMtx->mMtx[2][1] = rVec.z;
}

extern "C" void fn_803B21B0(TMtx34f* pMtx, const TVec3f& rVec) {
    pMtx->mMtx[0][2] = rVec.x;
    pMtx->mMtx[1][2] = rVec.y;
    pMtx->mMtx[2][2] = rVec.z;
}

#if 0  // fn_803B3010: enable once MarioSMG2_A.inc declares it as (Mario*)
extern "C" void fn_803B3010(Mario* self) {
    self->mMovementStates_LOW_WORD &= 0xFFFFFBFF;
    self->mMovementStates_HIGH_WORD &= 0xFFFFFFBF;
    (*reinterpret_cast<s32**>(reinterpret_cast<u8*>(self) + 0x478))[1] = -1;
    self->_4E0 = 0.0f;
}
#endif

extern "C" void fn_803B9A30(MarioActor*);
extern "C" void fn_803B9830(MarioActor*);
extern "C" void fn_8041BB90();
extern "C" void fn_804090C0(MarioState*);

extern "C" void fn_803A2FC0(Mario* self) {
    self->saveLastSafetyTrans();
    self->tryJump();
    if (self->isAnimationRun("その場足踏み上半身")) {
        self->stopAnimationUpper(nullptr, nullptr);
    }
    self->_420 = 0;
}

extern "C" void fn_803A42B0(Mario* self) {
    if (self->getPlayer()->mMovementStates_HIGH_WORD >> 10 & 1) {
        self->changeAnimation("ブレーキ滑り床", (const char*)nullptr);
        return;
    }
    self->changeAnimation("ブレーキ", (const char*)nullptr);
}

extern "C" bool fn_803A9690(Mario* self, const char* pName) {
    if (self->_970 == nullptr) {
        return false;
    }
    return strcmp(self->_970, pName) == 0;
}

extern "C" bool fn_803AA340(Mario* self) {
    self->isStatusActive(0x18);
    return false;
}

extern "C" bool fn_803ACE10(Mario* self) {
    bool ret = false;
    if (!(self->_20_LOW_WORD >> 21 & 1) || !self->isAnimationRun("しゃがみ基本")) {
        ret = true;
    }
    return ret;
}

extern "C" void fn_803AD120(Mario* self, const Triangle* pTri) {
    if (self->isSlipPolygon(pTri) == 0x20 && self->mTargetWalkSpeedIndex != 0) {
        self->_3F4 = 0.2f;
    }
}

extern "C" void fn_803B14E0(Mario* self) {
    fn_803B9A30(self->mActor);
    self->_10_HIGH_WORD &= 0xFFDFFFFF;
    fn_803B9830(self->mActor);
    fn_8041BB90();
    self->changeAnimation("落下", (const char*)nullptr);
}

extern "C" void fn_803B1720(Mario* self) {
    if (self->getCurrentStatus() != 0x26 && (self->mMovementStates_LOW_WORD >> 30 & 1)) {
        self->changeStatus(self->_7B4);
        self->stopWalk();
    }
}

extern "C" void fn_803B1780(Mario* self) {
    if (self->getCurrentStatus() == 0x26) {
        fn_804090C0(self->_7B4);
    }
}

extern "C" void fn_803B17C0(Mario* self) {
    if (self->getCurrentStatus() == 0x26) {
        self->closeStatus(self->_7B4);
    }
}

extern "C" void fn_803B1800(Mario* self) {
    if (self->getCurrentStatus() == 0x27) {
        *reinterpret_cast<f32*>(reinterpret_cast<u8*>(self->_7B8) + 0x14) = 1.0f;
        return;
    }
    self->changeStatus(self->_7B8);
}

extern "C" bool fn_803B1850(const Mario* self) {
    return self->getCurrentStatus() == 0x27;
}

extern "C" void fn_803B1880(Mario* self) {
    if (self->getCurrentStatus() != 0x28) {
        self->changeStatus(self->_7BC);
    }
}

extern "C" bool fn_803B1B70(const Mario* self);

extern "C" void fn_803B1B20(Mario* self) {
    if (fn_803B1B70(self)) {
        self->closeStatus(self->_7C0);
    }
}

extern "C" bool fn_803B1B70(const Mario* self) {
    return self->getCurrentStatus() == 0x29;
}

extern "C" f32 fn_8041BDD0();
extern "C" f32 fn_8041BDE0();
extern "C" HitSensor* fn_80038D20(const Triangle*);
extern "C" void fn_803BC440(MarioActor*, s32);

extern "C" f32 fn_803B1BA0(Mario* self) {
    if (self->isPlayerModeCloud() && self->_430 == 0) {
        return 24.0f;
    }
    return reinterpret_cast<const f32*>(self->mActor->mConst->getTable())[self->_430];
}

// NONMATCHING: the target keeps the Yoshi flag in a register variable (inlined bool helper); 64%
extern "C" bool fn_803B1C10(Mario* self) {
    bool ret = false;
    bool yoshi = false;
    if (self->isPlayerModeYoshi() && *reinterpret_cast<s32*>(static_cast<u8*>(fn_803B97F0(self->mActor)) + 0x134) > 0) {
        yoshi = true;
    }
    if (yoshi && self->isAnimationRun("基本")) {
        ret = true;
    }
    return ret;
}

extern "C" bool fn_803B1D60(Mario* self);

extern "C" f32 fn_803B1CA0(Mario* self) {
    if (fn_803B1D60(self)) {
        return fn_8041BDD0();
    }
    return self->mActor->mConst->getTable()->mJumpFrontSpeed;
}

extern "C" f32 fn_803B1D00(Mario* self) {
    if (fn_803B1D60(self)) {
        return fn_8041BDE0();
    }
    return self->mActor->mConst->getTable()->mJumpFrontReduction;
}

extern "C" bool fn_803B1D60(Mario* self) {
    bool ret = false;
    bool yoshi = false;
    if (self->isPlayerModeYoshi() && *reinterpret_cast<s32*>(static_cast<u8*>(fn_803B97F0(self->mActor)) + 0x134) > 0) {
        yoshi = true;
    }
    if (yoshi && !(self->getPlayer()->_10_HIGH_WORD >> 21 & 1)) {
        ret = true;
    }
    return ret;
}

extern "C" bool fn_803B26C0(Mario* self) {
    if (self->isPlayerModeHopper() && self->_A6C[0xC] != 0) {
        return true;
    }
    if (self->isPlayerModeInvincible() && self->_A6C[0xD] != 0) {
        return true;
    }
    return false;
}

extern "C" bool fn_803B2730(Mario* self) {
    return strcmp(MR::getSoundCodeString(self->_45C), "Sand") == 0;
}

extern "C" f32 fn_803B3870(Mario* self) {
    if (self->isStatusActive(1)) {
        return -0.9f;
    }
    return -0.999f;
}

extern "C" bool fn_803B3E90(Mario* self) {
    HitSensor* pSensor = fn_80038D20(self->mGroundPolygon);
    if (pSensor != nullptr) {
        LiveActor* pHost = MR::getSensorHost(pSensor);
        if (pHost != nullptr) {
            return MR::isEqualString(pHost->mName, "YoshiDashStep");
        }
    }
    return false;
}

extern "C" void fn_803B45E0(Mario* self, const char* pName, s32 arg) {
    self->changeAnimation(pName, (const char*)nullptr);
    fn_803BC440(self->mActor, arg);
}

extern "C" void fn_803B4630(Mario* self, const char* pName) {
    self->changeAnimation(pName, (const char*)nullptr);
    self->mActor->clearNullAnimation(-3);
}

extern "C" void fn_803B4670(Mario* self, const char* pName) {
    self->changeAnimationUpper(pName, nullptr);
    self->mActor->clearNullAnimation(-3);
}

extern "C" bool fn_803B4850(const Mario* self, const HitSensor* pSensor) {
    if (MR::getSensorHost(pSensor) != nullptr) {
        return MR::isEqualString(MR::getSensorHost(pSensor)->mName, "溶岩分離滝");
    }
    return false;
}
