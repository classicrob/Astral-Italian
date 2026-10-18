#pragma once

#include "Game/Screen/LayoutActor.hpp"

class ButtonPaneController;
class GalaxyMapGalaxyPlain;

namespace MR {
    class FunctorBase;
};  // namespace MR

class FileSelectButton : public LayoutActor {
public:
    /// @brief Creates a new `FileSelectButton`.
    /// @param pName The pointer to the null-terminated name of the object.
    FileSelectButton(const char* pName);

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void control();

    void disappear();
    void setCallbackFunctor(const MR::FunctorBase&, const MR::FunctorBase&, const MR::FunctorBase&, const MR::FunctorBase&);
    void shiftSelect();
    void exeSelect();
    void exeDecided();
    void exeWait();
    void exeDisappear();
    void createPaneControl();
    void createButtonController();
    void createOthers();

private:
    /* 0x2C */ ButtonPaneController* mButtonCtrl[4];
    /* 0x3C */ MR::FunctorBase* mCallbackFunctor[4];
    /* 0x4C */ bool (*mSelectCallback[4])();
    /* 0x5C */ s32 mSelectedButton;
    /* 0x60 */ GalaxyMapGalaxyPlain* mHelpText;
};
