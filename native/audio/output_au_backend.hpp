#pragma once
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <IOKit/pwr_mgt/IOPMLib.h>

namespace PetariNative::AudioOutputAU::Detail {
// Control-only injection seam. No function in this table is reachable from
// render(). Fake-device tests execute the actual create/recover/retire loop.
struct Api {
    decltype(&AudioComponentFindNext) component=AudioComponentFindNext;
    decltype(&AudioComponentInstanceNew) create=AudioComponentInstanceNew;
    decltype(&AudioComponentInstanceDispose) dispose=AudioComponentInstanceDispose;
    decltype(&AudioUnitGetProperty) unitGet=AudioUnitGetProperty;
    decltype(&AudioUnitSetProperty) unitSet=AudioUnitSetProperty;
    decltype(&AudioUnitInitialize) initialize=AudioUnitInitialize;
    decltype(&AudioUnitUninitialize) uninitialize=AudioUnitUninitialize;
    decltype(&AudioOutputUnitStart) start=AudioOutputUnitStart;
    decltype(&AudioOutputUnitStop) stop=AudioOutputUnitStop;
    decltype(&AudioUnitAddPropertyListener) unitListen=AudioUnitAddPropertyListener;
    decltype(&AudioUnitRemovePropertyListenerWithUserData) unitUnlisten=AudioUnitRemovePropertyListenerWithUserData;
    decltype(&AudioObjectGetPropertyData) objectGet=AudioObjectGetPropertyData;
    decltype(&AudioObjectGetPropertyDataSize) objectSize=AudioObjectGetPropertyDataSize;
    decltype(&AudioObjectHasProperty) objectHas=AudioObjectHasProperty;
    decltype(&AudioObjectAddPropertyListener) objectListen=AudioObjectAddPropertyListener;
    decltype(&AudioObjectRemovePropertyListener) objectUnlisten=AudioObjectRemovePropertyListener;
    decltype(&IORegisterForSystemPower) powerRegister=IORegisterForSystemPower;
    decltype(&IODeregisterForSystemPower) powerUnregister=IODeregisterForSystemPower;
    decltype(&IOServiceClose) powerClose=IOServiceClose;
    decltype(&IOAllowPowerChange) powerAllow=IOAllowPowerChange;
};
#ifdef PETARI_OUTPUT_AU_TESTING
bool setApiForTest(const Api&); // refused while a runtime exists
#endif
}
