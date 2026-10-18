#pragma once

#include "Game/NameObj/NameObjFactory.hpp"

class JMapInfoIter;

// SMG2 factory for shape-id (model-changeable) plant objects; replaces Petari's MR::getModelChangableObjCreator.
namespace PlantObjFactory {
    CreatorFuncPtr getCreator(const char*);
    NameObj* initChildObj(const JMapInfoIter&, int);
};  // namespace PlantObjFactory
