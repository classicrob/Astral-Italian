#include "Game/MapObj/SwingActionWatcher.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"
#ifdef GALAXY_NATIVE
#include "Game/Player/MarioCappy.hpp"
#endif
SwingActionWatcher::SwingActionWatcher(const char* name) : NameObj(name),mWasSwinging(false),mStarted(false) { MR::connectToSceneMapObjMovement(this); }
SwingActionWatcher::~SwingActionWatcher() {}
void SwingActionWatcher::movement() {
    bool swinging=MR::isPlayerSwingAction();
#ifdef GALAXY_NATIVE
    if (petariCappyUsesSpinAnimation()) swinging=false;
#endif
    bool started=false;
    if (!mWasSwinging && swinging) started=true;
    mStarted=started;
    mWasSwinging=swinging;
}
namespace MR {
    void createSwingActionWatcher() { createSceneObj(SceneObj_SwingActionWatcher); }
    bool isPlayerSwingActionStart() { return getSceneObj<SwingActionWatcher*>(SceneObj_SwingActionWatcher)->mStarted; }
}
