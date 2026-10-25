#include "Game/Player/MarioTeresa.hpp"
#include "Game/Util.hpp"
#include "Game/Animation/XanimePlayer.hpp"
#include "Game/Animation/XanimeResource.hpp"
#include "Game/LiveActor/ModelManager.hpp"
#include "Game/Map/HitInfo.hpp"
#include "Game/MapObj/BigFanHolder.hpp"
#include "Game/Player/Mario.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/MarioActorHelpers.hpp"
#include "Game/Player/MarioAnimator.hpp"
#include "Game/Player/MarioConst.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/MarioParts.hpp"

// unnamed in symbols.txt: 1-arg Mario::setFrontVecKeepUp
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/LiveActorUtil.hpp"
#include "Game/Util/MapUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ModelUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include "Game/Util/StringUtil.hpp"




XanimeGroupInfo teresaAnimeTable[] = {
    {{"基本"}, 1.0f, 16},
    {{""}},
};

XanimeBckTable2 teresaAnime2[] = {
    {{"基本"}, {{"wait", 1.0f}, {"run", 0.0f}}},
    {{""}, {{"", 0.0f}, {nullptr, 0.0f}}},
};

MarioTeresaModel* MarioTeresaModel::create(MarioActor* actor, bool luigi) {
    const char* modelName = luigi ? "TeresaLuigi" : "TeresaMario";
    if (!MR::isCreatePlayerResourceTeresa()) return nullptr;
    return new MarioTeresaModel(modelName, actor);
}

MarioTeresaModel::MarioTeresaModel(const char* modelName, MarioActor* actor)
    : mActor(nullptr), _4(0.0f), _8(0.0f), _C(0.0f), _10(0), mPlayer(nullptr), mResource(nullptr) {
    mActor = new MarioParts(actor, "テレサの皮", modelName, nullptr);
    mActor->_9C = "TeresaMario";
    mActor->initWithoutIter();
    mActor->kill();
    mActor->initFixedPosition(TVec3f(0.0f, 0.0f, 0.0f), TVec3f(0.0f, 0.0f, 0.0f), nullptr);
    MR::initShadowVolumeSphere(mActor, 90.0f);
    initAnimation();
}
void MarioTeresaModel::initAnimation() {
    _C = 0.0f;
    mResource = new XanimeResourceTable(MR::getResourceHolder(mActor), teresaAnimeTable, nullptr, nullptr, teresaAnime2, nullptr, nullptr, nullptr);
    mPlayer = new XanimePlayer(MR::getJ3DModel(mActor), mResource);
    mPlayer->setDefaultAnimation("基本");
    mPlayer->changeAnimationBck("基本");
    mActor->mModelManager->mXanimePlayer = mPlayer;
}
