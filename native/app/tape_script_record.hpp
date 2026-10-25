#pragma once
// Recorder-only script controls; no scene mutation or replay policy.
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <string>

namespace PetariNative::App::TapeScriptRecord {
struct Pointer { enum Kind { None, Position, Off } kind=None; float x=0,y=0; };
inline bool parsePointer(const std::string& text, Pointer& out) {
    if (text=="Pointer:off") { out={Pointer::Off,0,0};return true; }
    if (text.rfind("Pointer:",0)!=0) return false;
    const char* start=text.c_str()+8;char* middle=nullptr;char* end=nullptr;
    const float x=std::strtof(start,&middle);
    if (middle==start || *middle!=',') return false;
    const float y=std::strtof(middle+1,&end);
    if (end==middle+1 || *end || !std::isfinite(x) || !std::isfinite(y) || x<0 || y<0) return false;
    out={Pointer::Position,x,y};return true;
}
class Tail {
public:
    enum Action { None, Begin, Power, Reject };
    explicit Tail(std::uint64_t seconds=180):mDuration(seconds*1000000000ull) {
        if (seconds<180 || seconds>1200) mFailed=true;
    }
    bool queue() { if (mQueued || mStarted || mDone || mFailed) { mFailed=true;return false; }mQueued=true;return true; }
    Action step(std::uint64_t now,bool controlSeam,bool neutral) {
        if (mFailed) return Reject;
        if (!mQueued && !mStarted) return None;
        if (!neutral || (mStarted && now<mBegin)) {mFailed=true;return Reject;}
        if (!controlSeam || mDone) return None;
        if (mQueued) {mQueued=false;mBeginning=true;return Begin;}
        if (mStarted && now-mBegin>=mDuration) {mDone=true;return Power;}
        return None;
    }
    bool began(std::uint64_t now,bool controlSeam) {
        if (!mBeginning || !controlSeam) {mFailed=true;return false;}
        mBeginning=false;mStarted=true;mBegin=now;return true;
    }
    bool powerAllowed(std::uint64_t now,bool controlSeam) const {
        return !mFailed && mStarted && mDone && controlSeam && now>=mBegin && now-mBegin>=mDuration;
    }
    bool pending() const {return mQueued || mBeginning || (mStarted && !mDone);}
private:
    std::uint64_t mDuration=0,mBegin=0;
    bool mQueued=false,mBeginning=false,mStarted=false,mDone=false,mFailed=false;
};
}
