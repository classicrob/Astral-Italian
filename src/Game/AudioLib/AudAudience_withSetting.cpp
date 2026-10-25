#include "Game/AudioLib/AudAudience.hpp"
#include "Game/AudioLib/AudWrap.hpp"

inline void calcDistanceRates(const JAUAudience3DSetting& setting, f32& priorityRate, f32& fxMixRate) {
    priorityRate = setting.mPriorityDownRange / (setting.mFarDistance - setting.mNearDistance);
    fxMixRate = (setting.mMaxFxMix - setting.mMinFxMix) / (setting.mFarDistance - setting.mNearDistance);
}

void AudAudience_withSetting::updateSetting() {
    mFrontDolbyRate = (mSetting.mMinDolby - mSetting.mCenterDolby) / mSetting.mFrontDolbyZ;
    mBehindDolbyRate = (mSetting.mMaxDolby - mSetting.mCenterDolby) / mSetting.mBehindDolbyZ;
    mVolumeRate = (mSetting.mMinVolume - 1.0f) / (mSetting.mFarDistance * mCameraRate - mSetting.mNearDistance);
    calcDistanceRates(mSetting, mPriorityRate, mFxMixRate);
}

AudAudience_withSetting::AudAudience_withSetting() {
    updateSetting();
    mFarCamera = false;
    mCameraRate = 1.0f;
}

f32 AudAudience_withSetting::getCameraRate() const {
    return mCameraRate;
}

namespace AudWrap {
    s32 getWaveBankIdFromBankId(s32 bank) {
        static const s32 sTable[61][2] = {
            {0, 0},     {1, 1},     {10, 50},   {64, 2},    {72, 3},    {66, 4},    {67, 5},    {74, 6},    {65, 7},    {68, 8},
            {80, 9},    {8, 10},    {69, 11},   {81, 12},   {104, 13},  {105, 14},  {106, 15},  {107, 16},  {82, 17},   {108, 18},
            {83, 19},   {84, 20},   {85, 21},   {87, 22},   {9, 23},    {86, 24},   {127, 25},  {88, 26},   {89, 27},   {109, 28},
            {110, 29},  {90, 30},   {91, 31},   {126, 32},  {111, 33},  {70, 34},   {112, 35},  {113, 36},  {114, 37},  {92, 38},
            {93, 39},   {94, 40},   {95, 41},   {96, 42},   {115, 43},  {116, 44},  {97, 45},   {117, 46},  {98, 47},   {118, 48},
            {119, 49},  {125, 51},  {71, 52},   {99, 53},   {120, 54},  {75, 55},   {11, 56},   {78, 57},   {79, 58},   {77, 59},
            {124, 60},
        };
        for (s32 i = 0; i < 61; i++) {
            if (bank == sTable[i][0]) {
                return sTable[i][1];
            }
        }
        return -1;
    }

    bool isWaveArcExcludedFromSoundRequest(s32 waveBank, s32 waveArc) {
        static const s32 sTable[5][2] = {
            {34, 0}, {34, 1}, {34, 2}, {34, 3}, {34, 4},
        };
        for (s32 i = 0; i < 5; i++) {
            if (waveBank == sTable[i][0] && waveArc == sTable[i][1]) {
                return true;
            }
        }
        return false;
    }
}  // namespace AudWrap
