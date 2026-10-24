#include "nw4r/lyt/drawInfo.h"
#include "nw4r/lyt/layout.h"
#include "nw4r/lyt/resourceAccessor.h"
#include "nw4r/lyt/textBox.h"
#include "nw4r/ut/Font.h"
#include "nw4r/ut/ResFont.h"
#include "nw4r/ut/WideTextWriter.h"
#include <cstdio>
extern "C" {
#include <wstring.h>
}

namespace {
    using namespace nw4r;

    inline u8 ClampColor(s16 colVal) {
        return u8(colVal < 0 ? 0 : (colVal > 255 ? 255 : colVal));
    }

    ut::Color GetColor(const GXColorS10& src) {
        GXColor dst;
        dst.r = ClampColor(src.r);
        dst.g = ClampColor(src.g);
        dst.b = ClampColor(src.b);
        dst.a = ClampColor(src.a);
        return ut::Color(dst);
    }
};  // namespace
namespace nw4r {
    namespace lyt {
        namespace {
            inline void UnionRect(ut::Rect* dst, const ut::Rect& src) {
                dst->left = ut::Min(dst->left, src.left);
                dst->top = ut::Min(dst->top, src.top);
                dst->right = ut::Max(dst->right, src.right);
                dst->bottom = ut::Max(dst->bottom, src.bottom);
            }

            template<typename CharT>
            int CalcLineRectImpl(ut::Rect* rect, ut::TextWriterBase<CharT>* writer, const CharT* str, int len, f32 width, bool* wrap) {
                ut::PrintContext<CharT> context = {writer, str, 0.0f, 0.0f, 0};
                f32 x = 0.0f;

                const ut::Font* font = writer->GetFont();
                bool charSpace = false;
                ut::CharStrmReader reader = font->GetCharStrmReader();

                const CharT* previous = static_cast<const CharT*>(reader.GetCurrentPos());

                rect->left = 0.0f;
                rect->right = 0.0f;
                rect->top = ut::Min(0.0f, writer->GetLineHeight());
                rect->bottom = ut::Max(0.0f, writer->GetLineHeight());

                *wrap = false;

                reader.Set(str);
                ut::Rect previousRect = *rect;
                u16 ch = reader.Next();
                while (static_cast<const CharT*>(reader.GetCurrentPos()) - str <= len) {
                    if (ch < 0x20) {
                        typename ut::TagProcessorBase<CharT>::Operation operation;
                        ut::Rect tagRect(x, 0.0f, 0.0f, 0.0f);

                        context.str = static_cast<const CharT*>(reader.GetCurrentPos());
                        context.flags = !charSpace;

                        writer->SetCursorX(x);
                        operation = writer->GetTagProcessor().CalcRect(&tagRect, ch, &context);
                        reader.Set(context.str);

                        rect->left = ut::Min(rect->left, tagRect.left);
                        rect->top = ut::Min(rect->top, tagRect.top);
                        rect->right = ut::Max(rect->right, tagRect.right);
                        rect->bottom = ut::Max(rect->bottom, tagRect.bottom);

                        x = writer->GetCursorX();

                        if (rect->GetWidth() > width) {
                            *wrap = true;
                            break;
                        }
                        if (operation == ut::TagProcessorBase<CharT>::OPERATION_END_DRAW) {
                            return len;
                        } else if (operation == ut::TagProcessorBase<CharT>::OPERATION_NO_CHAR_SPACE) {
                            charSpace = false;
                        } else if (operation == ut::TagProcessorBase<CharT>::OPERATION_CHAR_SPACE) {
                            charSpace = true;
                        } else if (operation == ut::TagProcessorBase<CharT>::OPERATION_NEXT_LINE) {
                            break;
                        }
                    } else {
                        if (charSpace) {
                            x += writer->GetCharSpace();
                        }
                        charSpace = true;
                        if (writer->IsWidthFixed()) {
                            x += writer->GetFixedWidth();
                        } else {
                            x += writer->GetFont()->GetCharWidth(ch) * writer->GetScaleH();
                        }
                        rect->left = ut::Min(rect->left, x);
                        rect->right = ut::Max(rect->right, x);

                        if (rect->GetWidth() > width) {
                            *wrap = true;
                            break;
                        }
                    }
                    previous = static_cast<const CharT*>(reader.GetCurrentPos());
                    ch = reader.Next();
                    previousRect = *rect;
                }

                if (*wrap) {
                    if (previous != NULL) {
                        *rect = previousRect;
                        return previous - str;
                    }
                }
                return static_cast<const CharT*>(reader.GetCurrentPos()) - str;
            }

            template<typename CharT>
            NO_INLINE void CalcStringRectImpl(ut::Rect* rect, ut::TextWriterBase<CharT>* writer, const CharT* str, int len, f32 width) {
                rect->left = 0.0f;
                rect->right = 0.0f;
                rect->top = 0.0f;
                rect->bottom = 0.0f;
                writer->SetCursor(0.0f, 0.0f);
                do {
                    ut::Rect lineRect;
                    bool wrap;
                    const int count = CalcLineRectImpl(&lineRect, writer, str, len, width, &wrap);
                    str += count;
                    len -= count;
                    UnionRect(rect, lineRect);
                    if (wrap) {
                        CalcLineRectImpl(&lineRect, writer, L"\n", 1, width, &wrap);
                        UnionRect(rect, lineRect);
                    }
                } while (len > 0);
            }

            template<typename CharT>
            int CalcLineStrNum(f32* width, ut::TextWriterBase<CharT>* writer, const CharT* str, int len, f32 maxWidth, bool* wrap) {
                ut::Rect rect;
                ut::TextWriterBase<CharT> writerCopy = *writer;
                writerCopy.SetCursor(0.0f, 0.0f);
                int count = CalcLineRectImpl(&rect, &writerCopy, str, len, maxWidth, wrap);

                *width = rect.GetWidth();
                return count;
            }

            template<typename CharT>
            void CalcStringRect(ut::Rect* rect, ut::TextWriterBase<CharT>* writer, const CharT* str, int len, f32 width) {
                ut::TextWriterBase<CharT> writerCopy = *writer;
                CalcStringRectImpl(rect, &writerCopy, str, len, width);
            }
        }

        NW4R_UT_RUNTIME_TYPEINFO_DEFINITION(TextBox, Pane);

        TextBox::TextBox(const res::TextBox* pBlock, const ResBlockSet& resBlockSet) : Pane(pBlock) {
            u16 allocStrBufLen = static_cast< u16 >(pBlock->textBufBytes / sizeof(wchar_t));
            if (allocStrBufLen > 0) {
                allocStrBufLen -= 1;
            }

            Init(allocStrBufLen);

            if (pBlock->textStrBytes >= sizeof(wchar_t) && mTextBuf) {
                const wchar_t* const pBlockText = detail::ConvertOffsToPtr< wchar_t >(pBlock, pBlock->textStrOffset);
                const u16 resStrLen = static_cast< u16 >(pBlock->textStrBytes / sizeof(wchar_t) - 1);
                SetString(pBlockText, 0, resStrLen);
            }

            for (int i = 0; i < 2; ++i) {
                mTextColors[i] = pBlock->textCols[i];
            }

            mFontSize = pBlock->fontSize;
            mTextPosition = pBlock->textPosition;
            mBits.textAlignment = pBlock->textAlignment;
            mCharSpace = pBlock->charSpace;
            mLineSpace = pBlock->lineSpace;

            const res::Font* const fonts = detail::ConvertOffsToPtr< res::Font >(resBlockSet.pFontList, sizeof(*resBlockSet.pFontList));
            const char* const fontName = detail::ConvertOffsToPtr< char >(fonts, fonts[pBlock->fontIdx].nameStrOffset);

            if (ut::Font* pFont = resBlockSet.pResAccessor->GetFont(fontName)) {
                mpFont = pFont;
            } else if (void* fontRes = resBlockSet.pResAccessor->GetResource('font', fontName, 0)) {
                if (ut::ResFont* pResFont = Layout::NewObj<ut::ResFont>()) {
                    pResFont->SetResource(fontRes);
                    mpFont = pResFont;
                    mBits.bAllocFont = true;
                }
            }
            const u32* const matOffsTbl = detail::ConvertOffsToPtr<u32>(resBlockSet.pMaterialList, sizeof(*resBlockSet.pMaterialList));
            const res::Material* const pResMaterial = detail::ConvertOffsToPtr<res::Material>(resBlockSet.pMaterialList, matOffsTbl[pBlock->materialIdx]);
            mpMaterial = Layout::NewObj<Material>(pResMaterial, resBlockSet);
        }

        void TextBox::Init(u16 allocStrLen) {
            mTextBuf = 0;
            mTextBufBytes = 0;
            mTextLen = 0;
            mpFont = 0;
            mFontSize = Size(0, 0);
            SetTextPositionH(1);
            SetTextPositionV(1);
            mLineSpace = 0;
            mCharSpace = 0;
            mpTagProcessor = 0;
            memset(&mBits, 0, sizeof(mBits));

            if (allocStrLen > 0) {
                AllocStringBuffer(allocStrLen);
            }
        }

        TextBox::~TextBox() {
            SetFont(nullptr);

            if (mpMaterial != NULL && !mpMaterial->IsUserAllocated()) {
                Layout::DeleteObj(mpMaterial);
                mpMaterial = NULL;
            }

            FreeStringBuffer();
        }

        const ut::Color TextBox::GetVtxColor(u32 idx) const {
            return GetTextColor(idx / 2);
        }

        void TextBox::SetVtxColor(u32 idx, ut::Color color) {
            SetTextColor(idx / 2, color);
        }

        void TextBox::SetTextColor(u32 type, ut::Color value) {
            mTextColors[type] = value;
        }

        u8 TextBox::GetVtxColorElement(u32 idx) const {
            return reinterpret_cast< const u8* >(&mTextColors[idx / (4 * 2)])[idx % 4];
        }

        void TextBox::SetVtxColorElement(u32 idx, u8 value) {
            reinterpret_cast< u8* >(&mTextColors[idx / (4 * 2)])[idx % 4] = value;
        }

        const ut::Rect TextBox::GetTextDrawRect() const {
            ut::WideTextWriter writer;

            return GetTextDrawRect(&writer);
        }

        const ut::Rect TextBox::GetTextDrawRect(const DrawInfo& drawInfo) const {
            ut::WideTextWriter writer;

            return GetTextDrawRect(&writer);
        }

        // NONMATCHING: writer-copy temporaries, scalar loads, registers and stack layout differ.
        void TextBox::DrawSelf(const DrawInfo& drawInfo) {
            if (!mTextBuf || !mpFont || !mpMaterial) {
                return;
            }

            LoadMtx(drawInfo);

            ut::WideTextWriter writer;

            ut::Rect textRect = GetTextDrawRect(&writer);

            ut::Color topCol = detail::MultipleAlpha(mTextColors[0], mGlbAlpha);
            ut::Color btmCol = detail::MultipleAlpha(mTextColors[1], mGlbAlpha);
            writer.SetGradationMode(topCol != btmCol ? ut::CharWriter::GRADMODE_V : ut::CharWriter::GRADMODE_NONE);
            writer.SetTextColor(topCol, btmCol);
            ut::Color minCol = GetColor(mpMaterial->GetTevColor(0));
            ut::Color maxCol = GetColor(mpMaterial->GetTevColor(1));
            writer.SetColorMapping(minCol, maxCol);
            writer.SetupGX();

            GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
            const f32 align = GetTextAlignMag();
            const wchar_t* str = mTextBuf;
            writer.SetCursor(textRect.left, -textRect.top);
            const f32 textWidth = textRect.GetWidth();
            int len = mTextLen;
            while (len > 0) {
                bool wrap;
                f32 lineWidth;
                int count = CalcLineStrNum(&lineWidth, &writer, str, len, mSize.width, &wrap);
                f32 textPosX = textRect.left + align * (textWidth - lineWidth);
                writer.SetCursorX(textPosX);
                writer.PrintMutable(str, count);
                if (wrap) {
                    writer.PrintMutable(L"\n", 1);
                }
                str += count;
                len -= count;
            }
        }

        const ut::Font* TextBox::GetFont() const {
            return mpFont;
        }

        void TextBox::LoadMtx(const DrawInfo& info) {
            math::MTX34 mtx;
            if (info.IsMultipleViewMtxOnDraw()) {
                PSMTXConcat(info.GetViewMtx(), mGlbMtx, mtx);
            } else {
                PSMTXCopy(mGlbMtx, mtx);
            }
            mtx._01 = -mtx._01;
            mtx._11 = -mtx._11;
            mtx._21 = -mtx._21;
            GXLoadPosMtxImm(mtx, GX_PNMTX0);
            GXSetCurrentMtx(GX_PNMTX0);
        }

        u16 TextBox::GetStringBufferLength() const {
            if (mTextBufBytes == 0) {
                return 0;
            }

            return mTextBufBytes / sizeof(wchar_t) - 1;
        }

        // NONMATCHING: array-count arithmetic is hoisted and held in different registers.
        void TextBox::AllocStringBuffer(u16 len) {
            if (len == 0) {
                return;
            }

            u32 allocLen = len;
            const u32 bytes = (len + 1) * sizeof(wchar_t);

            if (bytes > mTextBufBytes) {
                FreeStringBuffer();
                mTextBuf = Layout::NewArray<wchar_t>(allocLen + 1);

                if (mTextBuf != NULL) {
                    mTextBufBytes = bytes;
                }
            }
        }

        void TextBox::FreeStringBuffer() {
            if (mTextBuf == NULL) {
                return;
            }

            Layout::DeletePrimArray(mTextBuf);
            mTextBuf = NULL;
            mTextBufBytes = 0;
        }

        u16 TextBox::SetString(const wchar_t* pStr, u16 pos) {
            u32 len = wcslen(pStr);
            if (mTextBuf == NULL) return 0;
            const u16 maxlen = GetStringBufferLength();
            if (pos >= maxlen) return 0;
            const u32 chars = ut::Min<u32>(len, maxlen - pos);
            memcpy(&mTextBuf[pos], pStr, chars * sizeof(wchar_t));
            mTextLen = pos + chars;
            mTextBuf[mTextLen] = L'\0';
            return chars;
        }

        u16 TextBox::SetString(const wchar_t* pStr, u16 pos, u16 len) {
            if (mTextBuf == NULL) {
                return 0;
            }

            const u16 maxlen = GetStringBufferLength();
            if (pos >= maxlen) {
                return 0;
            }

            const u32 chars = ut::Min<u32>(len, maxlen - pos);
            memcpy(&mTextBuf[pos], pStr, chars * sizeof(wchar_t));

            mTextLen = pos + chars;
            mTextBuf[mTextLen] = L'\0';

            return chars;
        }

        void TextBox::SetFont(const ut::Font* pFont) {
            if (mBits.bAllocFont) {
                Layout::DeleteObj(const_cast<ut::Font*>(mpFont));
                mBits.bAllocFont = false;
            }

            mpFont = pFont;

            if (mpFont != NULL) {
                SetFontSize(Size(static_cast< f32 >(mpFont->GetWidth()), static_cast< f32 >(mpFont->GetHeight())));
            } else {
                SetFontSize(Size(0.0f, 0.0f));
            }
        }

        // NONMATCHING: writer-copy loads and call-argument loads are scheduled differently.
        const ut::Rect TextBox::GetTextDrawRect(ut::WideTextWriter* pWriter) const {
            ut::Rect rect;

            pWriter->SetCursor(0.0f, 0.0f);

            pWriter->SetFont(*mpFont);
            pWriter->SetFontSize(mFontSize.width, mFontSize.height);

            pWriter->SetLineSpace(mLineSpace);
            pWriter->SetCharSpace(mCharSpace);

            if (mpTagProcessor != NULL) {
                pWriter->SetTagProcessor(mpTagProcessor);
            }
            CalcStringRect(&rect, pWriter, mTextBuf, mTextLen, mSize.width);
            const Size size(rect.GetWidth(), rect.GetHeight());
            math::VEC2 base = GetVtxPos();
            base.x += (mSize.width - size.width) * GetTextMagH();
            base.y -= (mSize.height - size.height) * GetTextMagV();
            rect.left = base.x;
            rect.right = base.x + size.width;
            rect.top = base.y;
            rect.bottom = base.y - size.height;

            return rect;
        }

        f32 TextBox::GetTextMagH() const {
            f32 mag = 0.0f;

            switch (GetTextPositionH()) {
            default:
            case 0: {
                mag = 0.0f;
                break;
            }

            case 1: {
                mag = 0.5f;
                break;
            }

            case 2: {
                mag = 1.0f;
                break;
            }
            }

            return mag;
        }

        f32 TextBox::GetTextMagV() const {
            f32 mag = 0.0f;

            switch (GetTextPositionV()) {
            default:
            case 0: {
                mag = 0.0f;
                break;
            }

            case 1: {
                mag = 0.5f;
                break;
            }

            case 2: {
                mag = 1.0f;
                break;
            }
            }

            return mag;
        }

        u32 TextBox::MakeDrawFlag() const {
            u32 flag = 0;

            switch (GetTextPositionH()) {
            case 1: {
                flag |= 1;
                break;
            }

            case 2: {
                flag |= 2;
                break;
            }

            default: {
                break;
            }
            }

            return flag;
        }

        f32 TextBox::GetTextAlignMag() const {
            switch (GetTextAlignment()) {
            default:
                return GetTextMagH();
            case 1:
                return 0.0f;
            case 2:
                return 0.5f;
            case 3:
                return 1.0f;
            }
        }
    };  // namespace lyt
};  // namespace nw4r
