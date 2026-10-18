#include "Game/Screen/MiiConfirmIcon.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util/LayoutUtil.hpp"

namespace NrvMiiConfirmIcon {
    class MiiConfirmIconNrvAppear : public Nerve { public: MiiConfirmIconNrvAppear(); virtual void execute(Spine* spine) const { static_cast<MiiConfirmIcon*>(spine->mExecutor)->exeAppear(); } static MiiConfirmIconNrvAppear sInstance; };
    MiiConfirmIconNrvAppear MiiConfirmIconNrvAppear::sInstance ATTRIBUTE_WEAK;
    class MiiConfirmIconNrvWait : public Nerve { public: MiiConfirmIconNrvWait(); virtual void execute(Spine* spine) const { static_cast<MiiConfirmIcon*>(spine->mExecutor)->exeWait(); } static MiiConfirmIconNrvWait sInstance; };
    MiiConfirmIconNrvWait MiiConfirmIconNrvWait::sInstance ATTRIBUTE_WEAK;
    class MiiConfirmIconNrvDisappear : public Nerve { public: MiiConfirmIconNrvDisappear(); virtual void execute(Spine* spine) const { static_cast<MiiConfirmIcon*>(spine->mExecutor)->exeDisappear(); } static MiiConfirmIconNrvDisappear sInstance; };
    MiiConfirmIconNrvDisappear MiiConfirmIconNrvDisappear::sInstance ATTRIBUTE_WEAK;
};  // namespace

MiiConfirmIcon::MiiConfirmIcon(const char* pName) : LayoutActor(pName, true) {
}

void MiiConfirmIcon::init(const JMapInfoIter& rIter) {
    initLayoutManager("MiiConfirmIcon", 1);
    initNerve(GET_NERVE_DIRECT(NrvMiiConfirmIcon, MiiConfirmIconNrvAppear));
}

void MiiConfirmIcon::appear(nw4r::lyt::TexMap* pTexMap, const wchar_t* pName) {
    MR::replacePaneTexture(this, "ShaMiiDummy", pTexMap, 0);
    MR::replacePaneTexture(this, "PicMiiDummy", pTexMap, 0);
    MR::setTextBoxMessageRecursive(this, "MiiName", pName);
    appear();
    setNerve(GET_NERVE_DIRECT(NrvMiiConfirmIcon, MiiConfirmIconNrvAppear));
}

void MiiConfirmIcon::disappear() {
    setNerve(GET_NERVE_DIRECT(NrvMiiConfirmIcon, MiiConfirmIconNrvDisappear));
}

void MiiConfirmIcon::exeAppear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "ButtonAppear", 0);
    }

    if (MR::isAnimStopped(this, 0)) {
        setNerve(GET_NERVE_DIRECT(NrvMiiConfirmIcon, MiiConfirmIconNrvWait));
    }
}

void MiiConfirmIcon::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "ButtonWait", 0);
    }
}

void MiiConfirmIcon::exeDisappear() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "ButtonEnd", 0);
    }

    if (MR::isAnimStopped(this, 0)) {
        kill();
    }
}

bool MiiConfirmIcon::isDisappear() const {
    return isNerve(GET_NERVE_DIRECT(NrvMiiConfirmIcon, MiiConfirmIconNrvDisappear));
}

void MiiConfirmIcon::appear() {
    LayoutActor::appear();
}

namespace NrvMiiConfirmIcon {
    MiiConfirmIconNrvAppear::MiiConfirmIconNrvAppear() {}
    MiiConfirmIconNrvWait::MiiConfirmIconNrvWait() {}
    MiiConfirmIconNrvDisappear::MiiConfirmIconNrvDisappear() {}
}
