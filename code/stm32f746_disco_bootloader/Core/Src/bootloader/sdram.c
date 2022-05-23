/*
 * sdram.c
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

/*
 *	SDRAM : IS42S32400F-6BL
 */

/*
 *  @brief	SDRAM Initialization sequence Function
 *  @param	RefreshCount refresh rate counter
 *  @retval	None
 */
void BSP_SDRAM_Initialization_sequence(uint32_t RefreshCount)
{
	//SDRAM
	static FMC_SDRAM_CommandTypeDef Command;

	__IO uint32_t tmpmrd = 0;

	/* Step 1: Configure a clock configuration enable command */
	Command.CommandMode            = FMC_SDRAM_CMD_CLK_ENABLE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

	/* Step 2: Insert 100 us minimum delay */
	/* Inserted delay is equal to 1 ms due to systick time base unit (ms) */
	HAL_Delay(1);

	/* Step 3: Configure a PALL (precharge all) command */
	Command.CommandMode            = FMC_SDRAM_CMD_PALL;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

	/* Step 4: Configure an Auto Refresh command */
	Command.CommandMode            = FMC_SDRAM_CMD_AUTOREFRESH_MODE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 4;//8
	Command.ModeRegisterDefinition = 0;

	/* Send the command */
	HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

	/* Step 5: Program the external memory mode register */ //SDRAM_MODEREG_CAS_LATENCY_2
	tmpmrd = (uint32_t)SDRAM_MODEREG_BURST_LENGTH_1          |\
                     SDRAM_MODEREG_BURST_TYPE_SEQUENTIAL   |\
                     SDRAM_MODEREG_CAS_LATENCY_3           |\
                     SDRAM_MODEREG_OPERATING_MODE_STANDARD |\
                     SDRAM_MODEREG_WRITEBURST_MODE_SINGLE;

	Command.CommandMode            = FMC_SDRAM_CMD_LOAD_MODE;
	Command.CommandTarget          = FMC_SDRAM_CMD_TARGET_BANK1;
	Command.AutoRefreshNumber      = 1;
	Command.ModeRegisterDefinition = tmpmrd;

	/* Send the command */
	HAL_SDRAM_SendCommand(&hsdram1, &Command, SDRAM_TIMEOUT);

	/* Step 6: Set the refresh rate counter */
	/* Set the device refresh rate */
	HAL_SDRAM_ProgramRefreshRate(&hsdram1, RefreshCount);
}

/*
 *  @brief	SDRAM에 0x00을 얼마 만큼 쓸지(초기화) 함수
 *  @param	block_size 0x00으로 초기화 할 블럭 개수.
 *  @retval	None
 */
void init_sdram(int block_size){
	int x = 0;
	for(x = 0 ; x < block_size *2 ; x++)
	{
		*((uint16_t*)SDRAMADDRESS_FRAMEBUFFER + 0 + x) = 0x0000;
	}
}

/*
 *  @brief	SDRAM 특정 주소 탐색
 *  @param	startaddress 읽어볼 시작 주소
 *			size	읽어볼 크기
 *  @retval	None
 */
void lookSDRAM(int startaddress,int size)
{
	int x = 0;
	short i,j = 0;

	//SDRAM pointer
	uint16_t* frame_buffer = (uint16_t*) startaddress;

	for(x = 0 ;x < size ;x=x+1)
	{
		i = *((uint16_t*)startaddress + x);
		printf("[%d]0x%8x: 0x%4x\n",x,frame_buffer+x, i);
	}
}

/*
 *  @brief	SDRAM 특정 주소 탐색
 *	@param	source_address 복사 할 주소
 *	 		target_address 붙여넣기 할 주소
 *	 		block_size 크기
 *  @retval	None
 */
void copymemory(uint32_t source_address,uint32_t target_address, int block_size)
{
	for(int x = 0 ; x < block_size; x++)
	{
		*((uint16_t*)target_address + x + SDRAM_OFFSET) = *((uint16_t*)source_address + x);
	}
}


