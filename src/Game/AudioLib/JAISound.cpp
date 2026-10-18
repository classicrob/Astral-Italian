#include "JSystem/JAudio2/JAISound.hpp"
#include "JSystem/JAudio2/JAIAudience.hpp"
#include "JSystem/JAudio2/JAISoundHandles.hpp"
#include "JSystem/JAudio2/JASReport.hpp"
#include "JSystem/JAudio2/JASTrack.hpp"

























bool JAISound::calc_JAISound_() {
    setCalcedOnce(true);
    if (isStopping() && JAISound_tryDie_()) {
        return false;
    }

    bool playing = isPlaying();
    if (playing) {
        mCount++;
    }

    bool paused = mStatus.isPaused();
    if (!paused) {
        mFader.calc();
    }

    bool currentlyPlaying = playing && !paused;
    if (currentlyPlaying) {
        mParams.mMove.calc();
        if (mAudible != nullptr) {
            mAudible->calc();
        }
        if (hasLifeTime()) {
            if (mLifeTime == 0) {
                stop_JAISound_();
            } else {
                mLifeTime--;
            }
        }
    }

    if (mAudience != nullptr && mAudible != nullptr) {
        mPriority = mAudience->calcPriority(mAudible);
        if (mPriority == -1 && !comesBack()) {
            stop_JAISound_();
        }
    } else {
        mPriority = 0;
    }

    return currentlyPlaying;
}




