#pragma once
namespace PetariNative::Headless {
// The caller supplies the accepted target-ready seam, not any earlier scene.
class ReloadPolicy {
public:
    bool observe(bool ready, bool reloadRequested) {
        if (mReady && reloadRequested) mRequested = true;
        mReady = mReady || ready;
        return mRequested;
    }
private:
    bool mReady = false;
    bool mRequested = false;
};
}
