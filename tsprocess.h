#pragma once

#include "stdafx.h"
#include <windows.h>
#include <stdint.h>

// #include "tsinout.h"


// 定数など

#define			READBUFSIZE				65536
#define			READBUFMERGIN			1024


// ディスク入出力用

typedef struct {
	HANDLE			hFile;
	int32_t			psize;
	int32_t			poffset;
	uint8_t			databuf[READBUFMERGIN + READBUFSIZE];
	uint8_t			*buf;
	int32_t			datasize;
	int32_t			pos;
	bool			bShowError;
} TsReadProcess;


// プロトタイプ宣言

int32_t			getPid(const uint8_t*);
int32_t			getPidValue(const uint8_t*);
bool			isPsiTop(const uint8_t*);
bool			isScrambled(const uint8_t*);
int32_t			getAdapFieldLength(const uint8_t*);
int32_t			getPointerFieldLength(const uint8_t*);
int32_t			getSectionLength(const uint8_t*);
int32_t			getLength(const uint8_t*);
int32_t			getPsiLength(const uint8_t*);

int32_t			getPsiPacket(TsReadProcess*, uint8_t*, const int32_t);
int32_t			parsePat(const uint8_t*, int32_t*);
void			parsePmt(const uint8_t*, int32_t*, int32_t*, int32_t*, int32_t*, const bool, const bool);
int32_t			compareForPidTable(const void*, const void*);
uint32_t		calc_crc32(const uint8_t*, const int32_t);
int				getPcrPid(const uint8_t*);
bool			isPcrData(const uint8_t*);
int64_t			getPcrValue(const uint8_t*);

int64_t			GetFileDataSize(HANDLE );
void			SeekFileData(HANDLE, const int64_t);
bool			ReadFileData(HANDLE, uint8_t *, const uint32_t, uint32_t *);
bool			WriteFileData(HANDLE, const uint8_t *, const uint32_t, uint32_t *);

void			initTsFileRead(TsReadProcess*, HANDLE, const int32_t);
void			setPointerTsFileRead(TsReadProcess*, const int64_t);
void			setPosTsFileRead(TsReadProcess*, const int32_t);
void			showErrorTsFileRead(TsReadProcess*, const bool);
uint8_t*		getPacketTsFileRead(TsReadProcess*);
