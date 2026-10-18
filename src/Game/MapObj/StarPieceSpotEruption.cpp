#include "Game/MapObj/StarPieceSpotEruption.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActionSoundUtil.hpp"

namespace NrvStarPieceSpotEruption {
    NEW_NERVE(StarPieceSpotEruptionNrvRest, StarPieceSpotEruption, Rest);
    class StarPieceSpotEruptionNrvEruption : public Nerve {
    public:
        void execute(Spine* spine) const {
            StarPieceSpotEruption* actor = static_cast<StarPieceSpotEruption*>(spine->mExecutor);
            if (MR::isFirstStep(actor)) actor->emit();
        }
        static StarPieceSpotEruptionNrvEruption sInstance;
    };
    StarPieceSpotEruptionNrvEruption StarPieceSpotEruptionNrvEruption::sInstance ATTRIBUTE_WEAK;
}

StarPieceSpotEruption::StarPieceSpotEruption(const char* name) : LiveActor(name), mEmitRange(150.0f), mEmitSpeed(30.0f) {}
void StarPieceSpotEruption::init(const JMapInfoIter& iter) {
    MR::initDefaultPos(this, iter);
    s32 count = 20;
    MR::getJMapInfoArg0NoInit(iter, &count);
    MR::getJMapInfoArg1NoInit(iter, &mEmitRange);
    MR::getJMapInfoArg2NoInit(iter, &mEmitSpeed);
    MR::declareStarPiece(this, count);
    initSound(3, "StarPieceSpotEruption", nullptr, TVec3f(0.0f));
    initEffectKeeper(2, "StarPieceSpotEruption", false);
    MR::connectToSceneMapObjMovement(this);
    MR::initUseStageSwitchReadA(this, iter);
    MR::setClippingTypeSphere(this, 3000.0f);
    MR::setClippingFarMax(this);
    initNerve(&NrvStarPieceSpotEruption::StarPieceSpotEruptionNrvRest::sInstance);
    makeActorAppeared();
}
void StarPieceSpotEruption::exeRest() {
    if (MR::isFirstStep(this)) MR::emitEffect(this, "Rest");
    if (MR::isValidSwitchA(this) && MR::isOnSwitchA(this))
        setNerve(&NrvStarPieceSpotEruption::StarPieceSpotEruptionNrvEruption::sInstance);
}
void StarPieceSpotEruption::emit() {
    MR::shakeCameraStrong();
    MR::tryRumblePadStrong(this, 0);
    MR::appearStarPiece(this, mPosition, MR::getDeclareRemnantStarPieceCount(this), mEmitSpeed, mEmitRange, false);
    MR::deleteEffect(this, "Rest");
    MR::emitEffect(this, "Eruption");
    MR::startActionSound(this, "Eruption");
}
StarPieceSpotEruption::~StarPieceSpotEruption() {}
