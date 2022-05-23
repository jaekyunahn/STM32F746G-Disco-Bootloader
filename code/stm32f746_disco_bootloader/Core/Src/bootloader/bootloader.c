/*
 * bootloader.c
 *
 *  Created on: May 2, 2022
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
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fmc.h"
#include "bootloader/bootloader.h"

/*
 *	Define
 */
// STM32 Flash Memory에 데이터 쓸때 SD Card에서 한번 읽을 크기
#define WRITE_FLASH_SIZE	512
// 수신 버퍼 최대 크기
#define PACKET_MAX_SIZE		2048
// fatfs 파일명 / 파일 주소 변수 최대 크기
#define MAX_FILESYSTEM_LEN	256
// load bmp image to sdram
#define BITMAP_READ_DATA_SIZE	1440	//x최대길이

/*
 * 	variable
 */
//SDRAM 이미지 저장 주소 - static
uint32_t image_sdram_pointer = (uint32_t)SDRAMADDRESS_IMAGEMEMORY;
// 터치좌표 x,y,변화 여부
int ts_location[3] = {-1,-1,0};
// uart 수신 데이터 index - static
int uartRxDataIndex = 0;
// uart rx dma로 최초 받은 데이터
char incomingdata[PACKET_MAX_SIZE] = "";
// 임시로 사용 할 배열 변수
char tmp[PACKET_MAX_SIZE] = "";
// STM32 Flash Memory 기록할 데이터 버퍼
char flash_write_buffer[WRITE_FLASH_SIZE] = "";
//file read buffer
char file_read_buffer[2048] = "";
// 펌웨어 업데이트 순서흐름 - static
//int fwupdateflow = 0;
// 파일 쓰기 Error횟수 - static
int writeerrorcount = 0;
// 최대 패킷 크기
int paket_size = 0;
// 패킷 카운트
int paket_count = 0;
// 마지막 패킷 크기
int last_paket_size = 0;
// 파일명 -> uart로 받은 정보
char file_name[MAX_FILESYSTEM_LEN] = "";
// 파일 주소 -> uart로 부터 받은 파일명을 특정 디렉터리 주소까지 결합 한 변수
char file_address[MAX_FILESYSTEM_LEN] = "";
//data paket
int datapaketIndex = 0;
// 부트로더 현재 작업
int bootloader_working_number = bootloader_idel;
// XOR checksum 1Byte
unsigned char check_sum = 0;
// STM32 Flash Memory 쓰기 시작 주소 - static
unsigned int flash_write_start_address = 0;
// STM32 Flash Memory 쓰기 종료 주소  - static
unsigned int flash_write_end_address = 0;
// FW update 현재 흐름
int updateMode = 0;
// STM32 Flash Memory 쓰기 횟수
int write_flash_count = 0;

/*
 *  @brief	Bootloader Main Function
 *  @param	None
 *  @retval	None
 */
void bootloader_application(void)
{
	//uart Rx DMA 사용
	HAL_UART_Receive_DMA(&huart1, incomingdata, sizeof(incomingdata));

	// ui 띄울지 여부
	// 0 : 비활성화
	// 1 : 활성화
	char enable_image[64] = "";

	// display string
	char display_log[128] = "";

	// 부트로더 터치 이벤트
	int bootloader_touch_event = 0;

	// 부트로더 app loop flag
	int bootloaderAppLoopFlag = 1;

	// 범용 변수
	int x,y;

	// touch 좌표
	int ts_location_x = -1, ts_location_y = -1;

	// 각종 결과 return 받는 변수
	int res = 0;
	float fres = 0.0f;

	// 읽은 파일 크기
	int iReadFileSize = 0;

	// SDRAM에 저장한 Image 관련 정보 구조체 선언 및 초기화
	struct READ_IMAGE_INFO bootloader_image[64];
	for(x = 0 ; x < 64 ; x++)
	{
		// 이미지 시작 주소
		bootloader_image[x].image_sdram_address = 0x00000000;
		// 가로길이
		bootloader_image[x].image_x_size = 0;
		// 세로길이
		bootloader_image[x].image_y_size = 0;
		// 시작 x좌표
		bootloader_image[x].image_x_start_location = 0;
		// 시작 y좌표
		bootloader_image[x].image_y_start_location = 0;
	}

	// 부트로더 설정파일 읽은 정보 저장 구조체
	struct BOOTCONFIG bootconfig;

	// 펌웨어 업데이트 진척도 결과 구조체
	struct CURRENT_PROGRESS current_res;
	current_res.flashwrite = 0;
	current_res.download = 0;

	/*
	 * 	init device
	 */

	// SDRAM init
	BSP_SDRAM_Initialization_sequence(REFRESH_COUNT);
	printf("[Bootloader]SDRAM init\n");
	// 0으로 초기화 할 범위 -> 당장 사용 안해도 됨
	//init_sdram(1048576);

	// touch init
	init_touch_screen();

	// LCD init
	init_lcd(fINT32ConvertToUINT16(LCD_COLOR_BLACK));
	printf("[Bootloader]LCD init\n");
	BSP_LCD_DisplayOn();
	printf("[Bootloader]LCD On\n");

	// SD Card init
	res=BSP_SD_Init();
	printf("BSP_SD_Init=%d\n",res);
	init_sd();

	//load config file
	memset(file_read_buffer,0x00,sizeof(file_read_buffer));
	iReadFileSize = file_size("/bootloader/script/main.txt");
	printf("[Bootloader]config file size=%d\n",iReadFileSize);
	fatfs_read_index_readsize("/bootloader/script/main.txt",file_read_buffer,0,iReadFileSize);
	printf("[Bootloader]read config file=%s\n",file_read_buffer);

	// 부트로더 설정 read
	bootconfig = parsingConfigFile(file_read_buffer, iReadFileSize);
	printf("autoboot=%d\n", bootconfig.autoboot);
	printf("bootlog=%d\n", bootconfig.bootlog);

	//image read
	// logo
	bootloader_image[logo_image_number] = readBitmapFileToSdram("/bootloader/source/logo.bmp");
	// window
	bootloader_image[select_window_image_number] = readBitmapFileToSdram("/bootloader/source/window.bmp");
	// booting 버튼
	bootloader_image[boot_button_image_number] = readBitmapFileToSdram("/bootloader/source/Booting.bmp");
	// download 버튼
	bootloader_image[down_button_image_number] = readBitmapFileToSdram("/bootloader/source/Download.bmp");
	// select 버튼
	bootloader_image[select_button_image_number] = readBitmapFileToSdram("/bootloader/source/select.bmp");
	// text bar
	bootloader_image[text_bar_image_number] = readBitmapFileToSdram("/bootloader/source/text_bar.bmp");
	// download_window_image_number
	bootloader_image[download_window_image_number] = readBitmapFileToSdram("/bootloader/source/fwupdate.bmp");
	
	//timer Start
	// 터치 스크린 데이터를 실시간으로 받기 위해 타이머로 작업, 타이머 인터럽트에서 call하는 함수는 bootloader.c의 callfunction_for_timer
	HAL_TIM_Base_Start_IT(&htim11);
	printf("[Bootloader]timer start\n");

#if 1
	if(bootconfig.bootlog == 1)
	{
		// 부팅로고 출력 사용
		enable_image[logo_image_number] = 1;

		// 부팅 로고 띄움. 0,0위치
		bootloader_image[logo_image_number].image_x_start_location = 0;
		bootloader_image[logo_image_number].image_y_start_location = 0;

		// SDRAMADDRESS_MAINFRAME에 쓰기
		imageCompositing(SDRAMADDRESS_MAINFRAME,bootloader_image[logo_image_number]);

		// SDRAMADDRESS_FRAMEBUFFER -> SDRAMADDRESS_MAINFRAME로 130560*2 Byte 복사
		copymemory(SDRAMADDRESS_MAINFRAME,SDRAMADDRESS_FRAMEBUFFER,130560);
	}
#else
	// 그래픽 테스트 용도
	// 창살 무늬 출력
	uint16_t* frame_buffer = (uint16_t*) 0xC0000000;
	for(y=0;y<272;y++)
	{
		for(x=0;x<480;x++)
		{
			if(x % 2 ==1){
				*(frame_buffer + y * 480 + x) = 0x0000;
			}
			if(x % 2 ==0){
				*(frame_buffer + y * 480 + x) = 0xFFFF;
			}
		}
	}

	//테스트 이미지 출력
	bootloader_image[0] = readBitmapFileToSdram("/bootloader/source/arme.bmp");
	bootloader_image[0].image_x_start_location = 0;
	bootloader_image[0].image_y_start_location = 0;
	imageCompositing(SDRAMADDRESS_MAINFRAME,bootloader_image[0]);
	copymemory(SDRAMADDRESS_MAINFRAME,SDRAMADDRESS_FRAMEBUFFER,130560);
#endif

	// 설정 파일에서 자동 부팅이 비활성화로 되어 있으면 동작
	if(bootconfig.autoboot == 0)
	{
		// 자동 부팅이 아닌 경우
		// 메뉴 선택을 위한 윈도우 좌표 설정
		bootloader_image[select_window_image_number].image_x_start_location = 50;
		bootloader_image[select_window_image_number].image_y_start_location = 30;
		enable_image[select_window_image_number] = 0;

		// select button
		//bootloader_image[select_button_image_number].image_x_start_location = 253;
		//bootloader_image[select_button_image_number].image_y_start_location = 245;
		//enable_image[select_button_image_number] = 1;

		// down
		bootloader_image[down_button_image_number].image_x_start_location = 329;
		bootloader_image[down_button_image_number].image_y_start_location = 245;
		enable_image[down_button_image_number] = 1;

		// boot
		bootloader_image[boot_button_image_number].image_x_start_location = 405;
		bootloader_image[boot_button_image_number].image_y_start_location = 245;
		enable_image[boot_button_image_number] = 1;

		// boot
		bootloader_image[download_window_image_number].image_x_start_location = 50;
		bootloader_image[download_window_image_number].image_y_start_location = 30;
		enable_image[download_window_image_number] = 0;

		//bootloader app loop
		while(bootloaderAppLoopFlag)
		{
			//-------------------------------------------------------------------------------------------------------------------------------------
			//터치 입력 정보 받기
			bootloader_touch_event = not_work;
			// 변화여부 감지
			if(ts_location[2] == 1)
			{
				//printf("[main]touch envent\n");
				ts_location_x = ts_location[0];
				ts_location_y = ts_location[1];

				// 선택 창 종료
				if(inside_location(420,431,30,43,ts_location_x,ts_location_y)){
					bootloader_touch_event = close_window;
				}
				// 선택창 open 버튼 입력 -> 당장 사용 x
				//if(inside_location(253,325,245,269,ts_location_x,ts_location_y)){
				//	bootloader_touch_event = select_window;
				//}
				// booting 버튼 입력
				if(inside_location(405,479,245,269,ts_location_x,ts_location_y)){
					bootloader_touch_event = boot;
				}
				// 다운로드 버튼 입력
				if(inside_location(329,401,245,269,ts_location_x,ts_location_y)){
					bootloader_touch_event = download;
				}

				ts_location[2] = 0;
			}

			//-------------------------------------------------------------------------------------------------------------------------------------

			switch(bootloader_touch_event)
			{
			case not_work:
				break;
			case select_window:
				printf("[main]select_window\n");
				if ((enable_image[select_window_image_number] == 0) && (enable_image[download_window_image_number] == 0))
				{
					enable_image[select_window_image_number] = 1;
					bootloader_working_number = select_image;
				}
				break;
			case boot:
				if ((enable_image[select_window_image_number] == 0) && (enable_image[download_window_image_number] == 0)) 
				{
					printf("[main]boot\n");
					bootloader_working_number = booting_now;
				}
				break;
			case download:
				printf("[main]download\n");
				if ((enable_image[select_window_image_number] == 0) && (enable_image[download_window_image_number] == 0))
				{
					enable_image[download_window_image_number] = 1;
					bootloader_working_number = fw_download;
				}
				break;
			case close_window:
				printf("[main]close_window\n");
				if (enable_image[select_window_image_number] == 1) {
					enable_image[select_window_image_number] = 0;
				}
				if (enable_image[download_window_image_number] == 1) {
					enable_image[download_window_image_number] = 0;
				}
				bootloader_working_number = bootloader_idel;
				break;
			}

			//-------------------------------------------------------------------------------------------------------------------------------------
			
			// function
			// 기능 구현 시 내부에서 절대 무한 루프 돌리면 안됨
			//동작 조건 통하여 한 작업 중에 다른 작업 동시 실행 못하게 방지 할 것
			switch(bootloader_working_number) {
			case bootloader_idel:
				break;
			case fw_download:
				current_res = firmwareUpdate();
				break;
			case select_image:
				break;
			case booting_now:
				bootloaderAppLoopFlag = 0;
				break;
			}

			// 프레임 조합.
			for(x = 0 ; x < 64 ; x++){
				if(enable_image[x] == 1){
					imageCompositing(SDRAMADDRESS_MAINFRAME,bootloader_image[x]);
				}
			}

			// 현재 터치좌표 display
			//memset(display_log,0x00,sizeof(display_log));
			//sprintf(display_log,"touch:%4d,%4d",ts_location[0],ts_location[1]);
			//fDisplayString(5,5,LCD_COLOR_WHITE,LCD_COLOR_BLACK,display_log);

			// 진척도 그리기
			// Down Load 창이 떠 있을 경우에만 그리기
			if (enable_image[download_window_image_number] == 1)
			{
				fres = (361.0f / 100.0f) * (float)current_res.download;
				drawingSquare(11, 72, (int)fres, 23,
					bootloader_image[download_window_image_number].image_x_start_location,
					bootloader_image[download_window_image_number].image_y_start_location,
					SDRAMADDRESS_MAINFRAME, fINT32ConvertToUINT16(LCD_COLOR_GREEN)
				);

				fres = (361.0f / 100.0f) * (float)current_res.flashwrite;
				drawingSquare(11, 144, (int)fres, 23,
					bootloader_image[download_window_image_number].image_x_start_location,
					bootloader_image[download_window_image_number].image_y_start_location,
					SDRAMADDRESS_MAINFRAME, fINT32ConvertToUINT16(LCD_COLOR_GREEN)
				);
			}

			// 최종 프레임 복붙 
			copymemory(SDRAMADDRESS_MAINFRAME,SDRAMADDRESS_FRAMEBUFFER,130560);
		}
	}

	// application으로 jump
	printf("jump to application\r\n");
	jump_to_application();
}

/*
 *  @brief	Application 영역 섹터 지우는 함수
 *  @param	None
 *  @retval	쓰기 성공 여부, 0은 성공, 나머지는 실패
 */
int app_Partition_erase(void)
{
	HAL_StatusTypeDef res = HAL_OK;

	uint32_t FirstSector = 0;
	uint32_t NbOfSectors = 0;
	uint32_t SECTORError = 0;

	static FLASH_EraseInitTypeDef EraseInitStruct;

	//flash unlock
	HAL_FLASH_Unlock();

	/* Get the 1st sector to erase */
	FirstSector = GetSector(FLASH_USER_START_ADDR);

	/* Get the number of sector to erase from 1st sector*/
	NbOfSectors = GetSector(FLASH_USER_END_ADDR) - FirstSector + 1;

	EraseInitStruct.TypeErase     = FLASH_TYPEERASE_SECTORS;
	EraseInitStruct.VoltageRange  = FLASH_VOLTAGE_RANGE_3;
	EraseInitStruct.Sector        = FirstSector;
	EraseInitStruct.NbSectors     = NbOfSectors;

	//flash Erase
	res = HAL_FLASHEx_Erase(&EraseInitStruct, &SECTORError);
	if(res != HAL_OK) {
		printf("[MCU][Error]HAL_FLASHEx_Erase=%d\n",res);
		return res;
	}

	//flash unlock
	HAL_FLASH_Lock();

	return res;
}

/*
 *	@brief	Gets the sector of a given address
 *	@param	Address	select flash sector
 *	@retval	The sector of a given address
 */
static uint32_t GetSector(uint32_t Address)
{
	uint32_t sector = 0;

	if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
	{
		sector = FLASH_SECTOR_0;
	}
	else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
	{
		sector = FLASH_SECTOR_1;
	}
	else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
	{
		sector = FLASH_SECTOR_2;
	}
	else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
	{
		sector = FLASH_SECTOR_3;
	}
	else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
	{
		sector = FLASH_SECTOR_4;
	}
	else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
	{
		sector = FLASH_SECTOR_5;
	}
	else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
	{
		sector = FLASH_SECTOR_6;
	}
	else
	{
		sector = FLASH_SECTOR_7;
	}
	return sector;
}

/*
 *  @brief	Flash 특정 주소에 데이터 쓰기 함수
 *  @param	Address		write flash start address
 *			end_address	write flash end address
 *			data		쓸 데이터 배열 시작 주소
 *  @retval	쓰기 성공 여부, 0은 성공, 나머지는 실패
 */
int flash_wrtie(unsigned int Address, unsigned int end_address, char *data)
{
	HAL_StatusTypeDef res = HAL_OK;
	int index = 0;
	int buffer = 0;

	//flash unlock
	HAL_FLASH_Unlock();

	while(Address < end_address)
	{
		buffer=fConvertString4BytesToInteger32Type(data + index);
		// 4Byte
		res = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Address, buffer);

		if(res != HAL_OK)
		{
			printf("[MCU][ERROR]%d\n",res);
			return res;
		}
		Address = Address + 4;
		index = index + 4;
	}

	//flash unlock
	HAL_FLASH_Lock();

	return res;
}

/*
 *  @brief	특정 영역으로 jump 하는 함수, 자체 부트로더에서 어플리케이션 영역으로 넘어 갈 때 사용
 *  @param	None
 *  @retval	None
 */
void jump_to_application(void)
{
	typedef void (*fptr)(void);
	uint32_t JumpAddress;
	fptr Jump_To_Application;

	JumpAddress = *(__IO uint32_t*) (APPLICATION_ADDRESS + 4);
	Jump_To_Application = (fptr) JumpAddress;

	SysTick->CTRL = 0;
	SysTick->LOAD = 0;
	SysTick->VAL = 0;

	HAL_RCC_DeInit();
	HAL_DeInit();

	__set_MSP(*(__IO uint32_t*) APPLICATION_ADDRESS);
	SCB->VTOR = APPLICATION_ADDRESS;
	Jump_To_Application();
}

/*
 *  @brief	부팅 설정 파일 parsing하여 필요 정보 추출하는 함수
 *  @param	filedata	부팅 파일 원본 데이터
 *			filesize	부팅 파일 크기
 *  @retval	struct BOOTCONFIG 형태로 반환. 자동부팅, 부팅로고 사용 할 지 여부
 */
struct BOOTCONFIG parsingConfigFile(char* filedata, int filesize)
{
	//설정 파일 구조 및 문법
    // 현재는 설정 항목이나 데이터는 아스키 코드 중 문자, 특수문자 , 숫자 같이 사람이 읽을 수 있는 정보만 받고 있으며
    // 데이터는 숫자인 경우 정수값만 받을 수 있음
    // <설정항목>=<설정데이터>;

	// 부팅 설정 구조체
    struct BOOTCONFIG res;
    res.autoboot = 0;
    res.bootlog = 0;

    // 범용 변수
    int x, y;

    // 현재 읽고 있는 설정 항목의 시작 위치
    int starindex = 0;
    // 현재 읽고 있는 설정 항목의 마지막 위치
    int index_last = 0;

    // 현재 읽은 설정 항목 임지 저장 변수
    char indexbuffer[128] = "";
    // 현재 읽은 설정 데이터 임지 저장 변수
    char databuffer[128] = "";

    // 부팅 설정 데이터 전부 검색
    for (x = 0; x < filesize; x++){
    	// 한 문장을 나타내는 세미콜론 발견시
        if (filedata[x]==';'){
            // 설정 항목과 데이터 구분 문다 '=' 발견하면  설정항목 마지막 인덱스 설정
            for (y = starindex; y < x; y++){
                if (filedata[y] == '=') {
                    index_last = y-1;
                    break;
                }
            }

            //임시 변수에 항목과 데이터 문자열 복사
            // index
            memset(indexbuffer,0x00,sizeof(indexbuffer));
            memcpy(indexbuffer, filedata+ starindex, sizeof(char)*(index_last - starindex + 1) );
            // data
            memset(databuffer, 0x00, sizeof(databuffer));
            memcpy(databuffer, filedata + (index_last+2), sizeof(char)*(x-(index_last+2)));

            //------------------------------------------------------------------------------------------------------------

            // 필요시 설정값 추가
            // auto booting 여부
            if (fCompareFunction(indexbuffer,"auto_boot",9)){
                res.autoboot = fConvertStringToInt32(databuffer);
            }
            // 부팅로고 사용 여부
            if (fCompareFunction(indexbuffer, "booting_logo", 12)){
                res.bootlog = fConvertStringToInt32(databuffer);
            }

            //------------------------------------------------------------------------------------------------------------

            // get next start -> 다음 항목 찾기 위해 문자나 숫자인 부분을 찾을때 까지 검색
            for (y = x + 1 ;y < filesize; y++){
                if ((filedata[y] >= 0x21) && (filedata[y] <= 0x7E)){
                    starindex = y;
                    break;
                }
            }
        }
    }

    return res;
}

/*
 *  @brief	SD card에서 비트맵 이미지 파일 읽어서 SDRAM에 저장 하는 함수
 *  @param	fileaddress	파일 주소. "/<폴더>/<폴더> ... <폴더>/<파일명>"
 *  @retval	struct READ_IMAGE_INFO 형태로 반환.
 */
struct READ_IMAGE_INFO readBitmapFileToSdram(char* fileaddress)
{
	// SDRAM에 저장 한 이미지 정보 구조체
	struct READ_IMAGE_INFO res;
	res.image_sdram_address =  (uint32_t)image_sdram_pointer;
	res.image_x_size = 0;
	res.image_y_size = 0;
	res.image_x_start_location = 0;
	res.image_y_start_location = 0;

	// 비트맵 헤더 구조체
	struct BMP_HEADER_STRUCT stBitmapHeader;

	// 각종 변수
	int x,y,z;

	// Bitmap 읽을 때 반전으로 읽기 위한 변수
	int rev_y = 0;

	// 픽셀 1개 데이터
	unsigned char pixeldata[3] = "";
	// RGB 565 format 1픽셀 저장 할 변수
	unsigned short pixel_rgb565 = 0;

	//sdram address index
	static uint32_t now_sdram_index = 0;

	//read buffer
	// 1536 >> Pixel당 3바이트, 4바이트 단위 , 128단위
	char file_read_buffer[BITMAP_READ_DATA_SIZE] = "";

	//file pointer
	int read_file_pointer = 0;

	// get loop count
	int count = 0;
	int imagewide = 0;

	// read bitmap header
	char sBitmapHeaderBuffer[54] = "";
	read_file_pointer = 54;

	//read bitmap header
	fatfs_read_index_readsize(fileaddress,sBitmapHeaderBuffer,0,read_file_pointer);

	//get File Size 4Byte
	stBitmapHeader.iBitmapFileSize = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 2);
	stBitmapHeader.iButmapRealDataOffsetValue = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 10);
	stBitmapHeader.iBackendHeaderSize = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 14);
	stBitmapHeader.iBitmapXvalue = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 18);
	stBitmapHeader.iBitmapYvalue = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 22);
	stBitmapHeader.iColorPlane = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(short), 26);
	stBitmapHeader.iPixelBitDeepValue = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(short), 28);
	stBitmapHeader.iCompressType = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 30);
	stBitmapHeader.iImageSize = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 34);
	stBitmapHeader.iPixcelPerMeterXvalue = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 38);
	stBitmapHeader.iPixcelPerMeterYvalue = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 42);
	stBitmapHeader.iColorPalet = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 46);
	stBitmapHeader.iCriticalColorCount = getBMPinfoFuncBinToInteger(sBitmapHeaderBuffer, sizeof(int), 50);
#if 0
    printf("\n----------------------------------------------\n");
    printf("[Log] iColorPalet           = %d\n", stBitmapHeader.iColorPalet);
    printf("[Log] iCriticalColorCount   = %d\n", stBitmapHeader.iCriticalColorCount);
    printf("[Log] iPixcelPerMeterYvalue = %d\n", stBitmapHeader.iPixcelPerMeterYvalue);
    printf("[Log] iPixcelPerMeterXvalue = %d\n", stBitmapHeader.iPixcelPerMeterXvalue);
    printf("[Log] iImageSize            = %d\n", stBitmapHeader.iImageSize);
    printf("[Log] iCompressType         = %d\n", stBitmapHeader.iCompressType);
    printf("[Log] iColorPalet           = %d\n", stBitmapHeader.iColorPalet);
    printf("[Log] iColorPlane           = %d\n", stBitmapHeader.iColorPlane);
    printf("[Log] iPixelBitDeepValue    = %d\n", stBitmapHeader.iPixelBitDeepValue);
    printf("[Log] iBitmapYvalue         = %d\n", stBitmapHeader.iBitmapYvalue);
    printf("[Log] iBitmapXvalue         = %d\n", stBitmapHeader.iBitmapXvalue);
    printf("[Log] iBackendHeaderSize    = %d\n", stBitmapHeader.iBackendHeaderSize);
    printf("[Log] iButmapDataOffsetsize = %d\n", stBitmapHeader.iButmapRealDataOffsetValue);
    printf("[Log] iBitmapFileSize       = %d\n", stBitmapHeader.iBitmapFileSize);
    printf("----------------------------------------------\n");
#endif

    // 이미지 세로 길이
    count = stBitmapHeader.iBitmapYvalue;
    // 이미지 가로 길이 (픽셀 * 컬러 값)
    imagewide = stBitmapHeader.iBitmapXvalue*3;
    // 이미지 가로 크기
	res.image_x_size = stBitmapHeader.iBitmapXvalue;
	// 이미지 세로 크기
	res.image_y_size = stBitmapHeader.iBitmapYvalue;

	// SDRAM에 저장 할 인덱스 변수
	now_sdram_index = 0;

	// 데이터 SD card에서 읽고 SDRAM에 저장
    for(y = 0 ; y < count ; y++){
    	// x사이즈 만큼 읽고, y반전순으로 읽고 -> 비트맵은 y 좌표가 반전 순으로 저장 되어 있다
    	rev_y = stBitmapHeader.iBitmapYvalue - y - 1;
    	// SD card에서 읽어 들일 버퍼 초기화
    	memset(file_read_buffer,0x00,sizeof(file_read_buffer));
    	// SD Card에서 읽어들일 파일 인덱스 설정
    	read_file_pointer = rev_y * (imagewide) + 54;

    	// SD card에서 이미지 데이터 불러오기
    	fatfs_read_index_readsize( fileaddress, file_read_buffer, read_file_pointer, sizeof(char)*(imagewide) );

    	// start sdram address printf
    	if(y == 0) {
    		printf("[readBitmapFileToSdram]image sdram pointer=%x\n",image_sdram_pointer);
    	}

    	for(x=0;x<sizeof(char)*(imagewide);x=x+3){
    		// RGB565 type으로 변환
    		pixeldata[0] = file_read_buffer[x];
    		pixeldata[1] = file_read_buffer[x+1];
    		pixeldata[2] = file_read_buffer[x+2];
    		pixel_rgb565 = fArrayConvertToUINT16(pixeldata);

    		// SDRAM에 저장
    		setpixel(pixel_rgb565, (uint32_t)image_sdram_pointer + now_sdram_index + 0);

    		// 인덱스 증가
    		now_sdram_index = now_sdram_index + 2;
    	}
    }

    // 현재 읽은 이미지 파일 크기 만큼 SDRAM의 이미지 저장 포인터 갱신
    image_sdram_pointer = (uint32_t)image_sdram_pointer + (res.image_x_size*res.image_y_size)*2;
    printf("[readBitmapFileToSdram]image sdram next pointer=%x\n",image_sdram_pointer);

    return res;
}

/*
 *  @brief	타이머 인터럽트에서 동작 할 touch function
 *  @param	None
 *  @retval	None
 */
void callfunction_for_timer(void)
{
	static int old_x=-1,old_y=-1;
	get_touch_location(ts_location);
	if((ts_location[0] != old_x) || (ts_location[1] != old_y)){
		ts_location[2] = 1;
	}
	old_x = ts_location[0];
	old_y = ts_location[1];
}

/*
 *  @brief	UART로 부터 파일을 SD card에 받고 STM32 Flash application 영역에 쓰기 하는 함수.
 *  @param	None
 *  @retval	None
 */
struct CURRENT_PROGRESS firmwareUpdate(void)
{
	// 파일 시스템 접근 결과 구조체
	struct FILE_WRITE_RES write_res;
	write_res.last_error_code = 0;
	write_res.last_error_function = 0;
	write_res.will_write_count = 0;

	// 현재 진척도 결과
	struct CURRENT_PROGRESS current_res;
	current_res.flashwrite = 0;
	current_res.download = 0;

	// file object pointer
	FIL filedownload;

	// 범용 변수
	int x = 0, y = 0;
	float ftmp = 0.0f;

	// HAL driver function 결과 받을 변수
	int res = 0;
	// uart 다운로드 할 때 사용 하는 임시 변수, 주로 패킷을 분리할 때 사용
	int tmpIndex = 0;

	// uart 수신 버퍼 parsing count 변수
	int parsing_count = 0;

	static int ifilesize = 0;
	static int flash_write_lastSize = 0;

	// STM32 Flash Memory 쓰기 횟수 Counter
	static int flash_write_count = 0;

	int write_data_size = 0;

	if(updateMode == uart_communication){
		// 패킷 끝부분 문자인 '\n'를 검색, 
		// 인덱스를 0부터 검색할 때 나중에 본 데이터 패킷에 있는 데이터를 마지막 부분으로 간주 할 수도 있어 내림 차순으로 검색한다
		for( x=(PACKET_MAX_SIZE-1); x>=0 ; x--){
			// x = 마지막 부분 위치
			// 호스트로 부터 다운 받을 데이터 정보 패킷 저리하는 부분,
			// HEAD 패킷은 file name , packet size , packet count , last packet data length , file size 로 구성되어 있다.
			if((fCompareFunction(incomingdata,"HEAD:",5))&&(incomingdata[x]=='\n')){
				// ','단위 Parsing Count 변수 초기화
				parsing_count = 0;
				// 패킷 본 데이터 시작 지점 
				tmpIndex = 5;
				// 본 데이터 구간에 있을 ','로 데이터 분리
				for(y = 6 ; y < x; y++){
					// 현재 검색한 데이터가 ','이고 마지막 parsing 데이터가 아닌 경우 처리
					if((incomingdata[y]==',') && (parsing_count < 4)){
						// 임시 변수 초기화
						memset(tmp,0x00,sizeof(tmp));
						// 임시 변수에 파싱한 문자 복사
						memcpy(tmp,incomingdata+tmpIndex, sizeof(char)*(y-tmpIndex));
						// 다음 파싱 문자의 시작 부분으로 임시 인덱스 설정
						tmpIndex = y+1;
						// 파싱 카운트가 0인 경우 -> 최초 파싱 할 문자열은 File name
						if(parsing_count == 0){
							//file_name
							memset(file_name,0x00,sizeof(file_name));
							sprintf(file_name,"%s",tmp);
						}
						// 파싱 카운트가 1인 경우 -> 2번째 파싱 데이터는 정수 packet size
						else if(parsing_count == 1){
							//paket_size
							paket_size=fConvertStringToInt32(tmp);
						}
						// 파싱 카운트가 2인 경우 -> 3번째 파싱 데이터는 정수 packet count
						else if(parsing_count == 2){
							//paket_size
							paket_count=fConvertStringToInt32(tmp);
						}
						// 파싱 카운트가 3인 경우 -> 4번째 파싱 데이터는 정수 마지막 packet size
						else if(parsing_count == 3){
							//last_paket_size
							last_paket_size=fConvertStringToInt32(tmp);
							if(last_paket_size == 0){
								last_paket_size = paket_size;
							}
						}
						// 파싱 반복 할때 마다 counter 올려 준다
						parsing_count++;
					}
					// 마지막 파싱 데이터는 File 실제 크기
					else if(parsing_count == 4){
						memset(tmp,0x00,sizeof(tmp));
						memcpy(tmp,incomingdata+tmpIndex, sizeof(char)*(y-tmpIndex+1));
						// file size
						ifilesize =fConvertStringToInt32(tmp);
					}
				}
				// reset uart rx dma
				// uart 수신은 DMA로 이루어지므로 위 데이터 패킷 처리가 끝나면 수신 버퍼 초기화, DMA 초기화를 진행 한다.
				memset(incomingdata,0x00,sizeof(incomingdata));
				HAL_UART_DMAStop(&huart1);
				HAL_UART_Receive_DMA(&huart1, incomingdata, sizeof(incomingdata));
				//일단 사용 x
				//fwupdateflow=1;
				// HEAD정보의 file name을 이용하여 bin data에 저장 한다.
				memset(file_address,0x00,sizeof(file_address));
				sprintf(file_address,"/bootloader/bin/%s",file_name);
				// 파일 존재시 삭제
				write_res.last_error_code = f_open(&filedownload, file_address, FA_CREATE_ALWAYS | FA_WRITE);
				write_res.last_error_function = 1;
				// 성공 여부 전송
				if(write_res.last_error_code != 0){
					// 에러 시
					printf("NH\n");
				}
				else{
					// 성공 시
					printf("AH\n");
				}
			}
			// DATA 패킷 처리 
			if(fCompareFunction(incomingdata,"DATA:",5)) {
				// 마지막 데이터 패킷일때 (전송 받는 파일 크기에 따라 가변 될 수 있음)
				if((datapaketIndex == (paket_count-1)) && (incomingdata[last_paket_size+6]=='\n')) {
					// 헤더 부분 제외한 시작 부분 인덱스
					tmpIndex = 5;
					// 임시 변수 초기화
					memset(tmp,0x00,sizeof(tmp));
					// 임시 변수에 패킷 헤더 부분 제외한 영역 복사
					memcpy(tmp,incomingdata+tmpIndex, sizeof(char)*(last_paket_size));
					// 패킷 오류 검사를 위한 XOR Checksum 실시
					check_sum = xor_checksum(tmp,last_paket_size);
					// 체크섬 검사가 전달 된 체크섬 데이터와 일치하면 처리
					if(check_sum == incomingdata[last_paket_size+5]) {
						//파일 쓰기
						write_res.will_write_count = sizeof(tmp);
						write_res.last_error_code = f_write(&filedownload, tmp, (UINT)last_paket_size, (UINT*)&write_data_size);
						write_res.last_error_function = 2;
						write_res.last_write_count = write_data_size;
						// reset uart rx dma
						// uart 수신은 DMA로 이루어지므로 위 데이터 패킷 처리가 끝나면 수신 버퍼 초기화, DMA 초기화를 진행 한다.
						memset(incomingdata,0x00,sizeof(incomingdata));
						HAL_UART_DMAStop(&huart1);
						HAL_UART_Receive_DMA(&huart1, incomingdata, sizeof(incomingdata));
						// 만약 파일 쓰기에 문제가 발생하는 경우 처리
						if(write_res.last_error_code != 0) {
							// 0.1초 delay
							HAL_Delay(100);
							// 파일 오브젝트 닫기
							f_close(&filedownload);
							// 0.1초 delay
							HAL_Delay(100);
							// 파일 마지막 부분 부터 쓰기로 open
							write_res.last_error_code = f_open(&filedownload, file_address, FA_OPEN_APPEND | FA_WRITE);
							write_res.last_error_function = 10;
							// Error Count +1 -> 디버깅 용도
							writeerrorcount++;
							// 에러 전달
							printf("ND\n");
						}
						else {
							// 다음 패킷을 위한 인덱스 증가
							datapaketIndex++;
							// 수신 양호
							printf("AD\n");
						}
					}
					// 체크섬 결과가 상이한 경우 에러 전달
					else{
						printf("ND\n");
					}
				}
				// 처음 부터 마지막 전 패킷 까지 일때 (packet 사이즈 full)
				else if((datapaketIndex < (paket_count-1)) && (incomingdata[paket_size+6]=='\n')){
					// 헤더 부분 제외한 시작 부분 인덱스
					tmpIndex = 5;
					// 임시 변수 초기화
					memset(tmp,0x00,sizeof(tmp));
					// 임시 변수에 패킷 헤더 부분 제외한 영역 복사
					memcpy(tmp,incomingdata+tmpIndex, sizeof(char)*(paket_size));
					// 패킷 오류 검사를 위한 XOR Checksum 실시
					check_sum = xor_checksum(tmp,paket_size);
					// 체크섬 검사가 전달 된 체크섬 데이터와 일치하면 처리
					if(check_sum == incomingdata[paket_size+5]){
						//파일 쓰기
						write_res.will_write_count = sizeof(tmp);
						write_res.last_error_code = f_write(&filedownload, tmp,  (UINT)paket_size, (UINT*)&write_data_size);
						write_res.last_error_function = 2;
						write_res.last_write_count = write_data_size;
						// reset uart rx dma
						// uart 수신은 DMA로 이루어지므로 위 데이터 패킷 처리가 끝나면 수신 버퍼 초기화, DMA 초기화를 진행 한다.
						memset(incomingdata,0x00,sizeof(incomingdata));
						HAL_UART_DMAStop(&huart1);
						HAL_UART_Receive_DMA(&huart1, incomingdata, sizeof(incomingdata));
						// 만약 파일 쓰기에 문제가 발생하는 경우 처리
						if(write_res.last_error_code != 0) {
							// 0.1초 delay
							HAL_Delay(100);
							// 파일 오브젝트 닫기
							f_close(&filedownload);
							// 0.1초 delay
							HAL_Delay(100);
							// 파일 마지막 부분 부터 쓰기로 open
							write_res.last_error_code = f_open(&filedownload, file_address, FA_OPEN_APPEND | FA_WRITE);
							write_res.last_error_function = 10;
							// Error Count +1 -> 디버깅 용도
							writeerrorcount++;
							// 에러 전달
							printf("ND\n");
						}
						else {
							// 다음 패킷을 위한 인덱스 증가
							datapaketIndex++;
							// 수신 양호
							printf("AD\n");
						}
					}
					// 체크섬 결과가 상이한 경우 에러 전달
					else {
						printf("ND\n");
					}
				}
			}
			// 파일 전송 종료
			if(fCompareFunction(incomingdata,"END",3)){
				HAL_Delay(100);
				// fclose
				write_res.last_error_code = f_close(&filedownload);
				write_res.last_error_function = 4;
				// 실패시
				if(write_res.last_error_code != 0){
					printf("NE\n");
				}
				// 성공시
				else{
					updateMode = flash_write_preparation;
					printf("AE\n");
					break;
				}
			}
		}
	}

	//------------------------------------------------------------------------------------------------------------------------------------------------------------------------
	// STM32 Flash Memory에 쓰기 하는 부분
	
	else if(updateMode == flash_write_preparation){
		//erase flash
		printf("app_Partition_erase start\n");
		app_Partition_erase();
		printf("app_Partition_erase end\n");
		// flash write count init
		flash_write_count = 0;
		// Flash memory에 기록 시작 할 주소
		flash_write_start_address = APPLICATION_ADDRESS;
		// Flash Memory 쓰기 반복 횟수
		write_flash_count = ifilesize / WRITE_FLASH_SIZE;
		// 설정 한 최대 블록 보다 작은 크기의 횟수가 존재 하면 +1
		if((ifilesize % WRITE_FLASH_SIZE )>0){
			flash_write_lastSize = (ifilesize % WRITE_FLASH_SIZE );
			write_flash_count++;
		}
		// next stage
		updateMode = flash_write_process;
	}
	else if(updateMode == flash_write_process){
		// 현재 쓰기 회차가 목표치보다 적으면 동작
		if(flash_write_count < write_flash_count){
			// Flash Memory 기록 할 배열 변수 초기화 -> flash memory 특성 상 빈 공간은 0xFF로 놔둠
			memset(flash_write_buffer,0xFF,sizeof(flash_write_buffer));
			// 다운로드 받은 파일 읽기
			// 읽을 때 한번에 읽는 게 아닌 특정 블록 단위로 읽어서 성공 하면 다음 인덱스로 넘어가고 실패하면 다시 읽어서 기록하는 방식으로 진행
			res = fatfs_read_index_readsize(file_address, flash_write_buffer, (int)( flash_write_count * WRITE_FLASH_SIZE ), WRITE_FLASH_SIZE);
			// 파일 읽기 성공 하면 동작
			if(res != -1){
				// Flash 영역에 쓸 때 마지막 회차 여부 확인
				if(flash_write_count == (write_flash_count - 1)){
					// 마지막 회차가 설정한 블록 크기와 다른 경우
					if(flash_write_lastSize != 0){
						flash_write_end_address = flash_write_start_address + flash_write_lastSize;
					}
					// 마지막 회차가 설정한 블록 크기와 같을 경우
					else{
						flash_write_end_address = flash_write_start_address + WRITE_FLASH_SIZE;
					}
				}
				// Flash Memory 쓰는 회차가 마지막이 아닌 경우
				else{
					flash_write_end_address = flash_write_start_address + WRITE_FLASH_SIZE;
				}
				// Flash Memory 쓰기
				flash_wrtie(flash_write_start_address, flash_write_end_address, flash_write_buffer);
				// 다음 Flash Memory 주소는 현 회차의 마지막 주소
				flash_write_start_address = flash_write_end_address;
				// Flash Memory 쓰기 Counter 증가
				flash_write_count++;
			}
		}
		// 쓰기 완료 
		else{
			updateMode = flash_write_completion;
		}
	}
	else if(updateMode == flash_write_completion){
		//NVIC_SystemReset();
		// 펌웨어 업데이트 완료 알림
		printf("FW update OK\n");
		// 부트로더 메인 flow를 idel로 -> update 이후 바로 부팅 해도 좋음
		bootloader_working_number = booting_now;
	}

	// 펌웨어 업데이트 진척도 백분위 계산
	// 다운로드 현황
	if (updateMode >= flash_write_preparation) {
		current_res.download = 100;
	}
	else {
		ftmp = ((float)datapaketIndex / (float)paket_count) * 100.0f;
		current_res.download = (int)ftmp;
	}
	// Flash Memory 쓰기 현황
	if (updateMode >= flash_write_completion) {
		current_res.flashwrite = 100;
	}
	else {
		ftmp = ((float)flash_write_count / (float)write_flash_count) * 100.0f;
		current_res.flashwrite = (int)ftmp;
	}

	return current_res;
}

/*
 *  @brief	Test Function
 *  @param	None
 *  @retval	None
 */
void test(void)
{
#if 0
	FIL fp;

	int res = 0;
	int write_data_size = 0;

	res = f_open(&fp, "/test.txt", FA_CREATE_ALWAYS | FA_WRITE);
	printf("f_open = %d\r\n",res);

	res = f_write(&fp, "0123456789", sizeof(char)*10, (UINT*)&write_data_size);
	printf("f_write = %d\r\n",res);

	res = f_write(&fp, "0123456789", sizeof(char)*10, (UINT*)&write_data_size);
	printf("f_write = %d\r\n",res);

	res = f_write(&fp, "0123456789", sizeof(char)*10, (UINT*)&write_data_size);
	printf("f_write = %d\r\n",res);

	res = f_write(&fp, "0123456789", sizeof(char)*10, (UINT*)&write_data_size);
	printf("f_write = %d\r\n",res);

	res = f_close(&fp);
	printf("f_close = %d\r\n",res);
#endif
}


