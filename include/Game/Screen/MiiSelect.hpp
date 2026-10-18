#pragma once

#include "Game/Map/FileSelectIconID.hpp"
#include "Game/Screen/LayoutActor.hpp"

namespace nw4r {
    namespace lyt {
        class TexMap;
    };  // namespace lyt
};  // namespace nw4r

namespace MR {
    class BitArray;
};  // namespace MR

namespace MiiSelectSub {
    class Page;
};  // namespace MiiSelectSub

class ButtonPaneController;
class MiiSelectIcon;

class MiiSelect : public LayoutActor {
    friend class MiiSelectSub::Page;

public:
    /// @brief Creates a new `MiiSelect`.
    /// @param pName A pointer to the null-terminated name of the object.
    MiiSelect(const char* pName);

    /// @brief Intializes the `MiiSelect` while being placed into a scene.
    /// @param[in] rIter The reference to an iterator over a `JMapInfo`.
    virtual void init(const JMapInfoIter& rIter);

    virtual void calcAnim();
    virtual void appear();
    virtual void control();

    void disappear();
    bool isAppearing() const;
    bool isSelected();
    bool isDummySelected();
    void getSelectedID(FileSelectIconID*);
    nw4r::lyt::TexMap* getSelectedMiiTexMap();
    void admitIcon();
    void prohibitIcon(const FileSelectIconID&);
    void invalidateSpecialMii(FileSelectIconID::EFellowID);
    void validateAllSpecialMii();
    void exeAppear();
    void exeWait();
    void exeScrollRight();
    void exeScrollLeft();
    void exeSelected();
    void exeDisappear();
    void exeDummySelected();
    void createButtons();
    void callbackLeft();
    void callbackRight();
    bool isLeftTriggered();
    bool isRightTriggered();
    void appearButtons();
    void disappearButtons();
    void updateButtons();
    void collectValidMiiIndex();
    void createPage();
    void flipPage();
    void setCurrentPageGroupA();
    void setCurrentPageGroupB();
    void setCurrentPageNum();
    void validateAllIcon();
    void refresh();
    void getIconID(FileSelectIconID*, s32) const;
    void onSelect(s32, nw4r::lyt::TexMap*);
    void onSelectDummy();
    s32 getIconNum();

    bool isExistMiiIcon() const {
        return mMiiNum != 0 || mFavoriteMiiNum != 0;
    }

    FileSelectIconID makeIconID(u32 id) const {
        FileSelectIconID iconID;
        getIconID(&iconID, id);
        return iconID;
    }

    void clearPage() {
        _1EC = 0;
    }

private:
    /* 0x02C */ ButtonPaneController* _20[2];
    /* 0x034 */ MR::BitArray* _28;
    /* 0x038 */ u16 mFellowIconNum;
    /* 0x03A */ u16 mFavoriteMiiNum;
    /* 0x03C */ u32 mFavoriteMiiIndices[10];
    /* 0x064 */ u16 mMiiNum;
    /* 0x068 */ u32 mMiiIndices[10];
    /* 0x090 */ u8 _84[360];
    /* 0x1F8 */ s32 _1EC;
    /* 0x1FC */ s32 _1F0;
    /* 0x200 */ nw4r::lyt::TexMap* _1F4;
    /* 0x204 */ MiiSelectSub::Page* _1F8;
    /* 0x208 */ MiiSelectSub::Page* _1FC;
    /* 0x20C */ u8 _200;
    /* 0x210 */ FileSelectIconID* _204;
};

namespace MiiSelectSub {
    class Page {
    public:
        /// @brief Creates a new `Page`.
        /// @param pHost A pointer to the owning actor instance.
        Page(MiiSelect* pHost);

        void refresh(s32);
        void movement();
        void calcAnim();
        void invalidateAllIcon();
        void validateAllIcon();
        void prohibitIcon(const FileSelectIconID&);

        /* 0x00 */ MiiSelectIcon* mIconArray[8];
        /* 0x20 */ bool _20;
        /* 0x24 */ MiiSelect* mHost;
        /* 0x28 */ s32 mBaseIndex;
    };
};  // namespace MiiSelectSub
