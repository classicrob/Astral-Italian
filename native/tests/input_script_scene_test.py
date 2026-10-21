#!/usr/bin/env python3
"""Compile the real input-script driver with scene/input test doubles; no game/app launch."""
import os
from pathlib import Path
import subprocess
import tempfile
import sys

root = Path(__file__).resolve().parents[2]
fixture = r'''
#include <cassert>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>
#include "smoke.hpp"
#include <petari/input.hpp>
struct TVec3f { float x=0,y=0,z=0; };
constexpr int SceneObj_MarioHolder=0, SceneInitializeState_End=1;
static bool ready=true, mario=true;
static int playerQueries=0;
static TVec3f vec;
struct GameSystemSceneController { bool isSceneInitializeState(int) { return ready; } };
static GameSystemSceneController controller;
struct GameSystem { GameSystemSceneController* mSceneController=&controller; };
static GameSystem systemFixture;
template<class T> struct SingletonHolder { static T* get() { return &systemFixture; } };
namespace MR {
bool isExistSceneObj(int) { ++playerQueries; return mario; }
bool isExistMario() { ++playerQueries; return mario; }
const TVec3f* getPlayerPos() { ++playerQueries; return &vec; }
const TVec3f* getPlayerVelocity() { ++playerQueries; return &vec; }
const TVec3f* getPlayerGravity() { ++playerQueries; return &vec; }
TVec3f getCamPos() { ++playerQueries; return vec; }
TVec3f getCamZdir() { ++playerQueries; return vec; }
bool isOnGroundPlayer() { ++playerQueries; return true; }
bool isPlayerDead() { ++playerQueries; return false; }
}
static std::vector<std::pair<PetariNative::Input::KeyCode,bool>> events;
namespace PetariNative::Input {
void keyEvent(KeyCode k, bool down, bool) { events.emplace_back(k,down); }
void focusChanged(bool) {}
std::string keyName(KeyCode k) { return k==Key::Backspace ? "Backspace" : "W"; }
bool parseKeyName(const std::string& s, KeyCode* k) { if(s=="Backspace") *k=Key::Backspace; else if(s=="W") *k=Key::W; else return false; return true; }
}
namespace PetariNative::App::Smoke {
static Observation obs;
Observation observeGame(bool player, bool) { auto o=obs; o.playerValid=player&&mario; o.pausePermitted=true; return o; }
}
'''
tests = r'''
using namespace PetariNative::App::Smoke;
using namespace PetariNative;
void reset(const char* scene="Game", const char* stage="StarShipGalaxy") {
 auto& s=script(); s=InputScript{}; s.phase=Phase::Run; s.sceneSpan=true;
 s.lastScene=scene; s.lastStage=stage; ready=true; mario=true; events.clear(); playerQueries=0;
 obs=Observation{}; obs.scene=scene; obs.stage=stage; obs.sceneReady=true;
}
void tick() { obs.sceneReady=ready; inputScriptFrame(); }
Line key(int frames=12) { Line l; l.frames=frames; l.keys={Input::Key::Backspace}; return l; }
Line wait(const char* scene, const char* stage="") { Line l; l.waitScene=scene; l.waitStage=stage; return l; }
int main(int argc,char**) {
 if(argc>1) { auto& s=script(); assert(s.lines.size()==2 && s.lines[0].waitScene=="ScenarioSelect" && s.lines[1].keys[0]==Input::Key::Backspace); return 0; }
 reset(); auto& s=script(); s.lines={key(),wait("Game","WorldMap01Galaxy"),Line{3}};
 tick(); assert(inputScriptOwnsSyntheticControls() && events.size()==1 && events.back().second);
 ready=false; tick(); assert(events.size()==2 && !events.back().second && s.line==1 && s.frame==1);
 tick(); assert(events.size()==2 && s.frame==1);
 ready=true; obs.stage="WorldMap01Galaxy"; tick(); assert(s.line==2 && events.size()==2 && s.held.empty());
 reset(); s.lines={key(1),Line{10}}; tick(); ready=false; tick(); assert(s.line==1 && s.frame==1 && events.size()==2);
 reset("ScenarioSelect",""); mario=false; s.lines={key()}; s.log=std::tmpfile(); tick();
 assert(events.size()==1 && playerQueries==0); std::rewind(s.log); char row[256]; assert(std::fgets(row,sizeof row,s.log));
 int commas=0; for(char c:std::string(row)) commas+=c==','; assert(commas==19); std::fclose(s.log); s.log=nullptr;
 reset(); s.lines={wait("ScenarioSelect"),key()}; tick(); assert(s.line==0 && events.empty() && s.frame==0);
 obs.scene="ScenarioSelect"; obs.stage=""; mario=false; tick(); assert(s.line==1 && events.empty()); tick(); assert(events.size()==1);
 reset(); s.sceneSpan=false; s.lines={key()}; tick(); ready=false; tick(); assert(!inputScriptOwnsSyntheticControls() && s.phase==Phase::Done && events.size()==2 && !events.back().second);
 reset(); s.phase=Phase::Wait; s.steadyNeeded=1; s.lines={key()}; tick(); tick(); assert(s.line==0 && events.size()==1 && s.frame==2);
 reset(); s.lines={key(30),wait("ScenarioSelect")}; s.lines[0].keys={Input::Key::Space}; tick();
 auto delayedObserveAUp=[] { if(!inputScriptOwnsSyntheticControls()) Input::keyEvent(Input::Key::Space,false,false); };
 auto before=events.size(); delayedObserveAUp(); assert(events.size()==before && s.held[0]==Input::Key::Space);
 s.line=1; s.lineFrame=0; tick(); assert(inputScriptOwnsSyntheticControls() && s.held.empty());
 before=events.size(); delayedObserveAUp(); assert(events.size()==before);
 reset(); s.sceneSpan=false; s.lines={key(30)}; s.lines[0].keys={Input::Key::Space}; tick();
 before=events.size(); delayedObserveAUp(); assert(events.size()==before+1 && !events.back().second);
 std::puts("PASS delayed observe A-up isolation, guard ownership, disabled control; interrupted release, completed-line preservation, unloaded pause, menu actor isolation, scene guard, legacy stop, initial scene");
}
'''
with tempfile.TemporaryDirectory(prefix='petari-scene-input-') as directory:
    tmp = Path(directory)
    for header in ('Scene/SceneObjHolder', 'System/GameSystem', 'System/GameSystemSceneController', 'Util/CameraUtil', 'Util/PlayerUtil', 'Util/SingletonHolder'):
        p = tmp/'Game'/(header+'.hpp')
        p.parent.mkdir(parents=True, exist_ok=True)
        p.write_text('#pragma once\n')
    source = tmp/'test.cpp'
    source.write_text(fixture+'\n#include "'+str(root/'native/app/input_script_probe.cpp')+'"\n'+tests)
    exe = tmp/'scene-input-test'
    compiler = sys.argv[1] if len(sys.argv)>1 else 'clang++'
    subprocess.run([compiler,'-std=c++17','-I'+str(tmp),'-I'+str(root/'native/include'),'-I'+str(root/'native/app'),str(source),'-o',str(exe),*sys.argv[2:]],check=True)
    env = {k:v for k,v in os.environ.items() if not k.startswith('PETARI_INPUT_SCRIPT')}
    subprocess.run([str(exe)],check=True,env=env)
    tape=tmp/'parse.txt'
    tape.write_text('scene ScenarioSelect\n6 Backspace\n')
    subprocess.run([str(exe),'parse'],check=True,env=dict(env,PETARI_INPUT_SCRIPT=str(tape),PETARI_INPUT_SCRIPT_SCENE_SPAN='1'))
