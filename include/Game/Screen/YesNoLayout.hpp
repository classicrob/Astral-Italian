#pragma once

#include "Game/Screen/SimpleLayout.hpp"

class YesNoController;

class YesNoLayout : public SimpleLayout {
public:
    /// @brief Creates a new `YesNoLayout`.
    YesNoLayout();

    virtual void init(const JMapInfoIter& rIter);
    virtual void appear();
    virtual void kill();
    virtual void control();

    void setSelectMessage(const char*, const char*);
    void exeSelecting() NO_INLINE;

    /* 0x2C */ YesNoController* mController;
};
