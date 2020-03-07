#pragma once

#include <stdint.h>


// 定数など

#define		ADR_APPINFO				0x28
#define		ADR_TIMEZONE			0x08
#define		ADR_RECYEAR				0x0a
#define		ADR_RECMONTH			0x0c
#define		ADR_RECDAY				0x0d
#define		ADR_RECHOUR				0x0e
#define		ADR_RECMIN				0x0f
#define		ADR_RECSEC				0x10
#define		ADR_DURHOUR				0x11
#define		ADR_DURMIN				0x12
#define		ADR_DURSEC				0x13
#define		ADR_MAKERID				0x14
#define		ADR_MODELCODE			0x16
#define		ADR_CHANNELNUM			0x18
#define		ADR_CHANNELNAME			0x1b
#define		ADR_PNAME				0x30
#define		ADR_PDETAIL				0x130

#define		ADR_MPDATA				0x10
#define		ADR_MPMAKERID			0x0c
#define		ADR_MPMODELCODE			0x0e
#define		ADR_GENRE				0x1c
#define		ADR_PEXTENDLEN			0x38
#define		ADR_PCHECKSUM			0x3a

#define		ADR_RECMODE_PANA		0x98
#define		ADR_RECSRC_PANA			0xA8
#define		ADR_GENRE_PANA			0xB0
#define		ADR_MAXBITRATE_PANA		0xBC

#define		MAKERID_PANASONIC		0x0103
#define		MAKERID_SONY			0x0108

#define		LEN_CHNAME				20
#define		LEN_PNAME				255
#define		LEN_PDETAIL				1200
#define		LEN_PEXTEND				1200

#define		FILE_INVALID			0
#define		FILE_RPLS				1
#define		FILE_188TS				188
#define		FILE_192TS				192



// プロトタイプ宣言

bool		readFileProgInfo(_TCHAR *, ProgInfo*, const CopyParams *);
int32_t		rplsTsCheck(HANDLE);
bool		rplsMakerCheck(const uint8_t *, const int32_t);
bool		readRplsProgInfo(HANDLE, ProgInfo *, const CopyParams *);
size_t		getRecSrcStr(WCHAR *, const size_t, const int32_t);
