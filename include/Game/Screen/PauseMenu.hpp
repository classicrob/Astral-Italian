#pragma once

#include "Game/Screen/LayoutActor.hpp"

class ButtonPaneController;
class AllStarList;
class SysInfoWindow;

class PauseMenu : public LayoutActor {
public:
    /// @brief Creates a new `PauseMenu`.
    PauseMenu();

    /// @brief Intializes the `PauseMenu` while being placed into a scene.
    /// @param rIter The reference to an iterator over a `JMapInfo`.
    virtual void init(const JMapInfoIter& rIter);

    virtual void appear();
    virtual void kill();
    virtual void control();

    void updateStarPane();
    void startPaneAnimWithoutButton(const char*);
    bool isPaneAnimStoppedWithoutButton() const;
    void forceToWaitAllButton();
    void exeSelecting();
    void exeDecided();
    void exeDisappear();
    void exeConfirm();
    void exeGameDataSave();
    void exeAllStarList();
    void restartSelecting();
    void registerPointer();

    ButtonPaneController* mUpButton;
    ButtonPaneController* mDownButton;
    ButtonPaneController* mListButton;
    AllStarList* mAllStarList;
    TVec2f mUpButtonPos;
    TVec2f mDownButtonPos;
    TVec2f mListButtonPos;
    SysInfoWindow* mSysInfoWindow;
    bool mWasDefaultLayoutActive;
    bool mNoPowerStars;
    s32 mSelectedResult;
};
