#include "Game/NPC/MeisterDemoExitItemRoom.hpp"
#include "Game/NPC/Meister.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace { const char* cDemoName = "アイテム部屋からの退室"; }
namespace NrvMeisterDemoExitItemRoom {
    NEW_NERVE(MeisterDemoExitItemRoomNrvWait, MeisterDemoExitItemRoom, Wait);
}
MeisterDemoExitItemRoom::MeisterDemoExitItemRoom(Meister* host, const JMapInfoIter&) : NerveExecutor("マイスターデモ[アイテム部屋からの退室]"), mHost(host), mRepositionHost(false) {
    if (MR::isDemoExist(cDemoName)) {
        MR::registerDemoActionFunctorDirect(mHost, MR::Functor_Inline(this, &MeisterDemoExitItemRoom::start), cDemoName, nullptr);
        mRepositionHost = MR::hasMarioFaceShipItemEventAtResult();
    }
    initNerve(&NrvMeisterDemoExitItemRoom::MeisterDemoExitItemRoomNrvWait::sInstance, 0);
}
void MeisterDemoExitItemRoom::exeWait() {
    if (MR::isFirstStep(this)) {
        if (!mRepositionHost) return;
        TVec3f position(0.0f);
        TVec3f rotation(0.0f);
        MR::findNamePos("マイスター位置[デフォルト]", &position, &rotation);
        MR::setPosition(mHost, position);
        MR::setRotation(mHost, rotation);
        mHost->refreshEventTalk();
    }
    if (MR::isDemoLastStep()) mHost->endDemoState();
}
void MeisterDemoExitItemRoom::start() { mHost->startDemoState(this); }
MeisterDemoExitItemRoom::~MeisterDemoExitItemRoom() {}
