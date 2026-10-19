#pragma pool_data off

#include "Game/Screen/LayoutCoreUtil.hpp"
#include "Game/Util.hpp"
#include "Game/Screen/CustomTagProcessor.hpp"
#include "Game/Screen/ReplaceTagProcessor.hpp"
#include "Game/Util/MemoryUtil.hpp"
#include "Game/Util/StringUtil.hpp"
#include "Game/Util/MessageUtil.hpp"
#include <cstring>
#include "Game/Util/ScreenUtil.hpp"
#include <cstdio>
#include <nw4r/db/assert.h>
#include <nw4r/lyt/layout.h>
#include <nw4r/lyt/pane.h>
#include <nw4r/lyt/resources.h>
#include <nw4r/lyt/textBox.h>
#include <nw4r/ut/TextWriterBase.h>
#include <revolution/mtx.h>

static const wchar_t sDefaultMsg[] = L"XXXXXXXX";

namespace LayoutCoreUtil {
    using namespace nw4r::lyt::res;

    f32 getPaneAutoScale(nw4r::lyt::Pane* pPane) {
        if (pPane == nullptr) {
            return 0.85f;
        }

        const ExtUserData* userData = pPane->FindExtUserDataByName("AutoScale");
        if (userData != nullptr) {
#ifdef NW4R_DEBUG
            if (userData->GetType() != EXTUSERDATATYPE_FLOAT)
                nw4r::db::Panic("types.h", 1020, "NW4R:Failed assertion GetType() == EXTUSERDATATYPE_FLOAT");
#endif
            return userData->GetValue< f32 >(0);
        }

        return getPaneAutoScale(pPane->mpParent);
    }

    nw4r::lyt::Layout* createLayout(const char* pLayoutName, LayoutHolder* pLayoutHolder) {
        char layoutResName[0x80];
        snprintf(layoutResName, sizeof(layoutResName), "%s.brlyt", pLayoutName);
        void* pLayoutRes = pLayoutHolder->GetResource('blyt', layoutResName, nullptr);

        nw4r::lyt::Layout* layout = new nw4r::lyt::Layout();
        layout->Build(pLayoutRes, pLayoutHolder);
        return layout;
    }

    const char* getPaneUserData(const nw4r::lyt::Pane* pPane, char* pUserData) {
        MR::copyMemory(pUserData, pPane->mUserData, 8);
        pUserData[8] = '\0';
        return pUserData;
    }

    // NONMATCHING: register allocation and equivalent single/double load of the initial 1.0 scale.
    void setTextBoxMessage(nw4r::lyt::TextBox* pTextBox, const wchar_t* pMsg) {
        wchar_t buffer[512];
        ReplaceTagProcessor::Replace(buffer, pMsg);
        u16 length = MR::getStringLengthWithMessageTag(buffer);
        pTextBox->SetString(buffer, 0, length);

        CustomTagProcessor* processor = static_cast<CustomTagProcessor*>(pTextBox->mpTagProcessor);
        processor->setFontScale(1.0f);

        nw4r::ut::TextWriterBase<wchar_t> writer;
        writer.SetCursor(0.0f, 0.0f);
        writer.SetFont(*pTextBox->GetFont());
        writer.SetFontSize(pTextBox->mFontSize.width, pTextBox->mFontSize.height);
        writer.SetLineSpace(pTextBox->mLineSpace);
        writer.SetCharSpace(pTextBox->mCharSpace);
        writer.SetWidthLimit(nw4r::math::F_MAX);
        writer.SetTagProcessor(pTextBox->mpTagProcessor);

        f32 width = writer.CalcStringWidth(buffer, length);
        nw4r::ut::Rect rect = pTextBox->GetTextDrawRect();
        if (rect.GetWidth() < width) {
            f32 minScale = getPaneAutoScale(pTextBox);
            f32 scale = rect.GetWidth() / width;
            processor->setFontScale(MR::max(scale, minScale));
        }
    }

    void initTextBoxPane(nw4r::lyt::TextBox* pTextBox, const char* pMessageId, u32 allocSize) {
        CustomTagProcessor* processor = new CustomTagProcessor(pTextBox);

        if (strncmp(pTextBox->mName, "Sha", 3) == 0) {
            processor->mIsShadow = true;
        } else {
            processor->mIsShadow = false;
        }

        if (strncmp(pTextBox->mName, "Txt", 3) == 0) {
            processor->mIsText = true;
        } else {
            processor->mIsText = false;
        }

        if (strncmp(pTextBox->mName, "Inf", 3) == 0) {
            processor->mIsInfo = true;
        } else {
            processor->mIsInfo = false;
        }

        pTextBox->mpTagProcessor = processor;

        const wchar_t* message = nullptr;
        if (pMessageId != nullptr) {
            message = MR::getGameMessageDirect(pMessageId);
        }

        if (message != nullptr) {
            reallocateTextBoxMessage(pTextBox, MR::getStringLengthWithMessageTag(message));
            setTextBoxMessage(pTextBox, message);
        } else {
            reallocateTextBoxMessage(pTextBox, allocSize - 1);
            const wchar_t* message = ::sDefaultMsg;
            setTextBoxMessage(pTextBox, message);
        }
    }

    void reallocateTextBoxMessage(nw4r::lyt::TextBox* pTextBox, u32 allocSize) {
        pTextBox->AllocStringBuffer(allocSize + 1);
    }
};  // namespace LayoutCoreUtil

namespace MR {
    // NONMATCHING: floating-point register allocation and load ordering.
    void convertScreenPosToLayoutPos(TVec2f* pLayoutPos, const TVec2f& rScreenPos) {
        pLayoutPos->x = rScreenPos.x * 608.0f / MR::getScreenWidth() - 304.0f;
        pLayoutPos->y = -(rScreenPos.y * MR::getScreenHeight() / MR::getScreenHeight() - (MR::getScreenHeight() * 0.5f));
    }

    // NONMATCHING: floating-point register allocation and load ordering.
    void convertLayoutPosToScreenPos(TVec2f* pScreenPos, const TVec2f& rLayoutPos) {
        pScreenPos->x = MR::getScreenWidth() * 0.5f + rLayoutPos.x * MR::getScreenWidth() / 608.0f;
        pScreenPos->y = MR::getScreenHeight() * 0.5f + -rLayoutPos.y * MR::getScreenHeight() / MR::getScreenHeight();
    }

    void convertPaneLocalPosToScreenPos(TVec2f* pScreenPos, const nw4r::lyt::Pane* pPane, const TVec2f& rLocalPos) {
#ifdef GALAXY_NATIVE
        if (!pPane) { pScreenPos->zero(); return; }
#endif
        TVec3f pos(rLocalPos);
        TVec3f layoutPos;
        PSMTXMultVec(pPane->mGlbMtx, pos, layoutPos);
        convertLayoutPosToScreenPos(pScreenPos, layoutPos);
    }
};  // namespace MR
