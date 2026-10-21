#include "draw_observer.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/MainLoopFramework.hpp"
#include "Game/System/GameSystemSceneController.hpp"
#include "Game/Scene/Scene.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Player/MarioHolder.hpp"
#include "Game/Player/MarioActor.hpp"
#include "Game/Player/Mario.hpp"
#include <JSystem/JUtility/JUTXfb.hpp>
#include <cstring>

namespace PetariNative::Visual {
namespace {
bool copyName(char (&out)[64],const char (&input)[64]) noexcept {
    const auto* end=static_cast<const char*>(std::memchr(input,0,64));
    if (!end) return false;
    std::memcpy(out,input,static_cast<std::size_t>(end-input));return true;
}
}
Snapshot readDrawSnapshot(const GameSystem* system,const MainLoopFramework* framework) noexcept {
    Snapshot out{};out.initState=-1;
    if (framework) {
        out.frameworkPresent=1;out.renderFlag=framework->mDoRenderFrame;
        out.renderInterval=framework->mRenderInterval;out.renderCounter=framework->mRenderCounter;
        if (out.renderFlag>1 || out.renderInterval<1) out.invalid=1;
        const auto* xfb=JUTXfb::getManager();
        if (xfb) {
            out.xfbPresent=1;out.bufferMode=xfb->mBufferNum;
            out.drawingIndex=xfb->mDrawingXfbIndex;out.drawnIndex=xfb->mDrawnXfbIndex;
            out.displayingIndex=xfb->mDisplayingXfbIndex;out.singleDrawingFlag=xfb->mSDrawingFlag;
            if (out.bufferMode<1 || out.bufferMode>3) out.invalid=1;
        }
    }
    const auto* controller=system ? system->mSceneController : nullptr;
    if (!controller) return out;
    out.controllerPresent=1;
    const auto& info=controller->mCurrSceneControlInfo;
    if (!copyName(out.key.scene,info.mScene) || !copyName(out.key.stage,info.mStage)) { out.invalid=1;return out; }
    out.key.placedScenario=info.mScenarioNo;out.key.selectedScenario=info.mSelectedScenarioNo;
    out.initState=static_cast<std::int32_t>(controller->mSceneInitializeState);
    if (out.initState<0 || out.initState>6) { out.invalid=1;return out; }
    const auto* current=controller->mScene;
    const auto* selected=controller->getCurrentSceneForExecute();
    out.currentSceneNonnull=current!=nullptr;out.executedSceneNonnull=selected!=nullptr;
    out.executedSceneIsCurrent=current && selected==current;
    out.initializedGame=out.initState==6 && std::strcmp(out.key.scene,"Game")==0;
    if (!out.initializedGame || !current || !current->mSceneObjHolder) return out;
    const auto* holder=static_cast<const MarioHolder*>(current->mSceneObjHolder->mObjs[SceneObj_MarioHolder]);
    const auto* actor=holder ? holder->mActor : nullptr;
    out.canonicalPlayerValid=actor && actor->mMario && actor->mMario->mPosHolder;
    return out;
}
} // namespace PetariNative::Visual
