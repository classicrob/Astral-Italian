#include "Game/MapObj/ActorStatePlayerPosReset.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
namespace NrvActorStatePlayerPosReset {
class ActorStatePlayerPosResetNrvWait : public Nerve {
public:
    virtual void execute(Spine*) const {}
    static ActorStatePlayerPosResetNrvWait sInstance;
};
ActorStatePlayerPosResetNrvWait ActorStatePlayerPosResetNrvWait::sInstance ATTRIBUTE_WEAK;
NEW_NERVE(ActorStatePlayerPosResetNrvClose, ActorStatePlayerPosReset, Close);
NEW_NERVE(ActorStatePlayerPosResetNrvReset, ActorStatePlayerPosReset, Reset);
NEW_NERVE(ActorStatePlayerPosResetNrvOpen, ActorStatePlayerPosReset, Open);
}
ActorStatePlayerPosReset::ActorStatePlayerPosReset(LiveActor* host, const char* name, bool demo) : ActorStateBase<LiveActor>("プレイヤー位置再セット状態", host), mPositionName(name), mPositionMtx(0), mPosition(0.0f, 0.0f, 0.0f), mUseDemo(demo) {
    initNerve(&NrvActorStatePlayerPosReset::ActorStatePlayerPosResetNrvWait::sInstance);
}
ActorStatePlayerPosReset::ActorStatePlayerPosReset(LiveActor* host, MtxPtr mtx, const TVec3f& pos, bool demo) : ActorStateBase<LiveActor>("プレイヤー位置再セット状態", host), mPositionName(0), mPositionMtx(mtx), mPosition(pos), mUseDemo(demo) {
    initNerve(&NrvActorStatePlayerPosReset::ActorStatePlayerPosResetNrvWait::sInstance);
}
bool ActorStatePlayerPosReset::isResetStart() {
    if (isNerve(&NrvActorStatePlayerPosReset::ActorStatePlayerPosResetNrvReset::sInstance)) return MR::isFirstStep(this);
    return false;
}
void ActorStatePlayerPosReset::appear() {
    mIsDead = false;
    if (mUseDemo) MR::requestStartDemoMarioPuppetableWithoutCinemaFrame(this, mHost, "マリオ再セット", &NrvActorStatePlayerPosReset::ActorStatePlayerPosResetNrvClose::sInstance, &NrvActorStatePlayerPosReset::ActorStatePlayerPosResetNrvWait::sInstance);
    else setNerve(&NrvActorStatePlayerPosReset::ActorStatePlayerPosResetNrvClose::sInstance);
}
void ActorStatePlayerPosReset::exeClose() {
    if (MR::isFirstStep(this)) MR::closeWipeFade(-1);
    if (!MR::isWipeActive()) {
        if (mPositionName) MR::setPlayerLinkPosAndWait(mHost, mPositionName, true);
        else if (mPositionMtx) {
            TPos3f mtx;
            mtx.set(mPositionMtx);
            MR::addTransMtxLocal(mtx.toMtxPtr(), mPosition);
            MR::setPlayerBaseMtx(mtx.toMtxPtr());
            MR::startBckPlayer("Wait", (const char*)0);
        }
        setNerve(&NrvActorStatePlayerPosReset::ActorStatePlayerPosResetNrvReset::sInstance);
    }
}
void ActorStatePlayerPosReset::exeOpen() {
    if (MR::isFirstStep(this)) MR::openWipeFade(-1);
    if (!MR::isWipeActive()) {
        if (mUseDemo) MR::endDemo(mHost, "マリオ再セット");
        kill();
    }
}
ActorStatePlayerPosReset::~ActorStatePlayerPosReset() {}
void ActorStatePlayerPosReset::exeReset() {
    if (MR::isStep(this, 30)) setNerve(&NrvActorStatePlayerPosReset::ActorStatePlayerPosResetNrvOpen::sInstance);
}
