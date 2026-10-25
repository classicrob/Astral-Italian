#include "nw4r/db/DbgPrintBase.h"
#include "nw4r/ut/TextWriterBase.h"
#include <cstdio>
#include <cstring>
#ifdef GALAXY_NATIVE
#include <petari/utf16.h>
#else
#include <wchar_io.h>
#endif
#include <revolution/gx.h>
#include <revolution/mtx.h>

namespace nw4r {
namespace db {
    template <typename CharT>
    MEMHeapHandle DbgPrintBase<CharT>::mHeapHandle;
    template <typename CharT>
    DbgPrintBase<CharT>* DbgPrintBase<CharT>::mInstance;

    namespace {
        inline int FormatText(char* dest, size_t size, const char* format, va_list args) {
            return vsnprintf(dest, size, format, args);
        }
        inline int FormatText(wchar_t* dest, size_t size, const wchar_t* format, va_list args) {
            return vswprintf(dest, size, format, args);
        }
    }

    template <typename CharT>
    DbgPrintBase<CharT>* DbgPrintBase<CharT>::GetInstance() {
        return mInstance;
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Initialize(void* buffer, u32 size, ut::Color color) {
        SetBuffer(buffer, size);
        mInstance = new DbgPrintBase(color, NULL);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Initialize(void* buffer, u32 size, const ut::Font& font, ut::Color color) {
        SetBuffer(buffer, size);
        mInstance = new DbgPrintBase(color, &font);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::SetFont(const ut::Font& font) { mFont = &font; }
    template <typename CharT>
    const ut::Font* DbgPrintBase<CharT>::GetFont() const { return mFont; }
    template <typename CharT>
    void DbgPrintBase<CharT>::SetTextColor(ut::Color color) { mTextColor = color; }
    template <typename CharT>
    ut::Color DbgPrintBase<CharT>::GetTextColor() const { return mTextColor; }
    template <typename CharT>
    void DbgPrintBase<CharT>::SetFontSize(f32 size) { mFontSize = size; }
    template <typename CharT>
    f32 DbgPrintBase<CharT>::GetFontSize() const { return mFontSize; }
    template <typename CharT>
    void DbgPrintBase<CharT>::SetVisible(bool visible) { mVisible = visible; }
    template <typename CharT>
    bool DbgPrintBase<CharT>::IsVisible() const { return mVisible; }

    template <typename CharT>
    void DbgPrintBase<CharT>::Registerf(int x, int y, const CharT* format, ...) {
        va_list args;
        va_start(args, format);
        VRegisterf(x, y, 1, format, args);
        va_end(args);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Registerf(int x, int y, int life, const CharT* format, ...) {
        va_list args;
        va_start(args, format);
        VRegisterf(x, y, life, format, args);
        va_end(args);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::VRegisterf(int x, int y, int life, const CharT* format, va_list args) {
        int alignment = 4;
        if (life == 1) alignment = -4;
        u32 size = (MEMGetAllocatableSizeForExpHeapEx(mHeapHandle, 4) - 0x1C) / 2;
        MEMSetGroupIDForExpHeap(mHeapHandle, 0);
        CharT* buffer = static_cast<CharT*>(MEMAllocFromExpHeapEx(mHeapHandle, size, alignment));
        if (buffer != NULL) {
            int length = FormatText(buffer, size, format, args);
            Register(x, y, life, buffer, length);
            MEMFreeToExpHeap(mHeapHandle, buffer);
        }
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Register(int x, int y, int life, const CharT* string, int length) {
        int alignment;
        u32 size;
        size = sizeof(DbgText);
        size += length;
        alignment = -4;
        if (life == 1) alignment = 4;
        MEMSetGroupIDForExpHeap(mHeapHandle, 0);
        DbgText* text = static_cast<DbgText*>(MEMAllocFromExpHeapEx(mHeapHandle, size, alignment));
        if (text != NULL) {
            text->x = x;
            text->y = y;
            text->life = life;
            text->length = length;
            // Both target instantiations copy the unscaled length in bytes.
            memcpy(text + 1, string, length);
            ut::List_Append(&mTextList, text);
        }
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Registf(int x, int y, const CharT* format, ...) {
        va_list args;
        va_start(args, format);
        VRegisterf(x, y, 1, format, args);
        va_end(args);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Registf(int x, int y, int life, const CharT* format, ...) {
        va_list args;
        va_start(args, format);
        VRegisterf(x, y, life, format, args);
        va_end(args);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::VRegistf(int x, int y, int life, const CharT* format, va_list args) {
        VRegisterf(x, y, life, format, args);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Regist(int x, int y, int life, const CharT* string, int length) {
        Register(x, y, life, string, length);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Reset() {
        if (mHeapHandle != NULL) {
            DbgText* text = GetFirstText();
            while (text != NULL) {
                DbgText* next = GetNextText(text);
                Unregister(text);
                text = next;
            }
        }
        ut::List_Init(&mTextList, 0x10);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Flush() {
        ut::TextWriterBase<CharT> writer;
        if (mFont == NULL) return;
        writer.SetFont(*mFont);
        writer.SetTextColor(mTextColor);
        if (mFontSize >= 0.0f) writer.SetFontSize(mFontSize);
        writer.SetupGX();
        DbgText* text = GetFirstText();
        while (text != NULL) {
            DbgText* next = GetNextText(text);
            if (mVisible) {
                writer.SetCursor(text->x, text->y);
                writer.Print(reinterpret_cast<const CharT*>(text + 1), text->length);
            }
            if (--text->life <= 0) Unregister(text);
            text = next;
        }
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Flush(int x, int y, int width, int height) {
        Mtx44 projection;
        Mtx position;
        f32 left = x;
        f32 top = y;
        f32 right = x + width;
        f32 bottom = y + height;
        C_MTXOrtho(projection, top, bottom, left, right, 0.0f, 1.0f);
        GXSetProjection(projection, GX_ORTHOGRAPHIC);
        PSMTXIdentity(position);
        GXLoadPosMtxImm(position, GX_PNMTX0);
        GXSetCurrentMtx(GX_PNMTX0);
        Flush();
    }

    template <typename CharT>
    DbgPrintBase<CharT>::DbgPrintBase(ut::Color color, const ut::Font* font)
        : mFont(font), mTextColor(color), mFontSize(-1.0f), mVisible(true) {
        ut::List_Init(&mTextList, 0x10);
    }

    template <typename CharT>
    DbgPrintBase<CharT>::~DbgPrintBase() {}

    template <typename CharT>
    typename DbgPrintBase<CharT>::DbgText* DbgPrintBase<CharT>::GetFirstText() {
        return static_cast<DbgText*>(ut::List_GetNext(&mTextList, NULL));
    }

    template <typename CharT>
    typename DbgPrintBase<CharT>::DbgText* DbgPrintBase<CharT>::GetNextText(DbgText* text) {
        return static_cast<DbgText*>(ut::List_GetNext(&mTextList, text));
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::Unregister(DbgText* text) {
        ut::List_Remove(&mTextList, text);
        MEMFreeToExpHeap(mHeapHandle, text);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::SetBuffer(void* buffer, u32 size) {
        mHeapHandle = MEMCreateExpHeapEx(buffer, size, 0);
    }

    template <typename CharT>
    void* DbgPrintBase<CharT>::ReleaseBuffer() {
        void* buffer = NULL;
        if (mHeapHandle != NULL) {
            delete mInstance;
            buffer = mHeapHandle;
            MEMDestroyExpHeap(mHeapHandle);
            mHeapHandle = NULL;
            mInstance = NULL;
        }
        return buffer;
    }

    template <typename CharT>
    void* DbgPrintBase<CharT>::operator new(size_t size) {
        MEMSetGroupIDForExpHeap(mHeapHandle, 0);
        return MEMAllocFromExpHeapEx(mHeapHandle, size, 4);
    }

    template <typename CharT>
    void DbgPrintBase<CharT>::operator delete(void* pointer) {
        MEMFreeToExpHeap(mHeapHandle, pointer);
    }

    template class DbgPrintBase<char>;
    template class DbgPrintBase<wchar_t>;
}
}
