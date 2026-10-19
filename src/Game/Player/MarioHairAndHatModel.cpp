#include "Game/Player/MarioHairAndHatModel.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include "Game/Util/ActorAnimUtil.hpp"
#include "Game/Util/ActorInitUtil.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"

namespace NrvMarioHairAndHatModel {
NEW_NERVE(MarioHairAndHatModelNrvNoCap, MarioHairAndHatModel, NoCap);
NEW_NERVE(MarioHairAndHatModelNrvAction, MarioHairAndHatModel, Action);
NEW_NERVE(MarioHairAndHatModelNrvWait, MarioHairAndHatModel, Wait);
}

MarioHairAndHatModel::MarioHairAndHatModel() : LiveActor("髪の毛＆帽子モデル"), _90(nullptr), _94(nullptr), _98(nullptr), _9C(false), _9D(false) {
    if (MR::isEqualStageName("MarioFaceShipGalaxy") && MR::getCurrentScenarioNo() == 2) {
        if (MR::isPlayerLuigi()) MR::initActorNoIter(this, "LuigiHair", "LuigiHairAnimMarioFaceShipGalaxy2", false);
        else MR::initActorNoIter(this, "MarioHair", "MarioHairAnimMarioFaceShipGalaxy2", false);
    } else if (MR::isEqualStageName("VsKoopaLv1Galaxy")) {
        if (MR::isPlayerLuigi()) MR::initActorNoIter(this, "LuigiHair", "LuigiHairAnimVsKoopaLv1Galaxy", false);
        else MR::initActorNoIter(this, "MarioHair", "MarioHairAnimVsKoopaLv1Galaxy", false);
    } else {
        if (MR::isPlayerLuigi()) MR::initActorNoIter(this, "LuigiHair", "LuigiHairAnim", false);
        else MR::initActorNoIter(this, "MarioHair", "MarioHairAnim", false);
    }
    MR::invalidateClipping(this);
    MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
}
MarioHairAndHatModel::~MarioHairAndHatModel() {}
void MarioHairAndHatModel::initAfterPlacement() {
    if (MR::isExistAreaObj("MarioNoCapArea")) initNerve(&NrvMarioHairAndHatModel::MarioHairAndHatModelNrvNoCap::sInstance);
    else initNerve(&NrvMarioHairAndHatModel::MarioHairAndHatModelNrvWait::sInstance);
}
void MarioHairAndHatModel::makeActorAppeared() { LiveActor::makeActorAppeared(); }
void MarioHairAndHatModel::makeActorDead() { LiveActor::makeActorDead(); }
void MarioHairAndHatModel::control() {}
void MarioHairAndHatModel::calcAndSetBaseMtx() {
    if (_90) {
        mPosition.set<f32>(_90[0][3], _90[1][3], _90[2][3]);
        MR::setBaseTRMtx(this, _90);
        return;
    }
    LiveActor::calcAndSetBaseMtx();
}
void MarioHairAndHatModel::fn_803EC760(MtxPtr matrix) { _94 = matrix; }
void MarioHairAndHatModel::fn_803EC770(MtxPtr matrix) { _98 = matrix; }
void MarioHairAndHatModel::fn_803EC780() {
    if (!isNerve(&NrvMarioHairAndHatModel::MarioHairAndHatModelNrvNoCap::sInstance)) {
        _90 = _94;
        if (MR::isExistAreaObj("MarioNoCapArea")) setNerve(&NrvMarioHairAndHatModel::MarioHairAndHatModelNrvNoCap::sInstance);
        else setNerve(&NrvMarioHairAndHatModel::MarioHairAndHatModelNrvWait::sInstance);
    }
}
bool MarioHairAndHatModel::fn_803EC7F0(const char* name) {
    if (isNerve(&NrvMarioHairAndHatModel::MarioHairAndHatModelNrvNoCap::sInstance)) return false;
    bool started = false;
    if (MR::fn_80008F00(this, name) && MR::isPlayerDefaultModel()) {
        MR::showModel(this);
        MR::startAction(this, name);
        started = true;
    } else MR::hideModelAndOnCalcAnim(this);
    _90 = _98;
    setNerve(&NrvMarioHairAndHatModel::MarioHairAndHatModelNrvAction::sInstance);
    return started;
}
bool MarioHairAndHatModel::fn_803EC8A0(const char* name, const BckCtrlData& data) {
    if (fn_803EC7F0(name)) { MR::reflectBckCtrlDataToAllAnim(this, name, data); return true; }
    return false;
}
void MarioHairAndHatModel::exeNoCap() {
    if (MR::isFirstStep(this)) { _90 = _94; MR::startAction(this, "NoCap"); MR::startPlayerNormalBva(); }
    if (MR::isInAreaObj("MarioNoCapArea", *MR::getPlayerPos()) && MR::isPlayerDefaultModel()) {
        MR::showModel(this); MR::hidePlayerJoint("Cap0");
    } else { MR::hideModelAndOnCalcAnim(this); MR::showPlayerJoint("Cap0"); }
}
void MarioHairAndHatModel::exeAction() { if (MR::isFirstStep(this)) _90 = _98; }
void MarioHairAndHatModel::exeWait() {
    if (MR::isFirstStep(this)) { _90 = _94; MR::startPlayerNormalBva(); MR::hideModelAndOnCalcAnim(this); MR::showPlayerJoint("Cap0"); }
}
