#include "Game/MapObj/RepeatBlock.hpp"
#include "Game/Util.hpp"
namespace NrvRepeatBlock {
    class RepeatBlockNrvWait : public Nerve {
    public:
        virtual void execute(Spine* spine) const { static_cast<RepeatBlock*>(spine->mExecutor)->exeWait(); }
        static RepeatBlockNrvWait sInstance;
    };
}
RepeatBlockAtom::RepeatBlockAtom(RepeatBlock* host) : LiveActor("リピートブロックアトム") {
    MR::setRotation(this,host->mRotation);
    MR::initActor(this,"RepeatBlock",false);
    MR::initCollisionParts(this,"RepeatBlock",host->getSensor("Body"),nullptr);
    initNerve(&NrvRepeatBlock::RepeatBlockNrvWait::sInstance);
}
