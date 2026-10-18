#pragma once

#include "Game/Screen/LayoutActor.hpp"

class BackButton;
class ButtonPaneController;

class GalaxyConfirmLayout : public LayoutActor {
public:
    /// @brief Creates a new `GalaxyConfirmLayout`.
    GalaxyConfirmLayout(s32 galaxyType = 0);

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void control();

    bool isSelected() const;
    bool isSelectedYes() const;
    bool isSelectionComplete() const;
    void registerMovePositions();
    void exeAppear();
    void requestMovementOn();
    void exeSelecting();
    void exeSelectedBackButton();
    void exeSelected();

private:
    /* 0x20 */ ButtonPaneController* mPaneCtrl;
    /* 0x24 */ BackButton* mBackButton;
    /* 0x34 */ s32 mGalaxyType;
    /* 0x38 */ s32 mPointingCooldown;
};
