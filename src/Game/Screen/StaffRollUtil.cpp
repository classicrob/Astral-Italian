#include "Game/Screen/StaffRollUtil.hpp"
#include "Game/Screen/StaffRollPart.hpp"
#include "Game/Screen/StaffRollLine.hpp"
#include "Game/Screen/MessageTagSkipTagProcessor.hpp"
#include "Game/Util/LayoutUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include <cstdlib>
#include <wstring.h>

namespace StaffRollUtil {
    StaffRollLine* createLine(const wchar_t* message, u32 line, s32 index, bool* endMarker) {
        wchar_t text[256];
        MR::getMessageLine(text, 256, message, line);
        wchar_t work[256] = {};
        wchar_t columns[2][256] = {};
        const wchar_t* src = text;
        wchar_t* dst = columns[0];
        s32 count = 0;
        bool title = false;
        while (true) {
            wchar_t c = *src;
            if (c == 0xE) {
                *dst++ = *src++;
                MessageEditorMessageTag tag(src);
                for (s32 i = 0; i < static_cast<s32>(tag.getSkipLength()); i++) {
                    *dst++ = *src++;
                }
                continue;
            }
            if (c == L'^') {
                if (src[1] == L'^' && endMarker) {
                    *endMarker = true;
                }
                return nullptr;
            }
            if (c == L'$') {
                title = true;
                src++;
                continue;
            }
            if (c == L'|' || c == 0) {
                count++;
                *dst = 0;
                if (c == 0) {
                    break;
                }
                dst = columns[count];
                src++;
                continue;
            }
            *dst++ = *src++;
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
            actor = new StaffRollLine(1, index);
            actor->initWithoutIter();
            MR::setTextBoxMessageRecursive(actor, "Title", L"");
            MR::setTextBoxMessageRecursive(actor, "LeftName", L"");
            MR::setTextBoxMessageRecursive(actor, "RightName", L"");
            MR::setTextBoxMessageRecursive(actor, "NameSingle", columns[0]);
            actor->appear();
            return actor;
        } else if (count == 2) {
            actor = new StaffRollLine(1, index);
            actor->initWithoutIter();
            MR::setTextBoxMessageRecursive(actor, "Title", L"");
            MR::setTextBoxMessageRecursive(actor, "LeftName", columns[0]);
            MR::setTextBoxMessageRecursive(actor, "RightName", columns[1]);
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

    bool readPartInfo(StaffRollPartInfo* info, const wchar_t* message, u32 line) {
        wchar_t text[256];
        MR::getMessageLine(text, 256, message, line);
        wchar_t columns[3][256] = {};
        const wchar_t* src = text;
        wchar_t* dst = columns[0];
        s32 count = 0;
        while (true) {
            wchar_t c = *src;
            if (c == 0xE) {
                *dst++ = *src++;
                MessageEditorMessageTag tag(src);
                for (s32 i = 0; i < static_cast<s32>(tag.getSkipLength()); i++) {
                    *dst++ = *src++;
                }
                continue;
            }
            if (c == L',' || c == 0) {
                count++;
                *dst = 0;
                if (c == 0) {
                    break;
                }
                dst = columns[count];
                src++;
                continue;
            }
            *dst++ = *src++;
        }
        if (columns[0][0] != L'#') {
            return false;
        }
        switch (columns[0][1]) {
        case L'T': info->mType = 0; break;
        case L'C': info->mType = 1; break;
        case L'B': info->mType = 2; break;
        }
        if (count == 3) {
            info->mPosition.x = wtoi(columns[1]);
            info->mPosition.y = wtoi(columns[2]);
        }
        return true;
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
