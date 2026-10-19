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

// PLACEHOLDER(objects): forces the leading .sdata2 pool order [1.0f, 0.0f, MR::epsilon(), 0.5f] that the target has but plain first-use order does not give
// (the target pools these ahead of the TU's own literals: probably the .sdata2 of header-inline bodies the original TU compiled whose text the linker deduped;
// function order already matches the target); resolve when those inline bodies are identified
void BenefitItemInvincible_FORCE_MATCH_SDATA2() {
    (void)1.0f;
    (void)0.0f;
    (void)MR::epsilon();
    (void)0.5f;
}

namespace {
    const f32 cAppearThrowUpSpd = 10.0f;
    const f32 cAppearThrowFwdSpd = 5.0f;
    const f32 cBoundUpSpd = 20.0f;
};  // namespace

BenefitItemInvincible::BenefitItemInvincible(const char* pName) : BenefitItemObj(pName, "PowerUpInvincible") {
    if (MR::isGalaxyDarkCometAppearInCurrentStage()) {
        _DC = 1;
    }
}

void BenefitItemInvincible::exeCatch() {
    MR::changePlayerModeInvincible();
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

void BenefitItemInvincible::appearThrowUp() {
    if (!_DE) {
        BenefitItemObj::appearThrowUp();
        return;
    }

    MR::startActionSound(this, "SyItemAppear", -1, -1, -1);

    setNerveShoot();

    MR::setVelocity(this, mUpAxis * ::cAppearThrowUpSpd + mFwdAxis * ::cAppearThrowFwdSpd);

    MR::invalidateClipping(this);
}

void BenefitItemInvincible::initEscape() {
    BenefitItemObj::initEscape();

    MR::calcGravity(this);

    _C4 = -mGravity * ::cBoundUpSpd;
    _C4 += mFwdAxis * 10.0f;

    _13A = 60;
}
// NONMATCHING: last ~5% is register choice: target keeps the temp vector pointer in r31 across the TVec3f ctor (callee-saved), ours re-derives it from r4
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

BenefitItemInvincible::~BenefitItemInvincible() {
}

