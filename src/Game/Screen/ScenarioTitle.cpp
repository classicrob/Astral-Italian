#include "Game/Screen/ScenarioTitle.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"

namespace NrvScenarioTitle {
    NEW_NERVE(ScenarioTitleNrvAppear, ScenarioTitle, Appear);
    NEW_NERVE(ScenarioTitleNrvWait, ScenarioTitle, Wait);
    NEW_NERVE(ScenarioTitleNrvEnd, ScenarioTitle, End);
}

ScenarioTitle::ScenarioTitle() : LayoutActor("シナリオ名表示", true) {
}

void ScenarioTitle::init(const JMapInfoIter& rInfo) {
    initNerve(GET_NERVE_DIRECT(NrvScenarioTitle, ScenarioTitleNrvAppear));
    MR::connectToSceneLayout(this);
    initLayoutManager("StageTitle", 1);
    MR::registerDemoSimpleCastAll(this);
    kill();
}

void ScenarioTitle::start() {
    appear();
    setNerve(GET_NERVE_DIRECT(NrvScenarioTitle, ScenarioTitleNrvAppear));
}

void ScenarioTitle::end() {
    setNerve(GET_NERVE_DIRECT(NrvScenarioTitle, ScenarioTitleNrvEnd));
}

void ScenarioTitle::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::setTextBoxMessageRecursive(this, "Galaxy", MR::getCurrentGalaxyNameOnCurrentLanguage());
        MR::setTextBoxMessageRecursive(this, "Scenario", MR::getCurrentScenarioNameOnCurrentLanguage());
        MR::startAnim(this, "Appear", 0);
    }

    MR::setNerveAtAnimStopped(this, GET_NERVE_DIRECT(NrvScenarioTitle, ScenarioTitleNrvWait), 0);
}

void ScenarioTitle::exeWait() {
}

void ScenarioTitle::exeEnd() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "End", 0);
    }

    if (MR::isAnimStopped(this, 0)) {
        kill();
    }
}
