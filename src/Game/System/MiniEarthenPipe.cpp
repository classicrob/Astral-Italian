#include "Game/System/MiniPartsActors.hpp"
#include "Game/System/WorldMapFunction.hpp"
#include "Game/Util/LiveActorUtil.hpp"

MiniEarthenPipe::MiniEarthenPipe() : ModelObj("ミニ土管", "MiniEarthenPipe", nullptr, -2, -2, -2, false) {
}

MiniEarthenPipe::~MiniEarthenPipe() {
}

bool MiniEarthenPipe::isUsable() const {
    return WorldMapFunction::isPointing(const_cast< MiniEarthenPipe* >(this));
}

bool MiniEarthenPipe::isUsableStrict() const {
    if (MR::isDead(this)) {
        return false;
    }

    WorldMapFunction::approachScale(const_cast< MiniEarthenPipe* >(this), 0.1f, 1.2f);
    return true;
}

void MiniEarthenPipe::control() {
    WorldMapFunction::returnScale(this, 0.05f, 1.0f);
}
