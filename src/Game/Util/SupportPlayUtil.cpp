#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/MapObj/SupportPlayDirector.hpp"
#include "Game/NPC/SupportTico.hpp"
#include "Game/Scene/SceneObjHolder.hpp"
#include "Game/Util.hpp"

namespace {
    SupportTico* getSupportTico() {
        return MR::getSceneObj< SupportTico* >(SceneObj_SupportTico);
    }
};  // namespace

namespace MR {
    void setSupportTicoAttachRangeGliBird() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->setAttachRange(12000.0f);
        }
    }

    void resetSupportTicoAttachRangeGliBird() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->setAttachRange(3000.0f);
        }
    }

    void setSupportTicoAttachRangeTubeSlider() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->setAttachRange(8000.0f);
        }

        setSupportTicoFollowPlayerYDir();
    }

    void resetSupportTicoAttachRangeTubeSlider() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->setAttachRange(3000.0f);
        }

        resetSupportTicoFollowPlayerYDir();
    }

    void setSupportTicoFollowPlayerYDir() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->setFollowPlayerYDir();
        }
    }

    void resetSupportTicoFollowPlayerYDir() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->resetFollowPlayerYDir();
        }
    }

    void offSupportTicoControl() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->offControl();
        }
    }

    void onSupportTicoControl() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->onControl();
        }
    }

    void requestSupportTicoMovementOn() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            MR::requestMovementOn(getSupportTico());
        }
    }

    void requestSupportTicoHide() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->requestHide();
        }
    }

    void requestSupportTicoShow() {
        if (MR::isExistSceneObj(SceneObj_SupportTico)) {
            getSupportTico()->requestShow();
        }
    }

    bool isSupportTicoShow() {
        if (!MR::isExistSceneObj(SceneObj_SupportTico)) {
            return false;
        }

        return getSupportTico()->mShow;
    }

    bool attachSupportTicoToTarget(const LiveActor* pTarget) {
        return getSupportTico()->tryAttachToTarget(pTarget);
    }

    void joinSupportPlayGroup(LiveActor* pActor) {
        MR::createSceneObj(SceneObj_SupportPlayDirector);
        MR::getSceneObj< SupportPlayDirector* >(SceneObj_SupportPlayDirector)->registerActor(pActor);
    }

    bool isSupportPlayActive() {
        return MR::getSceneObj< SupportPlayDirector* >(SceneObj_SupportPlayDirector)->isActive();
    }

    bool isNeedExplainSupportTico() {
        if (MR::isOnGameEventFlagSupportTicoAtFirst() || MR::isPreventDisplayInformationMessage() || MR::isExecScenarioStarter() ||
            MR::isStageStateScenarioOpeningCamera() || MR::isDemoActive() || MR::isStageFileSelect() || MR::isStageStoryBook()) {
            return false;
        }

        return true;
    }

    bool isMsgSupportPlayGroupStart(u32 msg) {
        return msg == 0x78;
    }

    bool isMsgSupportPlayGroupStop(u32 msg) {
        return msg == 0x79;
    }

    bool isMsgItemSupportTicoDemoShow(u32 msg) {
        return msg == 0x88;
    }

    bool isMsgItemSupportTicoDemoHide(u32 msg) {
        return msg == 0x89;
    }

    bool sendMsgSupportPlayGroupStart(LiveActor* pActor) {
        return MR::sendSimpleMsgToActor(0x78, pActor);
    }

    bool sendMsgSupportPlayGroupStop(LiveActor* pActor) {
        return MR::sendSimpleMsgToActor(0x79, pActor);
    }

    bool sendMsgSupportTicoSpin(HitSensor* pSender, HitSensor* pReceiver) {
        return MR::sendArbitraryMsg(0x13, pSender, pReceiver);
    }
};  // namespace MR
