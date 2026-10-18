#pragma once

#include <revolution/types.h>

// Base of all switch event listeners. SMG2 vtable (__vt__25ActorAppearSwitchListener and
// __vt__26SwitchEventFunctorListener): [0]=0, [1]=0, [2]=refresh (0x8), [3]=listenSwitchOnEvent (0xC),
// [4]=listenSwitchOffEvent (0x10), [5]=0. There is no virtual destructor slot.
class SwitchEventListener {
public:
    inline SwitchEventListener() {
    }

    virtual void refresh(bool useOn) {
        if (useOn) {
            listenSwitchOnEvent();
        } else {
            listenSwitchOffEvent();
        }
    }

    virtual void listenSwitchOnEvent() = 0;
    virtual void listenSwitchOffEvent() = 0;
};
