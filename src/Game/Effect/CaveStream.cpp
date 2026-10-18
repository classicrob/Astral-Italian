#include "Game/Effect/CaveStream.hpp"

CaveStream::CaveStream(const char* pName) : EffectObjR500F50(pName) {
}

void CaveStream::init(const JMapInfoIter& rIter) {
    SimpleEffectObj::init(rIter);
}

CaveStream::~CaveStream() {
}
