#pragma once

#include <stdint.h>
#include "convToUnicode.h"


extern const int32_t	jis12Table[];
extern int32_t			jis12RevTable[];
extern const size_t		JIS12TABLESIZE;

extern const int32_t	jis12WinTable[];
extern       int32_t	jis12WinRevTable[];
extern const size_t		JIS12WINTABLESIZE;

extern const int32_t	jis3Table[];
extern       int32_t	jis3RevTable[];
extern const size_t		JIS3TABLESIZE;

extern       int32_t	jis3CombIvsTable[];
extern       int32_t	jis3CombIvsRevTable[];
extern const size_t		JIS3COMBIVSTABLESIZE;

extern const int32_t	jis4Table[];
extern       int32_t	jis4RevTable[];
extern const size_t		JIS4TABLESIZE;

extern const int32_t	kigou1Table[];
extern       int32_t	kigou1RevTable[];
extern const size_t		KIGOU1TABLESIZE;

extern const char16_t	kigou2TableUTF16[][UTF16TABLELEN];
extern       char16_t	kigou2RevTableUTF16[][UTF16TABLELEN];
extern const size_t		KIGOU2TABLEUTF16SIZE;

extern const uint8_t	kigou2TableUTF8[][UTF8TABLELEN];
extern       uint8_t	kigou2RevTableUTF8[][UTF8TABLELEN];
extern const size_t		KIGOU2TABLEUTF8SIZE;

extern const int32_t	hiragana1Table[];
extern       int32_t	hiragana1RevTable[];
extern const size_t		HIRAGANA1TABLESIZE;


extern const int32_t	katakana1Table[];
extern       int32_t	katakana1RevTable[];
extern const size_t		KATAKANA1TABLESIZE;

extern const int32_t	kanaCommon1Table[];
extern       int32_t	kanaCommon1RevTable[];
extern const size_t		KANACOMMON1TABLESIZE;

extern const int32_t	hankaku1Table[];
extern       int32_t	hankaku1RevTable[];
extern const size_t		HANKAKU1TABLESIZE;

extern const int32_t	charSize1Table[];
extern       int32_t	charSize1RevTable[];
extern const size_t		CHARSIZE1TABLESIZE;

extern const int32_t	charSize2Table[];
extern       int32_t	charSize2RevTable[];
extern const size_t		CHARSIZE2TABLESIZE;
