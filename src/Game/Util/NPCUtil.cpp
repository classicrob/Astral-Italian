#include "Game/Util/NPCUtil.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/ModelObj.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/NPCActor.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NerveUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"

#include "Game/LiveActor/LodCtrl.hpp"
#include "Game/LiveActor/PartsModel.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/NPC/NPCFunction.hpp"
#include "Game/Util/ActorCameraUtil.hpp"
#include "Game/Util/ActorShadowUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/FileUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <cstdio>

namespace {
    static s32 sStarAppearSeStep = 103;
    static s32 sStarAppearSeStepCaretaker = 32;
    static s32 sStarAppearSeStepPenguinCoach = 95;
    static s32 sStarAppearSeStepTeresaRacer = 89;
    static s32 sStarAppearSeStepTrickRabbit = 22;
};  // namespace

namespace NrvTakeOutStar {
    NEW_NERVE(TakeOutStarNrvAnim, TakeOutStar, Anim);
    NEW_NERVE(TakeOutStarNrvDemo, TakeOutStar, Demo);
    NEW_NERVE(TakeOutStarNrvTerm, TakeOutStar, Term);
};  // namespace NrvTakeOutStar

namespace NrvFadeStarter {
    NEW_NERVE(FadeStarterNrvFade, FadeStarter, Fade);
    NEW_NERVE(FadeStarterNrvTerm, FadeStarter, Term);
};  // namespace NrvFadeStarter

namespace NrvDemoStarter {
    NEW_NERVE(DemoStarterNrvInit, DemoStarter, Init);
    NEW_NERVE(DemoStarterNrvFade, DemoStarter, Fade);
    NEW_NERVE(DemoStarterNrvWait, DemoStarter, Wait);
    NEW_NERVE(DemoStarterNrvTerm, DemoStarter, Term);
};  // namespace NrvDemoStarter

namespace MR {
    bool getNPCItemData(NPCActorItem* pItem, s32 type) {
        return NPCFunction::getNPCItemData(pItem, type);
    }




    void decidePose(NPCActor* pActor, const TVec3f& rUp, const TVec3f& rFront, const TVec3f& rPos, f32 upBlend, f32 frontBlend, f32 posBlend) {
        blendVec(&pActor->mPosition, pActor->mPosition, rPos, posBlend);
        if (upBlend == 1.0f && frontBlend == 1.0f) {
            makeQuatUpFront(&pActor->_A0, rUp, rFront);
        } else {
            blendQuatUpFront(&pActor->_A0, rUp, rFront, upBlend, frontBlend);
        }
    }




    void followRailPose(NPCActor* pActor, f32 directionBlend, f32 posBlend) {
        decidePose(pActor, -pActor->mGravity, getRailDirection(pActor), getRailPos(pActor), 1.0f, directionBlend, posBlend);
    }



    void setDefaultPose(NPCActor* pActor) {
        pActor->setToDefault();
    }




    void startNPCTalkCamera(const TalkMessageCtrl* pTalk, MtxPtr pActorMtx, f32 distanceScale, s32 frames) {
        startNPCTalkCamera(pTalk, pActorMtx, getPlayerBaseMtx(), distanceScale, frames);
    }

    void startNPCTalkCamera(const TalkMessageCtrl* pTalk, MtxPtr pActorMtx, MtxPtr pPlayerMtx, f32 distanceScale, s32 frames) {
        TVec3f actorPos;
        TVec3f up;
        TVec3f side;
        TVec3f playerPos;
        TVec3f offset(getMessageBalloonFollowOffset(pTalk));
        if (getMessageBalloonFollowMatrix(pTalk)) {
            pActorMtx = getMessageBalloonFollowMatrix(pTalk);
        }

        up.set< f32 >(pPlayerMtx[0][1], pPlayerMtx[1][1], pPlayerMtx[2][1]);
        actorPos.set< f32 >(pActorMtx[0][3], pActorMtx[1][3], pActorMtx[2][3]);
        playerPos.set< f32 >(pPlayerMtx[0][3], pPlayerMtx[1][3], pPlayerMtx[2][3]);
        if (normalizeOrZero(&up)) {
            up.set< f32 >(0.0f, 1.0f, 0.0f);
        }

        f32 distance = playerPos.distance(actorPos);
        f32 angle = MR::atan2(1.0f, cosDegree(67.5f));
        f32 height = vecKillElement(actorPos - playerPos, up, &side);
        f32 scale = (height + offset.y) / distance / 0.75f;
        scale = scale >= 1.0f ? scale : 1.0f;
        distance *= angle;
        distance *= scale;
        f32 cameraDistance = 2.0f * distance;
        f32 cameraHeight = height / 6.0f + offset.y;
        cameraDistance *= distanceScale;
        cameraDistance = cameraDistance >= 450.0f ? cameraDistance : 450.0f;
        startTalkCamera(actorPos, up, cameraHeight, cameraDistance, frames);
    }



    PartsModel* createNPCGoods(LiveActor* pActor, const char* pModelName, const char* pJointName) {
        PartsModel* pGoods = nullptr;
        if (!isNullOrEmptyString(pModelName) && isNPCItemFileExist(pModelName) && isExistJoint(pActor, pJointName)) {
            pGoods = createPartsModelNpcAndFix(pActor, "グッズ", pModelName, pJointName);
            pGoods->appear();
            if (getLightNumMax(pGoods) > 0) {
                initLightCtrl(pGoods);
            }
        }

        return pGoods;
    }

    PartsModel* createIndirectNPCGoods(LiveActor* pActor, const char* pModelName, const char* pJointName) {
        PartsModel* pGoods = nullptr;
        if (!isNullOrEmptyString(pModelName) && isNPCItemFileExist(pModelName) && isExistJoint(pActor, pJointName)) {
            pGoods = createPartsModelIndirectNpc(pActor, "グッズ", pModelName, getJointMtx(pActor, pJointName));
            pGoods->appear();
            if (getLightNumMax(pGoods) > 0) {
                initLightCtrl(pGoods);
            }
        }

        return pGoods;
    }



    bool isActionContinuous(const LiveActor* pActor) {
        return getBckCtrl(pActor)->getAttribute() == 0 && !isBckStopped(pActor);
    }

    bool isActionLoopedOrStopped(const LiveActor* pActor) {
        if (getBckCtrl(pActor)->getAttribute() == 0) {
            return isBckStopped(pActor);
        }

        return isBckLooped(pActor);
    }


    void startMoveAction(NPCActor* pActor) {
        if (isExistRail(pActor)) {
            adjustmentRailCoordSpeed(pActor, pActor->_10C, pActor->_110);
            moveRailRider(pActor);
            if (pActor->_124) {
                followRailPoseOnGround(pActor, pActor->_114);
            } else {
                followRailPose(pActor, pActor->_114, pActor->_114);
            }

            if (isRailReachedGoal(pActor)) {
                reverseRailDirection(pActor);
            }
        }
    }

    bool tryStartTalkAction(NPCActor* pActor) {
        const char* pAction;
        if (isTalkTalking(pActor->mMsgCtrl)) {
            if (pActor->mParam._1 && !pActor->turnToPlayer(pActor->mParam._8, pActor->mParam._C, pActor->mParam._10)) {
                pAction = pActor->mParam._20;
            } else {
                pAction = pActor->mParam._1C;
            }
        } else {
            return tryStartTurnAction(pActor);
        }

        if (!isNullOrEmptyString(pAction)) {
            return tryStartAction(pActor, pAction);
        }

        return false;
    }

    bool tryStartMoveTalkAction(NPCActor* pActor) {
        TalkMessageCtrl* pTalk = pActor->mMsgCtrl;
        if (!isExistRail(pActor)) {
            return tryStartTalkAction(pActor);
        }

        bool movingTalk = false;
        const char* pAction;
        if (isTalkTalking(pTalk) && !isShortTalk(pTalk)) {
            if (pActor->mParam._1 && !pActor->turnToPlayer(pActor->mParam._8, pActor->mParam._C, pActor->mParam._10)) {
                pAction = pActor->mParam._20;
            } else {
                pAction = pActor->mParam._1C;
            }
        } else {
            if (isNearZero(pActor->_10C) && isNearZero(getRailCoordSpeed(pActor))) {
                return tryStartTalkAction(pActor);
            }

            startMoveAction(pActor);
            if (isTalkTalking(pTalk)) {
                pAction = pActor->_120;
                movingTalk = true;
            } else {
                pAction = pActor->_11C;
            }
        }

        if (!isNullOrEmptyString(pAction)) {
            bool started = tryStartAction(pActor, pAction);
            if (movingTalk) {
                setBckRate(pActor, pActor->_118);
            } else {
                setBckRate(pActor, 1.0f);
            }

            return started;
        }

        return false;
    }

    bool tryStartTurnAction(NPCActor* pActor) {
        const char* pAction;
        if (isNearPlayer(pActor, pActor->mParam._4)) {
            if (pActor->mParam._0) {
                if (pActor->turnToPlayer(pActor->mParam._8, pActor->mParam._C, pActor->mParam._10)) {
                    pAction = pActor->mParam._14;
                } else {
                    pAction = pActor->mParam._18;
                }
            } else {
                pAction = pActor->mParam._14;
            }
        } else {
            if (pActor->mParam._0 || pActor->mParam._1) {
                if (pActor->turnToDefault(pActor->mParam._8)) {
                    pAction = pActor->mParam._14;
                } else {
                    pAction = pActor->mParam._18;
                }
            } else {
                pAction = pActor->mParam._14;
            }
        }

        if (isNullOrEmptyString(pAction)) {
            return false;
        }

        return tryStartAction(pActor, pAction);
    }

    bool tryStartMoveTurnAction(NPCActor* pActor) {
        if (!isExistRail(pActor)) {
            return tryStartTurnAction(pActor);
        }

        startMoveAction(pActor);
        if (isNullOrEmptyString(pActor->_11C)) {
            return false;
        }

        return tryStartAction(pActor, pActor->_11C);
    }
};  // namespace MR

namespace MR {
    bool tryStartReaction(NPCActor* pActor) {
        const char* pAction = nullptr;
        bool started = false;
        if (pActor->_128) {
            bool trampled = pActor->isTrampledStart();
            if (trampled) {
                pAction = pActor->_134;
            } else {
                bool reaction = pActor->isReactionStart();
                if (reaction) {
                    pAction = pActor->_13C;
                } else {
                    bool spin = pActor->isSpinAttackedStart();
                    if (spin) {
                        pAction = pActor->_130;
                    } else if (pActor->_E4) {
                        pAction = pActor->_138;
                    }
                }
            }
        }

        if (!isNullOrEmptyString(pAction)) {
            if (pActor->isTrampledStart() || pActor->isReactionStart()) {
                stopBck(pActor);
                startAction(pActor, pAction);
                started = true;
            } else {
                bool spin = pActor->isSpinAttackedStart();
                if (spin) {
                    started = tryStartAction(pActor, pAction);
                } else if (pActor->_E4) {
                    if (!(pActor->_134 && isActionStart(pActor, pActor->_134)) && !(pActor->_13C && isActionStart(pActor, pActor->_13C)) &&
                        !(pActor->_130 && isActionStart(pActor, pActor->_130))) {
                        bool pointing = pActor->isPointingStart();
                        if (pointing) {
                            started = tryStartAction(pActor, pAction);
                        } else if (isActionLoopedOrStopped(pActor)) {
                            startAction(pActor, pAction);
                        }
                    }
                }
            }
        } else {
            bool hasScale = pActor->mScaleController && pActor->mDelegator;
            if (hasScale) {
                if (pActor->isPointingStart() || pActor->isSpinAttackedStart() || pActor->isTrampledStart() || pActor->isReactionStart()) {
                    started = true;
                }
            }
        }

        return started;
    }

    bool tryTalkNearPlayerAndStartTalkAction(NPCActor* pActor) {
        tryStartTalkAction(pActor);
        return tryTalkNearPlayer(pActor->mMsgCtrl);
    }

    bool tryTalkNearPlayerAndStartMoveTalkAction(NPCActor* pActor) {
        tryStartMoveTalkAction(pActor);
        return tryTalkNearPlayer(pActor->mMsgCtrl);
    }

    bool tryTalkNearPlayerAtEndAndStartTalkAction(NPCActor* pActor) {
        tryStartTalkAction(pActor);
        return tryTalkNearPlayerAtEnd(pActor->mMsgCtrl);
    }

    bool tryTalkNearPlayerAtEndAndStartMoveTalkAction(NPCActor* pActor) {
        tryStartMoveTalkAction(pActor);
        return tryTalkNearPlayerAtEnd(pActor->mMsgCtrl);
    }



    bool tryStartReactionAndPushNerve(NPCActor* pActor, const Nerve* pNerve) {
        if (tryStartReaction(pActor)) {
            pActor->pushNerve(pNerve);
            return true;
        }

        return false;
    }

    bool tryStartReactionAndPopNerve(NPCActor* pActor) {
        if (tryStartReaction(pActor)) {
            pActor->pushNerve(pActor->popNerve());
            return false;
        }

        if (pActor->isScaleAnim()) {
            return false;
        }

        if (isActionLoopedOrStopped(pActor)) {
            pActor->popNerve();
            return true;
        }

        return false;
    }



};  // namespace MR

TakeOutStar::TakeOutStar(NPCActor* pActor, const char* pActionName, const char* pAnimName, const Nerve* pNerve)
    : NerveExecutor("パワースター取り出しデモ実行者"), mActor(pActor), mNerve(pNerve), mActionName(pActionName), mAnimName(pAnimName) {
    mStarModel = MR::createPowerStarDemoModel(mActor, "パワースターデモモデル", pActor->getBaseMtx());
    mStarModel->makeActorDead();

    initNerve(GET_NERVE(TakeOutStar, TakeOutStarNrvAnim));
}

bool TakeOutStar::takeOut() {
    if (isNerve(GET_NERVE(TakeOutStar, TakeOutStarNrvTerm))) {
        return true;
    }

    updateNerve();

    return false;
}



void TakeOutStar::exeAnim() {
    if (MR::isFirstStep(this)) {
        if (mNerve != nullptr) {
            mActor->pushNerve(mNerve);
        } else {
            mActor->tryPushNullNerve();
        }

        mStarModel->appear();
        MR::invalidateClipping(mStarModel);
        MR::requestMovementOn(mStarModel);
        MR::startAction(mStarModel, mAnimName);
        MR::startAction(mActor, mActionName);
    }

    s32 step = ::sStarAppearSeStep;

    if (MR::isEqualString(mAnimName, "TakeOutStarCaretaker")) {
        step = ::sStarAppearSeStepCaretaker;
    } else if (MR::isEqualString(mAnimName, "TakeOutStarTeresaRacer")) {
        step = ::sStarAppearSeStepTeresaRacer;
    } else if (MR::isEqualString(mAnimName, "TakeOutStarPenguinCoach")) {
        step = ::sStarAppearSeStepPenguinCoach;
    } else if (MR::isEqualString(mAnimName, "TakeOutStarTrickRabbit")) {
        step = ::sStarAppearSeStepTrickRabbit;
    }

    if (MR::isGreaterStep(this, step)) {
        if (MR::isInWater(mStarModel, TVec3f(0.0f, 0.0f, 0.0f))) {
            MR::startActionSound(mStarModel, "OjLvPowStarExistW");
        } else {
            MR::startActionSound(mStarModel, "OjLvPowStarExist");
        }
    }

    if (MR::isAnyAnimOneTimeAndStopped(mActor, mActionName)) {
        setNerve(GET_NERVE(TakeOutStar, TakeOutStarNrvDemo));
    }
}

void TakeOutStar::exeDemo() {
    if (MR::isFirstStep(this)) {
        TVec3f trans;
        MR::extractMtxTrans(MR::getJointMtx(mStarModel, "PowerStar"), &trans);
        MR::appearPowerStarContinueCurrentDemo(mActor, trans);
        mStarModel->kill();
    }

    if (MR::isEndPowerStarAppearDemo(mActor)) {
        MR::validateClipping(mStarModel);
        mActor->popNerve();
        setNerve(GET_NERVE(TakeOutStar, TakeOutStarNrvTerm));
    }
}


FadeStarter::FadeStarter(NPCActor* pActor, s32 a2) : NerveExecutor("フェード開始制御"), mActor(pActor), _C(), _10(a2) {
    initNerve(GET_NERVE(FadeStarter, FadeStarterNrvFade));
}


void FadeStarter::exeFade() {
    if (MR::isFirstStep(this)) {
        if (!mActor->isEmptyNerve()) {
            _C = mActor->popNerve();
        }

        mActor->tryPushNullNerve();
        MR::closeWipeFade(_10);
    }

    if (MR::isWipeActive()) {
        return;
    }

    mActor->popNerve();

    if (_C != nullptr) {
        mActor->pushNerve(_C);
        _C = nullptr;
    }

    MR::openWipeFade(_10);
    setNerve(GET_NERVE(FadeStarter, FadeStarterNrvTerm));
}


DemoStarter::DemoStarter(NPCActor* pActor) : NerveExecutor("デモ開始制御"), mActor(pActor) {
    initNerve(GET_NERVE(DemoStarter, DemoStarterNrvInit));
}




void DemoStarter::exeFade() {
    if (MR::isFirstStep(this)) {
        MR::invalidateClipping(mActor);
        MR::offPlayerControl();
        MR::closeWipeFade();
    }

    if (MR::isWipeActive()) {
        return;
    }

    setNerve(GET_NERVE(DemoStarter, DemoStarterNrvWait));
}


