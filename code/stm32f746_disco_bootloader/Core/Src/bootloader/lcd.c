/*
 * lcd.c
 *
 *  Created on: May 3, 2022
 *      Author: AhnJaeKyun
 */
#include "main.h"
#include "crc.h"
#include "dma2d.h"
#include "fatfs.h"
#include "i2c.h"
#include "ltdc.h"
#include "rng.h"
#include "rtc.h"
#include "sdmmc.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"
#include "bootloader/bootloader.h"
#include "bootloader/font.h"

/*
 *  @brief	Enables the display.
 *  @param	None
 *  @retval	None
 */
void BSP_LCD_DisplayOn(void){
	/* Display On */
	__HAL_LTDC_ENABLE(&hltdc);
	/* Assert LCD_DISP pin */
	HAL_GPIO_WritePin(GPIOI, GPIO_PIN_12, GPIO_PIN_SET); 
	/* Assert LCD_BL_CTRL pin */
	HAL_GPIO_WritePin(GPIOK, GPIO_PIN_3, GPIO_PIN_SET);  
}

/*
 *  @brief	Gets the LCD X size.
 *  @param	None
 *  @retval	Used LCD X size
 */
uint32_t BSP_LCD_GetXSize(void)
{
	return hltdc.LayerCfg[ActiveLayer].ImageWidth;
}

/*
 *  @brief	Gets the LCD Y size.
 *  @param	None
 *  @retval	Used LCD Y size
 */
uint32_t BSP_LCD_GetYSize(void)
{
	return hltdc.LayerCfg[ActiveLayer].ImageHeight;
}

/*
 *  @brief	특정 색상으로 LCD 초기화
 *  @param	iRGB565Format RGB565 type data
 *  @retval	None
 */
void init_lcd(unsigned short iRGB565Format)
{
	for(int x = 0 ; x < 261120 ; x++)
	{
		*((uint16_t*)SDRAMADDRESS_FRAMEBUFFER + 0 + x) = iRGB565Format;
	}
}

/*
 *  @brief	특정 부위에 픽셀 색상 set
 *  @param	iRGB565Format 설정 하려는 RGB565 Type의 색상
 *			address SDRAM Address로 접근
 *  @retval	None
 */
void setpixel(unsigned short iRGB565Format, uint32_t address)
{
	*((uint16_t*)address + SDRAM_OFFSET) = iRGB565Format;
}

/*
 *  @brief	특정 주소에 이미지 합성
 *  @param	address	 설정 하려는 주소
 *			imageInfoData	복사 하려는 이미지의 정보, 이미지가 있는 시작 주소, 크기, 위치 등
 *  @retval	None
 */
void imageCompositing(uint32_t address, struct READ_IMAGE_INFO imageInfoData)
{
	// SDRAM에서 데이터는 1차 배열로 간주 되기 때문에 픽셀 인덱스 사용 하여 순차적으로 읽는다.
	int pixel_index = 0;
	// 1차 배열 이미지 정보를 가지고 있는 크기에 기반 하여 몇번째 마다 줄 바꿈 할지 확인 용도로 사용하는 인덱스
	int image_x_index = 0,image_y_index = 0;
	
	// 이미지 구조체 정보에서 합성 할 이미지 사이즈 정보 set
	image_x_index = imageInfoData.image_x_start_location;
	image_y_index = imageInfoData.image_y_start_location;

	//이미지 복사
	for(pixel_index = 0 ; pixel_index < (imageInfoData.image_x_size * imageInfoData.image_y_size) ; pixel_index++)
	{
		// 복사 하려는 주소 < 복사 할 이미지 정보
		*((uint16_t*)address + ((image_y_index * 480) + image_x_index) + SDRAM_OFFSET ) = *((uint16_t*)imageInfoData.image_sdram_address + pixel_index);
		//1픽셀 복사 할 때마다 +1
		image_x_index++;
		// 현재 x index가 이미지 가로 크기 + 이미지 시작 위치일 때
		if(image_x_index == imageInfoData.image_x_size + imageInfoData.image_x_start_location)
		{
			// x index는 시작 위치, y 인덱스 증가
			image_x_index = imageInfoData.image_x_start_location;
			image_y_index++;
		}
	}
}

/*
 *  @brief	특정 부분에 아스키 코드 기반 문자 출력하는 함수
 *  @param	iLocationX : 글자 위치 x
 *			iLocationY : 글자 위치 y
 *			cData : 출력 할 글자
 *			char_color : 글자 색상
 *			back_color : 글자 배경 색, -1이면 따로 표현 x
 *  @retval	None
 */
void fDisplayChar(int iLocationX, int iLocationY, char cData, int char_color, int back_color)
{
	// 매번 연산 방지를 위해 미리 색상을 컨버팅 한다.
	short char_color_int16 = fINT32ConvertToUINT16(char_color);
	short back_color_int16 = fINT32ConvertToUINT16(back_color);

	// 아스키 코드 구조 상 사람이 볼 수 있는 문자는 0x20인 ' '부터 이므로 char data 에 - 32.
	// 폰트 구조는 가로가 8pixel인 1바이트(픽셀을 비트로 압축), 세로 12pixel인 구조로 되어 있기 때문에 문자 간 간격은 12바이트로 되어 있다
	int iIndexChar = (cData - 32) * 12;
	// 각종 범용성 변수
	int x, y, z, i = 0;

	// 열 단위 검색 : 12pixel
	for (y = 0; y < 12; y++)
	{
		// 행 단위 검색 : 8pixel => 1 byte로 압축
		for (x = 0; x < 1; x++)
		{
			// x pixel 가져오기 위해 비트 연산 
			for (z = 7; z >= 0; z--)
			{
				// 만약 해당 위치가 1비트 설정 되어 있으면 글자 색상으로 대체
				if (((Font12_Table[iIndexChar + (y + x)] >> z) & 0x1) == 1)
				{
					setpixel(char_color_int16,SDRAMADDRESS_MAINFRAME+(((iLocationY + y)*480*2)+(iLocationX*2 + (i*2))));
				}
				else//글자 배경
				{
					// 빈 여백인 경우 만약 배경 색이 따로 지정 되어 있다면 set pixel 하고 만일 배경색이 -1로 설정 되어 있으면 처리하지 않는다.
					if(back_color != -1){
						setpixel(back_color_int16,SDRAMADDRESS_MAINFRAME+(((iLocationY + y)*480*2)+(iLocationX*2 + (i*2))));
					}
				}
				i++;
			}
		}
		i = 0;
	}
}

/*
 *  @brief	특정 부분에 아스키 코드 기반 문자열 출력하는 함수
 *  @param	iLocationX : 문자열 시작 위치 x
 *			iLocationY : 문자열 시작 위치 y
 *			char_color : 문자열 색장
 *			back_color : 문자열 배경색, -1이면 따로 표현하지 않는다.
 *			*p : 출력할 문자열 포인터 주소
 *  @retval	None
 */
void fDisplayString(int iLocationX, int iLocationY, int char_color, int back_color, const char *p, ...)
{
	int x = 0;

	// Null Byte 나올 때 까지 반복, 출력하는 String은 아스키 코드 0x20~0x7E 이므로 0x00이 나올 수가 없다.
	while (*p != '\0')
	{
		fDisplayChar(iLocationX + x, iLocationY, *p, char_color, back_color);
		// 글자 크기 8pixel만큼 이동
		x = x + 8;
		// 다음 글자 주소
		p++;
	}
}

/*
 *  @brief	사각형 그리기
 *  @param	target_x : 사각형 시작 위치 x
 *			target_y : 사각형 시작 위치 y
 *			wide : 사각형 너비
 *			hight : 사각형 높이
 *			window_offset_x : 표현 할 x 오프셋. 만약에 전체 화면 기준이 아닌 특정 윈도우에서 시작 할 경우 편리상 만듦
 *			window_offset_y : 표현 할 y 오프셋. 
 *			address : 그릴 메모리 주소. 일반적인 이미지 주소보다 이미지 섞는 main frame에 하는걸 권장
 *			iRGB565Format : 사각형 색상
 *  @retval	None
 */
void drawingSquare(int target_x, int target_y, int wide, int hight, int window_offset_x, int window_offset_y, uint32_t address, unsigned short iRGB565Format)
{
	int x, y;
	for (y = target_y; y < target_y + hight; y++)
	{
		for (x = target_x; x < target_x+ wide; x++)
		{
			*((uint16_t*)address +( (y+ window_offset_y) * 480 + ((x+ window_offset_x)+1))) = iRGB565Format;
		}
	}
}


