#include "Game/Player/MarioDamage.hpp"
#include "Game/Player/HorrorDamageExecutor.hpp"
#include "Game/Util.hpp"
#include "Game/Enemy/KarikariDirector.hpp"
#include "Game/Map/CollisionCode.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioAbyssDamage.hpp"
#include "Game/Player/MarioAccess.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioBlown.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioDarkDamage.hpp"
#include "Game/Player/MarioFaint.hpp"
#include "Game/Player/MarioFireDamage.hpp"
#include "Game/Player/MarioFireDance.hpp"
#include "Game/Player/MarioFireRun.hpp"
#include "Game/Player/MarioFreeze.hpp"
#include "Game/Player/MarioMapCode.hpp"
#include "Game/Player/MarioParalyze.hpp"
#include "Game/Player/MarioSwim.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/SequenceUtil.hpp"

extern "C" LiveActor* fn_803B97F0(MarioActor*);
extern "C" void fn_803B9560(MarioActor*);
extern "C" void fn_803B94E0(MarioActor*);
extern "C" void fn_803BE8D0(MarioActor*, TVec3f*);
extern "C" void fn_803BE840(MarioActor*, const TVec3f&);
extern "C" void fn_803E4E00(MarioDamage*);
extern "C" void fn_803E5FA0(MarioFireDance*);


void MarioDamage_FORCE_MATCH_SDATA2() {
    (void)10.0f;
    (void)5.0f;
    (void)0.949999988f;
    (void)30.0f;
    (void)20.0f;
    (void)50.0f;
    (void)-10.0f;
    (void)0.699999988f;
    (void)0.100000001f;
    (void)0.25f;
}

MarioDamage::MarioDamage(MarioActor* pActor)
    : MarioState(pActor, MarioStatus_Damage), _11(0), _12(0), _14(0), _16(0), _18(0), _1C(0.0f, 0.0f, 0.0f), _28(nullptr), _2C(nullptr), _30(false),
      _34(0.0f, 0.0f, 0.0f) {
    mHorrorDamageExecutor = new HorrorDamageExecutor(this);
    _44 = 0;
}

bool MarioDamage::start() {
    _14 = 0;
    _18 = 0;
    if (_30) {
        _1C = _34;
        f32 mag = _34.length();
        TVec3f gravity(getAirGravityVec());
        gravity.scale(mag);
        TVec3f gravity2(gravity);
        gravity2.scale(0.75f);
        _1C.sub(gravity2);
        _1C.setLength(7.0f);
    }

    if (_1C.dot(getPlayer()->mFrontVec) > 0.0f) {
        changeAnimationNonStop("中後ダメージ");
        _28 = "中後ダメージ空中";
        _2C = "中後ダメージ着地";
        getPlayer()->setFrontVecKeepUp(_1C);
    } else {
        changeAnimationNonStop("中ダメージ");
        _28 = "中ダメージ空中";
        _2C = "中ダメージ着地";
        getPlayer()->setFrontVecKeepUp(-_1C);
    }

    if (!_11) {
        playEffect("ダメージ");
    }

    startPadVib(3);
    getPlayer()->mMovementStates._1 = false;
    getPlayer()->mMovementStates.jumping = true;
    getPlayer()->mMovementStates._B = false;
    getPlayer()->mMovementStates._3E = 0;
    _1C += -mActor->_240 * mActor->getConst().getTable()->mJumpHeightDamage;
    getPlayer()->mJumpVec = _1C;
    addVelocity(_1C);

    _12 = !_11;
    if (_11) {
        if (isPlayerModeYoshi()) {
            MR::startActionSound(fn_803B97F0(mActor), "Thrown", -1, -1, -1);
        } else {
            playSound("声投げられ");
        }
        _11 = 0;
        fn_803B9560(mActor);
    } else {
        playSound("ダメージ");
        if (isPlayerModeYoshi()) {
            MR::startActionSound(fn_803B97F0(mActor), "Damage", -1, -1, -1);
        } else {
            playSound("声小ダメージ");
        }
        mActor->decLifeMiddle();
        if (_30) {
            fn_803BE8D0(mActor, &_34);
        } else {
            fn_803E4E00(this);
        }
        fn_803B94E0(mActor);
    }

    _44 = 0;
    _30 = false;
    return true;
}

void MarioDamage::setVec(const TVec3f& rDirection) {
    MR::vecKillElement(rDirection, mActor->_240, &_1C);
    _1C.setLength(mActor->getConst().getTable()->mJumpDistDamage);
}

void MarioDamage::setVecSize(f32 horizontal, f32 vertical) {
    _1C.setLength(horizontal);
    _1C += -mActor->_240 * vertical;
}

void MarioDamage::stopHead(const TVec3f& rNormal) {
    if (!_18) {
        TVec3f horizontal;
        f32 verticalSpeed = MR::vecKillElement(_1C, mActor->_240, &horizontal);
        f32 towardWall = MR::vecKillElement(horizontal, rNormal, &_1C);
        _1C += mActor->_240 * verticalSpeed;
        if (towardWall < 0.0f) {
            _1C += rNormal * -towardWall * 0.5f;
        }
    } else {
        TVec3f horizontal;
        MR::vecKillElement(rNormal, getAirGravityVec(), &horizontal);
        if (!MR::normalizeOrZero(&horizontal)) {
            f32 towardWall = MR::vecKillElement(_1C, rNormal, &_1C);
            if (towardWall < 0.0f) {
                _1C += rNormal * -towardWall * 0.5f;
            }
        }
    }
}

extern "C" bool fn_803E48F0(MarioDamage*);
extern "C" bool fn_803DB380(void*, const char*, const char*);
extern "C" bool fn_803E4BF0(MarioDamage*);
extern "C" bool fn_803E4D00(MarioDamage*);

extern "C" bool fn_803E48F0(MarioDamage* p) {
    p->addVelocity(p->_1C);
    p->_1C += p->mActor->_240 * p->mActor->getConst().getTable()->mGravityDamage;
    if (p->_14 > 20) {
        if (p->_28) {
            p->changeAnimation(p->_28);
        }

        if (p->getPlayer()->_1C._0) {
            f32 vertical = MR::vecKillElement(p->_1C, p->getAirGravityVec(), &p->_1C);
            p->_1C.x *= 0.95f;
            p->_1C.y *= 0.95f;
            p->_1C.z *= 0.95f;
            p->_1C += p->getAirGravityVec() * vertical;
        }
    }

    if (p->getPlayer()->mMovementStates._1) {
        if (p->getPlayer()->mVerticalSpeed > 30.0f &&
            fn_803DB380(*(void**)((u8*)p->mActor + 0x1080), MR::getSensorHost(p->getSensor(p->getGroundPolygon()))->mName, "ダメージ中跳ねる")) {
            Mario* player = p->getPlayer();
            p->_1C += player->_368 * 20.0f;
            p->getPlayer()->mMovementStates._1 = false;
            p->getPlayer()->mMovementStates.jumping = true;
            return true;
        }

        p->getPlayer()->mMovementStates.jumping = false;
        p->playSound("吹っ飛び倒れ");
        p->playEffect("共通ダメージ着地");
        MR::vecKillElement(p->_1C, p->mActor->_240, &p->_1C);
        p->_18 = 1;
        p->_14 = 0;
        if (!p->mActor->mHealth) {
            p->_18 = 2;
            p->changeAnimation(p->_2C);
        } else if (MR::isGalaxyHorrorCometAppearInCurrentStage() && !p->isPlayerModeYoshi()) {
            p->_18 = 3;
        } else {
            p->changeAnimation(p->_2C);
        }
    } else if (!p->mActor->mHealth) {
        if (p->_14 > 240) {
            p->mActor->forceGameOverAbyss();
        }
    } else if (p->_44 > 360) {
        return false;
    }

    return true;
}

extern "C" bool fn_803E4BF0(MarioDamage* p) {
    if (!p->getPlayer()->mMovementStates._1) {
        p->getPlayer()->mMovementStates.jumping = true;
        p->_18 = 0;
        return true;
    }

    MR::vecKillElement(p->_1C, p->getAirGravityVec(), &p->_1C);
    p->addVelocity(p->_1C);
    p->_1C.x *= 0.95f;
    p->_1C.y *= 0.95f;
    p->_1C.z *= 0.95f;
    if (!p->isAnimationRun(p->_2C)) {
        return false;
    }

    if (p->_14 > 15 && p->checkTrgA()) {
        p->getPlayer()->tryJump();
        return false;
    }

    return true;
    return true;
}

extern "C" bool fn_803E4D00(MarioDamage* p) {
    if (!p->getPlayer()->mMovementStates._1) {
        p->getPlayer()->mMovementStates.jumping = true;
        p->_18 = 0;
        return true;
    }

    if (p->_14 == 40) {
        if (!p->mActor->mHealth) {
            p->mActor->forceGameOver();
        } else {
            return false;
        }
    }

    return true;
    return true;
}

bool MarioDamage::updateHorrorDamage() {
    if (_14 == 1) {
        mHorrorDamageExecutor->startBreak();
    }

    mHorrorDamageExecutor->updateNerve();
    if (mHorrorDamageExecutor->isBreakEnd()) {
        return false;
    }

    return true;
}

extern "C" void fn_803E4E00(MarioDamage* p) {
    TVec3f velocity(p->_1C);
    const TVec3f& gravity = p->getAirGravityVec();
    TVec3f vertical;
    JMAVECScaleAdd(&gravity, &velocity, &vertical, -gravity.dot(velocity));
    velocity.sub(vertical);
    TVec3f horizontal(vertical);
    horizontal.scale(0.5f);
    velocity.add(horizontal);
    fn_803BE840(p->mActor, velocity);
}

bool MarioDamage::update() {
    _14++;
    _44++;
    if (mActor->_EA4) {
        return true;
    }

    switch (_18) {
    case 0:
        if (!fn_803E48F0(this)) {
            return false;
        }
        break;
    case 1:
        if (!fn_803E4BF0(this)) {
            return false;
        }
        break;
    case 2:
        if (!fn_803E4D00(this)) {
            return false;
        }
        break;
    case 3:
        if (!updateHorrorDamage()) {
            return false;
        }
        break;
    }

    getPlayer()->mJumpVec = _1C;
    return true;
}

bool MarioDamage::close() {
    stopAnimation("ダメージ");
    stopAnimation("ダメージ着地", "基本");
    if (_12) {
        _16 = 120;
    }

    return true;
}

bool MarioDamage::notice() {
    if (!mActor->mHealth) {
        if (getNoticedStatus() == MarioStatus_Swim) {
            mActor->forceGameOver();
        }

        return true;
    }

    return false;
}

MarioFireDamage::MarioFireDamage(MarioActor* pActor) : MarioState(pActor, MarioStatus_FireDamage) {
    _12 = 0;
}

void MarioFireDamage::decAfterTimer() {
    if (_12 && !isStatusActiveID(MarioStatus_FireDance) && !isStatusActiveID(MarioStatus_FireRun)) {
        _12--;
    }
}

MarioAbyssDamage::MarioAbyssDamage(MarioActor* pActor) : MarioState(pActor, MarioStatus_AbyssDamage) {
    _12 = 0;
    _14 = 0;
    _18.zero();
}

bool MarioAbyssDamage::start() {
    _12 = 0;
    _14 = 0;
    mActor->forceGameOverAbyss();
    return false;
}

bool MarioAbyssDamage::update() {
    Mario* player = getPlayer();
    player->addTrans(_18, "Module");
    switch (_14) {
    case 0:
        _14++;
        _12 = 120;
        MR::requestStartGameOverDemo();
        break;
    case 1:
        if (_12) {
            _12--;
        }

        if (!_12) {
            mActor->forceGameOverAbyss();
            return false;
        }

        break;
    }

    return true;
}

bool MarioAbyssDamage::close() {
    return true;
}

const char* MarioDamage_FORCE_MATCH_DATA(u32 index) {
    switch (index) {
    case 0:
        return "炎ダメージ";
    case 1:
        return "炎ダメージ青煙";
    default:
        return "炎ダメージ煙";
    }
}

MarioFireRun::MarioFireRun(MarioActor* pActor) : MarioState(pActor, MarioStatus_FireRun) {
    _12 = 0;
    _14 = 0;
    _18 = 0.0f;
}

bool MarioFireRun::start() {
    _12 = mActor->getConst().getTable()->mFireRunTimer1;
    _14 = 0;
    stopAnimationUpper(nullptr);
    changeAnimation("ファイアラン前兆");
    if (!getPlayer()->mMovementStates._1) {
        _18 = -mActor->getConst().getTable()->mFireRunFirstJump;
    } else {
        _18 = 0.0f;
    }

    return true;
}

bool MarioFireRun::move() {
    if (getStickX() != 0.0f || getStickY() != 0.0f) {
        const TVec3f& padDir = getWorldPadDir();
        getPlayer()->setFrontVecKeepUp(padDir, mActor->getConst().getTable()->mFireRunTurnRatio);
    }

    if (getPlayer()->checkTrgA() || mActor->isRequestJump()) {
        Mario* player = getPlayer();
        player->mWalkSpeed = 1.0f;
        getPlayer()->tryJump();
        return false;
    }

    return true;
}

bool MarioFireRun::update() {
    switch (_14) {
    case 0:
        if (!getPlayer()->mMovementStates._1) {
            getPlayer()->mJumpVec = getAirGravityVec() * _18;
            addVelocity(getAirGravityVec() * _18);
            _18 += mActor->getConst().getTable()->mFireRunGravity;
            if (_18 > 50.0f) {
                _18 = 50.0f;
            }

            const TVec3f& velocity = mActor->_288;
            if (velocity.dot(getAirGravityVec()) < -10.0f) {
                _18 = 0.0f;
                addVelocity(getFrontVec() * 5.0f);
            }
        } else if (_12) {
            _12--;
        } else {
            _14++;
            _12 = mActor->getConst().getTable()->mFireRunTimer2;
            if (!mActor->mHealth) {
                _12 >>= 1;
            }

            _18 = 0.0f;
            changeAnimation("炎のランナー");
        }

        break;
    case 1:
        playSound("炎ダメージ炎上中");
        if (!getPlayer()->mMovementStates._1) {
            _14 = 2;
            _12 += mActor->getConst().getTable()->mFireRunTimer3;
        }

        addVelocity(getFrontVec() * mActor->getConst().getTable()->mFireRunSpeed);
        if (_12) {
            _12--;
        }

        if (!_12) {
            _12 = mActor->getConst().getTable()->mFireRunTimer3;
            _14++;
        }

        return move();
    case 2:
        if (mActor->isEnableNerveChange() && getStickP() > 0.7f) {
            return false;
        }

        if (!getPlayer()->mMovementStates._1) {
            getPlayer()->mJumpVec = getAirGravityVec() * _18;
            addVelocity(getAirGravityVec() * _18);
            _18 += mActor->getConst().getTable()->mFireRunGravity;
            if (_18 > 50.0f) {
                _18 = 50.0f;
            }

            const TVec3f& velocity = mActor->_288;
            if (velocity.dot(getAirGravityVec()) < -10.0f) {
                _18 = 0.0f;
                addVelocity(getFrontVec() * 5.0f);
            }

            if (_12) {
                _12--;
            }

            break;
        }

        const MarioConstTable* table = mActor->getConst().getTable();
        if (_12 > table->mFireRunTimer3) {
            addVelocity(getFrontVec() * table->mFireRunSpeed);
        } else {
            addVelocity(getFrontVec() * table->mFireRunSpeed * _12 / table->mFireRunTimer3);
        }

        if (mActor->isEnableNerveChange()) {
            if (_12) {
                _12--;
            }

            if (!_12) {
                return false;
            }
        }

        return move();
    }

    return true;
}

bool MarioFireRun::close() {
    if (!mActor->mHealth) {
        mActor->forceGameOver();
    }

    if (getPlayer()->mMovementStates.jumping) {
        stopAnimation("炎のランナー", "落下");
    } else {
        stopAnimation("炎のランナー", "基本");
        if (getStickP() < 0.1f) {
            playSound("声炎ダメージ終了");
        }
    }

    stopEffect("炎ダメージ煙");
    stopEffect("炎ダメージ青煙");
    mActor->_1B4 = 0;
    return true;
}

bool MarioFireRun::notice() {
    return false;
}

MarioFireDance::MarioFireDance(MarioActor* pActor) : MarioState(pActor, MarioStatus_FireDance), _2C(0) {
    _14.zero();
    _20 = 0.0f;
    _24 = 0;
    _26 = 0;
    _28 = 0;
    _29 = 0;
    _2A = 0;
}

bool MarioFireDance::start() {
    _2A = 0;
    _2C = 0;
    stopAnimationUpper(nullptr);
    _20 = -mActor->getConst().getTable()->mFireDanceFirstJump;
    Mario* player = getPlayer();
    MR::vecKillElement(player->mJumpVec, getAirGravityVec(), &_14);
    _24 = 0;
    _26 = 0;
    if (getPlayer()->mDrawStates._10) {
        _20 *= 0.25f;
        _14 = getPlayer()->getWallNorm() * mActor->getConst().getTable()->mFireDanceFirstJump * 0.5f;
        getPlayer()->setFrontVecKeepUp(getPlayer()->getWallNorm());
        _24 = 1;
        _26 = 60;
    }

    _28 = 0;
    impact();
    if (!*((u8*)getPlayer() + 0xa79)) {
        impactEffect();
    }
    startPadVib(3);
    getPlayer()->mJumpVec = getAirGravityVec() * _20;
    return true;
}

void MarioFireDance::impact() {
    changeAnimation("ファイアダンス");
    if (!getPlayer()->mDrawStates._10) {
        if (_14.length() > 2.0f * mActor->getConst().getTable()->mFireDanceMoveSpeed) {
            _14.setLength(2.0f * mActor->getConst().getTable()->mFireDanceMoveSpeed);
        }

        _14.setLength(0.5f * _14.length());
    }

    getPlayer()->mMovementStates._1 = false;
    getPlayer()->mMovementStates.jumping = true;
    _2C = 30;
}

void MarioFireDance::impactEffect() {
    playSound("ダメージ");
    playEffect("ダメージ");
    switch (_29) {
    case 0:
        if (isPlayerModeYoshi()) {
            MR::startActionSound(fn_803B97F0(mActor), "DamageFire", -1, -1, -1);
        } else {
            playSound("声炎ダメージ");
            playSound("炎ダメージ");
        }
        if (mActor->_1B4) {
            playEffect("炎ダメージ青煙");
        } else {
            playEffect("炎ダメージ煙");
        }

        break;
    case 1:
        if (isPlayerModeYoshi()) {
            MR::startActionSound(fn_803B97F0(mActor), "DamageFire", -1, -1, -1);
        } else {
            playSound("声針ダメージ");
            playSound("針ダメージ");
        }
        break;
    }
}

extern "C" void fn_803E5FA0(MarioFireDance* p) {
    if (!p->_29) {
        p->mActor->decLifeLarge();
    } else {
        p->mActor->decLifeMiddle();
    }

    if (!p->mActor->mHealth) {
        p->mActor->forceGameOverNonStop();
    }

    p->_20 = -p->mActor->getConst().getTable()->mFireDanceFirstJump;
    p->impact();
    p->impactEffect();
    p->startPadVib(3);
    p->_2A = 0;
}

bool MarioFireDance::update() {
    _2A++;
    if (_2C) {
        _2C--;
    }

    if (getPlayer()->mMovementStates._1 && !getPlayer()->isRising() && !_2C) {
        if (getPlayer()->_960 != 0x81 && getPlayer()->_960 != CollisionFloorCode_DamageFire && getPlayer()->_960 != CollisionFloorCode_Needle &&
            !*((u8*)mActor + 0x1064)) {
            if (_28 == 1) {
                if (MR::getPlayerLeft() == 0 && !mActor->mHealth) {
                    mActor->changeGameOverAnimation();
                    return true;
                }

                getPlayer()->connectToFireRun();
                return false;
            }

            _28++;
            _20 = -mActor->getConst().getTable()->mFireDanceSecondJump;
            impact();
            startPadVib(0UL);
            playSound("炎ダメージ復帰バウンド");
            if (_29 == 1) {
                playSound("声針ダメージ中");
            } else {
                playSound("声炎ダメージ中");
            }

            changeAnimation("ファイアダンス");
        } else {
            fn_803E5FA0(this);
        }
    }

    getPlayer()->mJumpVec = getAirGravityVec() * _20;
    addVelocity(getAirGravityVec() * _20);
    if (_20 < 0.0f) {
        _20 += mActor->getConst().getTable()->mFireDanceGravityRise;
    } else {
        _20 += mActor->getConst().getTable()->mFireDanceGravityDrop;
    }

    if (_20 > 50.0f) {
        _20 = 50.0f;
    }

    if (getStickP() != 0.0f) {
        if (_26) {
            _26--;
        } else {
            const TVec3f& padDir = getWorldPadDir();
            getPlayer()->setFrontVecKeepUp(padDir, mActor->getConst().getTable()->mFireDanceTurnRatio);
            _14 += getFrontVec() * mActor->getConst().getTable()->mFireDanceMoveAcc;
        }
    }

    MR::vecKillElement(_14, getAirGravityVec(), &_14);
    if (!_24 && _14.length() > mActor->getConst().getTable()->mFireDanceMoveSpeed) {
        _14.setLength(mActor->getConst().getTable()->mFireDanceMoveSpeed);
    }

    addVelocity(_14);
    return true;
}

bool MarioFireDance::close() {
    stopAnimation("ファイアダンス");
    stopEffect("炎ダメージ煙");
    stopEffect("炎ダメージ青煙");
    return true;
}

MarioDarkDamage::MarioDarkDamage(MarioActor* pActor) : MarioState(pActor, MarioStatus_DarkDamage) {
    _12 = 0;
    _14 = 0;
}

bool MarioDarkDamage::start() {
    _12 = 0;
    _14 = 0;
    return true;
}

bool MarioDarkDamage::update() {
    switch (_14) {
    case 0:
        _14++;
        _12 = 150;
        MR::requestStartGameOverDemo();
        break;
    case 1:
        if (_12) {
            _12--;
        }

        if (!_12) {
            mActor->forceKill(3);
            MarioActor* actor = mActor;
            actor->_481 = 1;
            actor->updateHand();
            actor->updateFace();
        }

        break;
    }

    if (_12) {
        playSound("ダークマター沈み");
    }

    return true;
}

bool MarioDarkDamage::notice() {
    return true;
}

bool MarioAbyssDamage::notice() {
    return true;
}