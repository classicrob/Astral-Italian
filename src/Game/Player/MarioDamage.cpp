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
#include "Game/Player/Yoshi.hpp"



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
            MR::startActionSound(mActor->fn_803B97F0(), "Thrown", -1, -1, -1);
        } else {
            playSound("声投げられ");
        }
        _11 = 0;
        mActor->fn_803B9560();
    } else {
        playSound("ダメージ");
        if (isPlayerModeYoshi()) {
            MR::startActionSound(mActor->fn_803B97F0(), "Damage", -1, -1, -1);
        } else {
            playSound("声小ダメージ");
        }
        mActor->decLifeMiddle();
        if (_30) {
            mActor->fn_803BE8D0(_34);
        } else {
            this->fn_803E4E00();
        }
        mActor->fn_803B94E0();
    }

    _44 = 0;
    _30 = false;
    return true;
}

void MarioDamage::setVec(const TVec3f& rDirection) {
    MR::vecKillElement(rDirection, mActor->mAppliedGravityVec, &_1C);
    _1C.setLength(mActor->getConst().getTable()->mJumpDistDamage);
}

void MarioDamage::setVecSize(f32 horizontal, f32 vertical) {
    _1C.setLength(horizontal);
    _1C += -mActor->mAppliedGravityVec * vertical;
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


bool MarioDamage::fn_803E48F0() {
    this->addVelocity(this->_1C);
    this->_1C += this->mActor->_240 * this->mActor->getConst().getTable()->mGravityDamage;
    if (this->_14 > 20) {
        if (this->_28) {
            this->changeAnimation(this->_28);
        }

        if (this->getPlayer()->_1C._0) {
            f32 vertical = MR::vecKillElement(this->_1C, this->getAirGravityVec(), &this->_1C);
            this->_1C.x *= 0.95f;
            this->_1C.y *= 0.95f;
            this->_1C.z *= 0.95f;
            this->_1C += this->getAirGravityVec() * vertical;
        }
    }

    if (this->getPlayer()->mMovementStates._1) {
        if (this->getPlayer()->mVerticalSpeed > 30.0f &&
            (*(MarioActor**)((u8*)this->mActor + 0x1080))->fn_803DB380(MR::getSensorHost(this->getSensor(this->getGroundPolygon()))->mName, "ダメージ中跳ねる")) {
            Mario* player = this->getPlayer();
            this->_1C += player->_368 * 20.0f;
            this->getPlayer()->mMovementStates._1 = false;
            this->getPlayer()->mMovementStates.jumping = true;
            return true;
        }

        this->getPlayer()->mMovementStates.jumping = false;
        this->playSound("吹っ飛び倒れ");
        this->playEffect("共通ダメージ着地");
        MR::vecKillElement(this->_1C, this->mActor->_240, &this->_1C);
        this->_18 = 1;
        this->_14 = 0;
        if (!this->mActor->mHealth) {
            this->_18 = 2;
            this->changeAnimation(this->_2C);
        } else if (MR::isGalaxyHorrorCometAppearInCurrentStage() && !this->isPlayerModeYoshi()) {
            this->_18 = 3;
        } else {
            this->changeAnimation(this->_2C);
        }
    } else if (!this->mActor->mHealth) {
        if (this->_14 > 240) {
            this->mActor->forceGameOverAbyss();
        }
    } else if (this->_44 > 360) {
        return false;
    }

    return true;
}

bool MarioDamage::fn_803E4BF0() {
    if (!this->getPlayer()->mMovementStates._1) {
        this->getPlayer()->mMovementStates.jumping = true;
        this->_18 = 0;
        return true;
    }

    MR::vecKillElement(this->_1C, this->getAirGravityVec(), &this->_1C);
    this->addVelocity(this->_1C);
    this->_1C.x *= 0.95f;
    this->_1C.y *= 0.95f;
    this->_1C.z *= 0.95f;
    if (!this->isAnimationRun(this->_2C)) {
        return false;
    }

    if (this->_14 > 15 && this->checkTrgA()) {
        this->getPlayer()->tryJump();
        return false;
    }

    return true;
    return true;
}

bool MarioDamage::fn_803E4D00() {
    if (!this->getPlayer()->mMovementStates._1) {
        this->getPlayer()->mMovementStates.jumping = true;
        this->_18 = 0;
        return true;
    }

    if (this->_14 == 40) {
        if (!this->mActor->mHealth) {
            this->mActor->forceGameOver();
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

void MarioDamage::fn_803E4E00() {
    TVec3f velocity(this->_1C);
    const TVec3f& gravity = this->getAirGravityVec();
    TVec3f vertical;
    JMAVECScaleAdd(&gravity, &velocity, &vertical, -gravity.dot(velocity));
    velocity.sub(vertical);
    TVec3f horizontal(vertical);
    horizontal.scale(0.5f);
    velocity.add(horizontal);
    this->mActor->fn_803BE840(velocity);
}

bool MarioDamage::update() {
    _14++;
    _44++;
    if (mActor->_EA4) {
        return true;
    }

    switch (_18) {
    case 0:
        if (!this->fn_803E48F0()) {
            return false;
        }
        break;
    case 1:
        if (!this->fn_803E4BF0()) {
            return false;
        }
        break;
    case 2:
        if (!this->fn_803E4D00()) {
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
            MR::startActionSound(mActor->fn_803B97F0(), "DamageFire", -1, -1, -1);
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
            MR::startActionSound(mActor->fn_803B97F0(), "DamageFire", -1, -1, -1);
        } else {
            playSound("声針ダメージ");
            playSound("針ダメージ");
        }
        break;
    }
}

void MarioFireDance::fn_803E5FA0() {
    if (!this->_29) {
        this->mActor->decLifeLarge();
    } else {
        this->mActor->decLifeMiddle();
    }

    if (!this->mActor->mHealth) {
        this->mActor->forceGameOverNonStop();
    }

    this->_20 = -this->mActor->getConst().getTable()->mFireDanceFirstJump;
    this->impact();
    this->impactEffect();
    this->startPadVib(3);
    this->_2A = 0;
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
            this->fn_803E5FA0();
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