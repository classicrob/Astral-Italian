#include "Game/MapObj/CloudSea.hpp"
#include "Game/MapObj/MapObjActorInitInfo.hpp"
CloudSea::CloudSea(const char* name) : MapObjActor(name) {
    mOffset.x = 0.0f;
    mOffset.y = 0.0f;
    mOffset.z = 0.0f;
}
CloudSea::~CloudSea() {}
void CloudSea::init(const JMapInfoIter& iter) {
    MapObjActor::init(iter);
    MapObjActorInitInfo info;
    info.setupHioNode("地形オブジェ");
    info.setupDefaultPos();
    info.setupConnectToScene();
    info.setupEffect(nullptr, 0);
    info.setupSound(4, nullptr);
    info.setupGroupClipping(16);
    initialize(iter, info);
}
void CloudSea::control() {}
