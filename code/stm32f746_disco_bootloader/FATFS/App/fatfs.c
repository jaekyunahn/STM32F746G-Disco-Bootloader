/**
  ******************************************************************************
  * @file   fatfs.c
  * @brief  Code for fatfs applications
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */

#include "fatfs.h"

uint8_t retSD;    /* Return value for SD */
char SDPath[4];   /* SD logical drive path */
FATFS SDFatFS;    /* File system object for SD logical drive */
FIL SDFile;       /* File object for SD */

/* USER CODE BEGIN Variables */
#include "main.h"
#include "bootloader/bootloader.h"

/* USER CODE END Variables */

void MX_FATFS_Init(void)
{
  /*## FatFS: Link the SD driver ###########################*/
  retSD = FATFS_LinkDriver(&SD_Driver, SDPath);

  /* USER CODE BEGIN Init */
  /* additional user code for init */
  /* USER CODE END Init */
}

/**
  * @brief  Gets Time from RTC
  * @param  None
  * @retval Time in DWORD
  */
DWORD get_fattime(void)
{
  /* USER CODE BEGIN get_fattime */
  return 0;
  /* USER CODE END get_fattime */
}

/* USER CODE BEGIN Application */

void init_sd(void)
{
	retSD = FATFS_LinkDriver(&SD_Driver, SDPath);
	printf("retSD=%d\r\n",retSD);
	FRESULT res;
	res = disk_initialize((BYTE) 0);
	printf("disk_initialize=%d\r\n",res);
	res = f_mount(&SDFatFS, (TCHAR const*)SDPath, 0);
	printf("f_mount=%d\r\n",res);
}

int file_size(char *file_address)
{
	FIL MyFile;
	int res = 0;
	res = f_open(&MyFile, file_address, FA_READ);
	if(res != 0)
	{
		return 0;
	}
	res = f_size(&MyFile);
	f_close(&MyFile);
	return res;
}

int fatfs_read_index_readsize(char *file_address ,char *read_data, int iStartPoint, int read_size)
{
	int res = 0;
	int read_data_size = 0;

	FIL fp;

	f_open(&fp, file_address, FA_READ);
	if((res >= 1)&&(res <= 19))
	{
		printf("[fatfs_read_startaddress]f_open error :%d \r\n",res);
		return -1;
	}

	res = f_lseek(&fp, iStartPoint );
	if((res >= 1)&&(res <= 19))
	{
		printf("[fatfs_read_startaddress]f_lseek error :%d \r\n",res);
		return -1;
	}

	res = f_read(&fp, read_data, sizeof(char)*read_size, (UINT*)&read_data_size);
	if((res >= 1)&&(res <= 19))
	{
		printf("[fatfs_read_startaddress]f_read error :%d \r\n",res);
		return -1;
	}
	res = f_close(&fp);
	if((res >= 1)&&(res <= 19))
	{
		printf("[fatfs_read_startaddress]f_close error :%d \r\n",res);
		return -1;
	}

	return read_data_size;
}

int deletfile(char *file_address)
{
	return f_unlink(file_address);
}

int renamefile(char *file_address, char *rename)
{

}

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
