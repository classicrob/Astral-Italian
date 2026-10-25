#define MR_TAN_NOINLINE
#define JGEOMETRY_VEC2_CTOR_NOINLINE
#define JGEOMETRY_VEC2_SUB_NOINLINE
#define JGEOMETRY_VEC3_COPY_NOINLINE
#include "Game/Util/StarPointerUtil.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/LiveActor.hpp"
#include "Game/Map/CollisionParts.hpp"
#include "Game/MapObj/StarPieceDirector.hpp"
#include "Game/Screen/LayoutManager.hpp"
#include "Game/Screen/StarPointerController.hpp"
#include "Game/Screen/StarPointerDirector.hpp"
#include "Game/Screen/StarPointerLayout.hpp"
#include "Game/Screen/StarPointerTarget.hpp"
#include "Game/System/GameSequenceDirector.hpp"
#include "Game/System/GameSystem.hpp"
#include "Game/System/GameSystemObjHolder.hpp"
#include "Game/System/StarPointerOnOffController.hpp"
#include "Game/System/WPadRumble.hpp"
#include "Game/Util/AreaObjUtil.hpp"
#include "Game/Util/CameraUtil.hpp"
#include "Game/Util/DemoUtil.hpp"
#include "Game/Util/GamePadUtil.hpp"
#include "Game/Util/JointUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/PlayerUtil.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/SupportPlayUtil.hpp"
#include "Game/Screen/StarPointerMovePositionHolder.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include <JSystem/JUtility/JUTVideo.hpp>
#include <revolution/wpad.h>

namespace {

    static const f32 hPointColRayLength = 99999.0f;

    class StarPointerTargetInfo {
    public:
        StarPointerTargetInfo(StarPointerTarget* pTarget, HitInfo* pHitInfo, CollisionParts* pCollisionParts)
            : mTarget(pTarget), mHitInfo(pHitInfo), mCollisionParts(pCollisionParts) {
        }

        /* 0x00 */ StarPointerTarget* mTarget;
        /* 0x04 */ HitInfo* mHitInfo;
        /* 0x08 */ CollisionParts* mCollisionParts;
    };

    typedef bool (*StarPointerFunc1)(StarPointerTargetInfo*, const TVec3f&, const TVec2f&, f32, f32);
    typedef bool (*StarPointerFunc2)(s32);

    StarPointerDirector* getStarPointerDirector() {
        return StarPointerFunction::getStarPointerDirector();
    }

    inline StarPointerLayout* getStarPointerLayout(s32 channel) {
        return getStarPointerDirector()->getStarPointerLayout(channel);
    }

    StarPointerOnOffController* getStarPointerOnOffController() NO_INLINE {
        return SingletonHolder< GameSystem >::get()->mSequenceDirector->mStarPointerOnOffController;
    }

    inline StarPointerInput* getStarPointerController(s32 channel) {
        return getStarPointerDirector()->getStarPointerController(channel);
    }

    bool always(s32 channel) {
        return true;
    }

    void onReaction(u64 touchID, s32 channel, bool enableTouch, bool disableShoot, bool singleTouch) {
        StarPointerLayout* layout = StarPointerFunction::getStarPointerDirector()->getStarPointerLayout(channel);
        if (layout != nullptr) {
            layout->mNewTouchedID = touchID;
            layout->mStartTouch = enableTouch;
            layout->mStartDisableShoot = disableShoot;
            layout->mStartSingleTouch = singleTouch;
        }
    }

    bool checkPointingTarget(StarPointerTargetInfo* pTargetInfo, const TVec3f& rOffset, const TVec2f& rPointerPos, f32 zMargin, f32 radiusMargin) {
        return pTargetInfo->mTarget->isPointing(rPointerPos, zMargin, radiusMargin);
    }

    bool checkPointingWithoutCheckZ(StarPointerTargetInfo* pTargetInfo, const TVec3f& rOffset, const TVec2f& rPointerPos, f32 zMargin,
                                    f32 radiusMargin) {
        return checkPointingTarget(pTargetInfo, rOffset, rPointerPos, ::hPointColRayLength, radiusMargin);
    }

    bool isStarPointerPointingCore(StarPointerTargetInfo* pTargetInfo, const LiveActor* pActor, s32 channel, StarPointerFunc1 targetPointCheckFunc,
                                   StarPointerFunc2 checkUpdateChannelFunc, bool enableTouch, bool disableShoot, bool singleTouch) {
        if (!getStarPointerLayout(channel)->mIsPointerValid) {
            return false;
        }

        if (!MR::isConnectedWPad(channel)) {
            return false;
        }

        if (channel == WPAD_CHAN1 && MR::isStarPointer2PTransparencyMode()) {
            return false;
        }

        if (channel == WPAD_CHAN0 && getStarPointerOnOffController()->compareMode(StarPointerMode_1PInvalid2PValid)) {
            return false;
        }

        StarPointerInput* controller = getStarPointerController(channel);
        if (!controller->isInScreen()) {
            return false;
        }

        StarPointerLayout* layout = getStarPointerLayout(channel);
        if (layout == nullptr) {
            return false;
        }

        if (!singleTouch && layout->isSingleTouch()) {
            return false;
        }

        if (singleTouch && layout->isTouch()) {
            return false;
        }

        if (!targetPointCheckFunc(pTargetInfo, controller->getWorldPosition(), controller->getScreenPosition(), controller->getViewDistanceZ(), layout->getRadius())) {
            return false;
        }

        onReaction(reinterpret_cast< u64 >(pActor), channel, enableTouch, disableShoot, singleTouch);

        if (!checkUpdateChannelFunc(channel)) {
            return false;
        }

        pTargetInfo->mTarget->mLastPointedChannel = channel;
        return true;
    }
};  // namespace

namespace MR {

    void initStarPointerGameScene() {
        StarPointerFunction::getStarPointerDirector()->init();
    }

    void createStarPointerLayout() {
        StarPointerFunction::getStarPointerDirector()->createLayout();
    }

    void onStarPointerSceneOut() {
        ::getStarPointerDirector()->invalidate();
        ::getStarPointerOnOffController()->setStateToBase(nullptr);
        WPadFunction::getWPadRumble(WPAD_CHAN0)->stop();
        WPadFunction::getWPadRumble(WPAD_CHAN1)->stop();
    }

    void setStarPointerModeBase() {
        ::getStarPointerOnOffController()->setStateToBase(nullptr);
    }

    void setStarPointerCameraMtxAtGameScene() {
        ::getStarPointerDirector()->setGameSceneCameraMtx();
    }

    bool isValidStarPointerTarget(const LiveActor* pActor) {
        return pActor->mStarPointerTarget != nullptr;
    }

    void initStarPointerTarget(LiveActor* pActor, f32 radius, const TVec3f& rOffset) {
        pActor->initActorStarPointerTarget(radius, &pActor->mPosition, pActor->getBaseMtx(), TVec3f(rOffset));
    }

    void initStarPointerTargetAtPos(LiveActor* pActor, f32 radius, const TVec3f* pPos, const TVec3f& rOffset) {
        pActor->initActorStarPointerTarget(radius, pPos, pActor->getBaseMtx(), TVec3f(rOffset));
    }


    void initStarPointerTargetAtJoint(LiveActor* pActor, const char* pJointName, f32 radius, const TVec3f& rOffset) {
        MtxPtr mtx = MR::getJointMtx(pActor, pJointName);
        pActor->initActorStarPointerTarget(radius, nullptr, mtx, TVec3f(rOffset));
    }

    void addStarPointerTargetCircle(LayoutActor* pActor, const char* pLayoutName, f32 radius, const TVec2f& rPosition, const char* pPaneName) {
        pActor->mPointingTarget->addTargetCircle(pActor, pLayoutName, radius, rPosition, pPaneName);
    }

    bool isStarPointerValid(s32 channel) {
        return ::getStarPointerLayout(channel)->mIsPointerValid && MR::isConnectedWPad(channel);
    }

    bool isStarPointerTargetClipped(const LiveActor* pActor) {
        StarPointerTarget* pTarget = pActor->mStarPointerTarget;
        if (pTarget == nullptr) {
            return true;
        }

        TVec3f pos(0.0f);
        pTarget->calcPosition(&pos);
        return MR::isJudgedToClipFrustum(pos, pTarget->mRadius3d);
    }

    bool isStarPointerPointing(const LiveActor* pActor, s32 channel, bool enableTouch, const char* pStrength) {
        if (channel == WPAD_CHAN0) {
            return MR::isStarPointerPointing1P(pActor, pStrength, enableTouch, false);
        } else {
            return MR::isStarPointerPointing2P(pActor, pStrength, enableTouch, false);
        }
    }


    bool tryStarPointerCheckWithoutRumble(LiveActor* pActor, bool enableTouch) {
        return MR::isStarPointerPointing1Por2P(pActor, nullptr, enableTouch, false);
    }

    bool isStarPointerPointingPane(const LayoutActor* pActor, const char* pLayoutName, s32 channel, bool enableTouch, const char* pStrength) {
        if (::getStarPointerLayout(channel)->mIsPointerValid) {
            return isStarPointerPointingPaneForMeterLayout(pActor, pLayoutName, channel, enableTouch, pStrength);
        }
        return false;
    }

    bool isStarPointerPointingPaneForMeterLayout(const LayoutActor* pActor, const char* pLayoutName, s32 channel, bool enableTouch,
                                                 const char* pStrength) {
        if (!MR::isConnectedWPad(channel)) {
            return false;
        }

        if (channel == WPAD_CHAN1 && isStarPointer2PTransparencyMode()) {
            return false;
        }

        if (channel == WPAD_CHAN0 && ::getStarPointerOnOffController()->compareMode(StarPointerMode_1PInvalid2PValid)) {
            return false;
        }

        if (!isStarPointerInScreen(channel)) {
            return false;
        }

        TVec2f pos = getStarPointerScreenPositionOrEdge(channel);
        if (pActor->getLayoutManager()->isPointing(pLayoutName, pos)) {
            ::onReaction(reinterpret_cast< u64 >(pActor) + reinterpret_cast< u64 >(pLayoutName), channel, enableTouch, false, false);

            if (pStrength != nullptr) {
                if (::getStarPointerLayout(channel)->isChanceToRumble()) {
                    MR::tryRumblePad(pActor, pStrength, channel);
                }
            }
            return true;
        }

        return false;
    }

    bool isStarPointerPointingTarget(const LayoutActor* pActor, const char* pLayoutName, s32 channel, bool enableTouch, const char* pStrength) {
        if (!MR::isConnectedWPad(channel)) {
            return false;
        }

        if (!::getStarPointerLayout(channel)->mIsPointerValid) {
            return false;
        }

        if (channel == WPAD_CHAN1 && isStarPointer2PTransparencyMode()) {
            return false;
        }

        if (channel == WPAD_CHAN0 && ::getStarPointerOnOffController()->compareMode(StarPointerMode_1PInvalid2PValid)) {
            return false;
        }

        if (!isStarPointerInScreen(channel)) {
            return false;
        }

        StarPointerLayoutTarget* target = pActor->mPointingTarget->getTarget(pLayoutName);
        TVec2f pos = getStarPointerScreenPositionOrEdge(channel);
        if (target->isPointing(pos)) {
            ::onReaction(reinterpret_cast< u64 >(pActor) + reinterpret_cast< u64 >(pLayoutName), channel, enableTouch, false, false);

            if (pStrength != nullptr) {
                if (::getStarPointerLayout(channel)->isChanceToRumble()) {
                    MR::tryRumblePad(pActor, pStrength, channel);
                }
            }
            return true;
        }

        return false;
    }

    bool isStarPointerPointing1P(const LiveActor* pActor, const char* pStrength, bool enableTouch, bool disableShoot) {
        StarPointerTargetInfo info(pActor->mStarPointerTarget, nullptr, nullptr);

        if (::isStarPointerPointingCore(&info, pActor, WPAD_CHAN0, ::checkPointingTarget, ::always, enableTouch, disableShoot, false)) {
            if (pStrength != nullptr && ::getStarPointerLayout(WPAD_CHAN0)->isChanceToRumble()) {
                MR::tryRumblePad(pActor, pStrength, WPAD_CHAN0);
            }
            return true;
        }
        return false;
    }

    bool isStarPointerPointing1PWithoutCheckZ(const LiveActor* pActor, const char* pStrength, bool enableTouch, bool disableShoot) {
        StarPointerTargetInfo info(pActor->mStarPointerTarget, nullptr, nullptr);

        if (::isStarPointerPointingCore(&info, pActor, WPAD_CHAN0, ::checkPointingWithoutCheckZ, ::always, enableTouch, disableShoot, false)) {
            if (pStrength != nullptr && ::getStarPointerLayout(WPAD_CHAN0)->isChanceToRumble()) {
                MR::tryRumblePad(pActor, pStrength, WPAD_CHAN0);
            }
            return true;
        }
        return false;
    }

    bool isStarPointerPointing2P(const LiveActor* pActor, const char* pStrength, bool enableTouch, bool disableShoot) {
        StarPointerTargetInfo info(pActor->mStarPointerTarget, nullptr, nullptr);

        if (::isStarPointerPointingCore(&info, pActor, WPAD_CHAN1, ::checkPointingTarget, ::always, enableTouch, disableShoot, false)) {
            if (pStrength != nullptr && ::getStarPointerLayout(WPAD_CHAN1)->isChanceToRumble()) {
                MR::tryRumblePad(pActor, pStrength, WPAD_CHAN1);
            }
            return true;
        }
        return false;
    }

    bool isStarPointerPointing2POnPressButton(const LiveActor* pActor, const char* pStrength, bool enableTouch, bool disableShoot) {
        StarPointerTargetInfo info(pActor->mStarPointerTarget, nullptr, nullptr);

        if (::isStarPointerPointingCore(&info, pActor, WPAD_CHAN1, ::checkPointingTarget, MR::testCorePadButtonA, enableTouch, disableShoot, false)) {
            if (pStrength != nullptr && ::getStarPointerLayout(WPAD_CHAN1)->isChanceToRumble()) {
                MR::tryRumblePad(pActor, pStrength, WPAD_CHAN1);
            }
            return true;
        }
        return false;
    }

    bool isStarPointerPointing2POnTriggerButton(const LiveActor* pActor, const char* pStrength, bool enableTouch, bool disableShoot) {
        StarPointerTargetInfo info(pActor->mStarPointerTarget, nullptr, nullptr);

        if (::isStarPointerPointingCore(&info, pActor, WPAD_CHAN1, ::checkPointingTarget, MR::testCorePadTriggerA, enableTouch, disableShoot,
                                        false)) {
            if (pStrength != nullptr) {
                MR::tryRumblePad(pActor, pStrength, WPAD_CHAN1);
            }
            return true;
        }

        return false;
    }

    bool isStarPointerPointing2PAttachTarget(const LiveActor* pActor) {
        return MR::attachSupportTicoToTarget(pActor);
    }

    bool isStarPointerPointingFileSelect(const LiveActor* pActor) {
        StarPointerTargetInfo info(pActor->mStarPointerTarget, nullptr, nullptr);
        return ::isStarPointerPointingCore(&info, pActor, WPAD_CHAN0, ::checkPointingWithoutCheckZ, ::always, true, false, false);
    }

    bool isStarPointerPointing1Por2P(const LiveActor* pActor, const char* pStrength, bool enableTouch, bool disableShoot) {
        StarPointerTargetInfo info(pActor->mStarPointerTarget, nullptr, nullptr);

        if (::isStarPointerPointingCore(&info, pActor, WPAD_CHAN0, ::checkPointingTarget, ::always, enableTouch, disableShoot, false)) {
            if (pStrength != nullptr && ::getStarPointerLayout(WPAD_CHAN0)->isChanceToRumble()) {
                MR::tryRumblePad(pActor, pStrength, WPAD_CHAN0);
            }
            return true;

        } else if (::isStarPointerPointingCore(&info, pActor, WPAD_CHAN1, ::checkPointingTarget, ::always, enableTouch, disableShoot, false)) {
            if (pStrength != nullptr && ::getStarPointerLayout(WPAD_CHAN1)->isChanceToRumble()) {
                MR::tryRumblePad(pActor, pStrength, WPAD_CHAN1);
            }
            return true;
        }
        return false;
    }

    bool requestBlueStarGuidance() {
        if (MR::isInAreaObj("BlueStarGuidanceArea", *MR::getPlayerPos()) && !MR::isSystemTalking()) {
            return ::getStarPointerDirector()->mGuidance->request1PGuidance("PointerGuidance_BlueStar", true);
        }

        return false;
    }




    // Original pool string at 80669944 has no surviving reference, likely from code stripped from the shipped build.
    char sTicoSeedGuidanceArea[] = "TicoSeedGuidanceCube";

    bool requestTicoFatGuidance() {
        if (MR::isDemoActive()) {
            return false;
        }

        return ::getStarPointerDirector()->mGuidance->request1PGuidance("PointerGuidance_TicoFat", true);
    }

    // Original pool string at 80669978 has no surviving reference, likely from code stripped from the shipped build.
    char sBigBubbleGuidance[] = "PointerGuidance_BigBubble";

    bool requestMarioLauncherGuidance() {
        return ::getStarPointerDirector()->mGuidance->request1PGuidance("PointerGuidance_MarioLauncher", true);
    }

    bool requestFileSelectGuidance() {
        return ::getStarPointerDirector()->mGuidance->request1PGuidance("System_FileSelect008", true);
    }




    // Original pool strings at 806699CC/806699E4 have no surviving references, likely from code stripped from the shipped build.
    char sFileSelectCopyGuidance[] = "System_FileSelect002";
    char sStarPieceLectureGuidance[] = "PointerGuidance_StarPieceLecture";

    bool requestYoshiEatGuidance(bool isShot) {
        if (!MR::isPlayerElementModeYoshi()) {
            return false;
        }

        if (!MR::isInAreaObj("YoshiEatGuidanceArea", *MR::getPlayerPos())) {
            return false;
        }

        if (MR::isSubjectiveCameraOnForObjClipping()) {
            return false;
        }

        if (isShot) {
            if (!MR::isYoshiTongueReadyToEat()) {
                return false;
            }
            return ::getStarPointerDirector()->mGuidance->request1PGuidance("PointerGuidance_YoshiShot", true);
        }

        return ::getStarPointerDirector()->mGuidance->request1PGuidance("PointerGuidance_YoshiEat", true);
    }

    bool requestYoshiCaptureGuidance() {
        if (!MR::isPlayerElementModeYoshi()) {
            return false;
        }

        if (!MR::isInAreaObj("YoshiCaptureGuidanceArea", *MR::getPlayerPos())) {
            return false;
        }

        if (MR::isSubjectiveCameraOnForObjClipping()) {
            return false;
        }

        return ::getStarPointerDirector()->mGuidance->request1PGuidance("PointerGuidance_YoshiCapture", true);
    }

    bool isExistStarPointerGuidance() {
        return ::getStarPointerDirector()->mGuidance->isExistGuidanceOrFrame();
    }

    bool isExistStarPointerGuidanceFrame1P() {
        return ::getStarPointerDirector()->mGuidance->isExistFrame1P();
    }

    void activeStarPointerGuidance() {
        if (::getStarPointerDirector()->mGuidance != nullptr) {
            ::getStarPointerDirector()->mGuidance->active();
        }
    }

    void deactiveStarPointerGuidance() {
        if (::getStarPointerDirector()->mGuidance != nullptr) {
            ::getStarPointerDirector()->mGuidance->deactive();
        }
    }


    bool isExistStarPointerTarget(const LiveActor* pActor) {
        return pActor->mStarPointerTarget != nullptr;
    }

    void setStarPointerTargetRadius3d(LiveActor* pActor, f32 radius) {
        pActor->mStarPointerTarget->mRadius3d = radius;
    }

    bool isStarPointerPointing(const TVec3f& rPos, f32 radius, s32 channel, bool enableTouch) {
        f32 r = getStarPointerRadius(channel);
        StarPointerTarget target(radius, &rPos, nullptr, TVec3f(0.0f, 0.0f, 0.0f));

        StarPointerInput* controller = ::getStarPointerController(channel);

        if (!::getStarPointerLayout(channel)->mIsPointerValid) {
            return false;
        }

        if (!controller->isInScreen()) {
            return false;
        }

        StarPointerLayout* layout = ::getStarPointerLayout(channel);
        if (layout == nullptr) {
            return false;
        }

        if (target.isPointing(controller->getScreenPosition(), controller->getViewDistanceZ(), layout->getRadius())) {
            ::onReaction(reinterpret_cast< u64 >(&rPos), channel, enableTouch, false, false);
            return true;
        }

        return false;
    }

    MtxPtr getStarPointerViewMtx() {
        return ::getStarPointerDirector()->mTransHolder->mViewMtx;
    }

    Mtx44Ptr getStarPointerProjMtx() {
        return ::getStarPointerDirector()->mTransHolder->mProjMtx;
    }

    f32 getStarPointerRadius(s32 channel) {
        if (::getStarPointerLayout(channel) == nullptr) {
            return 10.0f;
        }
        return ::getStarPointerLayout(channel)->mRadius;
    }

    const TVec2f* getStarPointerScreenPosition(s32 channel) {
        return &::getStarPointerController(channel)->getScreenPosition();
    }

    TVec2f getStarPointerScreenPositionOrEdge(s32 channel) {
        TVec2f pos(::getStarPointerController(channel)->getScreenPosition());
        StarPointerFunction::forceInsideScreenEdge(&pos);
        return pos;
    }

    f32 getStarPointerScreenSpeed(s32 channel) {
        return ::getStarPointerController(channel)->getScreenSpeed();
    }

    bool isStarPointerInScreen(s32 channel) {
        return ::getStarPointerController(channel)->isInScreen();
    }

    bool isStarPointerInScreenAnyPort(s32* pInScreenChannel) {
        for (s32 channel = 0; channel < StarPointerFunction::getNumStarPointer(); channel++) {
            if (isStarPointerInScreen(channel)) {
                if (pInScreenChannel != nullptr) {
                    *pInScreenChannel = channel;
                }
                return true;
            }
        }
        if (pInScreenChannel != nullptr) {
            *pInScreenChannel = -1;
        }
        return false;
    }

    s32* getStarPointerLastPointedPort(const LiveActor* pActor) {
        return &pActor->mStarPointerTarget->mLastPointedChannel;
    }

    const TVec3f& getStarPointerTargetOffset(const LiveActor* pActor) {
        return pActor->mStarPointerTarget->mOffset;
    }

    MtxPtr getStarPointerTargetMtx(const LiveActor* pActor) {
        return pActor->mStarPointerTarget->mMtx;
    }

    const TVec2f* getStarPointerScreenVelocity(s32 channel) {
        return &::getStarPointerController(channel)->getScreenVelocity();
    }


    const TVec3f* getStarPointerWorldPosUsingDepth(s32 channel) {
        return &::getStarPointerController(channel)->getWorldPosition();
    }

    void calcStarPointerWorldPointingPos(TVec3f* pPos, const TVec3f& rPos, s32 channel) {
        f32 camZ = MR::calcCameraDistanceZ(rPos);
        MR::calcWorldPositionFromScreen(pPos, ::getStarPointerController(channel)->getScreenPosition(), camZ);
    }


    bool calcStarPointerPosOnPlane(TVec3f* pPos, const TVec3f& rPlaneBasePos, const TVec3f& rPlaneNorm, s32 channel, bool clampEdge) {
        ::getStarPointerController(channel);  // something stripped

        TVec2f screenPos;
        if (clampEdge) {
            screenPos = getStarPointerScreenPositionOrEdge(channel);
        } else {
            screenPos.set(*getStarPointerScreenPosition(channel));
        }

        TVec3f camPos = MR::getCamPos();
        TVec3f pointRay;
        MR::calcWorldPositionFromScreen(&pointRay, screenPos, -1.0f);
        pointRay.sub(camPos);
        MR::normalize(&pointRay);

        TVec3f camNorm = -MR::getCamZdir();
        TVec3f planeNorm(rPlaneNorm);
        if (camNorm.dot(rPlaneNorm) < 0.0f) {
            planeNorm.set(-rPlaneNorm);
        }
        if (MR::checkHitSemilinePlane(pPos, camPos, pointRay, rPlaneBasePos, planeNorm) == true) {
            return true;
        }

        TVec3f forceIntersectPos;
#ifdef GALAXY_NATIVE
        if (!MR::checkHitSemilinePlane(&forceIntersectPos, camPos, pointRay, camPos + camNorm.scaleInline(-50000.0f), camNorm)) {
            pPos->set(rPlaneBasePos);
            return false;
        }
#else
        MR::checkHitSemilinePlane(&forceIntersectPos, camPos, pointRay, camPos + camNorm.scaleInline(-50000.0f), camNorm);
#endif

        TVec3f forceNorm;
        TVec3f planeOffset = rPlaneBasePos - forceIntersectPos;
        if (planeOffset.dot(planeNorm) < 0.0f) {
            forceNorm = -planeNorm;
        } else {
            forceNorm = planeNorm;
        }
#ifdef GALAXY_NATIVE
        if (!MR::checkHitSemilinePlane(pPos, forceIntersectPos, forceNorm, rPlaneBasePos, planeNorm)) {
            pPos->set(rPlaneBasePos);
            return false;
        }
#else
        MR::checkHitSemilinePlane(pPos, forceIntersectPos, forceNorm, rPlaneBasePos, planeNorm);
#endif
        return true;
    }

    bool calcStarPointerWorldVelocityDirectionOnPlane(TVec3f* pDir, const TVec3f& rPlaneBasePos, const TVec3f& rPlaneNorm, s32 channel) {
        StarPointerInput* controller = ::getStarPointerController(channel);
        TVec3f camPos = MR::getCamPos();
        const TVec2f& posA = controller->getScreenPosition();
        TVec2f posB = posA;
        posB.sub(controller->getScreenVelocity());
        TVec3f toPointA, toPointB;
        MR::calcWorldPositionFromScreen(&toPointA, posA, -1.0f);
        MR::calcWorldPositionFromScreen(&toPointB, posB, -1.0f);
        toPointA.sub(camPos);
        toPointB.sub(camPos);
        MR::normalize(&toPointA);
        MR::normalize(&toPointB);

        f32 projA = toPointA.dot(rPlaneNorm);
        if (MR::isNearZero(projA)) {
            return false;
        }

        f32 projB = toPointB.dot(rPlaneNorm);
        if (MR::isNearZero(projB)) {
            return false;
        }

        TVec3f camDiff = rPlaneBasePos;
        camDiff.sub(camPos);
        f32 projCam = rPlaneNorm.dot(camDiff);

        f32 distA = projCam / projA;
        f32 distB = projCam / projB;

        TVec3f planePosA, planePosB;
        planePosA.set(camPos);
        TVec3f offsetA(toPointA);
        offsetA.scale(distA);
        planePosA.add(offsetA);
        planePosB.set(camPos);
        TVec3f offsetB(toPointB);
        offsetB.scale(distB);
        planePosB.add(offsetB);
        pDir->set(planePosA);
        pDir->sub(planePosB);
        MR::normalizeOrZero(pDir);
        return true;
    }

    // NONMATCHING: MR::tan is inline in MathUtil.hpp; target calls the out-of-line tan__2MRFf (auto_03_8001C190)
    f32 calcPointRadius2D(const TVec3f& rPosition, f32 radius) {
        f32 fovyRad = MR::toRadian(MR::getFovy());
        f32 tan = MR::tan(fovyRad * 0.5f);
        f32 focalDist = (static_cast< s32 >(MR::getScreenHeight()) * 0.5f) / tan;

        TVec3f viewPos;
        MR::getCameraViewMtx().mult(rPosition, viewPos);

        return radius * focalDist / -viewPos.z;
    }


    bool calcStarPointerStrokeRotateMoment(TVec3f* pMoment, const TVec3f& rPosition, f32 radius, s32 channel) {
        if (!isStarPointerInScreen(channel)) {
            pMoment->zero();
            return false;
        }

        TVec2f screenVel = ::getStarPointerController(channel)->getScreenVelocity();

        f32 rad2D = MR::calcPointRadius2D(rPosition, radius);
        screenVel.scale(1.0f / rad2D);

        f32 speed = screenVel.length();
        if (speed < 0.00726f) {
            pMoment->zero();
            return false;
        }
        if (speed > 0.726f) {
            screenVel.scale(0.726f / speed);
        }

        TVec2f screenPos;
        screenPos.set(::getStarPointerController(channel)->getScreenPosition());

        TVec2f pos2D;
        MR::calcScreenPosition(&pos2D, rPosition);

        f32 dist;
        TVec2f dir;
        MR::separateScalarAndDirection(&dist, &dir, screenPos - pos2D);

        f32 ratio = dist / rad2D;
        if (ratio > 1.0f) {
            pMoment->zero();
            return false;
        }
        ratio = MR::clamp(ratio, 0.0f, 1.0f);

        TVec3f moment;
        moment.set< f32 >(screenVel.y, screenVel.x, 0.0f);

        TVec3f stroke;
        stroke.x = dir.x * ratio;
        stroke.y = -dir.y * ratio;
        stroke.z = MR::sqrt(1.0f - ratio * ratio);

        TQuat4f rot;
        TVec3f front;
        front.x = 0.0f;
        front.y = 0.0f;
        front.z = 1.0f;
        rot.setRotate(front, stroke);
        rot.rotate(moment);

        MR::getCameraInvViewMtx().mult33(moment, *pMoment);
        return true;
    }

    bool calcStarPointerScreenDistanceToTarget(const LiveActor* pActor, f32* pDistance, s32 channel) {
        TVec2f screenPos;
        if (!pActor->mStarPointerTarget->calcScreenPosition(&screenPos)) {
            return false;
        }

        StarPointerInput* controller = ::getStarPointerController(channel);
        if (!::getStarPointerLayout(channel)->mIsPointerValid) {
            return false;
        }

        if (!controller->isInScreen()) {
            return false;
        }

        TVec2f targetPos(controller->getScreenPosition());
        screenPos -= targetPos;
        *pDistance = screenPos.length();
        return true;
    }




    void startStarPointerModeTitle(void* pRequester) {
        ::getStarPointerOnOffController()->setStateToTitle(pRequester);
    }

    void startStarPointerModeFileSelect(void* pRequester) {
        ::getStarPointerOnOffController()->setStateToFileSelect(pRequester);
    }

    void startStarPointerModeGame(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_Game);
    }

    void startStarPointerModeDemo(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_Demo);
    }

    void startStarPointerModeDemoWithStarPointer(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_DemoWithStarPointer);
    }

    void startStarPointerModeDemoWithHandPointerFinger(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_DemoWithHandPointerFinger);
    }

    void startStarPointerModeDemoMarioDeath(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_DemoMarioDeath);
    }

    void startStarPointerModeMarioLauncher(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_MarioLauncher);
    }

    void startStarPointerModeHomeButton(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_HomeButton);
    }

    void startStarPointerModeChooseYesNo(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_ChooseYesNo);
    }

    void startStarPointerModePauseMenu(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_PauseMenu);
    }

    void startStarPointerModeScenarioSelectScene(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_ScenarioSelectScene);
    }

    void startStarPointerModeBlueStar(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_BlueStar);
    }

    void startStarPointerModePowerStarGetDemo(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_PowerStarGetDemo);
    }

    void startStarPointerModeStarPieceTarget(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_StarPieceTarget);
    }


    void startStarPointerModeSphereSelectorOnReaction(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_SphereSelectorFinger);
    }

    void startStarPointerModeEnding(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_Base);
    }



    void startStarPointerModeCommandStream(LiveActor* pActor) {
        ::getStarPointerOnOffController()->incModeCounter(pActor, StarPointerMode_CommandStream);
    }

    void startStarPointerModePictureBook(void* pRequester) {
        ::getStarPointerOnOffController()->incModeCounter(pRequester, StarPointerMode_PictureBook);
    }

    void endStarPointerMode(void* pRequester) {
        ::getStarPointerOnOffController()->popState(pRequester);
    }

    void requestStarPointerModeErrorWindow(void* pRequester) {
        ::getStarPointerOnOffController()->requestMode(pRequester, StarPointerMode_ErrorWindow);
    }

    void requestStarPointerModeSaveLoad(void* pRequester) {
        ::getStarPointerOnOffController()->requestMode(pRequester, StarPointerMode_SaveLoad);
    }



    void requestStarPointerModeBlueStarReady(void* pRequester) {
        ::getStarPointerOnOffController()->requestMode(pRequester, StarPointerMode_BlueStarReady);
    }

    void requestStarPointerModeCommandStream(void* pRequester) {
        ::getStarPointerOnOffController()->requestMode(pRequester, StarPointerMode_CommandStream);
    }



    bool isStarPointerModeBlueStarReady() {
        return ::getStarPointerOnOffController()->compareMode(StarPointerMode_BlueStarReady);
    }

    bool isStarPointerModeStarPieceTarget() {
        return ::getStarPointerOnOffController()->compareMode(StarPointerMode_StarPieceTarget);
    }

    bool isStarPointerModeMarioLauncher() {
        return ::getStarPointerOnOffController()->isMode(StarPointerMode_MarioLauncher);
    }

    bool isStarPointerModeHomeButton() {
        return ::getStarPointerOnOffController()->isMode(StarPointerMode_HomeButton);
    }

    bool isStarPointerModeErrorWindow() {
        return ::getStarPointerOnOffController()->isMode(StarPointerMode_ErrorWindow);
    }

};  // namespace MR

namespace StarPointerUtil {
    bool beginMovePositionRegistration(const void* pOwner, bool useSubPad) {
        if (::getStarPointerDirector()->mMovePositionHolders->mOwner == pOwner) {
            return false;
        }

        ::getStarPointerDirector()->mMovePositionHolders->beginRegistration(pOwner, useSubPad);
        return true;
    }

    void endMovePositionRegistration(const void*) {
        ::getStarPointerDirector()->mMovePositionHolders->requestMovement();
    }

    void enableMovePositionInput() {
        ::getStarPointerDirector()->mMovePositionHolders->mRegistered = true;
    }

    void disableMovePositionInput() {
        ::getStarPointerDirector()->mMovePositionHolders->mRegistered = false;
    }

    namespace {
        TVec2f sDefaultButtonOffset(60.0f, 20.0f);
    }

    TVec2f* getDefaultButtonOffsetVec2() {
        return &sDefaultButtonOffset;
    }

    void addStarPointerMovePosition(const char* pName, TVec2f* pPosition, const TVec2f& rOffset) {
        ::getStarPointerDirector()->mMovePositionHolders->addMovePosition(pName, pPosition, rOffset);
    }

    // NONMATCHING: stack layout (copy temp offsets swapped vs target)
    void addStarPointerMovePositionFromPane(LayoutActor* pActor, const char* pName, TVec2f* pOffset) {
        TVec2f trans;
        MR::copyPaneTrans(&trans, pActor, pName);
        TVec2f pos(trans);
        ::getStarPointerDirector()->mMovePositionHolders->addMovePosition(pName, &pos, *pOffset);
    }

    void setStarPointerMovePosition(const char* pName, TVec2f* pPosition) {
        ::getStarPointerDirector()->mMovePositionHolders->setPosition(pName, pPosition);
    }

    // NONMATCHING: stack layout (copy temp offsets swapped vs target)
    void refreshMovePositionsFromLayout(LayoutActor* pActor) {
        StarPointerMovePositionHolder* pHolder = ::getStarPointerDirector()->mMovePositionHolders;
        for (s32 i = 0; i < pHolder->mPositionCount; i++) {
            const char* pName = pHolder->getPositionName(i);
            if (MR::isExistPane(pActor, pName)) {
                TVec2f trans;
                MR::copyPaneTrans(&trans, pActor, pName);
                TVec2f pos(trans);
                pHolder->setPosition(pName, &pos);
            }
        }
    }

    void setDefaultAllMovePosition(const char* pName) {
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultLeft(pName);
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultRight(pName);
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultUp(pName);
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultDown(pName);
    }

    void setFirstMovePosition(const char* pName) {
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultLeft(pName);
    }

    void setLastMovePosition(const char* pName) {
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultRight(pName);
    }

    void setFirstAndLastMovePosition(const char* pFirstName, const char* pLastName) {
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultLeft(pFirstName);
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultRight(pLastName);
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultUp(pLastName);
        ::getStarPointerDirector()->mMovePositionHolders->setDefaultDown(pLastName);
    }

    void setConnectionMovePositionRight2Way(const char* pName0, const char* pName1) {
        ::getStarPointerDirector()->mMovePositionHolders->connectRight(pName0, pName1);
    }

    void setConnectionMovePositionDown2Way(const char* pName0, const char* pName1) {
        ::getStarPointerDirector()->mMovePositionHolders->connectDown(pName0, pName1);
    }

    void setConnectionMovePositionRight(const char* pName0, const char* pName1) {
        ::getStarPointerDirector()->mMovePositionHolders->connectRight(pName0, pName1);
        ::getStarPointerDirector()->mMovePositionHolders->connectRight(pName1, pName0);
    }

    void setConnectionMovePositionDown(const char* pName0, const char* pName1) {
        ::getStarPointerDirector()->mMovePositionHolders->connectDown(pName0, pName1);
        ::getStarPointerDirector()->mMovePositionHolders->connectDown(pName1, pName0);
    }
};  // namespace StarPointerUtil

namespace MR {
    void enableStarPointerShootStarPiece() {
        StarPointerDirector* director = ::getStarPointerDirector();
        director->mIsAllowP1StarPieceShot = true;
        director->mIsAllowP2StarPieceShot = true;
    }

    void disableStarPointerShootStarPiece() {
        StarPointerDirector* director = ::getStarPointerDirector();
        director->mIsAllowP1StarPieceShot = false;
        director->mIsAllowP2StarPieceShot = false;
    }

    bool isEnableStarPointerShootStarPiece(s32 channel) {
        StarPointerDirector* director = ::getStarPointerDirector();
        if (channel == WPAD_CHAN0) {
            return director->mIsAllowP1StarPieceShot;
        } else {
            return director->isEnableStarPointerShootStarPiece();
        }
    }

    bool isStarPointer2PTransparencyMode() {
        return ::getStarPointerOnOffController()->compareMode(StarPointerMode_FileSelect) ||
               ::getStarPointerOnOffController()->compareMode(StarPointerMode_ScenarioSelectScene) ||
               ::getStarPointerOnOffController()->compareMode(StarPointerMode_PauseMenu) ||
               ::getStarPointerOnOffController()->compareMode(StarPointerMode_ChooseYesNo) ||
               ::getStarPointerOnOffController()->compareMode(StarPointerMode_SphereSelectorFinger) ||
               ::getStarPointerOnOffController()->compareMode(StarPointerMode_SaveLoad) ||
               ::getStarPointerOnOffController()->compareMode(StarPointerMode_StarPieceTarget) ||
               ::getStarPointerOnOffController()->compareMode(StarPointerMode_MarioLauncher);
    }

    bool isStarPointer1PInvalid2PValidMode() {
        return ::getStarPointerOnOffController()->mMode == StarPointerMode_1PInvalid2PValid;
    }

    void setStarPointerDrawSyncToken() {
        StarPointerDirector* director = ::getStarPointerDirector();
        if (director->getStarPointerLayout(WPAD_CHAN0)->mIsPointerValid || director->getStarPointerLayout(WPAD_CHAN1)->mIsPointerValid) {
            ::getStarPointerDirector()->mPeekZ->setDrawSyncToken();
        }
    }

};  // namespace MR

const TVec2f& StarPointerController::getScreenPosition() const {
    return mPastInfo.mPos;
}

const TVec2f& StarPointerController::getScreenVelocity() const {
    return mScreenVel;
}

const TVec3f& StarPointerController::getWorldVelocity() const {
    return mWorldVel;
}

f32 StarPointerController::getScreenSpeed() const {
    return mScreenSpeed;
}

f32 StarPointerController::getViewDistanceZ() const {
    return mPastInfo.mViewDistZ;
}

s32 StarPointerController::getPastPointCount() const {
    return mPastPointNum;
}

s32 StarPointerController::getNextPastPointCount() const {
    return mNextPastPointNum;
}

const TVec2f& StarPointerController::getPastPosition(s32 index) const {
    return mPastPosition[index];
}

const TVec2f& StarPointerController::getNextPastPosition(s32 index) const {
    return mNextPastPosition[index];
}

const TVec3f& StarPointerController::getWorldPosition() const {
    return mWorldPos;
}
