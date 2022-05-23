/*
 * bootloader.h
 *
 *  Created on: May 2, 2022
 *      Author: AhnJaeKyun
 */

#ifndef INC_BOOTLOADER_H_
#define INC_BOOTLOADER_H_

/***********************************************************************************************
 *
 *										Include
 *
 ***********************************************************************************************/
#include <string.h>
#include <stdio.h>

/***********************************************************************************************
 *
 *										Define
 *
 ***********************************************************************************************/
//SDRAM
#define SDRAM_MODEREG_BURST_LENGTH_1             	((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_LENGTH_2             	((uint16_t)0x0001)
#define SDRAM_MODEREG_BURST_LENGTH_4             	((uint16_t)0x0002)
#define SDRAM_MODEREG_BURST_LENGTH_8             	((uint16_t)0x0004)
#define SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL      	((uint16_t)0x0000)
#define SDRAM_MODEREG_BURST_TYPE_INTERLEAVED     	((uint16_t)0x0008)
#define SDRAM_MODEREG_CAS_LATENCY_2              	((uint16_t)0x0020)
#define SDRAM_MODEREG_CAS_LATENCY_3              	((uint16_t)0x0030)
#define SDRAM_MODEREG_OPERATING_MODE_STANDARD    	((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_PROGRAMMED 	((uint16_t)0x0000)
#define SDRAM_MODEREG_WRITEBURST_MODE_SINGLE     	((uint16_t)0x0200)
#define SDRAM_TIMEOUT								((uint32_t)0xFFFF)
#define REFRESH_COUNT								((uint32_t)0x0603)

//lcd
#define SDRAMADDRESS_FRAMEBUFFER	((uint32_t)0xC0000000)
#define SDRAMADDRESS_MAINFRAME		((uint32_t)0xC003FC80)
#define SDRAMADDRESS_STRINGDISPLAY	((uint32_t)0xC007F900)
#define	SDRAMADDRESS_IMAGEMEMORY	((uint32_t)0xC00BF580)

//RGB565 Table
#define LCD_COLOR_BLUE          ((uint32_t)0xFF0000FF)
#define LCD_COLOR_GREEN         ((uint32_t)0xFF00FF00)
#define LCD_COLOR_RED           ((uint32_t)0xFFFF0000)
#define LCD_COLOR_CYAN          ((uint32_t)0xFF00FFFF)
#define LCD_COLOR_MAGENTA       ((uint32_t)0xFFFF00FF)
#define LCD_COLOR_YELLOW        ((uint32_t)0xFFFFFF00)
#define LCD_COLOR_LIGHTBLUE     ((uint32_t)0xFF8080FF)
#define LCD_COLOR_LIGHTGREEN    ((uint32_t)0xFF80FF80)
#define LCD_COLOR_LIGHTRED      ((uint32_t)0xFFFF8080)
#define LCD_COLOR_LIGHTCYAN     ((uint32_t)0xFF80FFFF)
#define LCD_COLOR_LIGHTMAGENTA  ((uint32_t)0xFFFF80FF)
#define LCD_COLOR_LIGHTYELLOW   ((uint32_t)0xFFFFFF80)
#define LCD_COLOR_DARKBLUE      ((uint32_t)0xFF000080)
#define LCD_COLOR_DARKGREEN     ((uint32_t)0xFF008000)
#define LCD_COLOR_DARKRED       ((uint32_t)0xFF800000)
#define LCD_COLOR_DARKCYAN      ((uint32_t)0xFF008080)
#define LCD_COLOR_DARKMAGENTA   ((uint32_t)0xFF800080)
#define LCD_COLOR_DARKYELLOW    ((uint32_t)0xFF808000)
#define LCD_COLOR_WHITE         ((uint32_t)0xFFFFFFFF)
#define LCD_COLOR_LIGHTGRAY     ((uint32_t)0xFFD3D3D3)
#define LCD_COLOR_GRAY          ((uint32_t)0xFF808080)
#define LCD_COLOR_DARKGRAY      ((uint32_t)0xFF404040)
#define LCD_COLOR_BLACK         ((uint32_t)0xFF000000)
#define LCD_COLOR_BROWN         ((uint32_t)0xFFA52A2A)
#define LCD_COLOR_ORANGE        ((uint32_t)0xFFFFA500)
#define LCD_COLOR_TRANSPARENT   ((uint32_t)0xFF000000)

// Base address of the Flash sectors Bank 1
#define ADDR_FLASH_SECTOR_0     ((uint32_t)0x08000000) /* Base @ of Sector 0, 32 Kbytes */
#define ADDR_FLASH_SECTOR_1     ((uint32_t)0x08008000) /* Base @ of Sector 2, 32 Kbytes */
#define ADDR_FLASH_SECTOR_2     ((uint32_t)0x08010000) /* Base @ of Sector 4, 32 Kbytes */
#define ADDR_FLASH_SECTOR_3     ((uint32_t)0x08018000) /* Base @ of Sector 5, 32 Kbytes */
#define ADDR_FLASH_SECTOR_4     ((uint32_t)0x08020000) /* Base @ of Sector 6, 128 Kbytes */
#define ADDR_FLASH_SECTOR_5     ((uint32_t)0x08040000) /* Base @ of Sector 7, 256 Kbytes */
#define ADDR_FLASH_SECTOR_6     ((uint32_t)0x08080000) /* Base @ of Sector 8, 256 Kbytes */
#define ADDR_FLASH_SECTOR_7     ((uint32_t)0x080C0000) /* Base @ of Sector 9, 256 Kbytes */

//application area
#define FLASH_USER_START_ADDR 	ADDR_FLASH_SECTOR_5
#define FLASH_USER_END_ADDR 	ADDR_FLASH_SECTOR_7

//jump to address(app)
#define	APPLICATION_ADDRESS		FLASH_USER_START_ADDR

#define SDRAM_OFFSET	0

/***********************************************************************************************
 *
 *										enum
 *
 ***********************************************************************************************/
enum bootloadermode
{
	//대기
	not_work = 0,
	// 앱 선택
	select_window = 1,
	// 부팅
	boot = 2,
	// 최신 펌웨어 찾기
	download = 3,
	// 윈도우 종료
	close_window = 4
};

enum bootloaderImage
{
	logo_image_number = 0,
	select_window_image_number = 1,
	boot_button_image_number = 2,
	down_button_image_number = 3,
	select_button_image_number = 4,
	text_bar_image_number = 5,
	download_window_image_number = 6
};

enum bootloader_working_case
{
	bootloader_idel = 0,
	fw_download = 1,
	select_image = 2,
	booting_now = 3
};

enum bootloader_update_flow
{
	uart_communication = 0,
	flash_write_preparation = 1,
	flash_write_process = 2,
	flash_write_completion = 3
};

/***********************************************************************************************
 *
 *										structer
 *
 ***********************************************************************************************/
//boot config
struct BOOTCONFIG{
	char autoboot;
	char bootlog;
};

//bitmap header
struct BMP_HEADER_STRUCT
{
	unsigned short	iMagicNumber;
	unsigned int 	iBitmapFileSize;
	unsigned short	bfReserved1;
	unsigned short	bfReserved2;
	unsigned int 	iButmapRealDataOffsetValue;
	unsigned int 	iBackendHeaderSize;
	unsigned int 	iBitmapXvalue;
	unsigned int 	iBitmapYvalue;
	unsigned short 	iColorPlane;
	unsigned short 	iPixelBitDeepValue;
	unsigned int 	iCompressType;
	unsigned int 	iImageSize;
	unsigned int 	iPixcelPerMeterXvalue;
	unsigned int 	iPixcelPerMeterYvalue;
	unsigned int 	iColorPalet;
	unsigned int 	iCriticalColorCount;
};

//read image struct
struct READ_IMAGE_INFO
{
	uint32_t image_sdram_address;
	int image_x_size;
	int image_y_size;
	int image_x_start_location;
	int image_y_start_location;
};

struct FILE_WRITE_RES
{
	int last_error_code;
	int last_error_function;
	int last_write_count;
	int will_write_count;
};
//current_progress
struct CURRENT_PROGRESS
{
	int download;
	int flashwrite;
};

/***********************************************************************************************
 *
 *										variable
 *
 ***********************************************************************************************/
static uint32_t	ActiveLayer = 0;

/***********************************************************************************************
 *
 *										Function
 *
 ***********************************************************************************************/
//bootloader
void bootloader_application(void);
int app_Partition_erase(void);
void jump_to_application(void);
static uint32_t GetSector(uint32_t Address);
struct BOOTCONFIG parsingConfigFile(char* filedata, int filesize);
struct READ_IMAGE_INFO readBitmapFileToSdram(char* fileaddress);
void callfunction_for_timer(void);
struct CURRENT_PROGRESS firmwareUpdate(void);
void test(void);
int flash_wrtie(unsigned int Address, unsigned int end_address, char *data);

//sdram
void BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount);
void lookSDRAM(int startaddress,int size);
void copymemory(uint32_t source_address,uint32_t target_address, int block_size);
void init_sdram(int block_size);

// lcd
void BSP_LCD_DisplayOn(void);
void init_lcd(unsigned short iRGB565Format);
void setpixel(unsigned short iRGB565Format, uint32_t address);
void imageCompositing(uint32_t address, struct READ_IMAGE_INFO imageInfoData);
void fDisplayString(int iLocationX, int iLocationY, int char_color, int back_color, const char *p, ...);
uint32_t BSP_LCD_GetYSize(void);
uint32_t BSP_LCD_GetXSize(void);
void drawingSquare(int target_x, int target_y, int wide, int hight, int window_offset_x, int window_offset_y, uint32_t address, unsigned short iRGB565Format);

//SDMMC
void init_sd(void);
int file_size(char *file_address);
int fatfs_read_index_readsize(char *file_address ,char *read_data, int iStartPoint, int read_size);
struct FILE_WRITE_RES file_write(char *file_address, char *write_data, int writesize, int writeindex, int write_mode);
struct FILE_WRITE_RES append_write(char *file_address, char *write_data, int writesize);
int deletfile(char *file_address);
int renamefile(char *file_address, char *rename);

//function
void fConvertInteger16TypeToString4Bytes(short fData, unsigned char* sData);
short fConvertString4BytesToInteger16Type(unsigned char* sData);
void fConvertInteger32TypeToString4Bytes(int fData, char* sData);
int fConvertString4BytesToInteger32Type(char* sData);
unsigned short fArrayConvertToUINT16(unsigned char* sData);
unsigned short fINT32ConvertToUINT16(unsigned int sData);
int fCompareFunction(char* source, char* target, int iSize);
int fConvertStringToInt32(char* source);
int getBMPinfoFuncBinToInteger(unsigned char* sData, int iDataSize, int iOffset);
int inside_location(int x1, int x2, int y1, int y2, int location_x, int location_y);
unsigned char xor_checksum(unsigned char *data, int size);

//touch screen
void init_touch_screen(void);
void get_touch_location(int *location);




#endif /* INC_BOOTLOADER_H_ */
