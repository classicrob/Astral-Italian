#pragma once

#include "nw4r/ut/Color.h"
#include "nw4r/ut/Font.h"
#include "nw4r/ut/list.h"
#include <revolution/mem/expHeap.h>
#include <va_list.h>

namespace nw4r {
namespace db {
    template <typename CharT>
    class DbgPrintBase {
    public:
        struct DbgText {
            int x;
            int y;
            int life;
            int length;
            ut::Link link;
        };

        static DbgPrintBase* GetInstance();
        static void Initialize(void*, u32, ut::Color);
        static void Initialize(void*, u32, const ut::Font&, ut::Color);
        void SetFont(const ut::Font&);
        const ut::Font* GetFont() const;
        void SetTextColor(ut::Color);
        ut::Color GetTextColor() const;
        void SetFontSize(f32);
        f32 GetFontSize() const;
        void SetVisible(bool);
        bool IsVisible() const;
        void Registerf(int, int, const CharT*, ...);
        void Registerf(int, int, int, const CharT*, ...);
        void VRegisterf(int, int, int, const CharT*, va_list);
        void Register(int, int, int, const CharT*, int) NO_INLINE;
        void Registf(int, int, const CharT*, ...);
        void Registf(int, int, int, const CharT*, ...);
        void VRegistf(int, int, int, const CharT*, va_list);
        void Regist(int, int, int, const CharT*, int);
        void Reset();
        void Flush();
        void Flush(int, int, int, int);
        DbgPrintBase(ut::Color, const ut::Font*);
        ~DbgPrintBase();
        DbgText* GetFirstText();
        DbgText* GetNextText(DbgText*);
        void Unregister(DbgText*);
        static void SetBuffer(void*, u32);
        static void* ReleaseBuffer();
        static void* operator new(size_t);
        static void operator delete(void*);

    private:
        const ut::Font* mFont;
        ut::List mTextList;
        ut::Color mTextColor;
        f32 mFontSize;
        bool mVisible;
        static MEMHeapHandle mHeapHandle;
        static DbgPrintBase* mInstance;
    };
}
}
