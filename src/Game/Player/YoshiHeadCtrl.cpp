#include "Game/Player/YoshiHeadCtrl.hpp"
#include "Game/Player/YoshiModelCtrl.hpp"
namespace {
const f32 sTargetRates[2]={50.0f,0.0f};
const s32 sTargetFrames[2]={30,6};
}
YoshiHeadCtrl::YoshiHeadCtrl(YoshiModelCtrl* model) : mModelCtrl(model),_4(0),mTargetCount(2) {
    for (s32 i=0;i<2;i++) { mEnabled[i]=false;mTargets[i]=nullptr;_E[i]=false; }
}
void YoshiHeadCtrl::fn_8041A9F0() {
    _4=0;mTargetCount=2;
    for (s32 i=0;i<2;i++) { mEnabled[i]=false;mTargets[i]=nullptr;_E[i]=false; }
    mModelCtrl->fn_804180E0();mModelCtrl->fn_80418110();
}
// NONMATCHING: integer and float table-address/load order only.
void YoshiHeadCtrl::fn_8041AA70(s32 index,const TVec3f* target) {
    bool update=true;
    if (_4!=0 && mTargetCount<index) update=false;
    if (!mEnabled[index]) { _4++;mEnabled[index]=true;mTargets[index]=target;_E[index]=false; }
    else { mTargets[index]=target;_E[index]=false; }
    if (update) {
        mModelCtrl->fn_80418110();
        f32 rate = sTargetRates[index];
        s32 frames = sTargetFrames[index];
        mModelCtrl->fn_80418060(mTargets[index],frames,rate);mTargetCount=index;
    }
}
// NONMATCHING: integer and float table-address/load order only.
void YoshiHeadCtrl::fn_8041AB50(s32 index) {
    if (mEnabled[index]) {
        mEnabled[index]=false;_4--;
        if (_4>0) {
            for(s32 i=0;i<2;i++) {
                if(mEnabled[i]==1) {
                    if(_E[i]==1) mModelCtrl->fn_80418100();
                    else {mModelCtrl->fn_80418110();mModelCtrl->fn_80418060(mTargets[i],sTargetFrames[i],sTargetRates[i]);}
                    mTargetCount=index;return;
                }
            }
        }
        mModelCtrl->fn_80418110();mModelCtrl->fn_80418090();
    }
}
void YoshiHeadCtrl::fn_8041AC40(s32 index) {
    if (mEnabled[index]) {
        mEnabled[index]=false;_4--;
        if (_4>0) {
            for(s32 i=0;i<2;i++) {
                if(mEnabled[i]==1) {
                    if(_E[i]==1) mModelCtrl->fn_80418100();
                    else {mModelCtrl->fn_80418110();mModelCtrl->fn_804180B0(mTargets[i],sTargetRates[i]);}
                    mTargetCount=index;return;
                }
            }
        }
        mModelCtrl->fn_80418110();mModelCtrl->fn_804180E0();
    }
}
void YoshiHeadCtrl::fn_8041AD20(s32 index) {
    if (!mEnabled[index]) return;
    _E[index]=true;
    bool update=false;
    if (_4==0 || mTargetCount>=index) update=true;
    if(update) mModelCtrl->fn_80418100();
}
bool YoshiHeadCtrl::fn_8041AD70(s32 index) const { return mEnabled[index]; }
