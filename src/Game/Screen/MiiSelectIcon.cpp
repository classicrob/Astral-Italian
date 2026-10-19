#include "Game/Screen/MiiSelectIcon.hpp"
#include "Game/Util.hpp"
#include "Game/LiveActor/Nerve.hpp"
#include "Game/Map/FileSelectIconID.hpp"
#include "Game/NPC/MiiFaceIcon.hpp"
#include "Game/NPC/MiiFaceRecipe.hpp"
#include "Game/Scene/SceneFunction.hpp"
#include "Game/Screen/ButtonPaneController.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/NativeUiObserve.hpp"
#include "Game/Util/ObjUtil.hpp"
#include "Game/Util/SoundUtil.hpp"
#include <nw4r/lyt/texMap.h>

namespace {
    static const f32 sCharacterTexFrame[] = {
        0.0f, 4.0f, 3.0f, 2.0f, 1.0f,
    };

}

namespace NrvMissSelectIcon {
    NEW_NERVE(MissSelectIconNrvCreate, MiiSelectIcon, Create);
    NEW_NERVE(MissSelectIconNrvWait, MiiSelectIcon, Wait);
    NEW_NERVE(MissSelectIconNrvSelected, MiiSelectIcon, Selected);
    NEW_NERVE(MissSelectIconNrvDisappear, MiiSelectIcon, Disappear);
    NEW_NERVE(MissSelectIconNrvInvalid, MiiSelectIcon, Invalid);
}

MiiSelectIcon::MiiSelectIcon(int movementType, int calcAnimType, int drawType, const char* pName)
    : LayoutActor(pName, true), mCurrentButton(), mFellowButton(), mMiiButton(), mIcon(), mMiiTexMap(), mFellowTexMap(), mIconID(new FileSelectIconID()), mIsMiiDummy(),
      mIsValid(true) {
    initLayoutManager("MiiIcon", 1);
    MR::createAndAddPaneCtrl(this, "MarioIcon", 3);
    MR::createAndAddPaneCtrl(this, "MiiIcon", 3);
    MR::invalidateParentAnim(this);
    createButton();
    createFaceImageObj();
    initNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvCreate));

    if (movementType < 0) {
        movementType = MR::MovementType_Layout;
    }

    if (calcAnimType < 0) {
        calcAnimType = MR::CalcAnimType_Layout;
    }

    if (drawType < 0) {
        drawType = MR::DrawType_LayoutDecoration;
    }

    MR::connectToScene(this, movementType, calcAnimType, MR::DrawBufferType_None, drawType);
}

void MiiSelectIcon::appear(const FileSelectIconID& rIconID) {
    if (MR::isDead(this)) {
        appear();
    }

    mIconID->set(rIconID);

    if (rIconID.isFellow()) {
        MR::showLayout(this);
        MR::startAnim(this, "Character", 0);
        MR::setAnimFrameAndStop(this, ::sCharacterTexFrame[rIconID.getFellowID()], 0);
        MR::startPaneAnim(this, "MarioIcon", "Character", 1);
        MR::setPaneAnimFrameAndStop(this, "MarioIcon", ::sCharacterTexFrame[rIconID.getFellowID()], 1);

        mCurrentButton = mFellowButton;

        MR::hidePane(this, "MiiIcon");
        MR::showPane(this, "MarioIcon");
        setNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvWait));
    } else if (rIconID.isMii()) {
        MR::hideLayout(this);
        mIcon->setIndex(rIconID.getMiiIndex());

        nw4r::lyt::TexMap* pTexMap = mMiiTexMap;

        MR::replacePaneTexture(this, "ShaMii", pTexMap, 0);
        MR::replacePaneTexture(this, "PicMii", pTexMap, 0);
        MR::replacePaneTexture(this, "PicMiiShine", pTexMap, 0);

        mCurrentButton = mMiiButton;

        MR::hidePane(this, "MarioIcon");
        MR::showPane(this, "MiiIcon");
        setNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvCreate));
    }

    mIsMiiDummy = false;

    MR::startPaneAnim(this, "MarioIcon", "Hide", 2);
    MR::setPaneAnimFrameAndStop(this, "MarioIcon", 0.0f, 2);
    MR::startPaneAnim(this, "MiiIcon", "Hide", 2);
    MR::setPaneAnimFrameAndStop(this, "MiiIcon", 0.0f, 2);
}

void MiiSelectIcon::appearMiiDummy() {
    if (MR::isDead(this)) {
        appear();
    }

    MR::showLayout(this);
    MR::startAnim(this, "Character", 0);
    MR::setAnimFrameAndStop(this, 7.0f, 0);
    MR::startPaneAnim(this, "MarioIcon", "Character", 1);
    MR::setPaneAnimFrameAndStop(this, "MarioIcon", 7.0f, 1);
    mCurrentButton = mFellowButton;
    MR::hidePane(this, "MiiIcon");
    MR::showPane(this, "MarioIcon");
    setNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvWait));
    mIsMiiDummy = true;
    MR::startPaneAnim(this, "MarioIcon", "Hide", 2);
    MR::setPaneAnimFrameAndStop(this, "MarioIcon", 0.0f, 2);
    MR::startPaneAnim(this, "MiiIcon", "Hide", 2);
    MR::setPaneAnimFrameAndStop(this, "MiiIcon", 0.0f, 2);
}

void MiiSelectIcon::validate() {
    if (isNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvSelected)) || isNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvInvalid))) {
        mFellowButton->forceToWait();
        mMiiButton->forceToWait();
        setNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvWait));
    }

    mIsValid = true;
}

void MiiSelectIcon::invalidate() {
    if (isNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvSelected))) {
        setNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvInvalid));
    }

    mIsValid = false;
}

void MiiSelectIcon::prohibit() {
    invalidate();
    MR::startPaneAnim(this, "MarioIcon", "Hide", 2);
    MR::setPaneAnimFrameAndStop(this, "MarioIcon", 1.0f, 2);
    MR::startPaneAnim(this, "MiiIcon", "Hide", 2);
    MR::setPaneAnimFrameAndStop(this, "MiiIcon", 1.0f, 2);
}

bool MiiSelectIcon::isSelected() const {
    return isNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvSelected));
}

bool MiiSelectIcon::isMiiDummy() const {
    return mIsMiiDummy;
}

void MiiSelectIcon::copyIconID(FileSelectIconID* pIconID) {
    pIconID->set(*mIconID);
}

nw4r::lyt::TexMap* MiiSelectIcon::getTexMap() {
    if (mIconID->isFellow()) {
        GXTexObj texObj;

#ifdef GALAXY_NATIVE
        nw4r::lyt::TexMap* source = MR::getLytTexMap(this, "PicMario", 0);
        if (!source || !mFellowTexMap) return nullptr;
        source->Get(&texObj);
#else
        MR::getLytTexMap(this, "PicMario", 0)->Get(&texObj);
#endif
        mFellowTexMap->Set(texObj);

        return mFellowTexMap;
    }

    return mMiiTexMap;
}

bool MiiSelectIcon::isPointing() const {
    return mCurrentButton->isPointing();
}

void MiiSelectIcon::exeCreate() {
    if (MR::isFirstStep(this)) {
        mIcon->requestToMakeIcon();
    }

    if (mIcon->mIsCreated) {
        setNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvWait));
    }
}

void MiiSelectIcon::exeWait() {
    if (MR::isFirstStep(this)) {
        MR::showLayout(this);
        mCurrentButton->appear();
    }

    if (mIsValid) {
        mCurrentButton->trySelect();
    }

    if (mCurrentButton->isPointingTrigger()) {
        MR::startSystemSE("SE_SY_BUTTON_CURSOR_ON");
    }

    if (mCurrentButton->mIsSelected) {
        setNerve(GET_NERVE_DIRECT(NrvMissSelectIcon, MissSelectIconNrvSelected));
    }
}

void MiiSelectIcon::exeSelected() {
}

void MiiSelectIcon::exeDisappear() {
    if (mCurrentButton->isHidden()) {
        if (mCurrentButton == mFellowButton) {
            mMiiButton->forceToHide();
        } else {
            mFellowButton->forceToHide();
        }

        kill();
    }
}

void MiiSelectIcon::exeInvalid() {
}

void MiiSelectIcon::appear() {
    LayoutActor::appear();
}

void MiiSelectIcon::control() {
    mFellowButton->update();
    mMiiButton->update();
}

void MiiSelectIcon::createButton() {
    mFellowButton = new ButtonPaneController(this, "MarioIcon", "BoxMario", 0, true);
    mFellowButton->mDisappearOnDecide = false;

    mMiiButton = new ButtonPaneController(this, "MiiIcon", "BoxMii", 0, true);
    mMiiButton->mDisappearOnDecide = false;
}

void MiiSelectIcon::createFaceImageObj() {
    nw4r::lyt::TexMap* pTexMap;

    mIcon =
        new MiiFaceIcon(128, 128, MiiFaceRecipe(RFLDataSource_Official, 0, RFLResolution_256, RFLExpFlag_Normal | RFLExpFlag_Blink), "Miiアイコン");
    mIcon->initWithoutIter();

    GXTexObj texObj;
#ifdef GALAXY_NATIVE
    nw4r::lyt::TexMap* source = MR::getLytTexMap(this, "PicMario", 0);
    if (!source) return;
    source->Get(&texObj);
#else
    MR::getLytTexMap(this, "PicMario", 0)->Get(&texObj);
#endif

    mFellowTexMap = new nw4r::lyt::TexMap(texObj);
    mMiiTexMap = MR::createLytTexMap(reinterpret_cast< ResTIMG* >(mIcon->mImageBuffer));
    pTexMap = mMiiTexMap;

    MR::replacePaneTexture(this, "ShaMii", pTexMap, 0);
    MR::replacePaneTexture(this, "PicMii", pTexMap, 0);
    MR::replacePaneTexture(this, "PicMiiShine", pTexMap, 0);
}
