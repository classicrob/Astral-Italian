#include "Game/Animation/AnmPlayer.hpp"
#include "Game/Util.hpp"
#include "Game/System/ResourceInfo.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/J3DGraphAnimator/J3DModel.hpp>
#include <JSystem/J3DGraphAnimator/J3DModelData.hpp>
#include <JSystem/JGeometry/TMatrix.hpp>

MaterialAnmPlayerBase::MaterialAnmPlayerBase(const ResTable* pResTable, J3DModelData* pModelData) : AnmPlayerBase(pResTable), mModelData(pModelData) {
}

void MaterialAnmPlayerBase::beginDiff() {
    if (mAnmRes != nullptr) {
        reflectFrame();
        attach(mAnmRes, mModelData);
    }
}

void MaterialAnmPlayerBase::endDiff() {
    if (mAnmRes != nullptr) {
        detach(mAnmRes, mModelData);
    }
}

