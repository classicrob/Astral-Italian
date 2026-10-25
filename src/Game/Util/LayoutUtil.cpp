#define NW4R_DEBUG
#define NW4R_DEBUG_OUTLINE
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Effect/MultiEmitter.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Screen/CustomTagProcessor.hpp"
#include "Game/Screen/IconAButton.hpp"
#include "Game/Screen/LayoutCoreUtil.hpp"
#include "Game/Screen/LayoutManager.hpp"
#include "Game/Screen/LayoutPaneCtrl.hpp"
#include "Game/Screen/PaneEffectKeeper.hpp"
#include "Game/Screen/SimpleLayout.hpp"
#include "Game/System/ResourceHolderManager.hpp"
#include "Game/Util/EffectUtil.hpp"
#include "Game/Util/MathUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SingletonHolder.hpp"
#include "Game/Util/StringUtil.hpp"
#include <JSystem/J2DGraph/J2DPicture.hpp>
#include <JSystem/J3DGraphAnimator/J3DAnimation.hpp>
#include <JSystem/JUtility/JUTTexture.hpp>
#include <nw4r/lyt/layout.h>
#include <nw4r/lyt/material.h>
#include <nw4r/lyt/picture.h>
#include <nw4r/lyt/textBox.h>

template nw4r::lyt::TextBox* nw4r::ut::DynamicCast< nw4r::lyt::TextBox*, nw4r::lyt::Pane >(nw4r::lyt::Pane*);

#ifndef GALAXY_NATIVE
// Matching-build instantiation; 4 is the Wii offset of the pane link (natively PaneList uses offsetof).
template const nw4r::lyt::Pane* nw4r::ut::LinkList< nw4r::lyt::Pane, 4 >::ConstIterator::operator->() const;
#endif
template void JGeometry::TVec2< f32 >::set< f32 >(f32, f32);
template void JGeometry::TBox2< f32 >::set(const TVec2f&, const TVec2f&);

#ifndef GALAXY_NATIVE
// Emission-order instantiation for the matching build; the native LinkList iterator differs.
template nw4r::lyt::PaneList::Iterator nw4r::ut::LinkList< nw4r::lyt::Pane, 4 >::Iterator::operator++(int);
#endif

extern "C" int vswprintf(wchar_t*, size_t, const wchar_t*, va_list);


#ifdef GALAXY_NATIVE
#include <petari/asset_diagnostics.hpp>
namespace {
    bool nativeLayoutPaneReady(const LayoutActor* actor, const char* name) {
        if (!actor || !actor->getLayoutManager()) {
            PetariNative::reportMissingLayoutReference("<no layout manager>", name, "pane");
            return false;
        }
        return actor->getLayoutManager()->getPane(name) != nullptr;
    }
}
#endif



namespace {
    void setInfluencedAlphaToChild(nw4r::lyt::Pane* pPane) {
#ifdef GALAXY_NATIVE
        if (!pPane) return;
#endif
        pPane->SetInfluencedAlpha(true);

        nw4r::lyt::PaneList& rChildren = pPane->GetChildList();

        for (nw4r::lyt::PaneList::Iterator it = rChildren.GetBeginIter(); it != rChildren.GetEndIter(); it++) {
            it->SetInfluencedAlpha(true);
        }
    }

    void showPaneRecursive(nw4r::lyt::Pane* pPane) {
#ifdef GALAXY_NATIVE
        if (!pPane) return;
#endif
        pPane->SetVisible(true);

        for (nw4r::lyt::PaneList::Iterator it = pPane->GetChildList().GetBeginIter(); it != pPane->GetChildList().GetEndIter(); ++it) {
            showPaneRecursive(&*it);
        }
    }

    void hidePaneRecursive(nw4r::lyt::Pane* pPane) {
#ifdef GALAXY_NATIVE
        if (!pPane) return;
#endif
        pPane->SetVisible(false);

        for (nw4r::lyt::PaneList::Iterator it = pPane->GetChildList().GetBeginIter(); it != pPane->GetChildList().GetEndIter(); ++it) {
            hidePaneRecursive(&*it);
        }
    }

    bool isExistPaneRecursive(const nw4r::lyt::Pane* pane, const char* name) NO_INLINE {
        if (MR::isEqualString(pane->mName, name)) return true;
        const nw4r::lyt::PaneList& children = pane->GetChildList();
        for (nw4r::lyt::PaneList::ConstIterator it = children.GetBeginIter(); it != children.GetEndIter(); ++it) {
            if (isExistPaneRecursive(&*it, name)) return true;
        }
        return false;
    }

    void initFrameCtrlReverse(J3DFrameCtrl* pFrameCtrl) {
        pFrameCtrl->setAttribute(pFrameCtrl->EMode_RESET);
        pFrameCtrl->setRate(-pFrameCtrl->mRate);
        pFrameCtrl->setFrame(pFrameCtrl->mEnd);
    }

    bool getTextDrawRectRecursive(nw4r::ut::Rect* pRect, const nw4r::lyt::Pane* pPane, bool initialized) {
#ifdef GALAXY_NATIVE
        if (!pPane) return false;
#endif
        const nw4r::lyt::PaneList& rChildren = pPane->GetChildList();

        for (nw4r::lyt::PaneList::ConstIterator it = rChildren.GetBeginIter(); it != rChildren.GetEndIter(); ++it) {
            initialized = getTextDrawRectRecursive(pRect, &*it, initialized);
        }

        const nw4r::lyt::TextBox* pTextBox = nw4r::ut::DynamicCast< const nw4r::lyt::TextBox* >(pPane);

        if (pTextBox != nullptr) {
            nw4r::ut::Rect rect = pTextBox->GetTextDrawRect(nw4r::lyt::DrawInfo());

            if (initialized) {
                if (rect.left < pRect->left) {
                    pRect->left = rect.left;
                }

                if (rect.top > pRect->top) {
                    pRect->top = rect.top;
                }

                if (rect.right > pRect->right) {
                    pRect->right = rect.right;
                }

                if (rect.bottom < pRect->bottom) {
                    pRect->bottom = rect.bottom;
                }
            } else {
                *pRect = rect;
                initialized = true;
            }
        }

        return initialized;
    }

    u32 getTextLineNumMaxRecursiveSub(const nw4r::lyt::Pane* pPane) {
#ifdef GALAXY_NATIVE
        if (!pPane) return 0;
#endif
        u32 max = 0;
        const nw4r::lyt::PaneList& rChildren = pPane->GetChildList();

        for (nw4r::lyt::PaneList::ConstIterator it = rChildren.GetBeginIter(); it != rChildren.GetEndIter(); ++it) {
            u32 count = getTextLineNumMaxRecursiveSub(&*it);

            if (count > max) {
                max = count;
            }
        }

        const nw4r::lyt::TextBox* pTextBox = nw4r::ut::DynamicCast< const nw4r::lyt::TextBox* >(pPane);

        if (pTextBox != nullptr) {
            u32 count = MR::countMessageLine(pTextBox->mTextBuf);

            if (count > max) {
                max = count;
            }
        }

        return max;
    }
};  // namespace

LayoutHolder* MR::createAndAddLayoutHolder(const char* pArcName) {
    return SingletonHolder< ResourceHolderManager >::get()->createAndAddLayoutHolder(pArcName, nullptr);
}

LayoutHolder* MR::createAndAddLayoutHolderRawData(const char* pArcPath) {
    return SingletonHolder< ResourceHolderManager >::get()->createAndAddLayoutHolderRawData(pArcPath);
}

void MR::createAndAddPaneCtrl(LayoutActor* pActor, const char* pPaneName, u32 animLayerNum) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    pActor->getLayoutManager()->createAndAddPaneCtrl(pPaneName, animLayerNum);
}

void MR::createAndAddGroupCtrl(LayoutActor* pActor, const char* pGroupName, u32 animLayerNum) {
    pActor->getLayoutManager()->createAndAddGroupCtrl(pGroupName, animLayerNum);
}

bool MR::isExistPaneCtrl(LayoutActor* pActor, const char* pPaneName) {
    return pActor->getLayoutManager()->isExistPaneCtrl(pPaneName);
}

bool MR::isExistPane(const LayoutActor* pActor, const char* pPaneName) {
    return ::isExistPaneRecursive(pActor->getLayoutManager()->getPane(nullptr), pPaneName);
}


void MR::setInfluencedAlphaToChild(const LayoutActor* pActor) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr)) { return; }
#endif
    ::setInfluencedAlphaToChild(getRootPane(pActor));
}


void MR::setLayoutAlphaFloat(const LayoutActor* pActor, f32 alpha) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr)) { return; }
#endif
    f32 alphaFloat = clamp(alpha, 0.0f, 1.0f);

    nw4r::lyt::Pane* pane = pActor->getLayoutManager()->getPane(nullptr);
    pane->mAlpha = 255.0f * alphaFloat;
}



void MR::executeTextBoxRecursive(LayoutActor* pActor, const char* pPaneName, const TextBoxRecursiveOperation& rOperation) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    nw4r::lyt::TextBox* pTextBox = nw4r::ut::DynamicCast< nw4r::lyt::TextBox* >(getPane(pActor, pPaneName));

    if (pTextBox != nullptr) {
        rOperation.execute(pTextBox);
    }

    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);

    for (nw4r::lyt::PaneList::Iterator it = pPane->GetChildList().GetBeginIter(); it != pPane->GetChildList().GetEndIter(); ++it) {
        executeTextBoxRecursive(pActor, it->mName, rOperation);
    }
}

void MR::setTextBoxGameMessageRecursive(LayoutActor* pActor, const char* pPaneName, const char* pMessageId) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    setTextBoxMessageRecursive(pActor, pPaneName, getGameMessageDirect(pMessageId));
}


void MR::setTextBoxSystemMessageRecursive(LayoutActor* pActor, const char* pPaneName, const char* pMessageId) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    setTextBoxMessageRecursive(pActor, pPaneName, getSystemMessageDirect(pMessageId));
}

void MR::setTextBoxMessageRecursive(LayoutActor* pActor, const char* pPaneName, const wchar_t* pMessage) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    executeTextBoxRecursive(pActor, pPaneName, TextBoxRecursiveSetMessage(pMessage));
}

void MR::setTextBoxFormatRecursive(LayoutActor* pActor, const char* pPaneName, const wchar_t* pFormat, ...) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    wchar_t message[256];
    va_list list;

    va_start(list, pFormat);
    vswprintf(message, ARRAY_SIZE(message), pFormat, list);
    va_end(list);

    setTextBoxMessageRecursive(pActor, pPaneName, message);
}

void MR::setTextBoxArgNumberRecursive(LayoutActor* pActor, const char* pPaneName, s32 number, s32 argIndex) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    executeTextBoxRecursive(pActor, pPaneName, TextBoxRecursiveSetArgNumber(number, argIndex));
}

void MR::setTextBoxArgStringRecursive(LayoutActor* pActor, const char* pPaneName, const wchar_t* pMessage, s32 argIndex) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    executeTextBoxRecursive(pActor, pPaneName, TextBoxRecursiveSetArgString(pMessage, argIndex));
}


void setTextBoxVerticalPositionRecursive(LayoutActor* pActor, const char* pPaneName, u8 position) {
    MR::executeTextBoxRecursive(pActor, pPaneName, TextBoxRecursiveSetVerticalPosition(position));
}


void MR::setTextBoxVerticalPositionCenterRecursive(LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    setTextBoxVerticalPositionRecursive(pActor, pPaneName, 1);
}

void MR::setTextBoxVerticalPositionBottomRecursive(LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    setTextBoxVerticalPositionRecursive(pActor, pPaneName, 2);
}

void setTextBoxHorizontalPositionRecursive(LayoutActor* pActor, const char* pPaneName, u8 position) {
    MR::executeTextBoxRecursive(pActor, pPaneName, TextBoxRecursiveSetHorizontalPosition(position));
}



void MR::updateClearTimeTextBox(LayoutActor* pActor, const char* pPaneName, u32 step) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    wchar_t clearTimeText[16];

    makeClearTimeString(clearTimeText, step);
    setTextBoxMessageRecursive(pActor, pPaneName, clearTimeText);
}

void MR::updateMinuteAndSecondTextBox(LayoutActor* pActor, const char* pPaneName, u32 step) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    wchar_t minuteAndSecondText[16];

    makeMinuteAndSecondString(minuteAndSecondText, step);
    setTextBoxMessageRecursive(pActor, pPaneName, minuteAndSecondText);
}

void MR::setTextBoxFontRecursive(LayoutActor* pActor, const char* pPaneName, nw4r::ut::Font* pFont) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    executeTextBoxRecursive(pActor, pPaneName, TextBoxRecursiveSetFont(pFont));
}

void MR::showPane(LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    getPane(pActor, pPaneName)->SetVisible(true);
}

void MR::showPaneRecursive(LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    ::showPaneRecursive(getPane(pActor, pPaneName));
}

void MR::hidePane(LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    getPane(pActor, pPaneName)->SetVisible(false);
}

void MR::hidePaneRecursive(LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    ::hidePaneRecursive(getPane(pActor, pPaneName));
}

bool MR::isHiddenPane(const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return true; }
#endif
    return !getPane(pActor, pPaneName)->IsVisible();
}

void MR::showScreen(LayoutActor* pActor) {
    LayoutManager* pLayoutManager = pActor->getLayoutManager();

    pLayoutManager->mIsScreenHidden = false;
}

void MR::hideScreen(LayoutActor* pActor) {
    LayoutManager* pLayoutManager = pActor->getLayoutManager();

    pLayoutManager->mIsScreenHidden = true;
}

void MR::setFollowPos(const TVec2f* pFollowPos, const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getPaneCtrl(pPaneName)) { return; }
#endif
    pActor->getLayoutManager()->getPaneCtrl(pPaneName)->mFollowPos = pFollowPos;
}

void MR::setFollowTypeReplace(const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getPaneCtrl(pPaneName)) { return; }
#endif
    LayoutPaneCtrl* pCtrl = pActor->getLayoutManager()->getPaneCtrl(pPaneName);
    pCtrl->mFollowType = 0;
}

void MR::setFollowTypeAdd(const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getPaneCtrl(pPaneName)) { return; }
#endif
    LayoutPaneCtrl* pCtrl = pActor->getLayoutManager()->getPaneCtrl(pPaneName);
    pCtrl->mFollowType = 1;
}

void MR::copyPaneTrans(TVec2f* pTrans, const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { pTrans->zero(); return; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);
    pTrans->x = pPane->mGlbMtx._03;
    pTrans->y = pPane->mGlbMtx._13;
    convertLayoutPosToScreenPos(pTrans, *pTrans);
}

f32 MR::getPaneTransX(const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return 0; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);
    TVec2f trans(pPane->mGlbMtx._03, pPane->mGlbMtx._13);
    convertLayoutPosToScreenPos(&trans, trans);
    return trans.x;
}

f32 MR::getPaneTransY(const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return 0; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);
    TVec2f trans(pPane->mGlbMtx._03, pPane->mGlbMtx._13);
    convertLayoutPosToScreenPos(&trans, trans);
    return trans.y;
}

void MR::setLayoutPosAtPaneTrans(LayoutActor* pActor, const LayoutActor* pFollowActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pFollowActor, pPaneName)) { return; }
#endif
    TVec2f trans;
    copyPaneTrans(&trans, pFollowActor, pPaneName);
    pActor->setTrans(trans);
}

void MR::copyPaneScale(TVec2f* pScale, const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { pScale->set(1.0f, 1.0f); return; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);
    pScale->set< f32 >(pPane->mScale.x, pPane->mScale.y);
}


void MR::setPaneScale(const LayoutActor* pActor, const char* pPaneName, f32 x, f32 y) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    nw4r::math::VEC2 scale(x, y);
    pActor->getLayoutManager()->getPane(pPaneName)->mScale = scale;
}

void MR::setLayoutScaleAtPaneScale(LayoutActor* pActor, const LayoutActor* pFollowActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pFollowActor, pPaneName)) { return; }
#endif
    TVec2f scale;
    copyPaneScale(&scale, pFollowActor, pPaneName);
    setPaneScale(pActor, nullptr, scale.x, scale.y);
}

void MR::copyPaneRotate(TVec3f* pRotate, const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { pRotate->zero(); return; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);
    pRotate->set< f32 >(pPane->mRotate.x, pPane->mRotate.y, pPane->mRotate.z);
}

void MR::setPaneRotate(const LayoutActor* pActor, f32 x, f32 y, f32 z, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    nw4r::math::VEC3 rotate;
    rotate.x = x;
    rotate.y = y;
    rotate.z = z;
    getPane(pActor, pPaneName)->mRotate = rotate;
}

void MR::setLayoutScalePosAtPaneScaleTrans(LayoutActor* pActor, const LayoutActor* pFollowActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pFollowActor, pPaneName)) { return; }
#endif
    setLayoutPosAtPaneTrans(pActor, pFollowActor, pPaneName);
    setLayoutScaleAtPaneScale(pActor, pFollowActor, pPaneName);
}

void MR::setLayoutScalePosAtPaneScaleTransIfExecCalcAnim(LayoutActor* pActor, const LayoutActor* pFollowActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pFollowActor, pPaneName)) { return; }
#endif
    if (isExecuteCalcAnimLayout(pActor)) {
        setLayoutScalePosAtPaneScaleTrans(pActor, pFollowActor, pPaneName);
    }
}

nw4r::lyt::TexMap* MR::createLytTexMap(const char* pArcName, const char* pTextureName) {
    JUTTexture texture(loadTexFromArc(pArcName, pTextureName, nullptr), 0);

    return new nw4r::lyt::TexMap(texture.getTexObj());
}

nw4r::lyt::TexMap* MR::createLytTexMap(ResTIMG* pImage) {
    JUTTexture texture(pImage, 0);

    return new nw4r::lyt::TexMap(texture.getTexObj());
}

nw4r::lyt::TexMap* MR::getLytTexMap(LayoutActor* pActor, const char* pPaneName, u8 textureIndex) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return nullptr; }
#endif
    nw4r::lyt::Picture* pPicture = nw4r::ut::DynamicCast< nw4r::lyt::Picture* >(getPane(pActor, pPaneName));
#ifdef GALAXY_NATIVE
    if (!pPicture || !pPicture->GetMaterial()) return nullptr;
#endif

    return const_cast< nw4r::lyt::TexMap* >(&pPicture->GetMaterial()->GetTexture(textureIndex));
}

void MR::replacePaneTexture(LayoutActor* pActor, const char* pPaneName, const nw4r::lyt::TexMap* pTexture, u8 textureIndex) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    nw4r::lyt::Picture* pPicture = nw4r::ut::DynamicCast< nw4r::lyt::Picture* >(getPane(pActor, pPaneName));
#ifdef GALAXY_NATIVE
    if (!pPicture || !pPicture->GetMaterial() || !pTexture) return;
#endif
    pPicture->GetMaterial()->SetTexture(textureIndex, *pTexture);
}

void MR::startAnim(LayoutActor* pActor, const char* pAnimName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !pActor->getLayoutManager()->getAnimTransform(pAnimName) || !pActor->getLayoutManager()->getPaneCtrl(nullptr)) { return; }
#endif
    pActor->getLayoutManager()->getPaneCtrl(nullptr)->start(pAnimName, animLayer);

    if (pActor->mEffectKeeper != nullptr) {
        pActor->mEffectKeeper->changeAnim();
    }
}

void MR::startAnimAtFirstStep(LayoutActor* pActor, const char* pAnimName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !pActor->getLayoutManager()->getAnimTransform(pAnimName)) { return; }
#endif
    if (isFirstStep(pActor)) {
        startAnim(pActor, pAnimName, animLayer);
    }
}

void MR::startPaneAnim(LayoutActor* pActor, const char* pPaneName, const char* pAnimName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getAnimTransform(pAnimName) || !pActor->getLayoutManager()->getPaneCtrl(pPaneName)) { return; }
#endif
    pActor->getLayoutManager()->getPaneCtrl(pPaneName)->start(pAnimName, animLayer);
}

void MR::startPaneAnimAtStep(LayoutActor* pActor, const char* pPaneName, const char* pAnimName, s32 step, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getAnimTransform(pAnimName)) { return; }
#endif
    if (isStep(pActor, step)) {
        startPaneAnim(pActor, pPaneName, pAnimName, animLayer);
    }
}

void MR::startPaneAnimAtFirstStep(LayoutActor* pActor, const char* pPaneName, const char* pAnimName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getAnimTransform(pAnimName)) { return; }
#endif
    if (isFirstStep(pActor)) {
        startPaneAnim(pActor, pPaneName, pAnimName, animLayer);
    }
}



void MR::startAnimAndSetFrameAndStop(LayoutActor* pActor, const char* pAnimName, f32 animFrame, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !pActor->getLayoutManager()->getAnimTransform(pAnimName)) { return; }
#endif
    startAnim(pActor, pAnimName, animLayer);
    setAnimFrameAndStop(pActor, animFrame, animLayer);
}

void MR::startPaneAnimAndSetFrameAndStop(LayoutActor* pActor, const char* pPaneName, const char* pAnimName, f32 animFrame, u32 animLayer) {
    pActor->getLayoutManager()->getPaneCtrl(pPaneName)->start(pAnimName, animLayer);
    MR::setPaneAnimFrameAndStop(pActor, pPaneName, animFrame, animLayer);
}

void MR::setAnimFrameAndStop(LayoutActor* pActor, f32 animFrame, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !getAnimCtrl(pActor, animLayer)) { return; }
#endif
    J3DFrameCtrl* pFrameCtrl = getAnimCtrl(pActor, animLayer);

    pFrameCtrl->setFrame(animFrame);
    pFrameCtrl->setRate(0.0f);
}

void MR::setAnimFrameAndStopAtEnd(LayoutActor* pActor, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr)) { return; }
#endif
    setAnimFrameAndStop(pActor, getAnimFrameMax(pActor, animLayer), animLayer);
}

void MR::setAnimFrameAndStopAdjustTextWidth(LayoutActor* pActor, const char* pPaneName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);
    nw4r::ut::Rect rect;
    ::getTextDrawRectRecursive(&rect, pPane, false);
    setAnimFrameAndStop(pActor, MR::abs(rect.GetWidth()), animLayer);
}

void MR::setAnimFrameAndStopAdjustTextHeight(LayoutActor* pActor, const char* pPaneName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);
    nw4r::ut::Rect rect;
    ::getTextDrawRectRecursive(&rect, pPane, false);
    setAnimFrameAndStop(pActor, MR::abs(rect.GetHeight()), animLayer);
}

void MR::setPaneAnimFrameAndStop(LayoutActor* pActor, const char* pPaneName, f32 animFrame, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !getPaneAnimCtrl(pActor, pPaneName, animLayer)) { return; }
#endif
    J3DFrameCtrl* pFrameCtrl = getPaneAnimCtrl(pActor, pPaneName, animLayer);

    pFrameCtrl->setFrame(animFrame);
    pFrameCtrl->setRate(0.0f);
}


void MR::setAnimFrame(LayoutActor* pActor, f32 animFrame, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !getAnimCtrl(pActor, animLayer)) { return; }
#endif
    getAnimCtrl(pActor, animLayer)->setFrame(animFrame);
}

void MR::setPaneAnimFrame(LayoutActor* pActor, const char* pPaneName, f32 animFrame, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !getPaneAnimCtrl(pActor, pPaneName, animLayer)) { return; }
#endif
    getPaneAnimCtrl(pActor, pPaneName, animLayer)->setFrame(animFrame);
}

void MR::setAnimRate(LayoutActor* pActor, f32 animRate, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !getAnimCtrl(pActor, animLayer)) { return; }
#endif
    getAnimCtrl(pActor, animLayer)->setRate(animRate);
}

void MR::setPaneAnimRate(LayoutActor* pActor, const char* pPaneName, f32 animRate, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !getPaneAnimCtrl(pActor, pPaneName, animLayer)) { return; }
#endif
    getPaneAnimCtrl(pActor, pPaneName, animLayer)->setRate(animRate);
}

void MR::stopAnim(LayoutActor* pActor, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !pActor->getLayoutManager()->getPaneCtrl(nullptr)) { return; }
#endif
    pActor->getLayoutManager()->getPaneCtrl(nullptr)->stop(animLayer);
}

void MR::stopPaneAnim(LayoutActor* pActor, const char* pPaneName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getPaneCtrl(pPaneName)) { return; }
#endif
    pActor->getLayoutManager()->getPaneCtrl(pPaneName)->stop(animLayer);
}

bool MR::isAnimStopped(const LayoutActor* pActor, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !pActor->getLayoutManager()->getPaneCtrl(nullptr)) { return true; }
#endif
    return pActor->getLayoutManager()->getPaneCtrl(nullptr)->isAnimStopped(animLayer);
}

bool MR::isPaneAnimStopped(const LayoutActor* pActor, const char* pPaneName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getPaneCtrl(pPaneName)) { return true; }
#endif
    return pActor->getLayoutManager()->getPaneCtrl(pPaneName)->isAnimStopped(animLayer);
}

f32 MR::getAnimFrame(const LayoutActor* pActor, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !getAnimCtrl(pActor, animLayer)) { return 0; }
#endif
    return getAnimCtrl(pActor, animLayer)->getFrame();
}

f32 MR::getPaneAnimFrame(const LayoutActor* pActor, const char* pPaneName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !getPaneAnimCtrl(pActor, pPaneName, animLayer)) { return 0; }
#endif
    return getPaneAnimCtrl(pActor, pPaneName, animLayer)->getFrame();
}

s16 MR::getAnimFrameMax(const LayoutActor* pActor, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !getAnimCtrl(pActor, animLayer)) { return 0; }
#endif
    return getAnimCtrl(pActor, animLayer)->getEnd();
}

s16 MR::getPaneAnimFrameMax(const LayoutActor* pActor, const char* pPaneName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !getPaneAnimCtrl(pActor, pPaneName, animLayer)) { return 0; }
#endif
    return getPaneAnimCtrl(pActor, pPaneName, animLayer)->getEnd();
}

s16 MR::getAnimFrameMax(const LayoutActor* pActor, const char* pAnimName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !pActor->getLayoutManager()->getAnimTransform(pAnimName)) { return 0; }
#endif
    return pActor->getLayoutManager()->getAnimTransform(pAnimName)->GetFrameSize();
}

J3DFrameCtrl* MR::getAnimCtrl(const LayoutActor* pActor, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr) || !pActor->getLayoutManager()->getPaneCtrl(nullptr) || !pActor->getLayoutManager()->getPaneCtrl(nullptr)->getFrameCtrl(animLayer)) { return nullptr; }
#endif
    return pActor->getLayoutManager()->getPaneCtrl(nullptr)->getFrameCtrl(animLayer);
}

J3DFrameCtrl* MR::getPaneAnimCtrl(const LayoutActor* pActor, const char* pPaneName, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName) || !pActor->getLayoutManager()->getPaneCtrl(pPaneName) || !pActor->getLayoutManager()->getPaneCtrl(pPaneName)->getFrameCtrl(animLayer)) { return nullptr; }
#endif
    return pActor->getLayoutManager()->getPaneCtrl(pPaneName)->getFrameCtrl(animLayer);
}

void MR::emitEffect(LayoutActor* pActor, const char* pEffectName) {
    pActor->mEffectKeeper->createEmitter(pEffectName);
}

void MR::deleteEffect(LayoutActor* pActor, const char* pEffectName) {
    pActor->mEffectKeeper->deleteEmitter(pEffectName);
}

void MR::forceDeleteEffect(LayoutActor* pActor, const char* pEffectName) {
    pActor->mEffectKeeper->forceDeleteEmitter(pEffectName);
}

void MR::deleteEffectAll(LayoutActor* pActor) {
    pActor->mEffectKeeper->deleteEmitterAll();
}

void MR::forceDeleteEffectAll(LayoutActor* pActor) {
    pActor->mEffectKeeper->forceDeleteEmitterAll();
}

void MR::setEffectHostMtx(LayoutActor* pActor, const char* pParam2, MtxPtr pHostMtx) {
    getEffect(pActor, pParam2)->setHostMtx(pHostMtx);
}

void MR::setEffectRate(LayoutActor* pActor, const char* pEffectName, f32 rate) {
    getEffect(pActor, pEffectName)->setRate(rate, -1);
}

void MR::setEffectDirectionalSpeed(LayoutActor* pActor, const char* pEffectName, f32 speed) {
    getEffect(pActor, pEffectName)->setDirectionalSpeed(speed, -1);
}

void MR::pauseOffEffectAll(LayoutActor* pActor) {
    MultiEmitter* pEmitter;
    PaneEffectKeeper* pKeeper = pActor->mEffectKeeper;

    for (s32 i = 0; i < pKeeper->mEmitters.size(); i++) {
        pEmitter = pKeeper->mEmitters.begin()[i];

        if (pEmitter != nullptr && pEmitter->isValid()) {
            pEmitter->pauseOff(-1);
        }
    }
}

bool MR::isRegisteredEffect(const LayoutActor* pActor, const char* pParam2) {
    if (pParam2 != nullptr) {
        return pActor->mEffectKeeper->getEmitter(pParam2) != nullptr;
    }

    return pActor->mEffectKeeper != nullptr;
}




bool MR::isStep(const LayoutActor* pActor, s32 step) {
    return pActor->getNerveStep() == step;
}

bool MR::isFirstStep(const LayoutActor* pActor) {
    return isStep(pActor, 0);
}

bool MR::isLessStep(const LayoutActor* pActor, s32 step) {
    if (!isNewNerve(pActor) && pActor->getNerveStep() < step) {
        return true;
    }

    return false;
}

bool MR::isGreaterStep(const LayoutActor* pActor, s32 step) {
    return pActor->getNerveStep() > step;
}

bool MR::isGreaterEqualStep(const LayoutActor* pActor, s32 step) {
    return pActor->getNerveStep() >= step;
}

bool MR::isIntervalStep(const LayoutActor* pActor, s32 step) {
    return pActor->getNerveStep() % step == 0;
}

bool MR::isNewNerve(const LayoutActor* pActor) {
    return pActor->getNerveStep() < 0;
}



static inline f32 calcNerveRate(const LayoutActor* pActor, s32 stepMax) {
    return stepMax <= 0 ? 1.0f : MR::clamp(static_cast< f32 >(pActor->getNerveStep()) / stepMax, 0.0f, 1.0f);
}

static inline f32 calcNerveRate(const LayoutActor* pActor, s32 stepMin, s32 stepMax) {
    return MR::clamp(MR::normalize(pActor->getNerveStep(), stepMin, stepMax), 0.0f, 1.0f);
}

f32 MR::calcNerveEaseInRate(const LayoutActor* pActor, s32 stepMax) {
    return getEaseInValue(calcNerveRate(pActor, stepMax), 0.0f, 1.0f, 1.0f);
}

f32 MR::calcNerveEaseInValue(const LayoutActor* pActor, s32 stepMin, s32 stepMax, f32 valueMin, f32 valueMax) {
    return getEaseInValue(calcNerveRate(pActor, stepMin, stepMax), valueMin, valueMax, 1.0f);
}

// NONMATCHING: equivalent early rate-default branch arrangement
f32 MR::calcNerveEaseOutValue(const LayoutActor* pActor, s32 stepMax, f32 valueStart, f32 valueEnd) {
    f32 rate;
    if (stepMax <= 0) {
        rate = 1.0f;
    } else {
        rate = clamp(static_cast< f32 >(pActor->getNerveStep()) / stepMax, 0.0f, 1.0f);
    }

    return getEaseOutValue(rate, valueStart, valueEnd, 1.0f);
}

void MR::setNerveAtStep(LayoutActor* pActor, const Nerve* pNerve, s32 step) {
    if (pActor->getNerveStep() == step) {
        pActor->setNerve(pNerve);
    }
}

void MR::setNerveAtAnimStopped(LayoutActor* pActor, const Nerve* pNerve, u32 animLayer) {
    if (isAnimStopped(pActor, animLayer)) {
        pActor->setNerve(pNerve);
    }
}

void MR::setNerveAtPaneAnimStopped(LayoutActor* pActor, const char* pPaneName, const Nerve* pNerve, u32 animLayer) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    if (isPaneAnimStopped(pActor, pPaneName, animLayer)) {
        pActor->setNerve(pNerve);
    }
}

void MR::killAtAnimStopped(LayoutActor* pActor, u32 animLayer) {
    if (isAnimStopped(pActor, animLayer)) {
        pActor->kill();
    }
}

bool MR::isDead(const LayoutActor* pActor) {
    return pActor->mFlag.mIsDead;
}

bool MR::isHiddenLayout(const LayoutActor* pActor) {
    return pActor->mFlag.mIsHidden;
}

void MR::showLayout(LayoutActor* pActor) {
    pActor->mFlag.mIsHidden = false;
    pActor->mFlag.mIsOffCalcAnim = false;
}

void MR::hideLayout(LayoutActor* pActor) {
    pActor->mFlag.mIsHidden = true;
    pActor->mFlag.mIsOffCalcAnim = true;
}

bool MR::isStopAnimFrame(const LayoutActor* pActor) {
    return pActor->mFlag.mIsStopAnimFrame;
}

void MR::stopAnimFrame(LayoutActor* pActor) {
    pActor->mFlag.mIsStopAnimFrame = true;
}

void MR::releaseAnimFrame(LayoutActor* pActor) {
    pActor->mFlag.mIsStopAnimFrame = false;
}

void MR::onCalcAnim(LayoutActor* pActor) {
    pActor->mFlag.mIsOffCalcAnim = false;
}

void MR::offCalcAnim(LayoutActor* pActor) {
    pActor->mFlag.mIsOffCalcAnim = true;
}

bool MR::isExecuteCalcAnimLayout(const LayoutActor* pActor) {
    if (pActor->mFlag.mIsDead) {
        return false;
    }

    if (pActor->mLayoutManager == nullptr) {
        return false;
    }

    return !pActor->mFlag.mIsOffCalcAnim;
}

bool MR::isExecuteDrawLayout(const LayoutActor* pActor) {
    if (pActor->mFlag.mIsDead) {
        return false;
    }

    if (pActor->mLayoutManager == nullptr) {
        return false;
    }

    return !pActor->mFlag.mIsHidden;
}

SimpleLayout* MR::createSimpleLayout(const char* pName, const char* pArcName, u32 rootPaneAnimLayerNum) {
    SimpleLayout* simpleLayout;

    simpleLayout = new SimpleLayout(pName, pArcName, rootPaneAnimLayerNum, MR::DrawType_Layout);
    simpleLayout->initWithoutIter();

    return simpleLayout;
}

SimpleLayout* MR::createSimpleLayoutTalkParts(const char* pName, const char* pArcName, u32 rootPaneAnimLayerNum) {
    SimpleLayout* simpleLayout;

    simpleLayout = new SimpleLayout(pName, pArcName, rootPaneAnimLayerNum, MR::DrawType_0x4C);
    simpleLayout->initWithoutIter();

    return simpleLayout;
}

nw4r::lyt::Pane* MR::getPane(const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return nullptr; }
#endif
    return pActor->getLayoutManager()->getPane(pPaneName);
}

nw4r::lyt::Pane* MR::getRootPane(const LayoutActor* pActor) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, nullptr)) { return nullptr; }
#endif
    return pActor->getLayoutManager()->getPane(nullptr);
}

void MR::calcTextBoxRectRecursive(TBox2f* pBox, const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { pBox->i.zero(); pBox->f.zero(); return; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);
    nw4r::ut::Rect rect;

    if (::getTextDrawRectRecursive(&rect, pPane, false)) {
        TVec2f min;
        TVec2f max;
        convertPaneLocalPosToScreenPos(&min, pPane, TVec2f(rect.left, rect.top));
        convertPaneLocalPosToScreenPos(&max, pPane, TVec2f(rect.right, rect.bottom));
        pBox->set(min, max);
    } else {
        pBox->i.zero();
        pBox->f.zero();
    }
}

u32 MR::getTextLineNumMaxRecursive(const LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return 0; }
#endif
    nw4r::lyt::Pane* pPane = getPane(pActor, pPaneName);

    if (pPane != nullptr) {
        return ::getTextLineNumMaxRecursiveSub(pPane);
    }

    return 0;
}

void MR::invalidateParentAnim(LayoutActor* pActor) {
    LayoutManager* pLayoutManager = pActor->getLayoutManager();

    pLayoutManager->mUseRecursiveAnimationBinding = 0;
}

namespace {
    f32 getCometColorAnimFrameFromId(s32 id) {
        f32 frame = 0.0f;

        switch (id) {
        case 0:
            return frame;
        case 1:
            frame = 4.0f;
            break;
        case 2:
            frame = 1.0f;
            break;
        case 3:
            frame = 2.0f;
            break;
        case 4:
            frame = 3.0f;
            break;
        default:
            break;
        }

        return frame;
    }
};  // namespace


void MR::setTextBoxNumberRecursive(LayoutActor* pActor, const char* pPaneName, s32 number) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    setTextBoxFormatRecursive(pActor, pPaneName, L"%d", number);
}

void MR::clearTextBoxMessageRecursive(LayoutActor* pActor, const char* pPaneName) {
#ifdef GALAXY_NATIVE
    if (!nativeLayoutPaneReady(pActor, pPaneName)) { return; }
#endif
    setTextBoxMessageRecursive(pActor, pPaneName, L"");
}

IconAButton* MR::createAndSetupIconAButton(LayoutActor* pActor, bool connectToScene, bool connectToPause) {
    IconAButton* iconAButton;

    iconAButton = new IconAButton(connectToScene, connectToPause);
    iconAButton->initWithoutIter();
    iconAButton->setFollowActorPane(pActor, "AButtonPosition");

    return iconAButton;
}
