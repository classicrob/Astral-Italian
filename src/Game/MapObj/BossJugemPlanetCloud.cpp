#include "Game/MapObj/BossJugemPlanetCloud.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
namespace NrvBossJugemPlanetCloud {
    class BossJugemPlanetCloudNrvWaitWhite : public Nerve {
    public:
        virtual void execute(Spine*) const;
        static BossJugemPlanetCloudNrvWaitWhite sInstance;
    };
    BossJugemPlanetCloudNrvWaitWhite BossJugemPlanetCloudNrvWaitWhite::sInstance ATTRIBUTE_WEAK;
    NEW_NERVE(BossJugemPlanetCloudNrvWhiteToGray,BossJugemPlanetCloud,WhiteToGray);
    NEW_NERVE(BossJugemPlanetCloudNrvWaitGray,BossJugemPlanetCloud,WaitGray);
    NEW_NERVE(BossJugemPlanetCloudNrvGrayToWhite,BossJugemPlanetCloud,GrayToWhite);
    void BossJugemPlanetCloudNrvWaitWhite::execute(Spine* spine) const {
        BossJugemPlanetCloud* actor = static_cast<BossJugemPlanetCloud*>(spine->mExecutor);
        if (MR::isOnSwitchA(actor) && MR::calcDistanceToPlayer(actor) < 7000.0f) actor->setNerve(&BossJugemPlanetCloudNrvWhiteToGray::sInstance);
    }
}
BossJugemPlanetCloud::BossJugemPlanetCloud(const char* name) : LiveActor(name) {}
BossJugemPlanetCloud::~BossJugemPlanetCloud() {}
void BossJugemPlanetCloud::init(const JMapInfoIter& iter) {
    MR::initActor(this,iter,false);
    MR::startAction(this,"White");
    initNerve(&NrvBossJugemPlanetCloud::BossJugemPlanetCloudNrvWaitWhite::sInstance);
    makeActorAppeared();
}
void BossJugemPlanetCloud::attackSensor(HitSensor* sender,HitSensor* receiver) {
    if (isNerve(&NrvBossJugemPlanetCloud::BossJugemPlanetCloudNrvWaitGray::sInstance) && (MR::isSensorPlayerOrRide(receiver) || MR::isSensorEnemy(receiver))) MR::sendMsgEnemyAttackElectric(receiver,sender);
}
bool BossJugemPlanetCloud::receiveMsgPlayerAttack(u32,HitSensor*,HitSensor*) { return false; }
void BossJugemPlanetCloud::exeWhiteToGray() {
    if (MR::isFirstStep(this)) MR::startAction(this,"WhiteToGray");
    if (MR::isActionEnd(this)) setNerve(&NrvBossJugemPlanetCloud::BossJugemPlanetCloudNrvWaitGray::sInstance);
}
void BossJugemPlanetCloud::exeWaitGray() {
    if (MR::isFirstStep(this)) MR::startAction(this,"Gray");
    if (!MR::isOnSwitchA(this)) setNerve(&NrvBossJugemPlanetCloud::BossJugemPlanetCloudNrvGrayToWhite::sInstance);
}
void BossJugemPlanetCloud::exeGrayToWhite() {
    if (MR::isFirstStep(this)) MR::startAction(this,"GrayToWhite");
    if (MR::isActionEnd(this)) kill();
}
