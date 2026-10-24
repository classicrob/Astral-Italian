#include "revolution/nand.h"
#include "revolution/sc.h"
#include "revolution/os/OSFont.h"

static const char* const s_MaxBlocksUS[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\213\363\202\253\227e\227\312\202\252\210\331\217\355\202\305\202\267\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nThere is no more available space in\nWii system memory. Refer to the Wii\nOperations Manual for details.",
    "\n\n\nDer Speicher der Wii-Konsole ist belegt.\nBitte lies die Wii-Bedienungsanleitung,\num weitere Informationen zu erhalten.",
    "\n\n\nIl n'y a pas assez d'espace libre\ndans la m\351moire de la console Wii.\nVeuillez vous r\351f\351rer au Mode d'emploi\nde la Wii pour plus de d\351tails.",
    "\n\n\nNo queda espacio libre en la memoria\nde la consola Wii. Consulta el manual\nde operaciones de la consola Wii para\nobtener m\341s informaci\363n.",
    "\n\n\nNon c'\350 pi\371 spazio libero nella memoria\ndella console Wii. Per maggiori\ninformazioni, consulta il manuale di\nistruzioni della console Wii.",
    "\n\n\nEr is geen vrije ruimte meer in het\ninterne geheugen van het Wii-systeem.\nLees de handleiding voor meer informatie.",
};

static const char* const s_MaxBlocksEU[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\213\363\202\253\227e\227\312\202\252\210\331\217\355\202\305\202\267\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nThere is no more available space in\nthe Wii System Memory. Please refer to\nthe Wii Operations Manual for details.",
    "\n\n\nDer Speicher der Wii-Konsole ist belegt.\nBitte lies die Wii-Bedienungsanleitung,\num weitere Informationen zu erhalten.",
    "\n\n\nIl n'y a pas assez d'espace libre dans\nla m\351moire de la console Wii. Veuillez\nvous r\351f\351rer au mode d'emploi Wii pour\nplus de d\351tails.",
    "\n\n\nNo queda espacio libre en la memoria de\nla consola Wii. Consulta el manual de\ninstrucciones de la consola Wii para\nobtener m\341s informaci\363n.",
    "\n\n\nNon c'\350 pi\371 spazio libero nella memoria\ndella console Wii. Per maggiori\ninformazioni, consulta il manuale di\nistruzioni della console Wii.",
    "\n\n\nEr is geen vrije ruimte meer in het\ninterne geheugen van het Wii-systeem.\nLees de handleiding voor meer informatie.",
};

static const char* const s_MaxFilesUS[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\213\363\202\253\203t\203@\203C\203\213\220\224\202\252\210\331\217\355\202\305\202\267\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nThere is no more available space in\nWii system memory. Refer to the Wii\nOperations Manual for details.",
    "\n\n\nDer Speicher der Wii-Konsole ist belegt.\nBitte lies die Wii-Bedienungsanleitung,\num weitere Informationen zu erhalten.",
    "\n\n\nIl n'y a pas assez d'espace libre dans\nla m\351moire de la console Wii. Veuillez\nvous r\351f\351rer au Mode d'emploi de la Wii\npour plus de d\351tails.",
    "\n\n\nNo queda espacio libre en la memoria de\nla consola Wii. Consulta el manual de\noperaciones de la consola Wii para\nobtener m\341s informaci\363n.",
    "\n\n\nImpossibile salvare altri dati nella\nmemoria della console Wii. Per maggiori\ninformazioni, consulta il manuale di\nistruzioni della console Wii.",
    "\n\n\nEr is geen ruimte meer beschikbaar\nin het interne geheugen van het\nWii-systeem. Lees de handleiding voor\nmeer informatie.",
};

static const char* const s_MaxFilesEU[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\213\363\202\253\203t\203@\203C\203\213\220\224\202\252\210\331\217\355\202\305\202\267\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nThere is no more available space in\nthe Wii System Memory. Please refer to\nthe Wii Operations Manual for details.",
    "\n\n\nDer Speicher der Wii-Konsole ist belegt.\nBitte lies die Wii-Bedienungsanleitung,\num weitere Informationen zu erhalten.",
    "\n\n\nIl n'y a pas assez d'espace libre dans\nla m\351moire de la console Wii. Veuillez\nvous r\351f\351rer au mode d'emploi Wii pour\nplus de d\351tails.",
    "\n\n\nNo queda espacio libre en la memoria de\nla consola Wii. Consulta el manual de\ninstrucciones de la consola Wii para\nobtener m\341s informaci\363n.",
    "\n\n\nImpossibile salvare altri dati nella\nmemoria della console Wii. Per maggiori\ninformazioni, consulta il manuale di\nistruzioni della console Wii.",
    "\n\n\nEr is geen ruimte meer beschikbaar\nin het interne geheugen van het\nWii-systeem. Lees de handleiding voor\nmeer informatie.",
};

static const char* const s_CorruptUS[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\252\211\363\202\352\202\334\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nThe Wii system memory has been damaged.\nRefer to the Wii Operations Manual for\ndetails.",
    "\n\n\nDer Speicher der Wii-Konsole\nist besch\344digt. Bitte lies die\nWii-Bedienungsanleitung, um weitere\nInformationen zu erhalten.",
    "\n\n\nLa m\351moire de la console Wii a \351t\351 \nendommag\351e. Veuillez vous r\351f\351rer au\nMode d'emploi de la Wii pour plus de\nd\351tails.",
    "\n\n\nLa memoria de la consola Wii\nest\341 da\361ada. Consulta el manual de\noperaciones de la consola Wii para\nobtener m\341s informaci\363n.",
    "\n\n\nLa memoria della console Wii e\ndanneggiata. Per maggiori informazioni,\nconsulta il manuale di istruzioni della\nconsole Wii.",
    "\n\n\nHet interne geheugen van het\nWii-systeem is beschadigd. Lees de\nWii-handleiding voor meer informatie.",
};

static const char* const s_CorruptEU[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\252\211\363\202\352\202\334\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nThe Wii System Memory has been damaged.\nPlease refer to the Wii Operations Manual\nfor details.",
    "\n\n\nDer Speicher der Wii-Konsole\nist beschadigt. Bitte lies die\nWii-Bedienungsanleitung, um weitere\nInformationen zu erhalten.",
    "\n\n\nLa m\351moire de la console Wii est\nendommag\351e. Veuillez vous r\351f\351rer au\nmode d'emploi Wii pour plus de d\351tails.\n",
    "\n\n\nLa memoria de la consola Wii est\341 da\361ada.\nConsulta el manual de instrucciones de la\nconsola Wii para obtener m\341s informaci\363n.",
    "\n\n\nLa memoria della console Wii e\ndanneggiata. Per maggiori informazioni,\nconsulta il manuale di istruzioni della\nconsole Wii.",
    "\n\n\nHet interne geheugen van het\nWii-systeem is beschadigd. Lees de\nWii-handleiding voor meer informatie.",
};

static const char* const s_AccessUS[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\217\221\202\253\215\236\202\335/\223\307\202\335\217o\202\265\202\252\n\202\305\202\253\202\334\202\271\202\361\202\305\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nCould not access Wii system memory.\nRefer to the Wii Operations Manual for\ndetails.",
    "\n\n\nAuf den Speicher der Wii-Konsole konnte\nnicht zugegriffen werden. Bitte lies die\nWii-Bedienungsanleitung, um weitere\nInformationen zu erhalten.",
    "\n\n\nImpossible d'acc\351der \340 la m\351moire de\nla console Wii. Veuillez vous r\351f\351rer\nau Mode d'emploi de la Wii pour plus\nde d\351tails.",
    "\n\n\nNo se ha podido acceder a la memoria de\nla consola Wii. Consulta el manual de\noperaciones de la consola Wii para\nobtener m\341s informaci\363n.",
    "\n\n\nImpossibile accedere alla memoria della\nconsole Wii. Per maggiori informazioni,\nconsulta il manuale di istruzioni della\nconsole Wii.",
    "\n\n\nHet interne geheugen van het Wii-systeem\nkan niet worden gelezen of beschreven.\nLees de Wii-handleiding voor meer\ninformatie.",
};

static const char* const s_AccessEU[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\217\221\202\253\215\236\202\335/\223\307\202\335\217o\202\265\202\252\n\202\305\202\253\202\334\202\271\202\361\202\305\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nCould not access the Wii System Memory.Please refer to the Wii Operations Manual\nfor details.",
    "\n\n\nAuf den Speicher der Wii-Konsole konnte\nnicht zugegriffen werden. Bitte lies die\nWii-Bedienungsanleitung, um weitere\nInformationen zu erhalten.",
    "\n\n\nImpossible d'acc\351der \340 la m\351moire de la\nconsole Wii. Veuillez vous r\351f\351rer au\nmode d'emploi Wii pour plus de d\351tails.",
    "\n\n\nNo se ha podido acceder a la memoria de\nla consola Wii. Consulta el manual de\ninstrucciones de la consola Wii para\nobtener m\341s informaci\363n.",
    "\n\n\nImpossibile accedere alla memoria della\nconsole Wii. Per maggiori informazioni,\nconsulta il manuale di istruzioni della\nconsole Wii.",
    "\n\n\nHet interne geheugen van het Wii-systeem\nkan niet worden gelezen of beschreven.\nLees de Wii-handleiding voor meer\ninformatie.",
};

static const char* const s_UnexpectedUS[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\217\221\202\253\215\236\202\335/\223\307\202\335\217o\202\265\222\206\202\311\n\203G\203\211\201[\202\252\224\255\220\266\202\265\202\334\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nAn error occurred while accessing Wii\nsystem memory. Refer to the Wii\nOperations Manual for details.",
    "\n\n\nBeim Zugriff auf den Speicher der\nWii-Konsole ist ein Fehler aufgetreten.\nBitte lies die Wii-Bedienungsanleitung,\num weitere Informationen zu erhalten.",
    "\n\n\nUne erreur est survenue pendant le\nprocessus de lecture ou d'\351criture\ndans la m\351moire de la console Wii.\nVeuillez vous r\351f\351rer au Mode d'emploi\nde la Wii pour plus de d\351tails.",
    "\n\n\nSe ha producido un error al intentar\nacceder a la memoria de la consola Wii.\nConsulta el manual de operaciones\nde la consola Wii para obtener m\341s\ninformaci\363n.",
    "\n\n\nSi \350 verificato un errore durante la\nlettura o la modifica dei dati\nall'interno della memoria della\nconsole Wii. Per maggiori informazioni,\nconsulta il manuale di istruzioni della\nconsole Wii.",
    "\n\n\nEr is een fout opgetreden tijdens het\nlezen of beschrijven van het interne\ngeheugen van het Wii-systeem. Lees de\nWii-handleiding voor meer informatie.",
};

static const char* const s_UnexpectedEU[] = {
    "\n\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\217\221\202\253\215\236\202\335/\223\307\202\335\217o\202\265\222\206\202\311\n\203G\203\211\201[\202\252\224\255\220\266\202\265\202\334\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\n\nAn error occurred during the process of\nreading from or writing to the Wii System\nMemory. Please refer to the Wii Operations\nManual for details.",
    "\n\n\nBeim Zugriff auf den Speicher der\nWii-Konsole ist ein Fehler aufgetreten.\nBitte lies die Wii-Bedienungsanleitung,\num weitere Informationen zu erhalten.",
    "\n\n\nUne erreur est survenue avec la m\351moire\nde la console Wii pendant le processus\nde lecture ou d'\351criture. Veuillez vous\nr\351f\351rer au mode d'emploi Wii pour plus\nde d\350tails.",
    "\n\n\nSe ha producido un error durante la\nlectura o escritura de la memoria de\nla consola Wii. Consulta el manual de\ninstrucciones de la consola Wii para\nobtener m\341s informaci\363n.",
    "\n\n\nSi \350 verificato un errore durante la\nlettura o la modifica dei dati\nall'interno della memoria della\nconsole Wii. Per maggiori informazioni,\nconsulta il manuale di istruzioni della\nconsole Wii.",
    "\n\n\nEr is een fout opgetreden tijdens het\nlezen of beschrijven van het interne\ngeheugen van het Wii-systeem. Lees de\nWii-handleiding voor meer informatie.",
};

static const char* const s_MaxBlocksCompact[] = {
    "\n\n\203G\203\211\201[\203R\201[\203h\202S\202O\202T\201B\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\213\363\202\253\227e\227\312\202\252\210\331\217\355\202\305\202\267\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\nError #405,\n\nThere is no more available space in\nWii system memory. Refer to the Wii\nOperations Manual for details.",
};

static const char* const s_MaxFilesCompact[] = {
    "\n\n\203G\203\211\201[\203R\201[\203h\202S\202O\202U\201B\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\213\363\202\253\203t\203@\203C\203\213\220\224\202\252\210\331\217\355\202\305\202\267\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\nError #406,\n\nThere is no more available space in\nWii system memory. Refer to the Wii\nOperations Manual for details.",
};

static const char* const s_CorruptCompact[] = {
    "\n\n\203G\203\211\201[\203R\201[\203h\202S\202O\202W\201B\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\252\211\363\202\352\202\334\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\nError #408,\n\nThe Wii system memory has been damaged.\nRefer to the Wii Operations Manual for\ndetails.",
};

static const char* const s_AccessCompact[] = {
    "\n\n\203G\203\211\201[\203R\201[\203h\202S\202P\202P\201B\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\217\221\202\253\215\236\202\335/\223\307\202\335\217o\202\265\202\252\n\202\305\202\253\202\334\202\271\202\361\202\305\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\nError #411,\n\nCould not access Wii system memory.\nRefer to the Wii Operations Manual for\ndetails.",
};

static const char* const s_UnexpectedCompact[] = {
    "\n\n\203G\203\211\201[\203R\201[\203h\202S\202P\202Q\201B\n\nWii\226{\221\314\225\333\221\266\203\201\203\202\203\212\202\314\217\221\202\253\215\236\202\335/\223\307\202\335\217o\202\265\222\206\202\311\n\203G\203\211\201[\202\252\224\255\220\266\202\265\202\334\202\265\202\275\201B\n\202\255\202\355\202\265\202\255\202\315Wii\226{\221\314\202\314\216\346\210\265\220\340\226\276\217\221\202\360\202\250\223\307\202\335\n\202\255\202\276\202\263\202\242\201B",
    "\n\nError #412,\n\nAn error occurred while accessing Wii\nsystem memory. Refer to the Wii\nOperations Manual for details.",
};

typedef struct {
    ISFSError error;
    s32 region;
    const char* const* messages;
} NANDErrorMessageEntry;

static void (*NANDErrorFunc)(ISFSError);

static inline const char* const* getErrorMessages(ISFSError error, s32 region) {
    u32 index = 0;
    NANDErrorMessageEntry entries[] = {
        {ISFS_ERROR_MAXBLOCKS, 0, s_MaxBlocksUS},
        {ISFS_ERROR_MAXFILES, 0, s_MaxFilesUS},
        {ISFS_ERROR_CORRUPT, 0, s_CorruptUS},
        {ISFS_ERROR_BUSY, 0, s_AccessUS},
        {IOS_ERROR_FAIL_ALLOC, 0, s_AccessUS},
        {ISFS_ERROR_UNKNOWN, 0, s_UnexpectedUS},
        {IOS_ERROR_UNKNOWN, 0, s_UnexpectedUS},
        {ISFS_ERROR_MAXBLOCKS, 1, s_MaxBlocksEU},
        {ISFS_ERROR_MAXFILES, 1, s_MaxFilesEU},
        {ISFS_ERROR_CORRUPT, 1, s_CorruptEU},
        {ISFS_ERROR_BUSY, 1, s_AccessEU},
        {IOS_ERROR_FAIL_ALLOC, 1, s_AccessEU},
        {ISFS_ERROR_UNKNOWN, 1, s_UnexpectedEU},
        {IOS_ERROR_UNKNOWN, 1, s_UnexpectedEU},
        {ISFS_ERROR_MAXBLOCKS, 2, s_MaxBlocksCompact},
        {ISFS_ERROR_MAXFILES, 2, s_MaxFilesCompact},
        {ISFS_ERROR_CORRUPT, 2, s_CorruptCompact},
        {ISFS_ERROR_BUSY, 2, s_AccessCompact},
        {IOS_ERROR_FAIL_ALLOC, 2, s_AccessCompact},
        {ISFS_ERROR_UNKNOWN, 2, s_UnexpectedCompact},
        {IOS_ERROR_UNKNOWN, 2, s_UnexpectedCompact},
    };
    while ((u8)index < sizeof(entries) / sizeof(entries[0])) {
        if (entries[(u8)index].region == region) {
            if (entries[(u8)index].error == error) {
                return entries[(u8)index].messages;
            }
            index++;
        } else {
            index += 7;
        }
    }
    return s_UnexpectedUS;
}

// NONMATCHING: equivalent signed region 4/5 test lowering and loop-NOP
// placement after three forms; all 256 region values and other code agree.
static void __NANDShowErrorMessage(ISFSError error) {
    GXColor background = {0, 0, 0, 0};
    GXColor foreground = {255, 255, 255, 0};
    const char* const* messages;
    const char* message;
    s32 region;

    if (SCGetLanguage() == SC_LANG_JAPANESE) {
        OSSetFontEncode(1);
    } else {
        OSSetFontEncode(0);
    }

    region = SCGetProductGameRegion();
    if ((u32)(region - 4) > 1) {
        if (region != 2) {
            messages = getErrorMessages(error, 0);
        } else {
            messages = getErrorMessages(error, 1);
        }
    } else {
        messages = getErrorMessages(error, 2);
    }

    if (SCGetLanguage() > SC_LANG_DUTCH) {
        message = messages[SC_LANG_ENGLISH];
    } else {
        message = messages[SCGetLanguage()];
    }
    OSFatal(foreground, background, message);
}

BOOL NANDSetAutoErrorMessaging(BOOL enable) {
    BOOL previous;
    BOOL interrupts = OSDisableInterrupts();
    previous = NANDErrorFunc != NULL;
    NANDErrorFunc = enable ? __NANDShowErrorMessage : NULL;
    OSRestoreInterrupts(interrupts);
    return previous;
}

void __NANDPrintErrorMessage(ISFSError error) {
    if (NANDErrorFunc) {
        NANDErrorFunc(error);
    }
}
