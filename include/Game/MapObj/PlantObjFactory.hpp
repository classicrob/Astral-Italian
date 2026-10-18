#pragma once

#include "Game/NameObj/NameObjFactory.hpp"

class JMapInfoIter;

// SMG2 factory for shape-id (model-changeable) plant objects.
namespace PlantObjFactory {
    template <typename T> NameObj* createNameObj(const char* name) { return new T(name); }
    CreatorFuncPtr getCreator(const char*);
    NameObj* initChildObj(const JMapInfoIter&, int);
}
