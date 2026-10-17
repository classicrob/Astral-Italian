#pragma once

#include "Game/AudioLib/AudBgm.hpp"

class AudBgmKeeper {
public:
    enum BgmType {
        BgmType_Single = 0,
        BgmType_Multi = 1,
    };

    /// @brief Creates a new `AudBgmKeeper`.
    AudBgmKeeper();

    ~AudBgmKeeper() {};

    AudBgm* get(BgmType);
    void release(AudBgm*);
    AudSingleBgm* getValidSingleBgm();
    AudMultiBgm* getValidMultiBgm();

    /* 0x000 */ AudSingleBgm mSingleBgm[2];
    /* 0x3C8 */ AudMultiBgm mMultiBgm[2];
    /* 0x7D8 */ u8 mSingleBgmActiveFlags;
    /* 0x7D9 */ u8 mMultiBgmActiveFlags;
};
