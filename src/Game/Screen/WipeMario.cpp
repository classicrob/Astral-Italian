#include "Game/Screen/WipeMario.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Util.hpp"
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
namespace NrvWipeMario {
    class WipeMarioNrvOpen : public Nerve { public: virtual void execute(Spine*) const {} static WipeMarioNrvOpen sInstance; };
    class WipeMarioNrvWipeOut : public Nerve { public: virtual void execute(Spine*) const; static WipeMarioNrvWipeOut sInstance; };
    class WipeMarioNrvClose : public Nerve { public: virtual void execute(Spine*) const {} static WipeMarioNrvClose sInstance; };
    class WipeMarioNrvWipeIn : public Nerve { public: virtual void execute(Spine*) const; static WipeMarioNrvWipeIn sInstance; };
    WipeMarioNrvOpen WipeMarioNrvOpen::sInstance;
    WipeMarioNrvWipeOut WipeMarioNrvWipeOut::sInstance;
    WipeMarioNrvClose WipeMarioNrvClose::sInstance;
    WipeMarioNrvWipeIn WipeMarioNrvWipeIn::sInstance;
}
using namespace NrvWipeMario;
WipeMario::WipeMario() : WipeLayoutBase("マリオワイプ"), mWipeFrames(90) {}
void WipeMario::init(const JMapInfoIter&) {
    initNerve(&WipeMarioNrvOpen::sInstance);
    initLayoutManager("WipeMario", 1);
}
void WipeMario::exeWipeOut() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "out", 0);
        if (mWipeFrames > 0) MR::setAnimRate(this, static_cast<f32>(MR::getAnimCtrl(this, 0)->getEnd()) / mWipeFrames, 0);
        else MR::setAnimRate(this, 1.0f, 0);
        MR::showLayout(this);
    }
    if (MR::isAnimStopped(this, 0)) setNerve(&WipeMarioNrvClose::sInstance);
}
void WipeMario::exeWipeIn() {
    if (MR::isFirstStep(this)) {
        MR::startAnim(this, "in", 0);
        if (mWipeFrames > 0) MR::setAnimRate(this, static_cast<f32>(MR::getAnimCtrl(this, 0)->getEnd()) / mWipeFrames, 0);
        else MR::setAnimRate(this, 1.0f, 0);
        MR::showLayout(this);
    }
    if (MR::isAnimStopped(this, 0)) setNerve(&WipeMarioNrvOpen::sInstance);
}
void WipeMario::wipe(s32 frames) {
    if (isNerve(&WipeMarioNrvOpen::sInstance)) setNerve(&WipeMarioNrvWipeOut::sInstance);
    else if (isNerve(&WipeMarioNrvClose::sInstance)) setNerve(&WipeMarioNrvWipeIn::sInstance);
    mWipeFrames = frames;
}
void WipeMario::forceClose() {
    setNerve(&WipeMarioNrvClose::sInstance);
    MR::showLayout(this);
    MR::startAnim(this, "out", 0);
    MR::setAnimFrameAndStop(this, MR::getAnimCtrl(this, 0)->getEnd(), 0);
}
void WipeMario::forceOpen() { setNerve(&WipeMarioNrvOpen::sInstance); MR::hideLayout(this); }
bool WipeMario::isOpen() const { return isNerve(&WipeMarioNrvOpen::sInstance); }
bool WipeMario::isClose() const { return isNerve(&WipeMarioNrvClose::sInstance); }
bool WipeMario::isWipeIn() const { return isNerve(&WipeMarioNrvWipeIn::sInstance); }
bool WipeMario::isWipeOut() const { return isNerve(&WipeMarioNrvWipeOut::sInstance); }
s32 WipeMario::getWipeType() const { return 2; }
WipeMario::~WipeMario() {}
namespace NrvWipeMario {
    inline void WipeMarioNrvWipeIn::execute(Spine* spine) const { static_cast<WipeMario*>(spine->mExecutor)->exeWipeIn(); }
    inline void WipeMarioNrvWipeOut::execute(Spine* spine) const { static_cast<WipeMario*>(spine->mExecutor)->exeWipeOut(); }
}
