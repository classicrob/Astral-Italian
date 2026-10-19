#include "Game/NPC/TicoStateEatCoin.hpp"
#include "Game/NPC/EatDemoCoin.hpp"
#include "Game/NPC/NPCActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"

namespace NrvTicoStateEatCoin {
    NEW_NERVE(TicoStateEatCoinNrvTurn, TicoStateEatCoin, Turn);
    NEW_NERVE(TicoStateEatCoinNrvEat, TicoStateEatCoin, Eat);
    NEW_NERVE(TicoStateEatCoinNrvWait, TicoStateEatCoin, Wait);
    NEW_NERVE(TicoStateEatCoinNrvComplete, TicoStateEatCoin, Complete);
}
namespace NrvEatDemoCoin {
    EatDemoCoinNrvShoot EatDemoCoinNrvShoot::sInstance;
    EatDemoCoinNrvShoot::EatDemoCoinNrvShoot() {}
    void EatDemoCoinNrvShoot::execute(Spine* spine) const { static_cast<EatDemoCoin*>(spine->mExecutor)->exeShoot(); }
}
using namespace NrvTicoStateEatCoin;

namespace TicoStateEatCoinFunction {
    void calcMouthPosition(TVec3f* position, const NPCActor* actor) {
        MR::copyJointPos(actor, "Mouth", position);
        TVec3f up(0.0f, 1.0f, 0.0f);
        actor->mOrientationQuat.getYDir(up);
        TVec3f offset(-up * 50.0f);
        position->add(offset);
    }
}

TicoStateEatCoin::TicoStateEatCoin(NPCActor* actor, s32 coins)
    : TicoStateBase("コイン食べState", actor), mActor(actor), mCoins(nullptr), mRequiredCoins(coins),
      mMouthPosition(0.0f, 0.0f, 0.0f), mGrowthStage(0), mAbsorbSoundCooldown(0) {
    mCoins = new EatDemoCoin*[5];
    for (s32 i = 0; i < 5; i++) mCoins[i] = new EatDemoCoin("コイン食べデモ用コイン");
    initNerve(&TicoStateEatCoinNrvTurn::sInstance);
}

TicoStateBase::~TicoStateBase() {}

void TicoStateEatCoin::appear() { mIsDead = false; setNerve(&TicoStateEatCoinNrvTurn::sInstance); }
void TicoStateEatCoin::control() { updateAbsorbSound(); mAbsorbSoundCooldown--; }
bool TicoStateEatCoin::isComplete() const { return isNerve(&TicoStateEatCoinNrvComplete::sInstance); }

#pragma dont_inline on
// NONMATCHING: calls and constants match; the getCamXdir and by-value direction temporaries sit in swapped stack slots and mActor is reloaded after the copy (register/stack residue).
void TicoStateEatCoin::exeTurn() {
    if (MR::isFirstStep(this)) TicoStateEatCoinFunction::calcMouthPosition(&mMouthPosition, mActor);
    TVec3f cameraPosition(0.0f, 0.0f, 0.0f);
#ifdef GALAXY_NATIVE
    const TVec3f cameraX = MR::getCamXdir();
    const TVec3f cameraPos = MR::getCamPos();
    JMAVECScaleAdd(&cameraX, &cameraPos, &cameraPosition, 300.0f);
#else
    JMAVECScaleAdd(&MR::getCamXdir(), &MR::getCamPos(), &cameraPosition, 300.0f);
#endif
    TVec3f direction(0.0f, 0.0f, 1.0f);
    direction.sub(cameraPosition, mMouthPosition);
    if (MR::faceToVector(&mActor->mOrientationQuat, direction, mActor->mParam._8)) setNerve(&TicoStateEatCoinNrvEat::sInstance);
}

#pragma dont_inline reset

// NONMATCHING: the camera position and direction temporary stack slots are swapped.
void TicoStateEatCoin::exeEat() {
    if (MR::isFirstStep(this)) { MR::decCoin(mRequiredCoins); mGrowthStage = 0; }
    if (MR::isStep(this, 20)) {
        MR::startAction(mActor, "Eat0Normal");
        MR::startBva(mActor, "Big0");
        mGrowthStage = 0;
    }
    s32 coins = mRequiredCoins < 30 ? mRequiredCoins : 30;
    if (MR::isStep(this, (coins / 3) * 5 + 20)) {
        MR::startAction(mActor, "Eat1Normal");
        MR::startBva(mActor, "Big1");
        mGrowthStage = 1;
    }
    if (MR::isStep(this, ((coins * 2) / 3) * 5 + 20)) {
        MR::startAction(mActor, "Eat2Normal");
        MR::startBva(mActor, "Big2");
        mGrowthStage = 2;
    }
    MR::startActionSound(mActor, "SmLvTicofatEating", getSoundLevel(), -1, -1);
    TicoStateEatCoinFunction::calcMouthPosition(&mMouthPosition, mActor);
    s32 duration = coins * 5;
    if (MR::isGreaterStep(this, duration + 20)) {
        for (s32 i = 0; i < 5; i++) if (!MR::isDead(mCoins[i])) return;
        setNerve(&TicoStateEatCoinNrvWait::sInstance);
    } else if (!MR::isGreaterEqualStep(this, duration) && MR::isIntervalStep(this, 5)) {
        EatDemoCoin* coin = nullptr;
        for (s32 i = 0; i < 5; i++) {
            if (MR::isDead(mCoins[i])) { coin = mCoins[i]; break; }
        }
        if (coin) {
            TVec3f position(0.0f, 0.0f, 0.0f);
#ifdef GALAXY_NATIVE
            const TVec3f cameraX = MR::getCamXdir();
            const TVec3f cameraPos = MR::getCamPos();
            JMAVECScaleAdd(&cameraX, &cameraPos, &position, 300.0f);
#else
            JMAVECScaleAdd(&MR::getCamXdir(), &MR::getCamPos(), &position, 300.0f);
#endif
            coin->shoot(position, &mMouthPosition);
            MR::stopCSSound(5, 0);
            MR::startCSSound("CS_COIN", nullptr, 0, 0, 1.0f);
        }
    }
}

void TicoStateEatCoin::exeWait() {
    if (MR::isFirstStep(this)) MR::tryRumblePadMiddle(this, 0);
    MR::startActionSound(mActor, "SmLvTicofatEating", getSoundLevel(), -1, -1);
    if (MR::isStep(this, 30)) setNerve(&TicoStateEatCoinNrvComplete::sInstance);
}

void TicoStateEatCoin::exeComplete() {
    if (MR::isFirstStep(this)) {
        mActor->mParam.setMoveAction("Joy2", "Joy2");
        MR::startActionSound(mActor, "SmTicofatMax", -1, -1, -1);
    }
    MR::tryStartTurnAction(mActor);
    MR::startActionSound(mActor, "SmLvTicofatGlad", getSoundLevel(), -1, -1);
}

s32 TicoStateEatCoin::getSoundLevel() const {
    s32 level = mGrowthStage;
    if (level < 0) level = 0;
    if (level > 32) return 32;
    return level;
}

void TicoStateEatCoin::updateAbsorbSound() {
    if ((isNerve(&TicoStateEatCoinNrvEat::sInstance) || isNerve(&TicoStateEatCoinNrvWait::sInstance)) && mAbsorbSoundCooldown <= 0) {
        for (s32 i = 0; i < 5; i++) {
            EatDemoCoin* coin = mCoins[i];
            if (!MR::isDead(coin) && MR::isStep(coin, 19)) {
                MR::startActionSound(mActor, "AbsorbCoin", -1, -1, -1);
                mAbsorbSoundCooldown = 5;
                return;
            }
        }
    }
}

EatDemoCoin::EatDemoCoin(const char* name) : LiveActor(name), mStartPosition(0.0f, 0.0f, 0.0f), mTargetPosition(nullptr) {
    mQuat.set<f32>(0.0f, 0.0f, 0.0f, 1.0f);
    MR::initActor(this, JMapInfoIter(), "Coin", false);
    MR::connectToSceneItemStrongLight(this);
    MR::invalidateClipping(this);
    initNerve(&NrvEatDemoCoin::EatDemoCoinNrvShoot::sInstance);
    makeActorDead();
}

#pragma dont_inline on
void EatDemoCoin::shoot(const TVec3f& start, const TVec3f* target) {
    MR::tryRumblePadWeak(this, 0);
    mStartPosition.set<f32>(start);
    mTargetPosition = target;
    mPosition.set<f32>(mStartPosition);
    mScale.setAll<f32>(0.8f);
    TVec3f direction(0.0f, 0.0f, 1.0f);
    direction.sub(*target, start);
    PSVECMag(&direction);
    PSVECNormalize(&direction, &direction);
    MR::makeQuatUpFront(&mQuat, MR::getCamYdir(), direction);
    MR::requestMovementOn(this);
    makeActorAppeared();
    setNerve(&NrvEatDemoCoin::EatDemoCoinNrvShoot::sInstance);
}

#pragma dont_inline reset

void EatDemoCoin::exeShoot() {
    MR::vecBlend(&mPosition, mStartPosition, *mTargetPosition, MR::calcNerveRate(this, 20));
    MR::rotateQuatLocalYDegree(&mQuat, mQuat, 30.0f);
    if (MR::isStep(this, 20)) {
        kill();
    }
}

void EatDemoCoin::calcAndSetBaseMtx() {
    MR::setBaseTRMtx(this, mQuat);
}

TicoStateEatCoin::~TicoStateEatCoin() {}
EatDemoCoin::~EatDemoCoin() {}
