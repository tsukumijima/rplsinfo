// rplsproginfo.cpp
//

#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <locale.h>

#include "rplsinfo.h"
#include "tsprocess.h"
#include "rplsproginfo.h"
#include "tsproginfo.h"
#include "convToUnicode.h"


// 定数など

#define		RPLSFILESIZE			(16 * 1024)


// マクロ定義

#define		printMsg(fmt, ...)		_tprintf(_T(fmt), __VA_ARGS__)
#define		printErrMsg(fmt, ...)	_tprintf(_T(fmt), __VA_ARGS__)


//

bool readFileProgInfo(_TCHAR *fname, ProgInfo* proginfo, const CopyParams* param)
{
	HANDLE	hFile = CreateFile(fname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hFile == INVALID_HANDLE_VALUE) {
		printErrMsg("番組情報元ファイル %s を開くのに失敗しました.\n", fname);
		return	false;
	}

	const int32_t	srcfiletype = rplsTsCheck(hFile);

	if ((srcfiletype != FILE_188TS) && (srcfiletype != FILE_192TS) && (srcfiletype != FILE_RPLS)) {										// 無効なファイルの場合
		printErrMsg("番組情報元ファイル %s は有効なTS, rplsファイルではありません.\n", fname);
		CloseHandle(hFile);
		return	false;
	}

	// 番組情報の読み込み

	_wfullpath(proginfo->fullpath, fname, _MAX_PATH);																	// フルパス名取得
	_wsplitpath_s(proginfo->fullpath, NULL, 0, NULL, 0, proginfo->fname, _MAX_PATH, proginfo->fext, _MAX_PATH);			// ベースファイル名と拡張子
	
	proginfo->fsize = GetFileDataSize(hFile);																			// ファイルサイズ取得

	bool	bResult;

	if (srcfiletype == FILE_RPLS) {
		bResult = readRplsProgInfo(hFile, proginfo, param);
	}
	else {
		bResult = readTsProgInfo(hFile, proginfo, srcfiletype, param);
	}

	if (!bResult) {
		printErrMsg("番組情報元ファイル %s から有効な番組情報を検出できませんでした.\n", fname);
		CloseHandle(hFile);
		return	false;
	}

	CloseHandle(hFile);

	return	true;
}

int32_t rplsTsCheck(HANDLE hReadFile)
{
	uint32_t	numRead;
	uint8_t		buf[1024] = { 0 };

	SeekFileData(hReadFile, 0);
	ReadFileData(hReadFile, buf, 1024, &numRead);

	if ((buf[0] == 'P') && (buf[1] == 'L') && (buf[2] == 'S') && (buf[3] == 'T')) {														// rpls file
		return	FILE_RPLS;
	}
	else if ((buf[188 * 0] == 0x47) && (buf[188 * 1] == 0x47) && (buf[188 * 2] == 0x47) && (buf[188 * 3] == 0x47)) {						// 188 byte packet ts
		return	FILE_188TS;
	}
	else if ((buf[192 * 0 + 4] == 0x47) && (buf[192 * 1 + 4] == 0x47) && (buf[192 * 2 + 4] == 0x47) && (buf[192 * 3 + 4] == 0x47)) {		// 192 byte packet ts
		return	FILE_192TS;
	}

	return	FILE_INVALID;																													// unknown file
}


bool rplsMakerCheck(const uint8_t *buf, const int32_t idMaker)
{
	const uint32_t	mpadr = (buf[ADR_MPDATA] << 24) + (buf[ADR_MPDATA + 1] << 16) + (buf[ADR_MPDATA + 2] << 8) + buf[ADR_MPDATA + 3];
	if (mpadr == 0) return false;

	const uint8_t	*mpdata = buf + mpadr;
	const int32_t	makerid = (mpdata[ADR_MPMAKERID] << 8) + mpdata[ADR_MPMAKERID + 1];

	if (makerid != idMaker) return false;

	return true;
}


bool readRplsProgInfo(HANDLE hFile, ProgInfo *proginfo, const CopyParams *param)
{

	// ファイル読み込み

	uint8_t		buf[RPLSFILESIZE];
	uint32_t	numRead;

	SeekFileData(hFile, 0);
	ReadFileData(hFile, buf, RPLSFILESIZE, &numRead);

	// rpls情報の取得

	const bool	bSonyRpls = rplsMakerCheck(buf, MAKERID_SONY);														// メーカーチェック．sony及びpanasonic
	const bool	bPanaRpls = rplsMakerCheck(buf, MAKERID_PANASONIC);

	const uint8_t	*appinfo = buf + ADR_APPINFO;
	const uint8_t	*mpdata  = buf + (buf[ADR_MPDATA] << 24) + (buf[ADR_MPDATA + 1] << 16) + (buf[ADR_MPDATA + 2] << 8) + buf[ADR_MPDATA + 3];

	// 録画日時

	proginfo->recyear	= (appinfo[ADR_RECYEAR]  >> 4) * 1000 + (appinfo[ADR_RECYEAR]  & 0x0F) * 100 + (appinfo[ADR_RECYEAR + 1] >> 4) * 10 + (appinfo[ADR_RECYEAR + 1] & 0x0F);
	proginfo->recmonth	= (appinfo[ADR_RECMONTH] >> 4) * 10   + (appinfo[ADR_RECMONTH] & 0x0F);
	proginfo->recday	= (appinfo[ADR_RECDAY]   >> 4) * 10   + (appinfo[ADR_RECDAY]   & 0x0F);
	proginfo->rechour	= (appinfo[ADR_RECHOUR]  >> 4) * 10   + (appinfo[ADR_RECHOUR]  & 0x0F);
	proginfo->recmin	= (appinfo[ADR_RECMIN]   >> 4) * 10   + (appinfo[ADR_RECMIN]   & 0x0F);
	proginfo->recsec	= (appinfo[ADR_RECSEC]   >> 4) * 10   + (appinfo[ADR_RECSEC]   & 0x0F);

	// 録画期間

	proginfo->durhour	= (appinfo[ADR_DURHOUR] >> 4) * 10 + (appinfo[ADR_DURHOUR] & 0x0F);
	proginfo->durmin	= (appinfo[ADR_DURMIN]  >> 4) * 10 + (appinfo[ADR_DURMIN]  & 0x0F);
	proginfo->dursec	= (appinfo[ADR_DURSEC]  >> 4) * 10 + (appinfo[ADR_DURSEC]  & 0x0F);

	// タイムゾーン

	proginfo->rectimezone = appinfo[ADR_TIMEZONE];

	// メーカーID, メーカー機種コード

	proginfo->makerid	= appinfo[ADR_MAKERID]   * 256 + appinfo[ADR_MAKERID + 1];
	proginfo->modelcode	= appinfo[ADR_MODELCODE] * 256 + appinfo[ADR_MODELCODE + 1];

	// 放送種別情報（panasonicレコ向け）

	proginfo->recsrc	= bPanaRpls ? mpdata[ADR_RECSRC_PANA] * 256 + mpdata[ADR_RECSRC_PANA + 1] : -1;				// 放送種別情報を取得．パナ以外なら放送種別情報無し(-1)

	// チャンネル番号, チャンネル名（放送局名）

	proginfo->chnum		= appinfo[ADR_CHANNELNUM] * 256 + appinfo[ADR_CHANNELNUM + 1];
	proginfo->chnamelen = (int32_t)conv_to_unicode((char16_t*)proginfo->chname, CONVBUFSIZE, appinfo + ADR_CHANNELNAME + 1, (size_t)appinfo[ADR_CHANNELNAME], param->bCharSize, param->bIVS);

	// 番組名

	proginfo->pnamelen = (int32_t)conv_to_unicode((char16_t*)proginfo->pname, CONVBUFSIZE, appinfo + ADR_PNAME + 1, (size_t)appinfo[ADR_PNAME], param->bCharSize, param->bIVS);

	// 番組内容

	const size_t	pdetaillen = appinfo[ADR_PDETAIL] * 256 + appinfo[ADR_PDETAIL + 1];
	proginfo->pdetaillen = (int32_t)conv_to_unicode((char16_t*)proginfo->pdetail, CONVBUFSIZE, appinfo + ADR_PDETAIL + 2, pdetaillen, param->bCharSize, param->bIVS);

	if (bSonyRpls)		// sonyレコーダーの場合
	{
		// 番組内容詳細

		const int32_t	pextendlen = mpdata[ADR_PEXTENDLEN] * 256 + mpdata[ADR_PEXTENDLEN + 1];
		proginfo->pextendlen = (int32_t)conv_to_unicode((char16_t*)proginfo->pextend, CONVBUFSIZE, appinfo + ADR_PDETAIL + 2 + pdetaillen, pextendlen, param->bCharSize, param->bIVS);

		// 番組ジャンル情報

		for (int32_t i = 0; i < 3; i++) proginfo->genre[i] = (mpdata[ADR_GENRE + i * 4 + 0] == 0x01) ? mpdata[ADR_GENRE + i * 4 + 1] : -1;
	}

	if (bPanaRpls)		// panasonicレコーダーの場合
	{
		proginfo->pextendlen = -1;														// 「番組内容詳細」を有しない

		// 番組ジャンル情報

		for (int32_t i = 0; i < 3; i++) proginfo->genre[i] = -1;

		switch (mpdata[ADR_GENRE_PANA])
		{
		case 0xD5:
			proginfo->genre[2] = ((mpdata[ADR_GENRE_PANA + 1] & 0x0F) << 4) + (mpdata[ADR_GENRE_PANA + 1] >> 4);
		case 0xE5:
			proginfo->genre[1] = ((mpdata[ADR_GENRE_PANA + 2] & 0x0F) << 4) + (mpdata[ADR_GENRE_PANA + 2] >> 4);
		case 0xE9:
			proginfo->genre[0] = ((mpdata[ADR_GENRE_PANA + 3] & 0x0F) << 4) + (mpdata[ADR_GENRE_PANA + 3] >> 4);
			break;
		default:
			break;
		}
	}

	if (!bSonyRpls && !bPanaRpls)		// sony, pana以外
	{
		for (int32_t i = 0; i < 3; i++) proginfo->genre[i] = -1;						// 番組ジャンル情報無し
		proginfo->pextendlen = -1;														// 「番組内容詳細」を有しない
	}

	return true;
}


int compareForRecSrcStr(const void *item1, const void *item2)
{
	return wcscmp(*(WCHAR**)item1, *(WCHAR**)item2);
}


size_t getRecSrcStr(WCHAR *dst, const size_t maxbufsize, const int32_t src)
{
	static const WCHAR	*nameList[] =
	{
		L"TD",		L"地上デジタル",
		L"BD",		L"BSデジタル",
		L"C1",		L"CSデジタル1",
		L"C2",		L"CSデジタル2",
		L"iL",		L"i.LINK(TS)",
		L"MV",		L"AVCHD",
		L"SK",		L"スカパー(DLNA)",
		L"DV",		L"DV入力",
		L"TA",		L"地上アナログ",
		L"NL",		L"ライン入力"
	};

	static bool		bTableInitialized = false;

	if (!bTableInitialized)
	{
		qsort(nameList, sizeof(nameList) / sizeof(WCHAR*) / 2, sizeof(WCHAR*) * 2, compareForRecSrcStr);
		bTableInitialized = true;
	}

	static const WCHAR	*errNameList[] =
	{
		L"unknown",
		L"n/a"
	};

	const WCHAR	*srcStr = errNameList[1];

	if (src != -1)
	{
		WCHAR  key[3];
		key[0] = (WCHAR)((src >> 8) & 0x00FF);
		key[1] = (WCHAR)(src & 0x00FF);
		key[2] = 0;
		WCHAR *pkey = key;

		void *result = bsearch(&pkey, nameList, sizeof(nameList) / sizeof(WCHAR*) / 2, sizeof(WCHAR*) * 2, compareForRecSrcStr);

		if (result != NULL) {
			srcStr = *((WCHAR**)result + 1);
		}
		else {
			srcStr = errNameList[0];
		}
	}

	size_t i = 0;

	while (i < maxbufsize)
	{
		dst[i] = srcStr[i];
		if (srcStr[i++] == 0) break;
	}

	return i - 1;
}