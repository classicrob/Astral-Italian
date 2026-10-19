#include "JSystem/J3DGraphBase/J3DMaterial.hpp"
#include "JSystem/J3DGraphBase/J3DSys.hpp"

void J3DMaterial::diff(u32 diffFlags) {
    if (j3dSys.getMatPacket()->isEnabled_Diff()) {
        j3dSys.getMatPacket()->beginDiff();

        mTevBlock->diff(diffFlags);
        mIndBlock->diff(diffFlags);
        mPEBlock->diff(diffFlags);
        if (diffFlags & J3DDiffFlag_KonstColor) {
            J3DGDSetGenMode(mTexGenBlock->getTexGenNum(), mColorBlock->getColorChanNum(), mTevBlock->getTevStageNum(),
                            mIndBlock->getIndTexStageNum(), static_cast<GXCullMode>(mColorBlock->getCullMode()));
            J3DGDSetNumTexGens(mTexGenBlock->getTexGenNum());
        }
        mTexGenBlock->diff(diffFlags);
        mColorBlock->diff(diffFlags);

        j3dSys.getMatPacket()->endDiff();
    }
}
