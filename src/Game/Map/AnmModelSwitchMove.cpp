#include "Game/MapObj/AnmModelObj.hpp"
#include "Game/MapObj/SimpleMapObj.hpp"
#include "Game/Map/ProductMapCreator.hpp"

#pragma push
#pragma dont_inline on
template <> NameObj* ProductMapCreator::createNameObj<SimpleMapObj>(const char* pName) {
    return new SimpleMapObj(pName);
}
#pragma pop
template <> NameObj* ProductMapCreator::createNameObj<AnmModelSwitchMove>(const char* pName) {
    return new AnmModelSwitchMove(pName);
}
template <> NameObj* ProductMapCreator::createNameObj<AnmModelSwitchMoveEndKill>(const char* pName) {
    return new AnmModelSwitchMoveEndKill(pName);
}

AnmModelSwitchMoveEndKill::AnmModelSwitchMoveEndKill(const char* pName) : AnmModelSwitchMove(pName) {
}

AnmModelSwitchMove::~AnmModelSwitchMove() {
}
