#include <cstdio>
#ifdef GALAXY_NATIVE
#include <petari/utf16.h>
#else
#include <wchar_io.h>
#endif
#include "nw4r/math/constant.h"
#include "nw4r/ut/TextWriterBase.h"
#include "nw4r/ut/inlines.h"

namespace nw4r {
    namespace ut {
        template < typename CharT >
        typename TextWriterBase< CharT >::TagProcessor TextWriterBase< CharT >::mDefaultTagProcessor;
        template < typename CharType >
        TextWriterBase< CharType >::TextWriterBase()
            : CharWriter(), mWidthLimit(nw4r::math::F_MAX), mCharSpace(0), mLineSpace(0), mTabWidth(4), mDrawFlag(DEFAULT_DRAWFLAG),
              mTagProcessor(&mDefaultTagProcessor) {
        }

        template < typename CharType >
        TextWriterBase< CharType >::~TextWriterBase() {
        }

        template < typename CharType >
        f32 TextWriterBase< CharType >::GetLineHeight() const {
            const Font* font = GetFont();
            const int linefeed = (font != NULL) ? font->GetLineFeed() : 0;
            return linefeed * GetScaleV() + mLineSpace;
        }

        template < typename CharType >
        f32 TextWriterBase< CharType >::CalcStringWidth(StreamType str, int length) const {
            Rect rect;

            CalcStringRect(&rect, str, length);

            return rect.GetWidth();
        }

        template < typename CharType >
        f32 TextWriterBase< CharType >::Print(StreamType str, int length) {
            TextWriterBase< CharType > myCopy = *this;
            f32 width = myCopy.PrintImpl(str, length, false);
            SetCursor(myCopy.GetCursorX(), myCopy.GetCursorY());
            return width;
        }

        template < typename CharType >
        f32 TextWriterBase< CharType >::CalcLineWidth(StreamType str, int length) {
            Rect rect;
            TextWriterBase< CharType > myCopy = *this;
            myCopy.SetCursor(0, 0);
            myCopy.CalcLineRectImpl(&rect, &str, length);
            return rect.GetWidth();
        }

        template < typename CharType >
        bool TextWriterBase< CharType >::CalcLineRectImpl(Rect* pRect, StreamType* pStr, int length) {
            const StreamType str = *pStr;
            const StreamType end = str + length;
            const bool bUseLimit = (mWidthLimit < nw4r::math::F_MAX);
            PrintContext< CharType > context = {this, str, 0, 0, 0};
            f32 x = 0;
            bool bCharSpace = false;
            bool bOverLimit = false;
            StreamType prevStreamPos = NULL;
            Rect prevRect;
            CharStrmReader reader = GetFont()->GetCharStrmReader();

            pRect->left = 0;
            pRect->right = 0;
            pRect->top = Min(0.0f, GetLineHeight());
            pRect->bottom = Max(0.0f, GetLineHeight());
            prevRect = *pRect;

            reader.Set(str);
            prevStreamPos = NULL;

            for (CharCode code = reader.Next(); reinterpret_cast< StreamType >(reader.GetCurrentPos()) <= end;) {
                if (code < ' ') {
                    typename TagProcessor::Operation operation;
                    Rect rect(x, 0, 0, 0);

                    context.str = reinterpret_cast< StreamType >(reader.GetCurrentPos());
                    context.flags = 0;
                    context.flags |= bCharSpace ? 0 : CONTEXT_NO_CHAR_SPACE;
                    SetCursorX(x);

                    if (bUseLimit && (code != '\n') && (prevStreamPos != NULL)) {
                        PrintContext< CharType > context2 = context;
                        TextWriterBase< CharType > myCopy = *this;
                        Rect rect;

                        context2.writer = &myCopy;
                        operation = mTagProcessor->CalcRect(&rect, code, &context2);

                        if ((rect.GetWidth() > 0.0f) && (myCopy.GetCursorX() - context.xOrigin > mWidthLimit)) {
                            bOverLimit = true;
                            code = '\n';
                            reader.Set(prevStreamPos);
                            continue;
                        }
                    }

                    operation = mTagProcessor->CalcRect(&rect, code, &context);
                    reader.Set(context.str);

                    pRect->left = Min(pRect->left, rect.left);
                    pRect->top = Min(pRect->top, rect.top);
                    pRect->right = Max(pRect->right, rect.right);
                    pRect->bottom = Max(pRect->bottom, rect.bottom);
                    x = GetCursorX();

                    if (operation == TagProcessor::OPERATION_END_DRAW) {
                        *pStr += length;
                        return false;
                    } else if (operation == TagProcessor::OPERATION_NO_CHAR_SPACE) {
                        bCharSpace = false;
                    } else if (operation == TagProcessor::OPERATION_CHAR_SPACE) {
                        bCharSpace = true;
                    } else if (operation == TagProcessor::OPERATION_NEXT_LINE) {
                        break;
                    }
                } else {
                    f32 dx = 0.0f;

                    if (bCharSpace) {
                        dx += GetCharSpace();
                    }

                    if (IsWidthFixed()) {
                        dx += GetFixedWidth();
                    } else {
                        dx += GetFont()->GetCharWidth(code) * GetScaleH();
                    }

                    if (bUseLimit && (prevStreamPos != NULL)) {
                        if (x + dx > mWidthLimit) {
                            bOverLimit = true;
                            code = '\n';
                            reader.Set(prevStreamPos);
                            continue;
                        }
                    }

                    x += dx;
                    pRect->left = Min(pRect->left, x);
                    pRect->right = Max(pRect->right, x);
                    bCharSpace = true;
                }

                if (bUseLimit) {
                    prevStreamPos = reinterpret_cast< StreamType >(reader.GetCurrentPos());
                }

#ifdef GALAXY_NATIVE
                // The original reads the character after the end before the loop test stops it (past
                // a one-character buffer here and there). Leave the stream one character past the end
                // without reading it.
                if (reinterpret_cast< StreamType >(reader.GetCurrentPos()) >= end) {
                    reader.Set(end + 1);
                    break;
                }
#endif
                code = reader.Next();
            }

            *pStr = reinterpret_cast< StreamType >(reader.GetCurrentPos());

            return bOverLimit;
        }

        template < typename CharType >
        void TextWriterBase< CharType >::CalcStringRect(Rect* pRect, StreamType str, int length) const {
            TextWriterBase< CharType > myCopy = *this;
            myCopy.CalcStringRectImpl(pRect, str, length);
        }

        template < typename CharType >
        void TextWriterBase< CharType >::CalcStringRectImpl(Rect* pRect, StreamType str, int length) {
            const StreamType end = str + length;
            int remain = length;
            StreamType pos = str;

            pRect->left = 0;
            pRect->right = 0;
            pRect->top = 0;
            pRect->bottom = 0;

            SetCursor(0, 0);

            do {
                Rect rect;
                CalcLineRectImpl(&rect, &pos, remain);
                remain = (end - pos);

                pRect->left = Min(pRect->left, rect.left);
                pRect->top = Min(pRect->top, rect.top);
                pRect->right = Max(pRect->right, rect.right);
                pRect->bottom = Max(pRect->bottom, rect.bottom);
            } while (remain > 0);
        }

        template < typename CharType >
        f32 TextWriterBase< CharType >::AdjustCursor(f32* xOrigin, f32* yOrigin, StreamType str, int length) {
            f32 textWidth = 0;
            f32 textHeight = 0;

            if (!IsDrawFlagSet((HORIZONTAL_ALIGN_MASK | HORIZONTAL_ORIGIN_MASK | VERTICAL_ORIGIN_MASK),
                               (HORIZONTAL_ALIGN_LEFT | HORIZONTAL_ORIGIN_LEFT | VERTICAL_ORIGIN_BASELINE)) &&
                !IsDrawFlagSet((HORIZONTAL_ALIGN_MASK | HORIZONTAL_ORIGIN_MASK | VERTICAL_ORIGIN_MASK),
                               (HORIZONTAL_ALIGN_LEFT | HORIZONTAL_ORIGIN_LEFT | VERTICAL_ORIGIN_TOP))) {
                Rect textRect;
                CalcStringRect(&textRect, str, length);
                textWidth = textRect.left + textRect.right;
                textHeight = textRect.top + textRect.bottom;

            }

            if (IsDrawFlagSet(HORIZONTAL_ORIGIN_MASK, HORIZONTAL_ORIGIN_CENTER)) {
                *xOrigin -= textWidth / 2;
            } else if (IsDrawFlagSet(HORIZONTAL_ORIGIN_MASK, HORIZONTAL_ORIGIN_RIGHT)) {
                *xOrigin -= textWidth;
            }

            if (IsDrawFlagSet(VERTICAL_ORIGIN_MASK, VERTICAL_ORIGIN_MIDDLE)) {
                *yOrigin -= textHeight / 2;
            } else if (IsDrawFlagSet(VERTICAL_ORIGIN_MASK, VERTICAL_ORIGIN_BOTTOM)) {
                *yOrigin -= textHeight;
            }

            if (IsDrawFlagSet(HORIZONTAL_ALIGN_MASK, HORIZONTAL_ALIGN_CENTER)) {
                const f32 width = CalcLineWidth(str, length);
                const f32 offset = (textWidth - width) / 2;
                SetCursorX(*xOrigin + offset);
            } else if (IsDrawFlagSet(HORIZONTAL_ALIGN_MASK, HORIZONTAL_ALIGN_RIGHT)) {
                const f32 width = CalcLineWidth(str, length);
                const f32 offset = textWidth - width;
                SetCursorX(*xOrigin + offset);
            } else {
                SetCursorX(*xOrigin);
            }

            if (IsDrawFlagSet(VERTICAL_ORIGIN_MASK, VERTICAL_ORIGIN_BASELINE)) {
                SetCursorY(*yOrigin);
            } else {
                SetCursorY(*yOrigin + GetFontAscent());
            }

            return textWidth;
        }


        template <typename CharType> CharType* TextWriterBase<CharType>::mFormatBuffer = NULL;
        template <typename CharType> int TextWriterBase<CharType>::mFormatBufferSize = 256;

        template <> int TextWriterBase<char>::VSNPrintf(char* buffer, u32 size, const char* format, va_list args) {
            return vsnprintf(buffer, size, format, args);
        }
        template <> int TextWriterBase<wchar_t>::VSNPrintf(wchar_t* buffer, u32 size, const wchar_t* format, va_list args) {
            return vswprintf(buffer, size, format, args);
        }

        template <typename CharType>
        void TextWriterBase<CharType>::SetLineHeight(f32 height) {
            const Font* font = GetFont();
            const int linefeed = font != NULL ? font->GetLineFeed() : 0;
            mLineSpace = height - linefeed * GetScaleV();
        }

        template <typename CharType>
        void TextWriterBase<CharType>::CalcVStringRect(Rect* rect, StreamType format, va_list args) const {
#ifdef GALAXY_NATIVE
            CharType* buffer = mFormatBuffer != NULL ? mFormatBuffer : static_cast<CharType*>(__builtin_alloca(sizeof(CharType) * mFormatBufferSize));
#else
            CharType* buffer = mFormatBuffer != NULL ? mFormatBuffer : static_cast<CharType*>(__alloca(mFormatBufferSize));
#endif
            int length = VSNPrintf(buffer, mFormatBufferSize, format, args);
            length = Min(length, mFormatBufferSize - 1);
            CalcStringRect(rect, buffer, length);
        }

        template <typename CharType>
        f32 TextWriterBase<CharType>::CalcFormatStringWidth(StreamType format, ...) const {
            Rect rect;
            va_list args;
            va_start(args, format);
            CalcVStringRect(&rect, format, args);
            va_end(args);
            return rect.GetWidth();
        }
        template <typename CharType>
        f32 TextWriterBase<CharType>::CalcFormatStringHeight(StreamType format, ...) const {
            Rect rect;
            va_list args;
            va_start(args, format);
            CalcVStringRect(&rect, format, args);
            va_end(args);
            return rect.GetHeight();
        }
        template <typename CharType>
        void TextWriterBase<CharType>::CalcFormatStringRect(Rect* rect, StreamType format, ...) const {
            va_list args;
            va_start(args, format);
            CalcVStringRect(rect, format, args);
            va_end(args);
        }
        template <typename CharType>
        f32 TextWriterBase<CharType>::CalcStringHeight(StreamType str, int length) const {
            Rect rect;
            CalcStringRect(&rect, str, length);
            return rect.GetHeight();
        }
        template <typename CharType>
        f32 TextWriterBase<CharType>::VPrintf(StreamType format, va_list args) {
#ifdef GALAXY_NATIVE
            CharType* buffer = mFormatBuffer != NULL ? mFormatBuffer : static_cast<CharType*>(__builtin_alloca(sizeof(CharType) * mFormatBufferSize));
#else
            CharType* buffer = mFormatBuffer != NULL ? mFormatBuffer : static_cast<CharType*>(__alloca(mFormatBufferSize));
#endif
            int length = VSNPrintf(buffer, mFormatBufferSize, format, args);
            return Print(buffer, Min(length, mFormatBufferSize - 1));
        }
        template <typename CharType>
        f32 TextWriterBase<CharType>::Printf(StreamType format, ...) {
            va_list args;
            va_start(args, format);
            f32 width = VPrintf(format, args);
            va_end(args);
            return width;
        }
        template <typename CharType>
        f32 TextWriterBase<CharType>::VPrintfMutable(StreamType format, va_list args) {
#ifdef GALAXY_NATIVE
            CharType* buffer = mFormatBuffer != NULL ? mFormatBuffer : static_cast<CharType*>(__builtin_alloca(sizeof(CharType) * mFormatBufferSize));
#else
            CharType* buffer = mFormatBuffer != NULL ? mFormatBuffer : static_cast<CharType*>(__alloca(mFormatBufferSize));
#endif
            int length = VSNPrintf(buffer, mFormatBufferSize, format, args);
            return PrintMutable(buffer, Min(length, mFormatBufferSize - 1));
        }
        template <typename CharType>
        f32 TextWriterBase<CharType>::PrintfMutable(StreamType format, ...) {
            va_list args;
            va_start(args, format);
            f32 width = VPrintfMutable(format, args);
            va_end(args);
            return width;
        }
        template <typename CharType>
        f32 TextWriterBase<CharType>::PrintMutable(StreamType str, int length) {
            return PrintImpl(str, length, true);
        }

        template <typename CharType>
        f32 TextWriterBase< CharType >::PrintImpl(StreamType str, int length, bool adjustBaseline) {
            NW4R_POINTER_ASSERT_AT(273, this);
            NW4R_POINTER_ASSERT_AT(274, str);
            NW4R_POINTER_ASSERT_AT(275, GetFont());
            NW4R_MIN_ASSERT_AT(276, length, 0);

            f32 xOrigin = GetCursorX();
            f32 yOrigin = GetCursorY();
            const bool bUseLimit = (mWidthLimit < nw4r::math::F_MAX);
            const f32 orgCursorX = xOrigin;
            const f32 orgCursorY = yOrigin;
            f32 xCursorAdj = 0.0f;
            f32 yCursorAdj = 0.0f;
            f32 textWidth = 0;
            bool bCharSpace = false;
            StreamType prevStreamPos = str;
            StreamType prevNewLinePos = str;

            {
                textWidth = AdjustCursor(&xOrigin, &yOrigin, str, length);
                xCursorAdj = orgCursorX - GetCursorX();
                yCursorAdj = orgCursorY - GetCursorY();
            }

            PrintContext< CharType > context = {this, str, xOrigin, yOrigin, 0};
            CharStrmReader reader = GetFont()->GetCharStrmReader();
            reader.Set(str);

            for (CharCode code = reader.Next(); reinterpret_cast< StreamType >(reader.GetCurrentPos()) - str <= length;) {
                if (code < ' ')

                {
                    typename TagProcessor::Operation operation;
                    context.str = reinterpret_cast< StreamType >(reader.GetCurrentPos());
                    context.flags = 0;
                    context.flags |= bCharSpace ? 0 : CONTEXT_NO_CHAR_SPACE;

                    if (bUseLimit && (code != '\n') && (prevStreamPos != prevNewLinePos)) {
                        PrintContext< CharType > context2 = context;
                        TextWriterBase< CharType > myCopy = *this;
                        Rect rect;

                        context2.writer = &myCopy;
                        operation = mTagProcessor->CalcRect(&rect, code, &context2);

                        if ((rect.GetWidth() > 0.0f) && (myCopy.GetCursorX() - context.xOrigin > mWidthLimit)) {
                            code = '\n';
                            reader.Set(prevStreamPos);
                            continue;
                        }
                    }

                    operation = mTagProcessor->Process(code, &context);

                    if (operation == TagProcessor::OPERATION_NEXT_LINE) {
                        NW4R_POINTER_ASSERT_AT(343, context.str);

                        if (IsDrawFlagSet(HORIZONTAL_ALIGN_MASK, HORIZONTAL_ALIGN_CENTER)) {
                            const int remain = length - (context.str - str);
                            const f32 width = CalcLineWidth(context.str, remain);
                            const f32 offset = (textWidth - width) / 2;
                            SetCursorX(context.xOrigin + offset);
                        } else if (IsDrawFlagSet(HORIZONTAL_ALIGN_MASK, HORIZONTAL_ALIGN_RIGHT)) {
                            const int remain = length - (context.str - str);
                            const f32 width = CalcLineWidth(context.str, remain);
                            const f32 offset = textWidth - width;
                            SetCursorX(context.xOrigin + offset);
                        } else {
                            const f32 width = GetCursorX() - context.xOrigin;
                            textWidth = Max(textWidth, width);

                            SetCursorX(context.xOrigin);
                        }

                        if (bUseLimit) {
                            prevNewLinePos = reinterpret_cast< StreamType >(reader.GetCurrentPos());
                        }
                        bCharSpace = false;
                    } else if (operation == TagProcessor::OPERATION_NO_CHAR_SPACE) {
                        bCharSpace = false;
                    } else if (operation == TagProcessor::OPERATION_CHAR_SPACE) {
                        bCharSpace = true;
                    } else if (operation == TagProcessor::OPERATION_END_DRAW) {
                        break;
                    }

                    NW4R_POINTER_ASSERT_AT(388, context.str);
                    reader.Set(context.str);
                } else

                {
                    const f32 baseY = GetCursorY();

                    if (bUseLimit && (prevStreamPos != prevNewLinePos)) {
                        f32 baseX = GetCursorX();
                        f32 charSpace = bCharSpace ? GetCharSpace() : 0.0f;
                        f32 charWidth = IsWidthFixed() ? GetFixedWidth() : GetFont()->GetCharWidth(code) * GetScaleH();

                        if (baseX - xOrigin + charSpace + charWidth > mWidthLimit) {
                            code = '\n';
                            reader.Set(prevStreamPos);
                            continue;
                        }
                    }

                    if (bCharSpace) {
                        MoveCursorX(GetCharSpace());
                    }
                    bCharSpace = true;

                    {
                        const Font* pFont = GetFont();
                        const f32 adj = -pFont->GetBaselinePos() * GetScaleV();
                        MoveCursorY(adj);
                    }

                    CharWriter::Print(code);


                    SetCursorY(baseY);
                }

                if (bUseLimit) {
                    prevStreamPos = reinterpret_cast< StreamType >(reader.GetCurrentPos());
                }

#ifdef GALAXY_NATIVE
                // As in CalcLineRectImpl: stop at the end without reading the character after it.
                if (reinterpret_cast< StreamType >(reader.GetCurrentPos()) - str >= length) {
                    break;
                }
#endif
                code = reader.Next();
            }

            {
                const f32 width = GetCursorX() - context.xOrigin;
                textWidth = Max(textWidth, width);
            }

            if (IsDrawFlagSet(VERTICAL_ORIGIN_MASK, VERTICAL_ORIGIN_MIDDLE) || IsDrawFlagSet(VERTICAL_ORIGIN_MASK, VERTICAL_ORIGIN_BOTTOM)) {
                SetCursorY(orgCursorY);
            } else if (adjustBaseline) {
                if (IsDrawFlagSet(VERTICAL_ORIGIN_MASK, VERTICAL_ORIGIN_TOP)) {
                    SetCursorY(GetCursorY() - GetFontAscent());
                }
            } else {
                MoveCursorY(yCursorAdj);
            }

            return textWidth;
        }

        template class TextWriterBase< char >;
        template class TextWriterBase< wchar_t >;
    };  // namespace ut
};  // namespace nw4r
