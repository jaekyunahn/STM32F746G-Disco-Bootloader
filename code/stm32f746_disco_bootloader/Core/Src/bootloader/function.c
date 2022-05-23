/*
 * function.c
 *
 *  Created on: May 3, 2022
 *      Author: AhnJaeKyun
 */
#include "main.h"
#include "bootloader/bootloader.h"

/*
 *	union
 */
union uIntegerStringConvert
{
	short uIntegerData;
	char uStringData[2];
};

union uInteger32StringConvert
{
	int uIntegerData;
	char uStringData[4];
};

/*
 *  @brief	Bin data -> short type 전환
 *  @param	sData : bin 형태 data
 *  @retval	short type 정수
 */
short fConvertString4BytesToInteger16Type(unsigned char* sData)
{
	union uIntegerStringConvert uF;
	memcpy(uF.uStringData, sData, sizeof(short));
	return uF.uIntegerData;
}

/*
 *  @brief	short type -> Bin data 전환
 *  @param	sData : 변환 된 bin data 받을 포인터 주소
 *			fData : 변환 할 short type 정수
 *  @retval	None
 */
void fConvertInteger16TypeToString4Bytes(short fData, unsigned char* sData)
{
	union uIntegerStringConvert uF;
	uF.uIntegerData = fData;
	memcpy(sData, uF.uStringData, sizeof(short));
}

/*
 *  @brief	Bin data -> int type 전환
 *  @param	sData : bin 형태 data
 *  @retval	int type 정수
 */
int fConvertString4BytesToInteger32Type(char* sData)
{
	union uInteger32StringConvert uF;
	memcpy(uF.uStringData, sData, sizeof(int));
	return uF.uIntegerData;
}

/*
 *  @brief	int type -> Bin data 전환
 *  @param	sData : 변환 된 bin data 받을 포인터 주소
 *			fData : 변환 할 int type 정수
 *  @retval	None
 */
void fConvertInteger32TypeToString4Bytes(int fData, char* sData)
{
	union uInteger32StringConvert uF;
	uF.uIntegerData = fData;
	memcpy(sData, uF.uStringData, sizeof(int));
}

/*
 *  @brief	Convert RGB 3Byte data to RGB565 format
 *  @param	sData : 3byte 형태의 픽셀 정보 (BGR888)
 *  @retval	RGB565 format 
 */
unsigned short fArrayConvertToUINT16(unsigned char* sData)
{
	return ((((sData[2]) & 0xf8) << 8) | (((sData[1]) & 0xfc) << 3) | (((sData[0]) & 0xf8) >> 3));
}

/*
 *  @brief	Convert RGB int data to RGB565 format
 *  @param	int 형태의 ABRG 8888 데이터
 *  @retval	RGB565 format 
 */
unsigned short fINT32ConvertToUINT16(unsigned int sData)
{
	return ((((sData >> 16) & 0xf8) << 8) | (((sData >> 8) & 0xfc) << 3) | (((sData) & 0xf8) >> 3));
}

/*
 *  @brief	문자열 비교
 *  @param	source : 비교 하려는 문자열
 *			target : 비교 할 문자열
 *			iSize : 비교 할 길이
 *  @retval	None
 */
int fCompareFunction(char* source, char* target, int iSize) {
	int i = 0;
	for (i = 0; i < iSize; i++) {
		if (source[i] != target[i]) {
			return 0;
		}
	}
	return 1;
}

/*
 *  @brief  문자열 형태의 숫자를 int32값으로 변환
 *  @param  source  문자열 에서 정수형으로 바꿀 문자열 시작 주소
 *  @retval int32로 변환 된 값
 */
int fConvertStringToInt32(char* source) {
	int buf = source[0] - 0x30;
	int res = 0;
	int i = 1;
	res = res + buf;
	while (1) {
		if ((source[i] >= 0x30) && (source[i] <= 0x39)) {
			res = res * 10;
			buf = source[i] - 0x30;
			res = res + buf;
		}
		else {
			break;
		}
		i++;
	}
	return res;
}

/*
 *  @brief	비트맵 헤더 전용 변환기. 
 *  @param	sData : bin array data
 *			iDataSize : 추출 할 데이터 크기
 *			iOffset : 입력 된 배열에서 변환 시작 할 위치
 *  @retval	int type 데이터
 */
int getBMPinfoFuncBinToInteger(unsigned char* sData, int iDataSize, int iOffset)
{
    unsigned char data[128] = "";
    unsigned int iResult = 0;

    memset(data, 0x00, sizeof(data));
    memcpy(data, sData + iOffset, iDataSize);

    switch (iDataSize) {
    case 1:
        iResult = data[0];
        break;
    case 2:
        iResult = data[0] + (data[1] * 256);
        break;
    case 3:
        iResult = data[0] + (data[1] * 256) + (data[2] * 65536);
        break;
    case 4:
        iResult = data[0] + (data[1] * 256) + (data[2] * 65536) + (data[3] * 4294967296);
        break;
    default:
        break;
    }

    return iResult;
}

/*
 *  @brief	특정 범위 내 존재 하는지 여부 함수화
 *  @param	None
 *  @retval	None
 */
int inside_location(int x1, int x2, int y1, int y2, int location_x, int location_y)
{
	int res = 0;

	if(((location_x >= x1)&&(location_x <= x2))&&((location_y >= y1)&&(location_y <= y2)))
	{
		res = 1;
	}
	return res;
}

/*
 *  @brief	Xor checksum
 *  @param	data : 체크섬 검사할 배열데이터
 *			size : 배열 데이터 에서 검사할 크기
 *  @retval	1Byte XOR Check sum data
 */
unsigned char xor_checksum(unsigned char *data, int size)
{
	unsigned char res =0;
	for(int x = 0; x < size; x++)
		res = res ^ data[x];
	return res;
}