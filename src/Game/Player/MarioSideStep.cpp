#include "Game/Player/MarioSideStep.hpp"
#include "Game/Util.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioWall.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"

MarioSideStep::MarioSideStep(MarioActor* actor) : MarioState(actor, MarioStatus_SideStep) {
}

bool MarioSideStep::start() {
    if (!isAnimationRun("壁押し")) {
        playSound("声壁押し");
    }
    changeAnimation("壁押し", 0UL);
    mActor->setBlendMtxTimer(10);
    return true;
}

// NONMATCHING: equivalent vector-helper inlining, saved registers, and string-pool materialization.
bool MarioSideStep::update() {
    if (!((getPlayer()->mMovementStates.mLowWord >> 23) & 1U)) {
        return false;
    }
    if (((getPlayer()->mFlags20.mWord >> 26) & 1U)) {
        return false;
    }
    if (((getPlayer()->mDrawStates.mWord >> 21) & 1U)) {
        return false;
    }
    if (MR::isNormalTalking()) {
        return false;
    }
    if (mActor->_468) {
        return false;
    }
    Mario* player = getPlayer();
    bool cannotSideStep = !isSlipFloorCode(player->_960);
    if (cannotSideStep) {
        return false;
    }
    if (getStickP() < 0.1f) {
        changeAnimation("壁ウエイト");
    } else {
        TVec3f direction(getWorldPadDir());
        MR::vecKillElement(direction, getPlayer()->getAirGravityVec(), &direction);
        MR::normalizeOrZero(&direction);
        TVec3f side;
        f32 forward = MR::vecKillElement(direction, getPlayer()->getWallNorm(), &side);
        if (forward < -0.866f) {
            if (!isAnimationRun("壁押し")) {
                playSound("声壁押し");
            }
            changeAnimation("壁押し", 0UL);
            const Triangle* wall = getPlayer()->getWallPolygon();
            if (wall) {
                HitSensor* wallSensor = wall->mSensor;
                MR::sendArbitraryMsg(0xB1, wallSensor, mActor->getSensor("Body"));
            }
        } else if (forward > 0.0f) {
            if (forward > 0.707f) {
                getPlayer()->setFrontVecKeepUp(getPlayer()->getWallNorm());
            }
            getPlayer()->mMovementStates.mLowBits.mBit23 = 0;
            return false;
        } else {
            side *= (1.0f + 0.25f * forward) * getStickP();
            if (side.dot(getPlayer()->mSideVec) < 0.0f) {
                if (!isAnimationRun("壁右歩き")) {
                    changeAnimation("壁右歩き");
                }
            } else if (!isAnimationRun("壁左歩き")) {
                changeAnimation("壁左歩き");
            }
            addVelocity(side * 6.0f - getPlayer()->getWallNorm() * 6.0f);
        }
    }
    if (checkTrgA()) {
        getPlayer()->resetInline();
        getPlayer()->setFrontVecKeepUp(-getPlayer()->getWallNorm());
        getPlayer()->tryJump();
        return false;
    }
    if (mActor->isRequestRush()) {
        if (isPlayerModeFire()) {
            mActor->updateFireSpin();
        } else {
            getPlayer()->tryWallPunch();
        }
        return false;
    }
    if (!((getPlayer()->mMovementStates.mLowWord >> 30) & 1U)) {
        getPlayer()->keepDistFrontWall();
        getPlayer()->tryDrop();
        return false;
    }
    return getPlayer()->fixWallingPosition(false) != false;
}

bool MarioSideStep::close() {
    stopAnimation("壁ウエイト");
    stopAnimation("壁右歩き");
    stopAnimation("壁左歩き");
    stopAnimation("壁押し");
    getPlayer()->startBas(nullptr, false, 0.0f, 0.0f);
    return true;
}
