#include "Game/MapObj/BenefitItemInvincible.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/SoundUtil.hpp"

void BenefitItemInvincible_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.0f;
    (void)MR::epsilon();
    (void)0.5f;
}

namespace {
    f32 cAppearThrowUpSpd = 10.0f;
    f32 cAppearThrowFwdSpd = 5.0f;
    f32 cBoundUpSpd = 20.0f;
};  // namespace

BenefitItemInvincible::BenefitItemInvincible(const char* pName) : BenefitItemObj(pName, "PowerupInvincible") {
    if (MR::isGalaxyDarkCometAppearInCurrentStage()) {
        _DC = 1;
    }
}

void BenefitItemInvincible::exeCatch() {
    MR::startSystemSE("SE_SY_POWER_UP_2");

    MR::hideModel(this);

    MR::changePlayerItemStatus(7);

    kill();
}

void BenefitItemInvincible::init(const JMapInfoIter& rIter) {
    BenefitItemObj::init(rIter);

    if (_DC) {
        return;
    }

    if (_E0) {
        MR::setShadowVolumeCylinderRadius(this, nullptr, 60.0f);
    }

    _13A = 0;

    TVec3f axisY, axisZ;
    MR::getRotatedAxisZ(&axisZ, mRotation);
    mFwdAxis.set(axisZ);

    MR::getRotatedAxisY(&axisY, mRotation);
    mUpAxis.set(axisY);
}

void BenefitItemInvincible::initModelAndEfx() {
    initModelManagerWithAnm(mModelName, nullptr, false);
    initEffectKeeper(5, "BenefitItemObj", false);

    initBinder(60.0f, 60.0f, 0);
    initHitSensor(1);
    MR::addHitSensorMapObj(this, "body", 4, 60.0f, TVec3f(0.0f, 60.0f, 0.0f));

    MR::startBrk(this, "PowerupInvincible");
}

void BenefitItemInvincible::appearThrowUp() {
    if (!_DE) {
        BenefitItemObj::appearThrowUp();
        return;
    }

    MR::startSystemSE("SE_SY_ITEM_APPEAR");

    setNerveShoot();

    mVelocity = mUpAxis * ::cAppearThrowUpSpd + mFwdAxis * ::cAppearThrowFwdSpd;

    MR::invalidateClipping(this);
}

void BenefitItemInvincible::initEscape() {
    BenefitItemObj::initEscape();

    MR::calcGravity(this);

    _C4 = -mGravity * ::cBoundUpSpd;
    _C4 += mFwdAxis * 10.0f;

    _13A = 60;
}
void BenefitItemInvincible::doEscape() {
    MR::calcGravity(this);

    MR::addVelocityToGravity(this, 1.0f);

    if (MR::isBindedGround(this)) {
        MR::vecKillElement(mFwdAxis, mGravity, &mFwdAxis);
        MR::normalizeOrZero(&mFwdAxis);

        _C4 = mFwdAxis * 10.0f - mGravity * ::cBoundUpSpd;

        MR::startActionSound(this, "OjPowInvincibleBound");

        _D0.zero();
    } else {
        _C4.x *= 0.98f;
        _C4.y *= 0.98f;
        _C4.z *= 0.98f;
        _FA = 1;

        _D0 += mGravity * 0.5f;

        if (_D0.length() > 20.0f) {
            _D0.setLength(20.0f);
        }
    }

    if (_13A > 0) {
        _13A--;
    }

    if (_13A == 0) {
        MR::validateHitSensors(this);
    }
}

void BenefitItemInvincible::runEfx(const char* pEffect) {
    char str[0x40];
    if (!strcmp(pEffect, "Glow")) {
        strcpy(str, pEffect);
        strcat(str, "PowerUpInvincible");
        MR::emitEffect(this, str);
    }
}

void BenefitItemInvincible::stopEfx(const char* pEffect) {
    char str[0x40];
    if (!strcmp(pEffect, "Glow")) {
        strcpy(str, pEffect);
        strcat(str, "PowerUpInvincible");
        MR::emitEffect(this, str);
    }
}

BenefitItemInvincible::~BenefitItemInvincible() {
}

void BenefitItemInvincible::runBck(const char*) {
}
