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

    ButtonPaneController* _20;
    ButtonPaneController* _24;
    ButtonPaneController* _38;
    AllStarList* mAllStarList;
    TVec2f mUpButtonPos;
    TVec2f mDownButtonPos;
    TVec2f mListButtonPos;
    SysInfoWindow* _30;
    bool _54;
    bool mNoPowerStars;
    s32 mSelectedResult;
};
