#include "Game/Map/FileSelectModel.hpp"
#include "Game/Util/LiveActorUtil.hpp"

namespace NrvFileSelectModel {
    void FileSelectModelNrvOpen::execute(Spine* pSpine) const {
        FileSelectModel* actor = reinterpret_cast<FileSelectModel*>(pSpine->mExecutor);
        if (MR::isFirstStep(actor)) {
            MR::startAction(actor, "normal");
        }
    }
    FileSelectModelNrvOpen FileSelectModelNrvOpen::sInstance ATTRIBUTE_WEAK;

    void FileSelectModelNrvBlinkOnce::execute(Spine* pSpine) const {
        reinterpret_cast<FileSelectModel*>(pSpine->mExecutor)->exeBlinkOnce();
    }
    FileSelectModelNrvBlinkOnce FileSelectModelNrvBlinkOnce::sInstance ATTRIBUTE_WEAK;

    void FileSelectModelNrvClose::execute(Spine* pSpine) const {
        FileSelectModel* actor = reinterpret_cast<FileSelectModel*>(pSpine->mExecutor);
        if (MR::isFirstStep(actor)) {
            MR::startAction(actor, "close");
        }
    }
    FileSelectModelNrvClose FileSelectModelNrvClose::sInstance ATTRIBUTE_WEAK;

    void FileSelectModelNrvBlink::execute(Spine* pSpine) const {
        reinterpret_cast<FileSelectModel*>(pSpine->mExecutor)->exeBlink();
    }
    FileSelectModelNrvBlink FileSelectModelNrvBlink::sInstance ATTRIBUTE_WEAK;
};  // namespace NrvFileSelectModel
