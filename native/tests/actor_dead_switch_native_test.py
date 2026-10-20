#!/usr/bin/env python3
"""Test the production kill body with the canonical native StageSwitchCtrl layout."""
import hashlib
import json
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[2]
HARNESS = r'''
#include "Game/Map/StageSwitch.hpp"
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <vector>
static_assert(offsetof(StageSwitchCtrl,mUsesAutoDeadSwitch)==4*sizeof(void*));
static_assert(sizeof(void*)==8);
static std::vector<int> events;
static std::function<void()> validEffect;
StageSwitchCtrl::StageSwitchCtrl(const JMapInfoIter&)
 :mSW_A(nullptr),mSW_B(nullptr),mSW_Appear(nullptr),mSW_Dead(nullptr),mUsesAutoDeadSwitch(false),mSW_Param(nullptr) {}
bool StageSwitchCtrl::isValidSwitchDead() const {
    events.push_back(2); const bool result=mSW_Dead!=nullptr;
    if(validEffect) validEffect(); return result;
}
struct LiveActor {
    StageSwitchCtrl* mStageSwitchCtrl=nullptr;
    void makeActorDead(){events.push_back(1);}
    void kill();
};
namespace MR {void onSwitchDead(LiveActor*){events.push_back(3);}}
BODY
static int checks;
static void check(bool ok){++checks;if(!ok)std::abort();}
int main(){
    alignas(256) unsigned char storage[512]{};
    StageSwitchCtrl ctrl{JMapInfoIter{}},other{JMapInfoIter{}};
    LiveActor actor;
    for(int hasCtrl=0;hasCtrl<2;++hasCtrl) for(int valid=0;valid<2;++valid)
    for(int automatic=0;automatic<2;++automatic) for(int wrongByte=0;wrongByte<2;++wrongByte){
        // These pointer values are stored only; no test dereferences their pointees.
        // Their low byte deliberately disagrees with the actual boolean field.
        ctrl.mSW_Appear=reinterpret_cast<SwitchIdInfo*>(storage+wrongByte);
        ctrl.mSW_Dead=valid?reinterpret_cast<SwitchIdInfo*>(storage):nullptr;
        ctrl.mUsesAutoDeadSwitch=automatic;
        actor.mStageSwitchCtrl=hasCtrl?&ctrl:nullptr;
        events.clear();actor.kill();
        std::vector<int> expected{1};
        if(hasCtrl){expected.push_back(2);if(valid&&automatic)expected.push_back(3);}
        check(events==expected);
    }
    actor.mStageSwitchCtrl=&ctrl;ctrl.mSW_Dead=reinterpret_cast<SwitchIdInfo*>(storage);
    ctrl.mUsesAutoDeadSwitch=false;other.mUsesAutoDeadSwitch=true;
    validEffect=[&]{actor.mStageSwitchCtrl=&other;};
    events.clear();actor.kill();
    check(events==std::vector<int>({1,2,3})); // controller is reloaded after the query
    std::printf("%d production kill/canonical native-layout checks passed; flag offset=%zu\n",
                checks,offsetof(StageSwitchCtrl,mUsesAutoDeadSwitch));
}
'''


def main():
    path=ROOT/"src/Game/LiveActor/LiveActor.cpp"
    source=path.read_text();start=source.index("void LiveActor::kill()")
    end=source.index("\n}",start)+len("\n}");body=source[start:end]
    results=[]
    with tempfile.TemporaryDirectory(prefix="smg2-dead-switch-") as temp:
        temp=Path(temp)
        for mutant in (False,True):
            # Select the preserved Wii statement inside a native-layout build.
            code=body.replace("#ifdef GALAXY_NATIVE","#if 0") if mutant else body
            cpp=temp/("legacy.cpp" if mutant else "native.cpp");cpp.write_text(HARNESS.replace("BODY",code))
            exe=cpp.with_suffix("");cmd=["clang++","-std=c++17","-DGALAXY_NATIVE=1","-O1",
                  "-fsanitize=address,undefined","-fno-sanitize-recover=all","-Wno-register"]
            for directory in ("native/include","include","libs/RVL_SDK/include","libs/JSystem/include","libs/nw4r/include"):
                cmd += ["-I",str(ROOT/directory)]
            subprocess.run(cmd+[str(cpp),"-o",str(exe)],check=True,capture_output=True,text=True)
            result=subprocess.run([str(exe)],capture_output=True,text=True)
            assert (result.returncode!=0)==mutant,result.stderr
            results.append({"legacy_offset":mutant,"exit":result.returncode,"stdout":result.stdout,"stderr":result.stderr})
    print(json.dumps({"source_sha256":hashlib.sha256(path.read_bytes()).hexdigest(),"results":results},indent=2))


if __name__=="__main__":main()
