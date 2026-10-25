#pragma once
namespace PetariNative::App::TapeScene {
// Install the optional pre-read observer before the game boots. Recording and
// replay are the only users; ordinary input has no game-state observer.
void install();
void nextFrame();
bool powerRequestObserved();
}
