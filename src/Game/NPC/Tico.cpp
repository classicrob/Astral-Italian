#include "Game/NPC/Tico.hpp"
#include <cstdio>
#include "Game/Util.hpp"
#include "Game/System/GameSequenceFunction.hpp"
#include "Game/Demo/AstroDemoFunction.hpp"
#include "Game/Demo/DemoFunction.hpp"
#include "Game/LiveActor/HitSensor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/NPC/TalkMessageFunc.hpp"
#include "Game/NPC/TicoDemoGetPower.hpp"
#include "Game/NameObj/NameObjArchiveListCollector.hpp"
#include "Game/Util/ActorMovementUtil.hpp"
#include "Game/Util/ActorSensorUtil.hpp"
#include "Game/Util/ActorSwitchUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/EventUtil.hpp"
#include "Game/Util/JMapUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/LightUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MtxUtil.hpp"
#include "Game/Util/NPCUtil.hpp"
#include "Game/Util/RailUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/TalkUtil.hpp"

namespace NrvTico {
    NEW_NERVE(TicoNrvNoReaction, Tico, NoReaction);
    NEW_NERVE(TicoNrvReaction, Tico, Reaction);
    NEW_NERVE(TicoNrvDelight, Tico, Delight);
    NEW_NERVE(TicoNrvReactionTalk, Tico, ReactionTalk);
    NEW_NERVE(TicoNrvAppearRail, Tico, AppearRail);
    NEW_NERVE(TicoNrvAppear, Tico, Appear);
    NEW_NERVE(TicoNrvTogglePlayerControl, Tico, TogglePlayerControl);
    NEW_NERVE(TicoNrvWait, Tico, Wait);
    NEW_NERVE(TicoNrvTalk, Tico, Talk);
    NEW_NERVE(TicoNrvMeta, Tico, Meta);
    NEW_NERVE(TicoNrvWaitSwitchABeforeRail, Tico, WaitSwitchABeforeRail);
    NEW_NERVE(TicoNrvMoveRail, Tico, MoveRail);
    NEW_NERVE(TicoNrvWaitAfterRail, Tico, WaitAfterRail);
    NEW_NERVE(TicoNrvWaitBeforeRail, Tico, WaitBeforeRail);
    NEW_NERVE(TicoNrvMoveRailForceTalk, Tico, MoveRailForceTalk);
    NEW_NERVE(TicoNrvFall, Tico, Fall);
    NEW_NERVE(TicoNrvFallRecover, Tico, FallRecover);

    class TicoNrvDemoDummy : public Nerve {
    public:
        virtual void execute(Spine* pSpine) const {}
        static TicoNrvDemoDummy sInstance;
    };
    TicoNrvDemoDummy TicoNrvDemoDummy::sInstance ATTRIBUTE_WEAK;

    NEW_NERVE(TicoNrvDemoWait, Tico, DemoWait);
};  // namespace NrvTico

namespace {
    static Color8 hPointLight[] = {Color8(0xFF, 0xFF, 0, 0xFF), Color8(0, 0x64, 0xFF, 0xFF), Color8(0, 0xFF, 0x32, 0xFF),
                                   Color8(0xFF, 0, 0, 0xFF),    Color8(0x78, 0, 0xFF, 0xFF), Color8(0xFF, 0x64, 0x64, 0xFF), Color8(0xFF, 0x80, 0, 0xFF)};
};  // namespace

extern "C" bool fn_800613B0(TalkMessageCtrl*);
extern "C" void fn_800209B0(LiveActor*, const JMapInfoIter&, const char*, const char*);

namespace MR {
    bool branchFuncGameProgress(u32);
    bool isStageMarioFaceShip();
}  // namespace MR

const f32 Tico::sFloatSeMinVolume = 0.2f;

Tico::Tico(const char* pName)
    : NPCActor(pName), _15C(), _160(0.0f, 0.0f, 0.0f), _16C(), mDemoGetPower(), _174(), _178Ctrl(), _178(), _17C(255, 255, 255, 255), _180(),
      mDemoStarter(this), _19C(false), _19D(false), _19E(false), mCameraInfo(nullptr), mDemoObj(nullptr) {
}

void Tico::initBase(const JMapInfoIter& rIter, s32 color) {
    NPCActorCaps caps("Tico");
    caps.setDefault();
    caps.mMessage = false;
    caps._94 = true;
    caps.mWaitNerve = GET_NERVE(Tico, TicoNrvWait);

    setDefaults2();
    _13C = "Spin";
    _12C = 1000.0f;
    mParam._14 = "Wait";
    mParam._18 = "Wait";
    mParam._1C = "Talk";
    mParam._20 = "Talk";
    _11C = "Fly";
    _120 = "Fly";
    _10C = 8.0f;
    _110 = 0.2f;
    mParam._C = 0.5f;
    mParam._10 = 12.0f;
    initialize(rIter, caps, "Tico", nullptr, nullptr);
    MR::startBrk(this, "ColorChange");
    MR::setBrkFrameAndStop(this, color);
    _17C = ::hPointLight[color];
    _180 = GET_NERVE(Tico, TicoNrvMeta);
    _160.set(mPosition);
    MR::startAction(this, "Wait");
    MR::setBckFrameAtRandom(this);
    _178 = MR::getJointMtx(this, "Body");
}

void Tico::initMessage(const JMapInfoIter& rIter, const char* pMessageId) {
    if (initTalkCtrl(rIter, pMessageId, TVec3f(0.0f, 120.0f, 0.0f), nullptr)) {
        MR::registerKillFunc(getMsgCtrl(), TalkMessageFunc< Tico >(this, &Tico::killFunc));
        MR::registerEventFunc(getMsgCtrl(), TalkMessageFunc< Tico >(this, &Tico::eventFunc));
        MR::registerBranchFunc(getMsgCtrl(), TalkMessageFunc< Tico >(this, &Tico::branchFunc));
        MR::setDistanceToTalk(mMsgCtrl, 350.0f);
    }

    s32 arg0 = 0;
    MR::getJMapInfoArg0NoInit(rIter, &arg0);
    s32 num = arg0;
    if (arg0 >= 4 && arg0 <= 6) {
        num = 0;
    }
    MR::getJMapInfoArg4NoInit(rIter, &num);
    char buf[0x80];
    snprintf(buf, 0x80, "Tico_StarPiece%d", num);
    _174 = MR::createTalkCtrlDirect(this, rIter, buf, TVec3f(0.0f, 120.0f, 0.0f), nullptr);
    if (_174) {
        MR::offRootNodeAutomatic(_174);
    }
}

void Tico::initMessage(const char* pMessageId) {
    JMapInfoIter iter;

    if (initTalkCtrlDirect(iter, pMessageId, TVec3f(0.0f, 120.0f, 0.0f), nullptr)) {
        MR::registerKillFunc(getMsgCtrl(), TalkMessageFunc< Tico >(this, &Tico::killFunc));
        MR::setDistanceToTalk(mMsgCtrl, 350.0f);
    }

    _174 = MR::createTalkCtrlDirect(this, iter, "Tico_StarPiece0", TVec3f(0.0f, 120.0f, 0.0f), nullptr);
    if (_174) {
        MR::offRootNodeAutomatic(_174);
    }
}

void Tico::initRail(const JMapInfoIter& rIter) {
    MR::initUseStageSwitchReadA(this, rIter);
    if (MR::isExistRail(this) && MR::isValidSwitchDead(this)) {
        s32 speed;
        if (MR::getRailArg0NoInit(this, &speed)) {
            _1A0 = speed;
        } else {
            _1A0 = 8.0f;
        }

        _10C = _1A0;
        mParam._C = 0.0f;
        mParam._10 = 0.0f;
        MR::onCalcGravity(this);
        MR::calcGravity(this);
        MR::followRailPose(this, 0.99f, 1.0f);
        MR::invalidateClipping(this);

        if (MR::isValidSwitchAppear(this)) {
            if (_15C == 6) {
                setNerve(GET_NERVE(Tico, TicoNrvMoveRail));
            } else {
                setNerve(GET_NERVE(Tico, TicoNrvMoveRailForceTalk));
            }
            pushNerve(GET_NERVE(Tico, TicoNrvAppearRail));
        } else if (MR::isValidSwitchA(this)) {
            setNerve(GET_NERVE(Tico, TicoNrvWaitSwitchABeforeRail));
        } else {
            setNerve(GET_NERVE(Tico, TicoNrvWaitBeforeRail));
        }
    }
}

void Tico::init(const JMapInfoIter& rIter) {
    s32 color = 0;
    MR::getJMapInfoArg0NoInit(rIter, &color);
    s32 behavior = 0;
    s32 clr = color;
    MR::getJMapInfoArg1NoInit(rIter, &behavior);
    _15C = behavior;
    initBase(rIter, clr);

    bool calcGravity = false;
    MR::getJMapInfoArg5NoInit(rIter, &calcGravity);
    if (calcGravity) {
        _19E = true;
        MR::onCalcGravity(this);
        MR::onCalcShadowAll(this);
    }

    bool noMessage = false;
    MR::getJMapInfoArg2NoInit(rIter, &noMessage);
    if (!noMessage && _15C != 2) {
        initMessage(rIter, "Tico");
    }

    MR::joinToGroupArray(this, rIter, mName, 32);

    if (_15C == 2) {
        const char* castName = "スピンドライバ初出";
        if (MR::getJMapInfoDemoCastID(rIter)) {
            MR::initTalkAnimDemoCast(this, rIter, castName, "DemoRideSpinDriverBlue");
        } else {
            fn_800209B0(this, rIter, castName, "DemoRideSpinDriver");
        }
        mDemoObj = MR::createDemoObjWithAnimArc("デモ用ベビチコ", "TicoBaby", "TicoBabyAnim", nullptr);
        MR::initTalkAnimDemoCast(this, rIter, castName, "DemoRideSpinDriver");
        reinterpret_cast< LiveActor* >(mDemoObj)->makeActorDead();
    } else {
        MR::tryRegisterDemoCast(this, rIter);
    }

    if (MR::isDemoCast(this, "マリオ顔惑星変化")) {
        setNerve(GET_NERVE(Tico, TicoNrvDemoWait));
    }

    if (MR::isStageMarioFaceShip()) {
        MR::registerDemoSimpleCastAll(this);
    }

    switch (_15C) {
    case 1:
        setNerve(GET_NERVE(Tico, TicoNrvFall));
        break;
    case 3:
        setNerve(GET_NERVE(Tico, TicoNrvTalk));
        break;
    case 2:
        setNerve(GET_NERVE(Tico, TicoNrvDemoDummy));
        makeActorDead();
        break;
    case 6:
    case 8:
        initRail(rIter);
        break;
    case 7:
        if (MR::isValidSwitchAppear(this)) {
            pushNerve(GET_NERVE(Tico, TicoNrvAppear));
        }
        break;
    case 9:
        if (GameSequenceFunction::hasRetryGalaxySequence()) {
            setNerve(GET_NERVE(Tico, TicoNrvTogglePlayerControl));
        } else {
            const JMapIdInfo& initInfo = MR::getInitializeStartIdInfo();
            const JMapIdInfo& curInfo = MR::getCurrentMarioStartIdInfo();
            if (!(curInfo == initInfo)) {
                MR::readMessage(mMsgCtrl);
            }
        }
        break;
    default:
        break;
    }
}

bool Tico::isReactionNerve() const {
    bool ret = false;
    if (!isNerve(GET_NERVE(Tico, TicoNrvReaction)) && !isNerve(GET_NERVE(Tico, TicoNrvDelight))) {
        ret = true;
    }
    return ret;
}

void Tico::initAfterPlacement() {
    if (_15C == 1) {
        TVec3f pos(mPosition);
        TVec3f dir(mGravity);
        dir.scale(500.0f);
        MR::getFirstPolyOnLineToMap(&pos, nullptr, mPosition, dir);
        MR::resetPosition(this, pos);
    }

    NPCActor::initAfterPlacement();
}

void Tico::calcAndSetBaseMtx() {
    MR::calcAndSetFloatBaseMtx(this, _16C);
}

void Tico::control() {
    _19C = false;

    if (_178) {
        TVec3f trans;
        MR::extractMtxTrans(_178, &trans);
        MR::requestPointLight(this, trans, _17C, 0.998646f, -1);
    }

    if (isNerve(_180)) {
        NPCActor::control();
    } else {
        if (!MR::isTimeKeepDemoActive()) {
            MR::startActionSound(this, "SmLvTicoWait");
        } else {
            MR::startActionSound(this, "WaitVolDown");
        }

        _160.set(mPosition);
        _16C = MR::calcFloatOffset(this, _16C, 150.0f);

        if (_19E) {
            f32 qx = _A0.x;
            f32 qy = _A0.y;
            f32 qz = _A0.z;
            f32 qw = _A0.w;
            TVec3f front;
            front.set< f32 >(2.0f * (qx * qz) + 2.0f * (qw * qy), 2.0f * (qy * qz) - 2.0f * (qw * qx), 1.0f - 2.0f * (qx * qx) - 2.0f * (qy * qy));
            TVec3f up;
            up.negate(mGravity);
            MR::blendQuatUpFront(&_B0, up, front, 1.0f, 0.0f);
        }

        NPCActor::control();
    }
}

bool Tico::receiveMsgPlayerAttack(u32 msg, HitSensor* pSender, HitSensor* pReceiver) {
    if (MR::isMsgStarPieceReflect(msg)) {
        return false;
    }

    if (MR::isMsgStarPieceAttack(msg)) {
        MR::limitedStarPieceHitSound();
        _19C = true;

        return true;
    }

    return NPCActor::receiveMsgPlayerAttack(msg, pSender, pReceiver);
}

bool Tico::tryReaction() {
    if (_19C && _174) {
        pushNerve(GET_NERVE(Tico, TicoNrvDelight));

        return true;
    }

    if (_19D && _178Ctrl && isSpinAttackedStart()) {
        pushNerve(GET_NERVE(Tico, TicoNrvReactionTalk));

        return true;
    }

    return MR::tryStartReactionAndPushNerve(this, GET_NERVE(Tico, TicoNrvReaction));
}

bool Tico::killFunc(u32) {
    setNerve(_180);

    return true;
}

bool Tico::eventFunc(u32) {
    _19D = true;

    return true;
}

bool Tico::branchFunc(u32 id) {
    return MR::branchFuncGameProgress(id);
}

void Tico::exeReaction() {
    if (_D8) {
        MR::startActionSound(this, "SmNpcTrampled");
        MR::startActionSound(this, "SvTicoTrampled");
    }

    if (isPointingSe()) {
        MR::startDPDHitSound();
        MR::startActionSound(this, "SvTicoPoint");
    }

    if (_DB) {
        MR::limitedStarPieceHitSound();
        MR::startActionSound(this, "SmButlerAbsorb");
    }

    if (isRailType()) {
        if (mCurNerve == GET_NERVE(Tico, TicoNrvMoveRail) || mCurNerve == GET_NERVE(Tico, TicoNrvMoveRailForceTalk)) {
            bool reachedEnd = false;
            if (MR::isRailReachedEdge(this) && !MR::isRailGoingToEnd(this)) {
                reachedEnd = true;
            }

            if (!reachedEnd) {
                MR::startMoveAction(this);
            }
        }
    }

    MR::tryStartReactionAndPopNerve(this);
}

void Tico::exeDelight() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, _13C);
        MR::resetAndForwardNode(_174, MR::getRandom(0l, 5l));
    }

    MR::tryTalkForce(_174);

    if (MR::isActionEnd(this)) {
        popNerve();
    }
}

void Tico::exeReactionTalk() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, _13C);
    }

    MR::tryTalkForce(_178Ctrl);

    if (MR::isActionEnd(this)) {
        MR::forwardNode(_178Ctrl);
        popNerve();
    }
}

void Tico::exeAppearRail() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");

        if (_15C == 6) {
            s32 arg1;
            if (!MR::getRailArg1NoInit(this, &arg1)) {
                MR::moveCoordToNearestPos(this, *MR::getPlayerPos());
                MR::followRailPose(this, 0.99f, 1.0f);
            }
        }
    }

    if (MR::isActionEnd(this)) {
        popNerve();
    }
}

void Tico::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Appear");
    }

    if (MR::isActionEnd(this)) {
        popNerve();
    }
}

void Tico::exeNoReaction() {
    if (mMsgCtrl != nullptr) {
        if (MR::tryTalkNearPlayerAndStartTalkAction(this)) {
            setNerve(GET_NERVE(Tico, TicoNrvWait));
        }
    } else {
        MR::tryStartTurnAction(this);
    }
}

void Tico::exeTogglePlayerControl() {
    if (MR::isStep(this, 3)) {
        MR::offPlayerControl();
    }

    bool isReady = false;
    if (MR::isOnGroundPlayer() && !MR::isStageStateScenarioOpeningCamera() && !MR::isExecScenarioStarter()) {
        isReady = true;
    }

    if (isReady) {
        bool isFar = !MR::isNearPlayer(this, 500.0f);
        if (isFar) {
            MR::onPlayerControl(true);
        }

        setNerve(GET_NERVE(Tico, TicoNrvWait));
    }
}

void Tico::exeWait() {
    if (tryReaction()) {
        return;
    }

    if (mMsgCtrl != nullptr) {
        MR::tryTalkNearPlayerAndStartTalkAction(this);
    } else {
        MR::tryStartTurnAction(this);
    }
}

void Tico::exeTalk() {
    if (tryReaction()) {
        return;
    }

    if (MR::tryTalkNearPlayerAtEndAndStartTalkAction(this)) {
        setNerve(GET_NERVE(Tico, TicoNrvMeta));
    }
}

void Tico::exeMeta() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Metamorphosis");
    }

    if (MR::isActionEnd(this)) {
        if (mCameraInfo) {
            MR::endActorCamera(this, mCameraInfo, false, -1);
        }

        kill();
    }
}

void Tico::exeFall() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "Fall");
    }

    if (MR::isActionEnd(this)) {
        setNerve(GET_NERVE(Tico, TicoNrvFallRecover));
    }
}

void Tico::exeFallRecover() {
    if (MR::isFirstStep(this)) {
        MR::startAction(this, "FallRecover");
        s32 frameMax = MR::getBckFrameMax(this);
        TVec3f diff(_C0);
        diff.sub(mPosition);
        TVec3f vel(diff);
        vel.scale(1.0f / frameMax);
        mVelocity.set(vel);
    }

    MR::tryTalkForce(mMsgCtrl);

    if (MR::isActionEnd(this)) {
        mVelocity.x = 0.0f;
        mVelocity.y = 0.0f;
        mVelocity.z = 0.0f;
        MR::setDefaultPose(this);
        setNerve(GET_NERVE(Tico, TicoNrvWait));
    }
}

void Tico::exeDemoWait() {
    turnToPlayer(mParam._8, mParam._C, mParam._10);
}

void Tico::exeWaitSwitchABeforeRail() {
    if (MR::isOnSwitchA(this)) {
        MR::invalidateClipping(this);

        if (_15C == 8) {
            setNerve(GET_NERVE(Tico, TicoNrvMoveRailForceTalk));
        } else {
            setNerve(GET_NERVE(Tico, TicoNrvWaitBeforeRail));
        }
    } else {
        MR::tryStartTurnAction(this);
    }
}

void Tico::exeMoveRail() {
    if (tryReaction()) {
        return;
    }

    bool reachedEnd = false;
    if (MR::isRailReachedEdge(this) && !MR::isRailGoingToEnd(this)) {
        reachedEnd = true;
    }

    if (reachedEnd) {
        setNerve(GET_NERVE(Tico, TicoNrvWaitAfterRail));
        return;
    }

    f32 diff = MR::calcDifferenceRailCoord(this, MR::calcNearestRailCoord(this, *MR::getPlayerPos()));
    TVec3f railPos;
    MR::calcRailPosAtCoord(&railPos, this, 10.0f * _1A0 + MR::getRailCoord(this));
    TVec2f screenPos;
    bool onScreen = true;
    if (!MR::calcScreenPosition(&screenPos, railPos) && !MR::calcScreenPosition(&screenPos, mPosition)) {
        onScreen = false;
    }

    f32 speedScale;
    if (diff < 200.0f) {
        speedScale = 1.0f;
    } else if (onScreen) {
        speedScale = 1.2f;
    } else {
        speedScale = 10.0f;
    }

    _10C = _1A0 * speedScale;
    if ((f32)__fabs((f64)(MR::getRailCoordSpeed(this) - _10C)) > 5.0f) {
        MR::setRailCoordSpeed(this, _10C);
    }

    if (diff > -1200.0f) {
        MR::tryStartMoveTurnAction(this);
    } else {
        setNerve(GET_NERVE(Tico, TicoNrvWaitBeforeRail));
    }
}

void Tico::exeWaitAfterRail() {
    if (tryReaction()) {
        return;
    }

    if (MR::isFirstStep(this)) {
        setInitPose();
        fn_800613B0(mMsgCtrl);
    }

    if (MR::isNear(this, MR::getRailPointPosEnd(this), 10.0f)) {
        if (MR::isNearPlayer(this, 500.0f)) {
            MR::followRailPose(this, _114, 1.0f);
            setNerve(GET_NERVE(Tico, TicoNrvMeta));
            return;
        }
    } else {
        MR::tryStartActionNotPlaying(this, _11C);
        MR::followRailPose(this, _114, _114);
        return;
    }

    if (mMsgCtrl != nullptr) {
        MR::tryTalkNearPlayerAndStartTalkAction(this);
    } else {
        MR::tryStartTurnAction(this);
    }
}

void Tico::exeWaitBeforeRail() {
    if (tryReaction()) {
        return;
    }

    if (MR::isFirstStep(this)) {
        fn_800613B0(mMsgCtrl);
    }

    f32 diff = MR::calcDifferenceRailCoord(this, MR::calcNearestRailCoord(this, *MR::getPlayerPos()));
    if (diff > -800.0f) {
        setNerve(GET_NERVE(Tico, TicoNrvMoveRail));
    }

    if (!fn_800613B0(mMsgCtrl)) {
        if (mMsgCtrl != nullptr) {
            MR::tryTalkNearPlayerAndStartTalkAction(this);
        } else {
            MR::tryStartTurnAction(this);
        }
    }
}

void Tico::exeMoveRailForceTalk() {
    if (tryReaction()) {
        return;
    }

    bool reachedEnd = false;
    if (MR::isRailReachedEdge(this) && !MR::isRailGoingToEnd(this)) {
        reachedEnd = true;
    }

    if (reachedEnd) {
        setNerve(GET_NERVE(Tico, TicoNrvWaitAfterRail));
    } else {
        MR::tryStartMoveTurnAction(this);
        if (mMsgCtrl) {
            MR::tryTalkForce(mMsgCtrl);
        }
    }
}
