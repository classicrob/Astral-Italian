#include "Game/Player/YoshiFruitCtrl.hpp"
#include "Game/Player/Yoshi.hpp"
#include "Game/Util.hpp"
namespace {
char sGreen[]="Green";
char sBlue[]="Blue";
char sYellow[]="Yellow";
char sRed[]="Red";
const char* sFruitNames[4] = {sGreen, sRed, sBlue, sYellow};
const char* sFruitTransitions[4][4] = {
    {nullptr,"GreenToRed","GreenToBlue","GreenToYellow"},
    {"RedToGreen",nullptr,nullptr,nullptr},
    {"BlueToGreen",nullptr,nullptr,nullptr},
    {"YellowToGreen",nullptr,nullptr,nullptr}
};
}
s32 YoshiFruitCtrl::findFruitIndex(const char* name) {
    for (u32 i=0;i<4;i++) { if (MR::isEqualString(sFruitNames[i],name)) return i; }
    return -1;
}
const char* YoshiFruitCtrl::findFruitTransition(s32 from,s32 to) {
    if (from<0 || to<0 || static_cast<u32>(from)>=4 || static_cast<u32>(to)>=4) return nullptr;
    return sFruitTransitions[from][to];
}
YoshiFruitCtrl::YoshiFruitCtrl(Yoshi* host) : mActor(host),mLightFruitActive(false),_5(false),_8(0),_C(0),_10(false),_11(false),_14(nullptr),_18(nullptr) {
    fn_80415830("Green");
}
void YoshiFruitCtrl::fn_80415630() {
    if (!MR::isEqualString(_14,_18) && MR::isBrkStopped(mActor)) fn_80415830(_18);
    if (_C==1) {
        if (!MR::isEqualString("YoshiDamageElectricDisappear",_18)) fn_80415830("YoshiDamageElectricDisappear");
    } else {
        if (_5) { fn_80415880("Blue"); return; }
        if (mLightFruitActive) { fn_80415880("Yellow"); return; }
        if (mActor->mDashTimer>0) { fn_80415880("Red"); return; }
        if (_10) {
            if (!MR::isEqualString("YoshiBombTimer",_18)) fn_80415830("YoshiBombTimer");
        } else if (_11) {
            if (!MR::isEqualString("YoshiBombTimerFast",_18)) fn_80415830("YoshiBombTimerFast");
        } else {
            switch (_8) {
            case 1: fn_80415880("Red"); return;
            case 2: fn_80415880("Blue"); return;
            case 3: fn_80415880("Yellow"); return;
            default: fn_80415880("Green"); return;
            }
        }
    }
}
void YoshiFruitCtrl::fn_804157E0() { mLightFruitActive=false;_5=false;_8=0;_C=0;fn_80415830("Green"); }
void YoshiFruitCtrl::fn_80415800() { _8=0; }
void YoshiFruitCtrl::fn_80415810() { _C=1; }
void YoshiFruitCtrl::fn_80415820() { _C=0; }
void YoshiFruitCtrl::fn_80415830(const char* name) { _18=name;_14=name;MR::startBrk(mActor,name);mActor->fn_8040CC30(name); }
void YoshiFruitCtrl::fn_80415880(const char* name) {
    if (!MR::isEqualString(name,_18)) {
        s32 to;
        const char* oldName=_18;
        to=findFruitIndex(name);
        const char* transition=findFruitTransition(findFruitIndex(oldName),to);
        if (transition) { _14=transition;MR::startBrk(mActor,transition);mActor->fn_8040CC30(transition); }
        else { _14=name;MR::startBrk(mActor,name);mActor->fn_8040CC30(name); }
        _18=name;
    }
}
