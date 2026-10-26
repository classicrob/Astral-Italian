#pragma once
// Recording instrument only. Counts the existing PRE-READ phase clock; never
// reads a scene, supplies a sample, requests power, or changes game readiness.
#include "tape_phase.hpp"

namespace PetariNative::App::TapeScriptRecord {
class IslandNormalObservation {
public:
    static constexpr std::uint64_t Packets=14400, MinimumValid=13292;
    enum Action { None, Began, Complete, Reject };
    Action step(const TapePhase::State& s,bool neutral) {
        if (mError) return Reject;
        if (mComplete) return None;
        if (!mStarted && s.stage!="IslandFleetGalaxy") return None;
        if (!mStarted && s.kind!="controller-normal") return None;
        if (s.scene!="Game" || s.stage!="IslandFleetGalaxy" || s.scenario!=1 || s.kind!="controller-normal")
            return fail("target scene/Normal phase changed");
        if (!neutral || (s.valid && s.dead)) return fail("input or death during neutral observation");
        if (!mStarted) {
            if (!s.firstNormal) return fail("missing first controller-Normal marker");
            mStarted=true;mFirst=s.inputFrame;
        } else if (s.firstNormal || mLast==UINT64_MAX || s.inputFrame!=mLast+1)
            return fail("Normal packet clock restarted, duplicated or skipped");
        mLast=s.inputFrame;++mCount;if(s.valid)++mValid;
        if (mCount==Packets) {
            if (mValid<MinimumValid) return fail("insufficient valid target coverage");
            mComplete=true;return Complete;
        }
        return mCount==1?Began:None;
    }
    std::uint64_t count() const {return mCount;}
    std::uint64_t valid() const {return mValid;}
    std::uint64_t first() const {return mFirst;}
    std::uint64_t last() const {return mLast;}
    const char* error() const {return mError;}
private:
    Action fail(const char* why) {mError=why;return Reject;}
    std::uint64_t mFirst=0,mLast=0,mCount=0,mValid=0;
    bool mStarted=false,mComplete=false;
    const char* mError=nullptr;
};
}
