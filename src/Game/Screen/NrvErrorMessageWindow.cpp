#include "Game/Screen/NrvErrorMessageWindow.hpp"
#include "Game/Screen/ErrorMessageWindow.hpp"
#include "Game/Util.hpp"

NrvErrorMessageWindow::ErrorMessageWindowNrvHide::ErrorMessageWindowNrvHide() {}
NrvErrorMessageWindow::ErrorMessageWindowNrvAppear::ErrorMessageWindowNrvAppear() {}
NrvErrorMessageWindow::ErrorMessageWindowNrvDisplay::ErrorMessageWindowNrvDisplay() {}
NrvErrorMessageWindow::ErrorMessageWindowNrvDisappear::ErrorMessageWindowNrvDisappear() {}
NrvErrorMessageWindow::ErrorMessageWindowNrvDisappearBeforeAppear::ErrorMessageWindowNrvDisappearBeforeAppear() {}

void NrvErrorMessageWindow::ErrorMessageWindowNrvDisappearBeforeAppear::execute(Spine* spine) const {
    ErrorMessageWindow* actor = static_cast<ErrorMessageWindow*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) MR::startAnim(actor, "End", 0);
    MR::setNerveAtAnimStopped(actor, &ErrorMessageWindowNrvAppear::sInstance, 0);
}

void NrvErrorMessageWindow::ErrorMessageWindowNrvDisappear::execute(Spine* spine) const {
    ErrorMessageWindow* actor = static_cast<ErrorMessageWindow*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) MR::startAnim(actor, "End", 0);
    MR::setNerveAtAnimStopped(actor, &ErrorMessageWindowNrvHide::sInstance, 0);
}

void NrvErrorMessageWindow::ErrorMessageWindowNrvDisplay::execute(Spine* spine) const {
    ErrorMessageWindow* actor = static_cast<ErrorMessageWindow*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) MR::startAnim(actor, "Wait", 0);
}

void NrvErrorMessageWindow::ErrorMessageWindowNrvAppear::execute(Spine* spine) const {
    ErrorMessageWindow* actor = static_cast<ErrorMessageWindow*>(spine->mExecutor);
    actor->exeAppear();
}

void NrvErrorMessageWindow::ErrorMessageWindowNrvHide::execute(Spine* spine) const {
    ErrorMessageWindow* actor = static_cast<ErrorMessageWindow*>(spine->mExecutor);
    if (MR::isFirstStep(actor)) actor->kill();
}

