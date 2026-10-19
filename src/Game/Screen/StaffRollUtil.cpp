#include "Game/Screen/StaffRollUtil.hpp"
#include "Game/Screen/StaffRollPart.hpp"
#include "Game/Screen/StaffRollLine.hpp"
#include "Game/Screen/MessageTagSkipTagProcessor.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include <cstdlib>
#include <wstring.h>

namespace StaffRollUtil {
    // NONMATCHING: tag-skip copy loop schedules the src/counter increments in a different order.
    StaffRollLine* createLine(const wchar_t* message, u32 line, s32 index, bool* endMarker) {
        wchar_t text[256];
        MR::getMessageLine(text, 256, message, line);
        wchar_t work[256] = {};
        const wchar_t* src = text;
        s32 count = 0;
        wchar_t columns[2][256] = {};
        wchar_t* dst = columns[0];
        bool title = false;
        while (true) {
            if (*src == 0xE) {
                *dst++ = *src++;
                MessageEditorMessageTag tag(src);
                for (s32 i = 0; i < static_cast<s32>(tag.getSkipLength()); i++) {
                    *dst++ = *src++;
                }
                continue;
            }
            if (*src == L'^') {
                if (src[1] == L'^' && endMarker) {
                    *endMarker = true;
                }
                return nullptr;
            }
            if (*src == L'$') {
                title = true;
                src++;
                continue;
            }
            if (*src != L'|' && *src != 0) {
                *dst++ = *src;
            } else {
                count++;
                *dst = 0;
                if (*src == 0) {
                    break;
                }
                dst = columns[count];
            }
            src++;
        }
        StaffRollLine* actor;
        if (title) {
            actor = new StaffRollLine(0, index);
            actor->initWithoutIter();
            MR::setTextBoxMessageRecursive(actor, "Title", columns[0]);
            MR::setTextBoxMessageRecursive(actor, "Name", L"");
            actor->appear();
            return actor;
        } else if (count == 1) {
            wchar_t* name = columns[0];
            actor = new StaffRollLine(1, index);
            actor->initWithoutIter();
            MR::setTextBoxMessageRecursive(actor, "Title", L"");
            MR::setTextBoxMessageRecursive(actor, "LeftName", L"");
            MR::setTextBoxMessageRecursive(actor, "RightName", L"");
            MR::setTextBoxMessageRecursive(actor, "NameSingle", name);
            actor->appear();
            return actor;
        } else if (count == 2) {
            wchar_t* left = columns[0];
            wchar_t* right = columns[1];
            actor = new StaffRollLine(1, index);
            actor->initWithoutIter();
            MR::setTextBoxMessageRecursive(actor, "Title", L"");
            MR::setTextBoxMessageRecursive(actor, "LeftName", left);
            MR::setTextBoxMessageRecursive(actor, "RightName", right);
            MR::setTextBoxMessageRecursive(actor, "NameSingle", L"");
            actor->appear();
            return actor;
        } else {
            return nullptr;
        }
    }

    StaffRollPart* createPart(const wchar_t* message, u32* line) {
        return new StaffRollPart(message, line);
    }

    // NONMATCHING: dst and inner-loop counter swap registers (r25/r28).
    bool readPartInfo(StaffRollPartInfo* info, const wchar_t* message, u32 line) {
        wchar_t text[256];
        MR::getMessageLine(text, 256, message, line);
        wchar_t columns[3][256] = {};
        wchar_t* dst = columns[0];
        s32 count = 0;
        const wchar_t* src = text;
        while (true) {
            if (*src == 0xE) {
                *dst++ = *src++;
                MessageEditorMessageTag tag(src);
                for (s32 i = 0; i < static_cast<s32>(tag.getSkipLength()); i++) {
                    *dst++ = *src++;
                }
                continue;
            }
            if (*src != L',' && *src != 0) {
                *dst++ = *src;
            } else {
                count++;
                *dst = 0;
                if (*src == 0) {
                    break;
                }
                dst = columns[count];
            }
            src++;
        }
        if (columns[0][0] == L'#') {
            if (columns[0][1] == L'T') {
                info->mType = 0;
            } else if (columns[0][1] == L'C') {
                info->mType = 1;
            } else if (columns[0][1] == L'B') {
                info->mType = 2;
            }
            if (count == 3) {
                info->mPosition.x = wtoi(columns[1]);
                info->mPosition.y = wtoi(columns[2]);
            }
            return true;
        }
        return false;
    }

    void initPane(LayoutActor* actor, TVec2f* position, const char* pane) {
        MR::createAndAddPaneCtrl(actor, pane, 1);
        MR::copyPaneTrans(position, actor, pane);
        MR::setFollowPos(position, actor, pane);
        MR::hidePane(actor, pane);
    }

    bool updatePaneAppear(LayoutActor* actor, const char* pane, const Nerve* next) {
        if (MR::isFirstStep(actor)) {
            MR::showPane(actor, pane);
            MR::startPaneAnim(actor, pane, "Appear", 0);
        }
        if (MR::isPaneAnimStopped(actor, pane, 0)) {
            actor->setNerve(next);
            return true;
        }
        return false;
    }

    bool updatePaneWait(LayoutActor* actor, const char* pane, s32 duration, const Nerve* next) {
        if (MR::isFirstStep(actor)) {
            MR::startPaneAnim(actor, pane, "Wait", 0);
        }
        if (MR::isGreaterEqualStep(actor, duration)) {
            actor->setNerve(next);
            return true;
        }
        return false;
    }

    bool updatePaneDisappear(LayoutActor* actor, const char* pane, const Nerve* next) {
        if (MR::isFirstStep(actor)) {
            MR::startPaneAnim(actor, pane, "End", 0);
        }
        if (MR::isPaneAnimStopped(actor, pane, 0)) {
            MR::hidePane(actor, pane);
            actor->setNerve(next);
            return true;
        }
        return false;
    }
}
