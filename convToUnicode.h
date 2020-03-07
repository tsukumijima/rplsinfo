#pragma once

#include <Windows.h>

#include <stdint.h>


// 定数など

#define		REGION_GL			0
#define		REGION_GR			1

#define		BANK_G0				0
#define		BANK_G1				1
#define		BANK_G2				2
#define		BANK_G3				3

#define		F_KANJI				0x42		// Gセットの終端符号
#define		F_ALPHA				0x4A
#define		F_HIRAGANA			0x30
#define		F_KATAKANA			0x31
#define		F_MOSAICA			0x32
#define		F_MOSAICB			0x33
#define		F_MOSAICC			0x34
#define		F_MOSAICD			0x35
#define		F_P_ALPHA			0x36
#define		F_P_HIRAGANA		0x37
#define		F_P_KATAKANA		0x38
#define		F_HANKAKU			0x49
#define		F_JIS1KANJI			0x39
#define		F_JIS2KANJI			0x3A
#define		F_KIGOU				0x3B

#define		F_DRCS0				0x40		// DRCSの終端符号
#define		F_DRCS1				0x41
#define		F_DRCS2				0x42
#define		F_DRCS3				0x43
#define		F_DRCS4				0x44
#define		F_DRCS5				0x45
#define		F_DRCS6				0x46
#define		F_DRCS7				0x47
#define		F_DRCS8				0x48
#define		F_DRCS9				0x49
#define		F_DRCS10			0x4a
#define		F_DRCS11			0x4b
#define		F_DRCS12			0x4c
#define		F_DRCS13			0x4d
#define		F_DRCS14			0x4e
#define		F_DRCS15			0x4f
#define		F_MACRO				0x70	

#define		F_DRCS1A			0x141		// Gセットの終端符号と被らないように誤魔化した
#define		F_DRCS2A			0x142		// 1バイトDRCSの偽終端符号
#define		F_DRCS3A			0x143
#define		F_DRCS4A			0x144
#define		F_DRCS5A			0x145
#define		F_DRCS6A			0x146
#define		F_DRCS7A			0x147
#define		F_DRCS8A			0x148
#define		F_DRCS9A			0x149
#define		F_DRCS10A			0x14a
#define		F_DRCS11A			0x14b
#define		F_DRCS12A			0x14c
#define		F_DRCS13A			0x14d
#define		F_DRCS14A			0x14e
#define		F_DRCS15A			0x14f
#define		F_MACROA			0x170

#define		F_UNKNOWN			0x100		// 内部処理用偽終端符号
#define		F_CONTROL			0x101
#define		F_KANACOMMON		0x102
#define		F_NULL				0x00

#define		C_HALF_CONTROL		0x01010000	// 内部処理用偽終端符号
#define		C_HALF_ALPHA		0x004A0000
#define		C_HALF_KANJI		0x00420000
#define		C_HALF_HIRAGANA		0x00300000
#define		C_HALF_KATAKANA		0x00310000
#define		C_HALF_KANACOMMON	0x01020000
#define		C_HALF_HANKAKU		0x00490000
#define		C_HALF_JIS1KANJI	0x00390000
#define		C_HALF_JIS2KANJI	0x003A0000
#define		C_HALF_KIGOU		0x003B0000
#define		C_FULL_CONTROL		0x11010000
#define		C_FULL_ALPHA		0x104A0000
#define		C_FULL_KANJI		0x10420000
#define		C_FULL_HIRAGANA		0x10300000
#define		C_FULL_KATAKANA		0x10310000
#define		C_FULL_KANACOMMON	0x11020000
#define		C_FULL_HANKAKU		0x10490000
#define		C_FULL_JIS1KANJI	0x10390000
#define		C_FULL_JIS2KANJI	0x103A0000
#define		C_FULL_KIGOU		0x103B0000
#define		C_FULLSIZE			0x10000000
#define		C_UNKNOWN			0x01000000
#define		C_MASK_CHARWIDTH	0xF0000000
#define		C_MASK_CHARCODE		0x0FFFFFFF

#define		UTF16TABLELEN			12
#define		UTF8TABLELEN			12
#define		MAXCONTROLSEQUENCE		5

#define		USE_BANK_NONE					255

#define		USE_BANK_G0_TO_GL				0
#define		USE_BANK_G1_TO_GL				1
#define		USE_BANK_G1_TO_GR				2
#define		USE_BANK_G2_TO_GL				3
#define		USE_BANK_G2_TO_GR				4
#define		USE_BANK_G2_SSHIFT				5
#define		USE_BANK_G3_TO_GL				6
#define		USE_BANK_G3_TO_GR				7
#define		USE_BANK_G3_SSHIFT				8

#define		USE_KANJI_BANK_G0_TO_GL			10
#define		USE_KANJI_BANK_G1_TO_GL			11
#define		USE_KANJI_BANK_G1_TO_GR			12
#define		USE_KANJI_BANK_G2_TO_GL			13
#define		USE_KANJI_BANK_G2_TO_GR			14
#define		USE_KANJI_BANK_G2_SSHIFT		15
#define		USE_KANJI_BANK_G3_TO_GL			16
#define		USE_KANJI_BANK_G3_TO_GR			17
#define		USE_KANJI_BANK_G3_SSHIFT		18

#define		USE_HIRAGANA_BANK_G0_TO_GL		20
#define		USE_HIRAGANA_BANK_G1_TO_GL		21
#define		USE_HIRAGANA_BANK_G1_TO_GR		22
#define		USE_HIRAGANA_BANK_G2_TO_GL		23
#define		USE_HIRAGANA_BANK_G2_TO_GR		24
#define		USE_HIRAGANA_BANK_G2_SSHIFT		25
#define		USE_HIRAGANA_BANK_G3_TO_GL		26
#define		USE_HIRAGANA_BANK_G3_TO_GR		27
#define		USE_HIRAGANA_BANK_G3_SSHIFT		28

#define		USE_KATAKANA_BANK_G0_TO_GL		30
#define		USE_KATAKANA_BANK_G1_TO_GL		31
#define		USE_KATAKANA_BANK_G1_TO_GR		32
#define		USE_KATAKANA_BANK_G2_TO_GL		33
#define		USE_KATAKANA_BANK_G2_TO_GR		34
#define		USE_KATAKANA_BANK_G2_SSHIFT		35
#define		USE_KATAKANA_BANK_G3_TO_GL		36
#define		USE_KATAKANA_BANK_G3_TO_GR		37
#define		USE_KATAKANA_BANK_G3_SSHIFT		38

#define		COST_G0_LS0						1
#define		COST_G1_LS1						1
#define		COST_G2_LS2						2
#define		COST_G3_LS3						2
#define		COST_G1_LS1R					2
#define		COST_G2_LS2R					2
#define		COST_G3_LS3R					2
#define		COST_G2_SS2						1
#define		COST_G3_SS3						1

#define		COST_GSET_TO_G0					3
#define		COST_1GSET_TO_G1				3
#define		COST_2GSET_TO_G1				4
#define		COST_1GSET_TO_G2				3
#define		COST_2GSET_TO_G2				4
#define		COST_1GSET_TO_G3				3
#define		COST_2GSET_TO_G3				4


// 構造体宣言

typedef struct {
	int32_t						region[2];
	int32_t						bank[4];
	bool						bSingleShift;
	int32_t						region_GL_backup;
	bool						bXCS;
	bool						bNormalSize;
} ConvStatus;

typedef			uint8_t		BankSet;

typedef struct BankElement {
	size_t						cost;
	size_t						tmpcost;
	BankSet						*bseq;
	size_t						blen;
	struct BankElement			*next;
} BankElement;

typedef struct BankGroup {
	int32_t						cType;
	int32_t						cCode;
	int32_t						numCType;
	BankElement					element0[3][7][7][7];
	BankElement					element1[2][7][7][7];
	BankElement					element2[3][7][7][7];
	BankElement					element3[2][7][7][7];
	BankElement					element4[3][7][7][7];
	BankElement					element5[4][7][7][7];
	BankElement					element6[2][7][7][7];
	BankElement					element7[3][7][7][7];
	BankElement					element8[4][7][7][7];
} BankGroup;

typedef struct BankUnit {
	int32_t						cType;
	size_t						cNum;
	BankGroup					group[3];
} BankUnit;

typedef struct BankRouteSelection {
	bool						b0;
	bool						b1;
	bool						b2;
	bool						b3;
	bool						b4;
	bool						b5;
	bool						b6;
	bool						b7;
	bool						b8;
} BankRouteSelection;


// プロトタイプ宣言

size_t		conv_to_unicode(char16_t*, const size_t, const uint8_t*, const size_t, const bool, const bool);
size_t		conv_to_unicode(uint8_t*, const size_t, const uint8_t*, const size_t, const bool, const bool);
size_t		conv_from_unicode(uint8_t*, const size_t, const char16_t*, const size_t, const bool);
size_t		conv_from_unicode(uint8_t*, const size_t, const uint8_t*, const size_t, const bool);

size_t		conv_to_U32T(uint32_t*, const size_t, const uint8_t*, const size_t);
size_t		conv_from_U32T(uint8_t*, const size_t, const uint32_t*, const size_t);

size_t		convU32T_to_UTF16(char16_t*, const size_t, const uint32_t*, const size_t);
size_t		convU32T_to_UTF8(uint8_t*, const size_t, const uint32_t*, const size_t);

int32_t		alphaConv(const int32_t, const bool);
int32_t		jis12Conv(const int32_t, const bool);
int32_t		jis12WinConv(const int32_t, const bool);
int32_t		jis3Conv(const int32_t, const bool);
size_t		jis3CombAndIvsConv(const int32_t, const int32_t, int32_t*, int32_t*);
size_t		jis3CombAndIvsRevConv(const int32_t, const int32_t, int32_t*, int32_t*);
int32_t		jis4Conv(const int32_t, const bool);
int32_t		hiragana1Conv(const int32_t, const bool);
int32_t		katakana1Conv(const int32_t, const bool);
int32_t		kanaCommon1Conv(const int32_t, const bool);
int32_t		hankaku1Conv(const int32_t, const bool);
int32_t		kigou1Conv(const int32_t, const bool);
size_t		kigou2ConvUTF16(const int32_t, char16_t*, const size_t);
size_t		kigou2ConvUTF8(const int32_t, uint8_t*, const size_t);
size_t		kigou2RevConvUTF16(const char16_t*, const size_t, int32_t*);
size_t		kigou2RevConvUTF8(const uint8_t*, const size_t, int32_t*);
int32_t		charSize1Conv(const int32_t, const bool);
int32_t		charSize2Conv(const int32_t, const bool);

void		initRevTable(const int32_t*, int32_t*, const size_t);
int			compareForTable(const void*, const void*);
int			compareForTable64(const void*, const void*);
int			compareForTable64Rev(const void*, const void*);
int			compareForTable2UTF16(const void*, const void*);
int			compareForTable2UTF8(const void*, const void*);
int			compareForTable2StrUTF16(const void*, const void*);
int			compareForTable2StrUTF8(const void*, const void*);
size_t		cmpStrUTF16(const char16_t*, const char16_t*);
size_t		cmpStrUTF8(const uint8_t*, const uint8_t*);

void		initConvStatus(ConvStatus*);
bool		isControlChar(const uint8_t);
bool		isUcControlChar(const int32_t);
bool		isOneByteGSET(const uint8_t);
bool		isTwoByteGSET(const uint8_t);
bool		isOneByteDRCS(const uint8_t);
bool		isTwoByteDRCS(const uint8_t);
int32_t		numGBank(const uint8_t);

size_t		writeUTF16Buf(char16_t*, const size_t, const size_t, const int32_t, const bool);
size_t		writeUTF8Buf(uint8_t*, const size_t, const size_t, const int32_t, const bool);
size_t		writeU32TBuf(uint32_t *, const size_t, const size_t, const uint32_t, const uint32_t, const ConvStatus *);
size_t		writeUTF16StrBuf(char16_t*, const size_t, const size_t, char16_t*, const size_t, const bool);
size_t		writeUTF8StrBuf(uint8_t*, const size_t, const size_t, uint8_t*, const size_t, const bool);
void		writeBuf(uint8_t*, const size_t, const size_t, const uint8_t);

size_t		changeConvStatus(const uint8_t*, size_t, ConvStatus*);
size_t		csiProc(const uint8_t*, size_t, ConvStatus*);
void		defaultMacroProc(const uint8_t, ConvStatus*);

int32_t		classOfCharUTF16(const char16_t*, const size_t, int32_t*, size_t*);
int32_t		classOfCharUTF8(const uint8_t*, const size_t, int32_t*, size_t*);
uint32_t	classOfCharU32T(const uint32_t, int32_t *);

BankSet*	checkMojiSequenceUTF16(const char16_t *, const size_t, const ConvStatus *);
BankSet*	checkMojiSequenceUTF8(const uint8_t *, const size_t, const ConvStatus *);
BankSet*	checkMojiSequenceU32T(const uint32_t *, const size_t, const ConvStatus *);

