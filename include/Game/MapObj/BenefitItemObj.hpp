#pragma once

#include "Game/LiveActor/LiveActor.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include <JSystem/JGeometry/TMatrix.hpp>

class ActorCameraInfo;
class FlashingCtrl;

class BenefitItemObj : public LiveActor {
public:
    /// @brief Creates a new `BenefitItemObj`.
    /// @param pName A pointer to the null-terminated name of the object.
    /// @param pModelName A pointer to the null-terminated name of the model.
    BenefitItemObj(const char* pName, const char* pModelName);

    virtual void init(const JMapInfoIter&);
    virtual void initAfterPlacement();
    virtual void appear();
    virtual void makeActorAppeared();
    virtual void control();
    virtual void calcAndSetBaseMtx();
    virtual bool receiveMsgPlayerAttack(u32, HitSensor*, HitSensor*);
    virtual bool receiveOtherMsg(u32, HitSensor*, HitSensor*);
    virtual void doRotateY();
    virtual void initEscape();
    virtual void doEscape();
    virtual void appearGround(s32 delay);
    virtual void appearThrowUp();
    virtual void appearThrowUpQuestionBox();
    virtual void exeCatch();
    virtual bool tryStartTicoHold();
    virtual bool tryEndTicoHold();
    virtual void attachSupportTico();
    virtual void exeCatchNerve();

    void appearPop();
    void appearHop();
    void appearThrow(const TVec3f&);
    void setNerveShoot();
    void exeShoot();
    void exeWait();
    void exeAppearGround();
    void exeEscape();
    void exePreEscape();
    void exeTicoHold();
    void exeTicoPull();
    bool canGet() const;
    // Petari-only API, kept so src/Game/Boss/TripodBossKinokoOneUp.cpp (not present in SMG2) still compiles.
    inline void setFollowMtx(MtxPtr) {}
    void calcBaseMtxWait();
    void calcBaseMtxTico();
    void calcBaseMtxMove(const TVec3f&);
    void updateUpVec();
    void blendFrontDir() NO_INLINE;
    bool tryStartDemo();
    bool tryEndDemo();
    bool tryVanish();

    /* 0x090 */ LiveActor* mCatchActor;
    /* 0x094 */ TVec3f _94;
    /* 0x0A0 */ TVec3f _A0;
    /* 0x0AC */ TVec3f _AC;
    /* 0x0B8 */ s16 _B8;
    /* 0x0BA */ s16 _BA;
    /* 0x0BC */ s16 _BC;
    /* 0x0BE */ s16 _BE;
    /* 0x0C0 */ s16 _C0;
    /* 0x0C4 */ TVec3f _C4;
    /* 0x0D0 */ TVec3f _D0;
    /* 0x0DC */ u8 _DC;
    /* 0x0DD */ u8 _DD;
    /* 0x0DE */ u8 _DE;
    /* 0x0DF */ u8 _DF;
    /* 0x0E0 */ u8 _E0;
    /* 0x0E4 */ s32 _E4;
    /* 0x0E8 */ TVec3f _E8;
    /* 0x0F4 */ u8 _F4;
    /* 0x0F5 */ u8 _F5;
    /* 0x0F6 */ u8 _F6;
    /* 0x0F7 */ u8 _F7;
    /* 0x0F8 */ u8 _F8;
    /* 0x0F9 */ u8 _F9;
    /* 0x0FA */ u8 _FA;
    /* 0x0FB */ u8 _FB;
    /* 0x0FC */ f32 _FC;
    /* 0x100 */ f32 _100;
    /* 0x104 */ TMtx34f _104;
    /* 0x134 */ const char* mModelName;
    /* 0x138 */ FlashingCtrl* mFlashCtrl;
    /* 0x13C */ ActorCameraInfo* mCameraInfo;
    /* 0x140 */ u32 _140;
    /* 0x144 */ u32 _144;
    /* 0x148 */ TVec3f _148;
};
