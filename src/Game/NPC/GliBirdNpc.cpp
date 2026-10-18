// NONMATCHING: literal addresses.
#include "Game/NPC/GliBirdNpc.hpp"
#include "Game/Util.hpp"
GliBirdNpc::GliBirdNpc(const char* name) : NPCActor(name) {}
GliBirdNpc::~GliBirdNpc() {}
void GliBirdNpc::init(const JMapInfoIter& iter) {
    NPCActorCaps caps("GliBirdNpc");
    caps.setDefault();
    caps.mMessage=false;
    caps._95=false;
    initialize(iter,caps,"GliBird","Npc",nullptr);
    mParam._14="GroundWait";
    mParam._1C="GroundWait";
    mParam._18=nullptr;
    mParam._20=nullptr;
    mParam._0=false;
    mParam._1=false;
    setDefaults();
    _12C=450.0f;
    MR::registerDemoSimpleCastAll(this);
    makeActorAppeared();
}
