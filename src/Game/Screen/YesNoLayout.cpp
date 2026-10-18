#include "Game/Screen/YesNoLayout.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Screen/YesNoController.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/ScreenUtil.hpp"
#include "Game/Util/StarPointerUtil.hpp"

namespace NrvYesNoLayout {
    NEW_NERVE(YesNoLayoutNrvSelecting, YesNoLayout, Selecting);
};  // namespace NrvYesNoLayout

YesNoLayout::YesNoLayout() : SimpleLayout("Yes/Noレイアウト", "SelectButton", 1, 71), mController(nullptr) {
}

void YesNoLayout::init(const JMapInfoIter& rIter) {
    MR::createAndAddPaneCtrl(this, "Left", 1);
    MR::createAndAddPaneCtrl(this, "Right", 1);
    initEffectKeeper(0, nullptr, nullptr);

    mController = new YesNoController(this);

    MR::registerDemoSimpleCastAll(this);
    initNerve(GET_NERVE(YesNoLayout, YesNoLayoutNrvSelecting));
}

void YesNoLayout::appear() {
    LayoutActor::appear();
    mController->appear();

    MR::startStarPointerModeChooseYesNo(this);
    setNerve(GET_NERVE(YesNoLayout, YesNoLayoutNrvSelecting));
}

void YesNoLayout::kill() {
    MR::endStarPointerMode(this);
    LayoutActor::kill();
}

void YesNoLayout::setSelectMessage(const char* pRightSelectMessage, const char* pLeftSelectMessage) {
    MR::setTextBoxGameMessageRecursive(this, "Right", pRightSelectMessage);
    MR::setTextBoxGameMessageRecursive(this, "Left", pLeftSelectMessage);
}

void YesNoLayout::control() {
    mController->update();
}

void YesNoLayout::exeSelecting() {
    if (!mController->_C) {
        kill();
    }
}
