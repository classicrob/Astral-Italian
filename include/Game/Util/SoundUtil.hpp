#pragma once

#include <revolution.h>
#include "Game/Util/BgmUtil.hpp"
#include <JSystem/JGeometry/TVec.hpp>

class AudAnmSoundObject;
class AudEffectDirector;
class ActorSoundHolder;
class AudSeKeeper;
class AudSoundObject;
class JAISoundHandle;
class JAISoundID;
class AudLifeAlertCounter;
class LiveActor;
class ActionSoundData;

namespace MR {
    JAISoundHandle* fn_8005AF70(JAISoundID, s32, s32, s32);
    JAISoundHandle* fn_8005B100(JAISoundID, s32, s32);
    void startCSSound(s32, const char*, s32, s32, f32);
    void startCSSoundAfterPlayed(s32, s32);
    void fn_800422F0(u32);
    void fn_800885B0(const ActionSoundData*, bool);
    bool isGravityBGMPlaying();
    void startCSSound1P(const char*, s32, f32);
    // SMG2-only (original names unknown; named by address)
    void startPlayFireMorphBGM();
    void fn_8001C280();
    void fn_8005A7F0(LiveActor*, f32);
    AudLifeAlertCounter* fn_8005BC60(s32);

    void setAudioSystemField9A0(s32);
    void startAppearPlanetJingle();
    void setSeVolumeSetLevelForMenu();
    enum BossBgmID {
        BossBgmID_01_A,
        BossBgmID_01_B,
        BossBgmID_02_A,
        BossBgmID_02_B,
        BossBgmID_03_A,
        BossBgmID_03_B,
        BossBgmID_05_A,
        BossBgmID_05_B,
        BossBgmID_Begoman = BossBgmID_01_A,
        BossBgmID_StinkBugA = BossBgmID_01_A,
        BossBgmID_StinkBugB = BossBgmID_01_B,
        BossBgmID_DinoPackunA = BossBgmID_03_A,
        BossBgmID_DinoPackunB = BossBgmID_03_B,
        BossBgmID_DodoryuA = BossBgmID_05_A,
        BossBgmID_DodoryuB = BossBgmID_05_B,
        BossBgmID_OtaKing = BossBgmID_01_A,
        BossBgmID_TombSpiderA = BossBgmID_03_A,
        BossBgmID_TombSpiderB = BossBgmID_03_B,
        BossBgmID_TripodBossA = BossBgmID_02_A,
        BossBgmID_TripodBossB = BossBgmID_02_B,
    };

    enum EventBgmID {
        EventBgmID_Hurry,
        EventBgmID_Chase,
        EventBgmID_Race_01,
        EventBgmID_Race_02,
        EventBgmID_PenguinRace = EventBgmID_Race_01,
        EventBgmID_TeresaRace = EventBgmID_Race_02,
        EventBgmID_Surfing = EventBgmID_Race_01,
    };

    JAISoundHandle* startSystemSE(const char* pName, s32 param2 = -1, s32 param3 = -1);
    JAISoundHandle* startSystemSE(JAISoundID id, s32 param2 = -1, s32 param3 = -1);
    void onDestroySceneKeepAllSound();
    JAISoundHandle* startSystemLevelSE(const char*, s32 param2 = -1, s32 param3 = -1);
    JAISoundHandle* startSystemLevelSE(JAISoundID, s32 param2 = -1, s32 param3 = -1);
    void stopSystemSE(const char* pName, u32 param2 = 0);
    void stopSystemSE(JAISoundID id, u32 param2 = 0);
    JAISoundHandle* startAtmosphereSE(const char* pName, s32 param2 = -1, s32 param3 = -1);
    JAISoundHandle* startAtmosphereLevelSE(const char* pName, s32 param2 = -1, s32 param3 = -1);
    JAISoundHandle* startAtmosphereSE(JAISoundID, s32, s32);
    JAISoundHandle* startSoundObject(AudSoundObject* pSoundObject, const char* pName);
    JAISoundHandle* startSoundObjectLevel(AudSoundObject* pSoundObject, const char* pName, s32 param3 = -1);
    JAISoundHandle* startSoundObjectLevel(AudSoundObject* pSoundObject, JAISoundID id, s32 param3 = -1);
    JAISoundHandle* startSoundObjectLevelParam(AudSoundObject* pSoundObject, const char* pName, s32 param3 = -1, s32 param4 = -1, s32 param5 = -1);
    JAISoundHandle* startSoundObjectLevelParam(AudSoundObject* pSoundObject, JAISoundID id, s32 param3 = -1, s32 param4 = -1, s32 param5 = -1);
    JAISoundHandle* startSound(const LiveActor*, const char*, s32 param3 = -1, s32 param4 = -1);
    void startSoundInCameraDistance(LiveActor*, const char*, s32, s32, f32);  // objects: SMG2 addition
    JAISoundHandle* startSound(const LiveActor*, JAISoundID, s32 param3 = -1, s32 param4 = -1);
    JAISoundHandle* startSoundSeVer(const LiveActor* pActor, const char* pName, s32 param3 = -1, s32 param4 = -1);
    JAISoundHandle* startLevelSound(const LiveActor* pActor, const char* pName, s32 param3 = -1, s32 param4 = -1, s32 param5 = -1);
    JAISoundHandle* startLevelSound(const LiveActor* pActor, JAISoundID id, s32 param3 = -1, s32 param4 = -1, s32 param5 = -1);
    JAISoundHandle* startLevelSoundSeVer(const LiveActor* pActor, const char* pName, s32 param3 = -1, s32 param4 = -1, s32 param5 = -1);
    void stopSound(const LiveActor* pActor, const char* pName, u32 param3 = 0);
    void stopSound(const LiveActor* pActor, JAISoundID id, u32 param3 = 0);
    void setSeVersion(const LiveActor*, u32);
    void releaseSoundHandle(const LiveActor*, const char*);
    bool startBas(const LiveActor*, const char*, bool, f32, f32, bool updateFoot = true);
    void actorSoundMovement(LiveActor*);
    void addJointToSeKeeper(AudSeKeeper*, const char*, const char*);
    void startLevelSoundSeKeeper(AudSeKeeper*, const char*, const char*);
    bool hasME();
    void startSystemME(const char*);
    void updateMapSoundCodeFoot(const LiveActor*);
    void setMapSondCodeGravity(const LiveActor*, s32);
    void startTalkSound(u8, const LiveActor*);
    void startRemixSound(s32, s32, f32);
    s32 getRemixMelodyNoteNum(s32);
    void limitedSound(const char*, s32) NO_INLINE;
    void stopStageBGM(u32);
    s32 unlockStageBGM();
    void setNextStageBGM(const char*);
    void moveVolumeStageBGM(f32, u32);
    void moveVolumeStageBGMForNoteFairy(f32, u32);
    bool isPlayingStageBgm();
    bool isPlayingStageBgmID(u32);
    bool isPlayingStageBgmName(const char*);
    bool isStopOrFadeoutStageBgmID(u32);
    bool isStopOrFadeoutBgmName(const char*);
    bool isPreparedStageBgm();
    void setStageBGMState(s32, u32);
    void setStageBGMStateBit(u32);
    void stopSubBGM(u32);
    void fn_8005BAD0();
    void moveVolumeSubBGM(f32, u32);
    bool isPlayingSubBgmID(u32);
    void playLevelMarioPinchBGM(bool);
    void stopMarioPinchBGMSoon();
    void submitTrigSE();
    void permitTrigSE();
    void submitLevelSE();
    void permitLevelSE();
    void submitSE();
    void permitSE();
    bool isPermitSE();
    void setCubeBgmChangeInvalid();
    bool isCubeBgmChangeInvalid();
    void clearBgmQueue();
    void setMicMtx(MtxPtr, s32);
    const TVec3f& getMicPos();
    f32 getMicDistance(const LiveActor*);
    void setSoundVolumeSetting(s32, u32);
    void recoverSoundVolumeSetting(u32);
    void setAudioEffectType(s32, s32, s32);
    void resetAudioEffect();  // Resets the AudEffectDirector audio effect type.
    void startCSSound(const char*, const char*, s32);
    void startCSSound(const char*, const char*, s32, s32, f32);
    void startCSSound2P(const char*, const char*);
    void startSpinHitSound(const LiveActor*);
    void startBlowHitSound(const LiveActor*);
    void startDPDHitSound();
    void startDPDFreezeLevelSound(const LiveActor*);
    void start2PJumpAssistSound();
    void start2PJumpAssistJustSound();
    void start2PAttackAssistSound();
    void limitedStarPieceHitSound();
    void startCurrentStageBGM(bool);
    JAISoundHandle* startEventBGM(s32);
    JAISoundHandle* startAfterBossBGM();
    // SoundUtil (enemies lane, callees named by address; lead renames symbols)
    void fn_8005A6E0(LiveActor*, const char*, bool, bool, f32, f32);
};  // namespace MR

namespace MR {
    void setSeVolumeSetLevel(s32, s32);
    void setSoundField9A4(bool);
    void stopSeCategories(u32);
}

namespace MR {
    void stopAndStartCSSound1P(const char*, s32, const char*, s32, f32);
    void stopAndStartCSSound2P(const char*, s32, const char*, s32, f32);
}

namespace MR {
    void startCSSound2P(const char*, const char*, s32, f32);
}

namespace MR {
    void startSoundInPlayerDistance(LiveActor*, const char*, s32, s32, f32);
}

namespace MR {
    void stopCSSound(s32, s32);
}

namespace MR {
    u32 getCSSoundID(const char*);
    JAISoundID fn_8005ADD0(const char*);
    // PLACEHOLDER(core): unnamed in auto_03_80041BE0 (wraps AudMeNameConverter::getMeID); resolve when named
    u32 fn_80042230(const char*);
}

namespace MR {
    void startStarGetJingle();
    void startGrandStarGetJingle();
}

namespace MR {
    // SoundUtil helpers, callees named by address (lead renames symbols)
    bool isDestroySceneKeepAllSound();
    void offDestroySceneKeepAllSound();
    // PLACEHOLDER(core): fn_8022CE70 = actor->mSoundHolder->mainSoundObj (0 if no holder); resolve when renamed
    AudAnmSoundObject* getActorSoundObject(const LiveActor*);
    // PLACEHOLDER(core): fn_8022CE90 = count of child sound objects; resolve when renamed
    s32 getActorSoundObjectCount(const LiveActor*);
    // PLACEHOLDER(core): fn_8022CEC0 = i-th child sound object; resolve when renamed
    AudSoundObject* getActorSoundObjectAt(const LiveActor*, s32);
    JAISoundHandle* startSoundObjectParam(AudSoundObject*, JAISoundID, s32, s32);
};  // namespace MR

namespace MR {
};  // namespace MR

namespace MR {
    bool setSoundPositionPtr(const LiveActor*, TVec3f*);
}

namespace MR {
    void startPlayerSoundForYoshiMode(const char*, const char*, s32);
}
namespace MR { void fn_8005AE10(LiveActor*); }

namespace MR { void stopAllSe(bool); }
