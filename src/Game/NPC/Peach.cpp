#include "Game/NPC/Peach.hpp"
#include "Game/Util.hpp"

Peach::Peach(const char* pName) : NPCActor(pName) {}

void Peach::init(const JMapInfoIter& rIter) {
    NPCActorCaps caps("Peach");
    caps.setDefault();
    initialize(rIter, caps, "Peach", nullptr, "PeachAnim");
    mParam.setMoveAction("Wait", "Turn");
    mParam.setTalkAction("Talk", "Turn");
    setDefaults();
    _94 = MR::createPartsModelMapObj(this, "ピーチ傘", "PeachUmbrella", getBaseMtx());
}

void Peach::control() {
    NPCActor::control();
    MR::updateNPCGoodsAnimation(this);
}

Peach::~Peach() {}
