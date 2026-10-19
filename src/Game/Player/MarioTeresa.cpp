#include "Game/Player/MarioTeresa.hpp"
#include "Game/Util.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/MapObj/BigFanHolder.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/MarioParts.hpp"

// unnamed in symbols.txt: 1-arg Mario::setFrontVecKeepUp
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"


void MarioTeresa_FORCE_MATCH_SDATA2() {
    (void)100.0f;
    (void)10.0f;
    (void)0.00100000005f;
    (void)0.00999999978f;
    (void)120.0f;
    (void)110.0f;
    (void)-0.100000001f;
    (void)0.100000001f;
    (void)50.0f;
    (void)0.939999998f;
    (void)1.5f;
    (void)0.200000003f;
    (void)200.0f;
    (void)0.99000001f;
    (void)60.0f;
    (void)-2.0f;
    (void)-0.400000006f;
    (void)-0.25f;
    (void)0.25f;
    (void)0.75f;
    (void)-0.5f;
    (void)0.0399999991f;
    (void)0.300000012f;
    (void)20.0f;
    (void)-2607.59448f;
    (void)2607.59448f;
    (void)0.707000017f;
    (void)-10.0f;
}

void MarioTeresa::updateDropFlag() {
    _58 = 0;
    if (_34.dot(getAirGravityVec()) > 1.0f) {
        TVec3f horizontal;
        f32 vertical = MR::vecKillElement(_34, getAirGravityVec(), &horizontal);
        if (MR::isNearZero(getStickP()) && horizontal.length() < vertical) {
            _58 = 1;
        }
    }
}

void MarioTeresa::resetTeresaMode() {
    _42 = 0;
    _44 = 0;
    _46 = 0;
    _48 = 0;
    _50 = 0.0f;
    _34.zero();
    _28.zero();
    _60.zero();
    _6C.zero();
    _78 = 0;
    _5C = 0;
}

bool MarioTeresa::isTeresaAccel() const {
    return _44 != 0;
}

MarioTeresa::MarioTeresa(MarioActor* pActor) : MarioState(pActor, MarioStatus_Teresa) {
    _14.zero();
    _20 = 0.0f;
    _24 = 0.0f;
    _40 = 0;
    _4C = 0.0f;
    _54 = 0.0f;
    _58 = 0;
    _59 = 0;
    resetTeresaMode();
}

bool MarioTeresa::close() {
    if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
        getPlayer()->mCometRestrictions &= ~1u;
    }

    return true;
}

bool MarioTeresa::update() {
    if (getPlayerMode() != PlayerMode_Teresa) {
        return false;
    }

    if (getPlayer()->isStatusActive(MarioStatus_Recovery)) {
        return true;
    }

    if (mActor->_EA4) {
        return true;
    }

    _50 = 0.0f;
    _14 = getJumpVec();
    if (!getPlayer()->_418) {
        _58 = 0;
    }

    getPlayer()->mMovementStates.mLowWord &= 0xFFEFFFFFU;
    _20 = checkHeight();
    _24 = mActor->getConst().getTable()->mTeresaWaitHeight;
    checkAccel();
    checkWind();
    checkGroundReflect();
    checkWallCeilReflect();
    addVelocity(_34);
    procNoControl();
    procNearGroundControl();
    procAirControl();
    procDrop();
    Mario* player = getPlayer();
    player->_3BC = 1;
    procControl();
    f32 speed = _14.length() / 10.0f;
    if (speed < 0.01f) {
        speed = 0.0f;
    }

    getPlayer()->mWalkSpeed = MR::clamp(speed, 0.0f, 1.0f);
    setJumpVec(_14);
    if (!_42) {
        getPlayer()->playSoundTeresaFlying();
    }

    return true;
}

f32 MarioTeresa::checkHeight() {
    f32 offset = 0.0f;
    f32 height = getPlayer()->mVerticalSpeed;
    MarioActor* actor;
    while (offset < 2.0f * (actor = mActor)->getConst().getTable()->mTeresaWaitHeight) {
        const TVec3f& position = actor->_240;
        bool found = false;
        f32 radius = 120.0f;
        s32 count = getPlayer()->fn_803B2A20(position + getGravityVec() * offset, radius, radius);
        for (u32 i = 0; i < count; i++) {
            const HitInfo* hit = getPlayer()->fn_803B2AA0(i);
            TVec3f normal(*hit->mParentTriangle.getNormal(0));
            TVec3f direction(hit->_64);
            direction.sub(direction, position);
            f32 distance = direction.length() - 110.0f;
            if (distance < 0.0f) {
                distance = 0.0f;
            }

            MR::normalizeOrZero(&direction);
            if (normal.dot(getGravityVec()) < -0.1f && direction.dot(getGravityVec()) > 0.1f) {
                if (height > distance) {
                    height = distance;
                }

                found = true;
            }
        }

        if (found) {
            break;
        }

        offset += radius;
    }

    return height;
}

void MarioTeresa::checkAccel() {
    if (checkTrgA()) {
        if (!_44) {
            getPlayer()->mFlags20.mWord |= 0x08000000U;
        }

        _44 = mActor->getConst().getTable()->mTeresaAccelTime;
    }

    if (_44) {
        _44--;
    }
}

void MarioTeresa::checkGroundReflect() {
    if (((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
        getPlayer()->mMovementStates.mLowWord &= ~Mario::MovementStates::Jumping;
    }

    if (!((getPlayer()->mMovementStates.mLowWord & Mario::MovementStates::Jumping) >> 31) && getPlayer()->_3CE) {
        getPlayer()->mDrawStates.mWord &= 0xFFF7FFFFU;
        getPlayer()->tryJump();
        cutGravityElementFromJumpVec(true);
        f32 vertical = MR::vecKillElement(_34, getAirGravityVec(), &_34);
        if (vertical < 0.0f) {
            _34 += getAirGravityVec() * vertical;
        }
    }
}

void MarioTeresa::procNoControl() {
    if (_46) {
        _46--;
        _48 = 0;
        _34.scale(mActor->getConst().getTable()->mTeresaWallReflectReduction);
    }

    if (MR::isNearZero(getStickP())) {
        _34.x *= 0.99f;
        _34.y *= 0.99f;
        _34.z *= 0.99f;
    }
}

void MarioTeresa::procNearGroundControl() {
    if (MR::isNearZero(getVelocity())) {
        return;
    }

    if (calcAngleD(getPlayer()->getShadowNorm()) < 60.0f) {
        if (!(_20 < 50.0f + _24)) {
            return;
        }

        if (_46) {
            return;
        }

        if (_34.dot(getPlayer()->getShadowNorm()) >= 0.0f) {
            return;
        }

        f32 speed = _34.length();
        TVec3f direction(_34);
        if (MR::normalizeOrZero(&direction)) {
            return;
        }

        TVec3f side;
        side.cross(direction, getPlayer()->getShadowNorm());
        direction.cross(getPlayer()->getShadowNorm(), side);
        direction.setLength(speed);
        f32 blend = MR::clamp((50.0f + _24 - _20) / 50.0f, 0.0f, 1.0f);
        blend *= 0.5f;
        f32 reduction = MR::diffAngleAbs(_34, direction) / 3.1415927f;
        MR::vecBlendSphere(_34, direction, &_34, blend);
        _34.scale(1.0f - reduction);
    }
}

void MarioTeresa::procDrop() {
    MarioActor* actor = mActor;
    bool descending = actor->getLastMove().dot(getGravityVec()) >= 0.0f;
    f32 ratio = 1.0f;
    bool nearGround = _20 < _24 + mActor->getConst().getTable()->mTeresaDropDownHeight;
    if (descending) {
        if (nearGround) {
            ratio = (_20 - _24) / mActor->getConst().getTable()->mTeresaDropDownHeight;
            if (ratio < 0.0f) {
                ratio = 0.0f;
            }
        }
    } else {
        if (nearGround) {
            ratio = 0.0f;
        }

        if (nearGround) {
            ratio = 0.0f;
        }
    }

    if (_20 > _24) {
        f32 distance = _20 - _24;
        f32 ratio = 1.0f;
        if (distance < 100.0f) {
            ratio = distance / 100.0f;
        }

        if (_58) {
            addTeresaVerticalVelocity(0.25f * ratio);
        } else {
            addTeresaVerticalVelocity(0.1f * ratio);
        }

        f32 vertical = MR::vecKillElement(_34, getAirGravityVec(), &_34);
        if (1.5f * vertical > _20 - _24) {
            vertical *= 0.75f;
        }

        _34 += getAirGravityVec() * vertical;
    } else {
        f32 vertical = MR::vecKillElement(_34, getAirGravityVec(), &_34);
        if (vertical < 0.0f) {
            _34 += getAirGravityVec() * vertical;
        }
    }

    if (_20 > 10.0f + _24) {
        TVec3f horizontal;
        MarioActor* actor = mActor;
        f32 vertical = MR::vecKillElement(actor->getLastMove(), getGravityVec(), &horizontal);
        f32 brake;
        if (vertical > -0.5f) {
            f32 dropSpeed = vertical;
            dropSpeed += 0.5f;
            brake = dropSpeed / mActor->getConst().getTable()->mTeresaDropBase;
            brake += 0.04f;
        } else {
            brake = mActor->getConst().getTable()->mTeresaRisingBrake;
        }

        MR::clamp(brake, 0.0f, 1.0f);
        getPlayer()->mDrawStates.mWord |= 0x00000008U;
    }
}

void MarioTeresa::addTeresaVerticalVelocity(f32 amount) {
    _34 += getAirGravityVec() * amount;
    TVec3f horizontal;
    f32 vertical = MR::vecKillElement(_34, getAirGravityVec(), &horizontal);
    f32 maximum = mActor->getConst().getTable()->mTeresaDropSpeedMax;
    f32 minimum = -mActor->getConst().getTable()->mTeresaRiseSpeedMax;
    minimum = 10.0f * minimum;
    maximum = 20.0f * maximum;
    if (_58) {
        f32 angle = 3.1415927f * (static_cast< f32 >(getPlayer()->_418) / mActor->getConst().getTable()->mTeresaWallThroughTime);
        maximum *= 1.0f + 0.5f * JMath::sSinCosTable.sinRadian(angle);
    }

    if (((getPlayer()->mDrawStates.mWord >> 0) & 1U) && _28.dot(getAirGravityVec()) > 0.707f) {
        maximum *= 2.0f;
    }

    vertical = MR::clamp(vertical, minimum, maximum);
    _34 = horizontal + getAirGravityVec() * vertical;
    _50 += amount;
}

void MarioTeresa::addTeresaHorizontalVelocity(const TVec3f& rVelocity) {
    TVec3f addition;
    MR::vecKillElement(rVelocity, getAirGravityVec(), &addition);
    _34 += addition;
    TVec3f horizontal;
    f32 vertical = MR::vecKillElement(_34, getAirGravityVec(), &horizontal);
    f32 maximum = mActor->getConst().getTable()->mTeresaHorizontalSpeedMax;
    if (((getPlayer()->mDrawStates.mWord >> 0) & 1U) && _28.dot(horizontal) > 0.707f) {
        maximum *= 2.0f;
    }

    if (horizontal.length() >= maximum) {
        horizontal.setLength(maximum);
    }

    _34 = horizontal + getAirGravityVec() * vertical;
}

bool MarioTeresa::start() {
    if (getPlayer()->_430 == 4) {
        TVec3f direction(-getPlayer()->_220);
        getPlayer()->setFrontVecKeepUp(direction);
    }

    Mario* player = getPlayer();
    player->_42A = 0;
    player->_430 = 0;
    getPlayer()->cancelSquatMode();
    changeAnimation("落下", "落下");

    if (MR::isGalaxyHorrorCometAppearInCurrentStage()) {
        getPlayer()->mCometRestrictions |= 1u;
    }
    _5C = 0;

    return true;
}

void MarioTeresa::checkWind() {
    TVec3f wind;
    f32 strength;
    BigFanFunction::calcWindInfo(&wind, mActor->_240, &strength);
    strength = MR::clamp(strength, 0.0f, 10.0f);
    if (strength > 0.0f) {
        playSound("テレサ風に乗る", 50.0f * strength);
    }

    if (!MR::isNearZero(strength)) {
        getPlayer()->mDrawStates.mWord |= 0x00000001U;
    }

    wind.scale(strength);
    _28.x *= 0.94f;
    _28.y *= 0.94f;
    _28.z *= 0.94f;
    _28 += wind * 0.1f * 1.5f;
    _34 += wind * 0.2f;
    if (_28.length() > 0.2f) {
        getPlayer()->mDrawStates.mWord |= 0x00000001U;
    }
}

// NONMATCHING: register allocation, stack temporaries and literal-pool layout.
void MarioTeresa::checkWallCeilReflect() {
    TVec3f normal;
    TVec3f wallNormal;
    bool ceiling = false;
    if (getPlayer()->calcDistToCeil(false) < 200.0f) {
        normal = getPlayer()->_3B0;
        ceiling = true;
    }

    bool wall = false;
    if (!ceiling && getPlayer()->getHitWallNorm(&wallNormal)) {
        wall = true;
        normal = wallNormal;
    }

    bool reflected = false;
    if (!ceiling && !wall && _78) {
        TVec3f combined(_6C);
        combined += _60;
        if (!MR::normalizeOrZero(&combined)) {
            reflected = true;
            normal = combined;
        }
    }
    _78 = 0;
    _60.zero();
    _6C.zero();

    if (ceiling || wall || reflected) {
        if (_5C) {
            _5C--;
            getPlayer()->addTrans(normal * 10.0f, nullptr);
        } else {
            f32 speed = MR::vecKillElement(_34, normal, &_34);
            if (speed < 0.0f) {
                if (speed > -5.0f) {
                    _34 += normal * 5.0f;
                } else {
                    _34 += normal * -speed;
                }
            } else {
                _34 += normal * speed * 1.5f;
            }
            _5C = 30;
            if (!_42) {
                playSound("テレサ壁反射");
                playSound("声壁反射");
                mActor->changeTeresaAnimation("hit", -1);
                MarioActor* actor = mActor;
                const TVec3f& wallNormal = getPlayer()->getWallNorm();
                actor->fn_803BB510(getPlayer()->getWallPos(), wallNormal);
                mActor->_946 = 10;
                _46 = mActor->getConst().getTable()->mTeresaWallReflectTime;
                _14.zero();
                getPlayer()->_418 = 0;
            }
        }
        _42++;
        return;
    }
    _42 = 0;
    _5C = 0;
}

void MarioTeresa::procAirControl() {
    if (getPlayer()->_418 < mActor->getConst().getTable()->mTeresaWallThroughTime - 30 && ((isTeresaAccel() && !_46) || _48)) {
        if (((getPlayer()->mFlags20.mWord >> 27) & 1U)) {
            if (_34.dot(getAirGravityVec()) > 0.0f) {
                addTeresaVerticalVelocity(-2.0f);
            } else {
                addTeresaVerticalVelocity(-0.4f);
            }
        } else {
            addTeresaVerticalVelocity(-0.25f);
        }

        if (isTeresaAccel()) {
            playSound("テレサ踏ん張り");
            if (((getPlayer()->mFlags20.mWord >> 27) & 1U)) {
                _48 = mActor->getConst().getTable()->mTeresaTrgOnPushTime1;
            } else if (_48 < 15) {
                _48 = mActor->getConst().getTable()->mTeresaTrgOnPushTime2;
            }
        }

        if (_48) {
            _48--;
        }
    }
}

void MarioTeresa::procControl() {
    if (!_46) {
        if (!MR::isNearZero(getStickP())) {
            getPlayer()->setFrontVecKeepUp(getWorldPadDir(), mActor->getConst().getTable()->mTeresaAirWalkTurnSpd);
        }

        if (getPlayer()->_402 < (mActor->getConst().getTable()->mAirWalkTime / 2)) {
            getAnimator()->setSpeed(1.5f);
        }

        TVec3f velocity(_14);
        f32 alignment = 1.0f;
        if (!MR::isNearZero(getStickP())) {
            velocity.dot(getWorldPadDir());
        }

        if (alignment < 0.0f) {
            alignment *= 0.3f;
        }

        f32 acceleration = 0.3f;
        if (getPlayer()->_418 > (mActor->getConst().getTable()->mTeresaWallThroughTime / 2)) {
            acceleration = 2.0f;
        }

        addTeresaHorizontalVelocity(getWorldPadDir() * alignment * acceleration);
    }

    if (checkTrgZ()) {
        if (_34.dot(getAirGravityVec()) < 0.0f) {
            MR::vecKillElement(_34, getAirGravityVec(), &_34);
        }

        mActor->changeTeresaAnimation("fallquicklystart", -1);
        playSound("声壁押し");
    }
}

void MarioTeresa::doTeresaReflection(const TVec3f& rNormal, bool effect) {
    TVec3f direction(rNormal);
    MR::normalizeOrZero(&direction);
    TVec3f tangent;
    f32 speed = MR::vecKillElement(_34, direction, &tangent);
    if (speed < 0.0f) {
        if (speed > -10.0f) {
            speed = -10.0f;
        }

        TVec3f horizontal;
        f32 vertical = MR::vecKillElement(direction * -speed, getAirGravityVec(), &horizontal);
        addTeresaHorizontalVelocity(horizontal * 2.0f);
        addTeresaVerticalVelocity(2.0f * vertical);
    } else {
        TVec3f horizontal;
        f32 vertical = MR::vecKillElement(direction, getAirGravityVec(), &horizontal);
        addTeresaHorizontalVelocity(horizontal);
        addTeresaVerticalVelocity(vertical);
    }

    _46 = mActor->getConst().getTable()->mTeresaWallReflectTime;
    _14.zero();
    if (!_42) {
        mActor->changeTeresaAnimation("hit", -1);
        playSound("声壁反射");
        if (effect) {
            playSound("テレサ壁反射");
            Mario* player = getPlayer();
            mActor->fn_803BB510(getPlayer()->_25C, player->_268);
        }
    }

    Mario* player = getPlayer();
    player->_418 = 0;
    mActor->_946 = 5;
    _42++;
}

XanimeGroupInfo teresaAnimeTable[] = {
    {{"基本"}, 1.0f, 16},
    {{""}},
};

XanimeBckTable2 teresaAnime2[] = {
    {{"基本"}, {{"wait", 1.0f}, {"run", 0.0f}}},
    {{""}, {{"", 0.0f}, {nullptr, 0.0f}}},
};

// NONMATCHING: only shared string-pool offsets differ.
void MarioTeresaModel::initAnimation() {
    _C = 0.0f;
    mResource = new XanimeResourceTable(MR::getResourceHolder(mActor), teresaAnimeTable, nullptr, nullptr, teresaAnime2, nullptr, nullptr, nullptr);
    mPlayer = new XanimePlayer(MR::getJ3DModel(mActor), mResource);
    mPlayer->setDefaultAnimation("基本");
    mPlayer->changeAnimationBck("基本");
    mActor->mModelManager->mXanimePlayer = mPlayer;
}


void MarioTeresa::fn_80405B10(const TVec3f& position) {
    _78 = 1;
    if (_60.x > position.x) _60.x = position.x;
    if (_60.y > position.y) _60.y = position.y;
    if (_60.z > position.z) _60.z = position.z;
    if (_6C.x < position.x) _6C.x = position.x;
    if (_6C.y < position.y) _6C.y = position.y;
    if (_6C.z < position.z) _6C.z = position.z;
}


MarioTeresaModel* MarioTeresaModel::create(MarioActor* actor, bool luigi) {
    const char* modelName = "TeresaMario";
    if (luigi) modelName = "TeresaLuigi";
    if (!MR::isCreatePlayerResourceTeresa()) return nullptr;
    return new MarioTeresaModel(modelName, actor);
}

MarioTeresaModel::MarioTeresaModel(const char* modelName, MarioActor* actor)
    : mActor(nullptr), _4(0.0f), _8(0.0f), _C(0.0f), _10(0), mPlayer(nullptr), mResource(nullptr) {
    mActor = new MarioParts(actor, "テレサの皮", modelName, nullptr);
    mActor->_9C = "TeresaMario";
    mActor->initWithoutIter();
    mActor->kill();
    mActor->initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    MR::initShadowVolumeSphere(mActor, 90.0f);
    initAnimation();
}
