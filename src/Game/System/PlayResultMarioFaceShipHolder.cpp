#include "Game/System/PlayResultMarioFaceShipHolder.hpp"

PlayResultMarioFaceShipHolder::PlayResultMarioFaceShipHolder() : NameObj("\x83v\x83\x8c\x83""C\x8c\x8b\x89\xca\x95\xdb\x8e\x9d[\x83}\x83\x8a\x83I\x8a\xe7\x98""f\x90\xaf]") {
    reset();
}

void PlayResultMarioFaceShipHolder::reset() {
    mUnk12 = false;
    mUnk13 = false;
    mUnk14 = false;
    mUnk18 = 0;
    mUnk1C = false;
}

PlayResultMarioFaceShipHolder::~PlayResultMarioFaceShipHolder() {
}
