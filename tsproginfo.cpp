// proginfo.cpp
//


#include "stdafx.h"
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "rplsinfo.h"
#include "tsprocess.h"
#include "tsproginfo.h"
#include "rplsproginfo.h"
#include "convToUnicode.h"


// 定数など

#define		CONVBUFSIZE				65536

#define		__USE_UTF_CODE_CRLF__


// マクロ定義

#define		printMsg(fmt, ...)		_tprintf(_T(fmt), __VA_ARGS__)
#define		printErrMsg(fmt, ...)	_tprintf(_T(fmt), __VA_ARGS__)


//


bool readTsProgInfo(HANDLE hFile, ProgInfo *proginfo, const int32_t psize, const CopyParams *param)
{
	uint8_t	psibuf[PSIBUFSIZE] = { 0 };

	const int32_t	tstype = getSitEit(hFile, psibuf, psize, param->tsfilepos, param->packet_limit);

	if (tstype == PID_SIT) {																	// SITを有するTSだった場合
		parseSit(psibuf, proginfo, param);
	}
	else if (tstype == PID_EIT) {																// EITを有するTSだった場合
		const int32_t serviceid = parseEit(psibuf, proginfo, param);
		if (getSdt(hFile, psibuf, psize, serviceid, param->tsfilepos, param->packet_limit)) parseSdt(psibuf, proginfo, serviceid, param);
	}
	else {
		return false;																			// 番組情報(SIT, EIT)を検出できなかった場合
	}

	proginfo->rectimezone = 18;																	// TSファイルソースでは内容に関係なくタイムゾーン18
	proginfo->makerid     = 0;																	// メーカーID は 0
	proginfo->modelcode   = 0;																	//  機種コードは 0 
	proginfo->recsrc      = -1;																	//放送種別情報無し

	return true;
}


int32_t getSitEit(HANDLE hFile, uint8_t *psibuf, const int32_t psize, const int32_t startpos, const int64_t max_packet_count)
{
	uint8_t		*buf;
	uint8_t		sitbuf[PSIBUFSIZE] = { 0 };
	uint8_t		eitbuf[PSIBUFSIZE] = { 0 };
	uint8_t		patbuf[PSIBUFSIZE] = { 0 };

	bool		bSitFound = false;
	bool		bEitFound = false;
	bool		bPatFound = false;

	bool		bSitStarted = false;
	bool		bEitStarted = false;
	bool		bPatStarted = false;

	int32_t		sitlen = 0;
	int32_t		eitlen = 0;
	int32_t		patlen = 0;

	int32_t		service_id = 0;

	TsReadProcess	tsin;
	initTsFileRead(&tsin, hFile, psize);
	setPosTsFileRead(&tsin, startpos);

	int64_t		packet_count = 0;

	while (1)
	{
		buf = getPacketTsFileRead(&tsin);
		if (buf == NULL) break;
		buf += (tsin.psize - 188);

		const int32_t	pid = getPid(buf);

		if ((pid == PID_SIT) || (pid == PID_EIT) || (pid == PID_PAT))
		{
			const bool		bPsiTop = isPsiTop(buf);
			const int32_t	adaplen = getAdapFieldLength(buf);
			const int32_t	pflen = getPointerFieldLength(buf);				// !bPsiTopな場合は無意味な値

			int32_t	len;
			bool	bTop = false;
			bool	bFirstSection = true;
			int32_t	i = adaplen + 4;

			while (i < 188)														// 188バイトパケット内での各セクションのループ
			{
				if (!bPsiTop) {
					len = 188 - i;
					bTop = false;
				}
				else if (bFirstSection && (pflen != 0)) {
					i++;
					len = pflen;
					bTop = false;
					bFirstSection = false;
				}
				else {
					if (bFirstSection && (pflen == 0)) {
						i++;
						bFirstSection = false;
					}
					if (buf[i] == 0xFF) break;									// TableIDのあるべき場所が0xFF(stuffing byte)ならそのパケットに関する処理は終了
					len = getSectionLength(buf + i) + 3;						// セクションヘッダはパケットにまたがって配置されることは無いはずなのでパケット範囲外(188バイト以降)を読みに行くことは無いはず．
					if (i + len > 188) len = 188 - i;
					bTop = true;
				}

				// SITに関する処理

				if (pid == PID_SIT) {
					if (bSitStarted && !bTop) {
						memcpy(sitbuf + sitlen, buf + i, len);
						sitlen += len;
					}
					if (bTop) {
						memset(sitbuf, 0, sizeof(sitbuf));
						memcpy(sitbuf, buf + i, len);
						sitlen = len;
						bSitStarted = true;
					}
					if (bSitStarted && (sitlen >= getSectionLength(sitbuf) + 3)) {
						const int32_t	seclen = getSectionLength(sitbuf);
						const uint32_t	crc = calc_crc32(sitbuf, seclen + 3);
						if ((crc == 0) && (sitbuf[0] == 0x7F)) {
							memcpy(psibuf, sitbuf, seclen + 3);
							bSitFound = true;
							break;
						}
						bSitStarted = false;
					}
				}

				// EITに関する処理

				if (pid == PID_EIT) {
					if (bEitStarted && !bTop) {
						memcpy(eitbuf + eitlen, buf + i, len);
						eitlen += len;
					}
					if (bTop) {
						memset(eitbuf, 0, sizeof(eitbuf));
						memcpy(eitbuf, buf + i, len);
						eitlen = len;
						bEitStarted = true;
					}
					if (bEitStarted && (eitlen >= getSectionLength(eitbuf) + 3)) {
						const int32_t	seclen = getSectionLength(eitbuf);
						const uint32_t	crc = calc_crc32(eitbuf, seclen + 3);
						const int32_t	sid = eitbuf[0x03] * 256 + eitbuf[0x04];
						if ((crc == 0) && (eitbuf[0] == 0x4E) && (eitbuf[0x06] == 0) && bPatFound && (sid == service_id) && (seclen > 15)) {
							memcpy(psibuf, eitbuf, seclen + 3);
							bEitFound = true;
							break;
						}
						bEitStarted = false;
					}
				}

				// PATに関する処理

				if ((pid == PID_PAT) && !bPatFound) {
					if (bPatStarted && !bTop) {
						memcpy(patbuf + patlen, buf + i, len);
						patlen += len;
					}
					if (bTop) {
						memset(patbuf, 0, sizeof(patbuf));
						memcpy(patbuf, buf + i, len);
						patlen = len;
						bPatStarted = true;
					}
					if (bPatStarted && (patlen >= getSectionLength(patbuf) + 3)) {
						const int32_t	seclen = getSectionLength(patbuf);
						const uint32_t	crc = calc_crc32(patbuf, seclen + 3);
						if ((crc == 0) && (patbuf[0] == 0x00)) {
							int32_t		j = 0x08;
							while (j < seclen - 1) {
								service_id = patbuf[j] * 256 + patbuf[j + 1];
								if (service_id != 0) {
									bPatFound = true;
									break;
								}
								j += 4;
							}
						}
						bPatStarted = false;
					}
				}

				if (bSitFound || bEitFound) break;

				i += len;
			}
		}

		if (bSitFound || bEitFound) break;

		if (max_packet_count != 0) {
			packet_count++;
			if (packet_count >= max_packet_count) break;
		}

	}

	if (bSitFound) return PID_SIT;
	if (bEitFound) return PID_EIT;

	return PID_INVALID;
}


bool getSdt(HANDLE hFile, uint8_t *psibuf, const int32_t psize, const int32_t serviceid, const int32_t startpos, const int64_t max_packet_count)
{
	uint8_t		*buf;
	uint8_t		sdtbuf[PSIBUFSIZE] = { 0 };

	bool		bSdtFound = false;
	bool		bSdtStarted = false;
	int32_t		sdtlen = 0;

	TsReadProcess	tsin;
	initTsFileRead(&tsin, hFile, psize);
	setPosTsFileRead(&tsin, startpos);

	int64_t		packet_count = 0;

	while (1)
	{
		buf = getPacketTsFileRead(&tsin);
		if (buf == NULL) break;
		buf += (psize - 188);

		const int32_t	pid = getPid(buf);

		if (pid == PID_SDT)
		{
			const bool		bPsiTop = isPsiTop(buf);
			const int32_t	adaplen = getAdapFieldLength(buf);
			const int32_t	pflen = getPointerFieldLength(buf);				// !bPsiTopな場合は無意味な値

			int32_t		len;
			bool		bTop = false;
			bool		bFirstSection = true;
			int32_t		i = adaplen + 4;

			while (i < 188)														// 188バイトパケット内での各セクションのループ
			{
				if (!bPsiTop) {
					len = 188 - i;
					bTop = false;
				}
				else if (bFirstSection && (pflen != 0)) {
					i++;
					len = pflen;
					bTop = false;
					bFirstSection = false;
				}
				else {
					if (bFirstSection && (pflen == 0)) {
						i++;
						bFirstSection = false;
					}
					if (buf[i] == 0xFF) break;									// TableIDのあるべき場所が0xFF(stuffing byte)ならそのパケットに関する処理は終了
					len = getSectionLength(buf + i) + 3;						// セクションヘッダはパケットにまたがって配置されることは無いはずなのでパケット範囲外(188バイト以降)を読みに行くことは無いはず
					if (i + len > 188) len = 188 - i;
					bTop = true;
				}

				// SDTに関する処理

				if ((pid == PID_SDT) && !bSdtFound) {
					if (bSdtStarted && !bTop) {
						memcpy(sdtbuf + sdtlen, buf + i, len);
						sdtlen += len;
					}
					if (bTop) {
						memset(sdtbuf, 0, sizeof(sdtbuf));
						memcpy(sdtbuf, buf + i, len);
						sdtlen = len;
						bSdtStarted = true;
					}
					if (bSdtStarted && (sdtlen >= getSectionLength(sdtbuf) + 3)) {
						const int32_t	seclen = getSectionLength(sdtbuf);
						const uint32_t	crc = calc_crc32(sdtbuf, seclen + 3);
						if ((crc == 0) && (sdtbuf[0] == 0x42)) {
							int32_t		j = 0x0B;
							while (j < seclen) {
								const int32_t	serviceID = sdtbuf[j] * 256 + sdtbuf[j + 1];
								const int32_t	slen = getLength(sdtbuf + j + 0x03);
								if (serviceID == serviceid) {
									memcpy(psibuf, sdtbuf, seclen + 3);
									bSdtFound = true;
									break;
								}
								j += (slen + 5);
							}
						}
						bSdtStarted = false;
					}
				}

				i += len;
			}
		}

		if (bSdtFound) break;

		if (max_packet_count != 0) {
			packet_count++;
			if (packet_count >= max_packet_count) break;
		}

	}

	return bSdtFound;
}


void mjd_dec(const int32_t mjd, int32_t *recyear, int32_t *recmonth, int32_t *recday)
{
	const int32_t	mjd2 = (mjd < 15079) ? mjd + 0x10000 : mjd;

	int32_t			year = (int32_t)(((double)mjd2 - 15078.2) / 365.25);
	const int32_t	year2 = (int32_t)((double)year  * 365.25);
	int32_t			month = (int32_t)(((double)mjd2 - 14956.1 - (double)year2) / 30.6001);
	const int32_t	month2 = (int32_t)((double)month * 30.6001);
	const int32_t	day = mjd2 - 14956 - year2 - month2;

	if ((month == 14) || (month == 15)) {
		year += 1901;
		month = month - 13;
	}
	else {
		year += 1900;
		month = month - 1;
	}

	*recyear = year;
	*recmonth = month;
	*recday = day;

	return;
}


int32_t mjd_enc(const int32_t year, const int32_t month, const int32_t day)
{
	const int32_t	year2 = (month > 2) ? year - 1900 : year - 1901;
	const int32_t	month2 = (month > 2) ? month + 1 : month + 13;

	const int32_t	a = (int32_t)(365.25 * (double)year2);
	const int32_t	b = (int32_t)(30.6001 * (double)month2);
	const int32_t	mjd = 14956 + day + a + b;

	return mjd & 0xFFFF;
}


int comparefornidtable(const void *item1, const void *item2)
{
	return *(int32_t*)item1 - *(int32_t*)item2;
}


int32_t getTbChannelNum(const int32_t networkID, const int32_t serviceID, int32_t remoconkey_id)
{
	static int32_t	nIDTable[] =																			// ネットワーク識別, リモコンキー識別の関係は、ARIB TR-B14 を参照した．
	{
		0x7FE0, 1,	0x7FE1, 2,	0x7FE2, 4,	0x7FE3, 6,	0x7FE4, 8,	0x7FE5, 5,	0x7FE6, 7,	0x7FE8, 12,		// 関東広域
		0x7FD1, 2,	0x7FD2, 4,	0x7FD3, 6,	0x7FD4, 8,	0x7FD5, 10,											// 近畿広域
		0x7FC1, 2,	0x7FC2, 1,	0x7FC3, 5,	0x7FC4, 6,	0x7FC5, 4,											// 中京広域			18局

		0x7FB2, 1,	0x7FB3, 5,	0x7FB4, 6,	0x7FB5, 8,	0x7FB6, 7,											// 北海道域
		0x7FA2, 4,	0x7FA3, 5,	0x7FA4, 6,	0x7FA5, 7,	0x7FA6, 8,											// 岡山香川
		0x7F92, 8,	0x7F93, 6,	0x7F94, 1,																	// 島根鳥取
		0x7F50, 3,	0x7F51, 2,	0x7F52, 1,	0x7F53, 5,	0x7F54, 6,	0x7F55, 8,	0x7F56, 7,					// 北海道（札幌）	20局

		0x7F40, 3,	0x7F41, 2,	0x7F42, 1,	0x7F43, 5,	0x7F44, 6,	0x7F45, 8,	0x7F46, 7,					// 北海道（函館）
		0x7F30, 3,	0x7F31, 2,	0x7F32, 1,	0x7F33, 5,	0x7F34, 6,	0x7F35, 8,	0x7F36, 7,					// 北海道（旭川）
		0x7F20, 3,	0x7F21, 2,	0x7F22, 1,	0x7F23, 5,	0x7F24, 6,	0x7F25, 8,	0x7F26, 7,					// 北海道（帯広）	21局

		0x7F10, 3,	0x7F11, 2,	0x7F12, 1,	0x7F13, 5,	0x7F14, 6,	0x7F15, 8,	0x7F16, 7,					// 北海道（釧路）
		0x7F00, 3,	0x7F01, 2,	0x7F02, 1,	0x7F03, 5,	0x7F04, 6,	0x7F05, 8,	0x7F06, 7,					// 北海道（北見）
		0x7EF0, 3,	0x7EF1, 2,	0x7EF2, 1,	0x7EF3, 5,	0x7EF4, 6,	0x7EF5, 8,	0x7EF6, 7,					// 北海道（室蘭）
		0x7EE0, 3,	0x7EE1, 2,	0x7EE2, 1,	0x7EE3, 8,	0x7EE4, 4,	0x7EE5, 5,								// 宮城				27局

		0x7ED0, 1,	0x7ED1, 2,	0x7ED2, 4,	0x7ED3, 8,	0x7ED4, 5,											// 秋田
		0x7EC0, 1,	0x7EC1, 2,	0x7EC2, 4,	0x7EC3, 5,	0x7EC4, 6,	0x7EC5, 8,								// 山形
		0x7EB0, 1,	0x7EB1, 2,	0x7EB2, 6,	0x7EB3, 4,	0x7EB4, 8,	0x7EB5, 5,								// 岩手
		0x7EA0, 1,	0x7EA1, 2,	0x7EA2, 8,	0x7EA3, 4,	0x7EA4, 5,	0x7EA5, 6,								// 福島				23局

		0x7E90, 3,	0x7E91, 2,	0x7E92, 1,	0x7E93, 6,	0x7E94, 5,											// 青森
		0x7E87, 9,																							// 東京
		0x7E77, 3,																							// 神奈川
		0x7E60, 1, 0x7E67, 3,																				// 群馬
		0x7E50, 1,																							// 茨城
		0x7E47, 3,																							// 千葉
		0x7E30, 1, 0x7E37, 3,																				// 栃木
		0x7E27, 3,																							// 埼玉
		0x7E10, 1,	0x7E11, 2,	0x7E12, 4,	0x7E13, 5,	0x7E14, 6,	0x7E15, 8,								// 長野
		0x7E00, 1,	0x7E01, 2,	0x7E02, 6,	0x7E03, 8,	0x7E04, 4,	0x7E05, 5,								// 新潟
		0x7DF0, 1,	0x7DF1, 2,	0x7DF2, 4,	0x7DF3, 6,														// 山梨				28 + 2局

		0x7DE0, 3,	0x7DE6, 10,																				// 愛知
		0x7DD0, 1,	0x7DD1, 2,	0x7DD2, 4,	0x7DD3, 5,	0x7DD4, 6,	0x7DD5, 8,								// 石川
		0x7DC0, 1,	0x7DC1, 2,	0x7DC2, 6,	0x7DC3, 8,	0x7DC4, 4,	0x7DC5, 5,								// 静岡
		0x7DB0, 1,	0x7DB1, 2,	0x7DB2, 7,	0x7DB3, 8,														// 福井
		0x7DA0, 3,	0x7DA1, 2,	0x7DA2, 1,	0x7DA3, 8,	0x7DA4, 6,											// 富山
		0x7D90, 3,	0x7D96, 7,																				// 三重
		0x7D80, 3,	0x7D86, 8,																				// 岐阜				27局

		0x7D70, 1,	0x7D76, 7,																				// 大阪
		0x7D60, 1,	0x7D66, 5,																				// 京都
		0x7D50, 1,	0x7D56, 3,																				// 兵庫
		0x7D40, 1,	0x7D46, 5,																				// 和歌山
		0x7D30, 1,	0x7D36, 9,																				// 奈良
		0x7D20, 1,	0x7D26, 3,																				// 滋賀
		0x7D10, 1,	0x7D11, 2,	0x7D12, 3,	0x7D13, 4,	0x7D14, 5,	0x7D15, 8,								// 広島
		0x7D00, 1,	0x7D01, 2,																				// 岡山
		0x7CF0, 3,	0x7CF1, 2,																				// 島根
		0x7CE0, 3,	0x7CE1, 2,																				// 鳥取				24局

		0x7CD0, 1,	0x7CD1, 2,	0x7CD2, 4,	0x7CD3, 3,	0x7CD4, 5,											// 山口
		0x7CC0, 1,	0x7CC1, 2,	0x7CC2, 4,	0x7CC3, 5,	0x7CC4, 6,	0x7CC5, 8,								// 愛媛
		0x7CB0, 1,	0x7CB1, 2,																				// 香川
		0x7CA0, 3,	0x7CA1, 2,	0x7CA2, 1,																	// 徳島
		0x7C90, 1,	0x7C91, 2,	0x7C92, 4,	0x7C93, 6,	0x7C94, 8,											// 高知				21局

		0x7C80, 3,	0x7C81, 2,	0x7C82, 1,	0x7C83, 4,	0x7C84, 5,	0x7C85, 7,	0x7C86, 8,					// 福岡
		0x7C70, 1,	0x7C71, 2,	0x7C72, 3,	0x7C73, 8,	0x7C74, 4,	0x7C75, 5,								// 熊本
		0x7C60, 1,	0x7C61, 2,	0x7C62, 3,	0x7C63, 8,	0x7C64, 5,	0x7C65, 4,								// 長崎
		0x7C50, 3,	0x7C51, 2,	0x7C52, 1,	0x7C53, 8,	0x7C54, 5,	0x7C55, 4,								// 鹿児島
		0x7C40, 1,	0x7C41, 2,	0x7C42, 6,	0x7C43, 3,														// 宮崎				29局

		0x7C30, 1,	0x7C31, 2,	0x7C32, 3,	0x7C33, 4,	0x7C34, 5,											// 大分
		0x7C20, 1,	0x7C21, 2,	0x7C22, 3,																	// 佐賀
		0x7C10, 1,	0x7C11, 2,	0x7C12, 3,	0x7C14, 5,	0x7C17, 8,											// 沖縄				13局

		0x7880, 3,	0x7881, 2,																				// 福岡（県域フラグ）	2局			計255局
	};

	static bool		bTableInit = false;

	if (!bTableInit) {
		qsort(nIDTable, sizeof(nIDTable) / sizeof(int32_t) / 2, sizeof(int32_t) * 2, comparefornidtable);
		bTableInit = true;
	}

	if (remoconkey_id == 0) {
		void *result = bsearch(&networkID, nIDTable, sizeof(nIDTable) / sizeof(int32_t) / 2, sizeof(int32_t) * 2, comparefornidtable);
		if (result == NULL) return 0;
		remoconkey_id = *((int*)result + 1);
	}

	return remoconkey_id * 10 + (serviceID & 0x0007) + 1;
}


size_t my_memcpy_s(void* dst, const size_t bufsize, const void* src, const size_t srclen)
{
	memcpy(dst, src, (srclen < bufsize) ? srclen : bufsize);

	return (srclen < bufsize) ? srclen : bufsize;
}


void parseSit(const uint8_t *sitbuf, ProgInfo *proginfo, const CopyParams* param)
{

	for (int32_t i = 0; i < 3; i++) proginfo->genre[i] = -1;			// 番組ジャンル情報をクリアしておく

	const int32_t	totallen = getSectionLength(sitbuf);
	const int32_t	firstlen = getLength(sitbuf + 0x08);

	// firstloop処理

	int32_t			mediaType = MEDIATYPE_UNKNOWN;
	int32_t			networkID = 0;
	int32_t			serviceID = 0;
	int32_t			remoconkey_id = 0;

	int32_t			i = 0x0A;

	while (i < 0x0A + firstlen)
	{
		switch (sitbuf[i])
		{
		case 0xC2:													// ネットワーク識別記述子
			mediaType = sitbuf[i + 5] * 256 + sitbuf[i + 6];
			networkID = sitbuf[i + 7] * 256 + sitbuf[i + 8];
			i += (sitbuf[i + 1] + 2);
			break;
		case 0xCD:													// TS情報記述子
			remoconkey_id = sitbuf[i + 2];
			i += (sitbuf[i + 1] + 2);
			break;
		default:
			i += (sitbuf[i + 1] + 2);
		}
	}


	// secondloop処理

	if ((mediaType == MEDIATYPE_CS) && ((networkID == 0x0001) || (networkID == 0x0003)))		// スカパー PerfecTV & SKY サービス (現在は使われていない、と思う)
	{
		while (i < totallen - 1)
		{
			serviceID = sitbuf[i] * 256 + sitbuf[i + 1];
			const int32_t secondlen = getLength(sitbuf + i + 2);

			i += 4;

			proginfo->chnum = serviceID & 0x03FF;												// serviceIDからチャンネル番号を取得

			const int32_t	j = i;

			uint8_t		tempbuf[CONVBUFSIZE];
			int32_t		pextendlen = 0;

			while (i < secondlen + j)
			{
				switch (sitbuf[i])
				{
					case 0xC3: {															// パーシャルトランスポートストリームタイム記述子
						const int32_t mjd = sitbuf[i + 3] * 256 + sitbuf[i + 4];
						mjd_dec(mjd, &proginfo->recyear, &proginfo->recmonth, &proginfo->recday);
						proginfo->rechour = (sitbuf[i + 5] >> 4) * 10 + (sitbuf[i + 5] & 0x0f);
						proginfo->recmin = (sitbuf[i + 6] >> 4) * 10 + (sitbuf[i + 6] & 0x0f);
						proginfo->recsec = (sitbuf[i + 7] >> 4) * 10 + (sitbuf[i + 7] & 0x0f);
						if ((sitbuf[i + 8] != 0xFF) && (sitbuf[i + 9] != 0xFF) && (sitbuf[i + 10] != 0xFF)) {
							proginfo->durhour = (sitbuf[i + 8] >> 4) * 10 + (sitbuf[i + 8] & 0x0f);
							proginfo->durmin = (sitbuf[i + 9] >> 4) * 10 + (sitbuf[i + 9] & 0x0f);
							proginfo->dursec = (sitbuf[i + 10] >> 4) * 10 + (sitbuf[i + 10] & 0x0f);
						}
						i += (sitbuf[i + 1] + 2);
						break;
					}
					case 0xB2:																// 局名と番組名に関する記述子
						if (sitbuf[i + 3] == 0x01) {
							const size_t	chnamelen = sitbuf[i + 2];
							const size_t	pnamelen  = sitbuf[i + 3 + chnamelen];
							proginfo->chnamelen = (int32_t)conv_to_unicode((char16_t*)proginfo->chname, CONVBUFSIZE, sitbuf + i + 4, chnamelen - 1, param->bCharSize, param->bIVS);
							proginfo->pnamelen  = (int32_t)conv_to_unicode((char16_t*)proginfo->pname, CONVBUFSIZE, sitbuf + i + 5 + chnamelen, pnamelen - 1, param->bCharSize, param->bIVS);
						}
						i += (sitbuf[i + 1] + 2);
						break;
					case 0x83: {															// 番組詳細情報に関する記述子
						int32_t len = sitbuf[i + 1] - 1;
						if (len + pextendlen > LEN_PEXTEND) {
							len = (LEN_PEXTEND - pextendlen > 0) ? LEN_PEXTEND - pextendlen : 0;
						}
						memcpy(tempbuf + pextendlen, sitbuf + i + 3, len);
						pextendlen += len;
						i += (sitbuf[i + 1] + 2);
						break;
					}
					default:
						i += (sitbuf[i + 1] + 2);
				}
			}

			proginfo->pextendlen = (int32_t)conv_to_unicode((char16_t*)proginfo->pextend, CONVBUFSIZE, tempbuf, pextendlen, param->bCharSize, param->bIVS);
		}
	}
	else // if( (mediaType == MEDIATYPE_BS) || (mediaType == MEDIATYPE_TB) || ( (mediaType == MEDIATYPE_CS) && (networkID == 0x000A) ) )		// 地上デジタル, BSデジタル, (恐らくスカパーHDも)
	{
		uint8_t		tempbuf[CONVBUFSIZE] = { 0 };
		int32_t		templen = 0;

		while (i < totallen - 1)
		{
			serviceID = sitbuf[i] * 256 + sitbuf[i + 1];				// サービスID

			const int32_t	secondlen = getLength(sitbuf + i + 2);

			if (mediaType == MEDIATYPE_TB) {
				proginfo->chnum = getTbChannelNum(networkID, serviceID, remoconkey_id);
			}
			else {
				proginfo->chnum = serviceID & 0x03FF;
			}

			proginfo->pextendlen = 0;
			templen = 0;

			i += 4;
			const int32_t	j = i;

			while (i < secondlen + j)
			{
				switch (sitbuf[i])
				{
					case 0xC3: {												// パーシャルトランスポートストリームタイム記述子
						const int32_t mjd = sitbuf[i + 3] * 256 + sitbuf[i + 4];
						mjd_dec(mjd, &proginfo->recyear, &proginfo->recmonth, &proginfo->recday);
						proginfo->rechour = (sitbuf[i + 5] >> 4) * 10 + (sitbuf[i + 5] & 0x0f);
						proginfo->recmin = (sitbuf[i + 6] >> 4) * 10 + (sitbuf[i + 6] & 0x0f);
						proginfo->recsec = (sitbuf[i + 7] >> 4) * 10 + (sitbuf[i + 7] & 0x0f);
						if ((sitbuf[i + 8] != 0xFF) && (sitbuf[i + 9] != 0xFF) && (sitbuf[i + 10] != 0xFF)) {
							proginfo->durhour = (sitbuf[i + 8] >> 4) * 10 + (sitbuf[i + 8] & 0x0f);
							proginfo->durmin = (sitbuf[i + 9] >> 4) * 10 + (sitbuf[i + 9] & 0x0f);
							proginfo->dursec = (sitbuf[i + 10] >> 4) * 10 + (sitbuf[i + 10] & 0x0f);
						}
						i += (sitbuf[i + 1] + 2);
						break;
					}
					case 0x54: {													// コンテント記述子
						int32_t numGenre = 0;
						for (uint32_t n = 0; n < sitbuf[i + 1]; n += 2) {
							if (((sitbuf[i + 2 + n] & 0xF0) != 0xE0) && (numGenre < 3)) {
								proginfo->genre[numGenre] = sitbuf[i + 2 + n];
								numGenre++;
							}
						}
						i += (sitbuf[i + 1] + 2);
						break;
					}
					case 0x48: {													// サービス記述子
						const int32_t provnamelen = sitbuf[i + 3];
						const int32_t servnamelen = sitbuf[i + 4 + provnamelen];
						proginfo->chnamelen = (int32_t)conv_to_unicode((char16_t*)proginfo->chname, CONVBUFSIZE, sitbuf + i + 5 + provnamelen, servnamelen, param->bCharSize, param->bIVS);
						i += (sitbuf[i + 1] + 2);
						break;
					}
					case 0x4D: {													// 短形式イベント記述子
						const int32_t prognamelen   = sitbuf[i + 5];
						const int32_t progdetaillen = sitbuf[i + 6 + prognamelen];
						proginfo->pnamelen   = (int32_t)conv_to_unicode((char16_t*)proginfo->pname, CONVBUFSIZE, sitbuf + i + 6, prognamelen, param->bCharSize, param->bIVS);
						proginfo->pdetaillen = (int32_t)conv_to_unicode((char16_t*)proginfo->pdetail, CONVBUFSIZE, sitbuf + i + 7 + prognamelen, progdetaillen, param->bCharSize, param->bIVS);
						i += (sitbuf[i + 1] + 2);
						break;
					}
					case 0x4E: {												// 拡張形式イベント記述子
						const int32_t	itemlooplen = sitbuf[i + 6];
						i += 7;
						const int32_t	k = i;
						while (i < itemlooplen + k)
						{
							const int32_t	idesclen = sitbuf[i];
							const int32_t	itemlen = sitbuf[i + 1 + idesclen];

							if (idesclen != 0) {
								if (proginfo->pextendlen != 0) {
									if (templen != 0) {
										if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextendlen += (int32_t)conv_to_unicode((char16_t*)proginfo->pextend + proginfo->pextendlen, CONVBUFSIZE - proginfo->pextendlen, tempbuf, templen, param->bCharSize, param->bIVS);
										templen = 0;
									}
									if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000D;
									if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000A;
									if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000D;
									if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000A;
								}
								if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextendlen += (int32_t)conv_to_unicode((char16_t*)proginfo->pextend + proginfo->pextendlen, CONVBUFSIZE - proginfo->pextendlen, sitbuf + i + 1, idesclen, param->bCharSize, param->bIVS);
								if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000D;
								if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000A;
							}
							templen += (int32_t)my_memcpy_s(tempbuf + templen, CONVBUFSIZE - templen, sitbuf + i + 2 + idesclen, itemlen);
							i += (idesclen + itemlen + 2);
						}
						const int32_t	iextlen = sitbuf[i];
						i += (iextlen + 1);
						break;
					}
					default:
						i += (sitbuf[i + 1] + 2);
				}
			}

			if (templen != 0) {
				if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextendlen += (int32_t)conv_to_unicode((char16_t*)proginfo->pextend + proginfo->pextendlen, CONVBUFSIZE - proginfo->pextendlen, tempbuf, templen, param->bCharSize, param->bIVS);
			}
		}
	}

	return;
}


int32_t parseEit(const uint8_t *eitbuf, ProgInfo *proginfo, const CopyParams* param)
{
	for (int32_t i = 0; i < 3; i++) proginfo->genre[i] = -1;			// 番組ジャンル情報をクリアしておく

	const int32_t 	totallen	= getSectionLength(eitbuf);
	const int32_t	serviceID	= eitbuf[0x03] * 256 + eitbuf[0x04];
	const int32_t	networkID	= eitbuf[0x0A] * 256 + eitbuf[0x0B];

	proginfo->chnum = ((networkID >= 0x7880) && (networkID <= 0x7FE8)) ? getTbChannelNum(networkID, serviceID, 0) : serviceID & 0x03FF;	// チャンネル番号の取得


	// totalloop処理

	int32_t		i = 0x0E;

	uint8_t		tempbuf[CONVBUFSIZE];
	int32_t		templen = 0;

	while (i < totallen - 1)
	{
		const int32_t	mjd = eitbuf[i + 2] * 256 + eitbuf[i + 3];
		mjd_dec(mjd, &proginfo->recyear, &proginfo->recmonth, &proginfo->recday);
		proginfo->rechour	= (eitbuf[i + 4] >> 4) * 10 + (eitbuf[i + 4] & 0x0f);
		proginfo->recmin	= (eitbuf[i + 5] >> 4) * 10 + (eitbuf[i + 5] & 0x0f);
		proginfo->recsec	= (eitbuf[i + 6] >> 4) * 10 + (eitbuf[i + 6] & 0x0f);
		if( (eitbuf[i + 7] != 0xFF) && (eitbuf[i + 8] != 0xFF) && (eitbuf[i + 9] != 0xFF) ) {
			proginfo->durhour	= (eitbuf[i + 7] >> 4) * 10 + (eitbuf[i + 7] & 0x0f);
			proginfo->durmin	= (eitbuf[i + 8] >> 4) * 10 + (eitbuf[i + 8] & 0x0f);
			proginfo->dursec	= (eitbuf[i + 9] >> 4) * 10 + (eitbuf[i + 9] & 0x0f);
		}

		const int32_t	seclen = getLength(eitbuf + i + 0x0A);

		i += 0x0C;

		// secondloop処理

		proginfo->pextendlen = 0;
		templen = 0;

		const int32_t	j = i;

		while (i < seclen + j)
		{
			switch (eitbuf[i])
			{
				case 0x54: {												// コンテント記述子
					int32_t	numGenre = 0;
					for (uint32_t n = 0; n < eitbuf[i + 1]; n += 2) {
						if ((eitbuf[i + 2 + n] & 0xF0) != 0xE0) {
							proginfo->genre[numGenre] = eitbuf[i + 2 + n];
							numGenre++;
						}
					}
					i += (eitbuf[i + 1] + 2);
					break;
				}
				case 0x4D: {												// 短形式イベント記述子
					const int32_t	prognamelen   = eitbuf[i + 5];
					const int32_t	progdetaillen = eitbuf[i + 6 + prognamelen];
					proginfo->pnamelen   = (int32_t)conv_to_unicode((char16_t*)proginfo->pname, CONVBUFSIZE, eitbuf + i + 6, prognamelen, param->bCharSize, param->bIVS);
					proginfo->pdetaillen = (int32_t)conv_to_unicode((char16_t*)proginfo->pdetail, CONVBUFSIZE, eitbuf + i + 7 + prognamelen, progdetaillen, param->bCharSize, param->bIVS);
					i += (eitbuf[i + 1] + 2);
					break;
				}
				case 0x4E: {												// 拡張形式イベント記述子
					const int32_t	itemlooplen = eitbuf[i + 6];
					i += 7;
					const int32_t	k = i;
					while (i < itemlooplen + k)
					{
						const int32_t	idesclen = eitbuf[i];
						const int32_t	itemlen = eitbuf[i + 1 + idesclen];

						if (idesclen != 0) {
							if (proginfo->pextendlen != 0) {
								if (templen != 0) {
									if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextendlen += (int32_t)conv_to_unicode((char16_t*)proginfo->pextend + proginfo->pextendlen, CONVBUFSIZE - proginfo->pextendlen, tempbuf, templen, param->bCharSize, param->bIVS);
									templen = 0;
								}
								if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000D;
								if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000A;
								if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000D;
								if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000A;
							}
							if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextendlen += (int32_t)conv_to_unicode((char16_t*)proginfo->pextend + proginfo->pextendlen, CONVBUFSIZE - proginfo->pextendlen, eitbuf + i + 1, idesclen, param->bCharSize, param->bIVS);
							if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000D;
							if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextend[proginfo->pextendlen++] = 0x000A;
						}
						templen += (int32_t)my_memcpy_s(tempbuf + templen, CONVBUFSIZE - templen, eitbuf + i + 2 + idesclen, itemlen);
						i += (idesclen + itemlen + 2);
					}
					const int32_t	iextlen = eitbuf[i];
					i += (iextlen + 1);
					break;
				}
				default:
					i += (eitbuf[i + 1] + 2);
			}
		}

		if (templen != 0) {
			if (proginfo->pextendlen < CONVBUFSIZE) proginfo->pextendlen += (int32_t)conv_to_unicode((char16_t*)proginfo->pextend + proginfo->pextendlen, CONVBUFSIZE - proginfo->pextendlen, tempbuf, templen, param->bCharSize, param->bIVS);
		}
	}

	return serviceID;
}


void parseSdt(const uint8_t *sdtbuf, ProgInfo *proginfo, const int32_t serviceid, const CopyParams* param)
{
	const int32_t	totallen = getSectionLength(sdtbuf);

	int32_t		i = 0x0B;

	while (i < totallen - 1)
	{
		const int32_t	serviceID = sdtbuf[i] * 256 + sdtbuf[i + 1];
		const int32_t	secondlen = getLength(sdtbuf + i + 3);

		if (serviceID == serviceid) {
			i += 5;
			const int32_t	j = i;
			while (i < secondlen + j)
			{
				switch (sdtbuf[i])
				{
					case 0x48: {													// サービス記述子
						const int32_t	provnamelen = sdtbuf[i + 3];
						const int32_t	servnamelen = sdtbuf[i + 4 + provnamelen];
						proginfo->chnamelen = (int32_t)conv_to_unicode((char16_t*)proginfo->chname, CONVBUFSIZE, sdtbuf + i + 5 + provnamelen, servnamelen, param->bCharSize, param->bIVS);
						i += (sdtbuf[i + 1] + 2);
						break;
					}
					default:
						i += (sdtbuf[i + 1] + 2);
				}
			}
			break;
		}
		else {
			i += (secondlen + 5);
		}
	}

	return;
}


size_t putGenreStr(WCHAR *buf, const size_t bufsize, const int32_t* genre)
{
	const WCHAR	*str_genreL[] = {
		L"ニュース／報道",			L"スポーツ",	L"情報／ワイドショー",	L"ドラマ",
		L"音楽",					L"バラエティ",	L"映画",				L"アニメ／特撮",
		L"ドキュメンタリー／教養",	L"劇場／公演",	L"趣味／教育",			L"福祉",
		L"予備",					L"予備",		L"拡張",				L"その他"
	};

	const WCHAR	*str_genreM[] = {
		L"定時・総合", L"天気", L"特集・ドキュメント", L"政治・国会", L"経済・市況", L"海外・国際", L"解説", L"討論・会談",
		L"報道特番", L"ローカル・地域", L"交通", L"-", L"-", L"-", L"-", L"その他",

		L"スポーツニュース", L"野球", L"サッカー", L"ゴルフ", L"その他の球技", L"相撲・格闘技", L"オリンピック・国際大会", L"マラソン・陸上・水泳",
		L"モータースポーツ", L"マリン・ウィンタースポーツ", L"競馬・公営競技", L"-", L"-", L"-", L"-", L"その他",

		L"芸能・ワイドショー", L"ファッション", L"暮らし・住まい", L"健康・医療", L"ショッピング・通販", L"グルメ・料理", L"イベント", L"番組紹介・お知らせ",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"その他",

		L"国内ドラマ", L"海外ドラマ", L"時代劇", L"-", L"-", L"-", L"-", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"その他",

		L"国内ロック・ポップス", L"海外ロック・ポップス", L"クラシック・オペラ", L"ジャズ・フュージョン", L"歌謡曲・演歌", L"ライブ・コンサート", L"ランキング・リクエスト", L"カラオケ・のど自慢",
		L"民謡・邦楽", L"童謡・キッズ", L"民族音楽・ワールドミュージック", L"-", L"-", L"-", L"-", L"その他",

		L"クイズ", L"ゲーム", L"トークバラエティ", L"お笑い・コメディ", L"音楽バラエティ", L"旅バラエティ", L"料理バラエティ", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"その他",

		L"洋画", L"邦画", L"アニメ", L"-", L"-", L"-", L"-", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"その他",

		L"国内アニメ", L"海外アニメ", L"特撮", L"-", L"-", L"-", L"-", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"その他",

		L"社会・時事", L"歴史・紀行", L"自然・動物・環境", L"宇宙・科学・医学", L"カルチャー・伝統芸能", L"文学・文芸", L"スポーツ", L"ドキュメンタリー全般",
		L"インタビュー・討論", L"-", L"-", L"-", L"-", L"-", L"-", L"その他",

		L"現代劇・新劇", L"ミュージカル", L"ダンス・バレエ", L"落語・演芸", L"歌舞伎・古典", L"-", L"-", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"その他",

		L"旅・釣り・アウトドア", L"園芸・ペット・手芸", L"音楽・美術・工芸", L"囲碁・将棋", L"麻雀・パチンコ", L"車・オートバイ", L"コンピュータ・ＴＶゲーム", L"会話・語学",
		L"幼児・小学生", L"中学生・高校生", L"大学生・受験", L"生涯教育・資格", L"教育問題", L"-", L"-", L"その他",

		L"高齢者", L"障害者", L"社会福祉", L"ボランティア", L"手話", L"文字（字幕）", L"音声解説", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"その他",

		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"-",

		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"-",

		L"BS/地上デジタル放送用番組付属情報", L"広帯域CSデジタル放送用拡張", L"衛星デジタル音声放送用拡張", L"サーバー型番組付属情報", L"-", L"-", L"-", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"-",

		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"-",
		L"-", L"-", L"-", L"-", L"-", L"-", L"-", L"その他"
	};

	size_t	len;

	if(genre[2] != -1) {
		len =  swprintf_s(buf, bufsize, L"%s 〔%s〕　%s 〔%s〕　%s 〔%s〕", str_genreL[genre[0] >> 4], str_genreM[genre[0]], str_genreL[genre[1] >> 4], str_genreM[genre[1]], str_genreL[genre[2] >> 4], str_genreM[genre[2]]);
	} else if(genre[1] != -1) {
		len =  swprintf_s(buf, bufsize, L"%s 〔%s〕　%s 〔%s〕", str_genreL[genre[0] >> 4], str_genreM[genre[0]], str_genreL[genre[1] >> 4], str_genreM[genre[1]]);
	} else if(genre[0] != -1) {
		len =  swprintf_s(buf, bufsize, L"%s 〔%s〕", str_genreL[genre[0] >> 4],  str_genreM[genre[0]]);
	} else {
		len =  swprintf_s(buf, bufsize, L"n/a");
	}

	return len;
}