/*
 * touchscreen.c
 *
 *  Created on: May 9, 2022
 *      Author: AhnJaeKyun
 */

#include "main.h"
#include "i2c.h"
#include "bootloader/bootloader.h"
#include "bootloader/ft5336.h"

//touch
#define TS_I2C_ADDRESS					((uint16_t)0x70)
#define TS_MULTI_TOUCH_SUPPORTED		1

#define TS_MAX_NB_TOUCH					((uint32_t) FT5336_MAX_DETECTABLE_TOUCH)

#define TS_NO_IRQ_PENDING				((uint8_t) 0)
#define TS_IRQ_PENDING					((uint8_t) 1)

#define TS_SWAP_NONE					((uint8_t) 0x01)
#define TS_SWAP_X						((uint8_t) 0x02)
#define TS_SWAP_Y						((uint8_t) 0x04)
#define TS_SWAP_XY						((uint8_t) 0x08)

typedef struct
{
  uint8_t  touchDetected;                /*!< Total number of active touches detected at last scan */
  uint16_t touchX[TS_MAX_NB_TOUCH];      /*!< Touch X[0], X[1] coordinates on 12 bits */
  uint16_t touchY[TS_MAX_NB_TOUCH];      /*!< Touch Y[0], Y[1] coordinates on 12 bits */

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
  uint8_t  touchWeight[TS_MAX_NB_TOUCH]; /*!< Touch_Weight[0], Touch_Weight[1] : weight property of touches */
  uint8_t  touchEventId[TS_MAX_NB_TOUCH];     /*!< Touch_EventId[0], Touch_EventId[1] : take value of type @ref TS_TouchEventTypeDef */
  uint8_t  touchArea[TS_MAX_NB_TOUCH];   /*!< Touch_Area[0], Touch_Area[1] : touch area of each touch */
  uint32_t gestureId; /*!< type of gesture detected : take value of type @ref TS_GestureIdTypeDef */
#endif  /* TS_MULTI_TOUCH_SUPPORTED == 1 */

} TS_StateTypeDef;

typedef enum
{
  TS_OK                = 0x00, /*!< Touch Ok */
  TS_ERROR             = 0x01, /*!< Touch Error */
  TS_TIMEOUT           = 0x02, /*!< Touch Timeout */
  TS_DEVICE_NOT_FOUND  = 0x03  /*!< Touchscreen device not found */
}TS_StatusTypeDef;

/**
 *  @brief TS_GestureIdTypeDef
 *  Define Possible managed gesture identification values returned by touch screen
 *  driver.
 */
typedef enum
{
  GEST_ID_NO_GESTURE = 0x00, /*!< Gesture not defined / recognized */
  GEST_ID_MOVE_UP    = 0x01, /*!< Gesture Move Up */
  GEST_ID_MOVE_RIGHT = 0x02, /*!< Gesture Move Right */
  GEST_ID_MOVE_DOWN  = 0x03, /*!< Gesture Move Down */
  GEST_ID_MOVE_LEFT  = 0x04, /*!< Gesture Move Left */
  GEST_ID_ZOOM_IN    = 0x05, /*!< Gesture Zoom In */
  GEST_ID_ZOOM_OUT   = 0x06, /*!< Gesture Zoom Out */
  GEST_ID_NB_MAX     = 0x07  /*!< max number of gesture id */

} TS_GestureIdTypeDef;

typedef enum
{
  TOUCH_EVENT_NO_EVT        = 0x00, /*!< Touch Event : undetermined */
  TOUCH_EVENT_PRESS_DOWN    = 0x01, /*!< Touch Event Press Down */
  TOUCH_EVENT_LIFT_UP       = 0x02, /*!< Touch Event Lift Up */
  TOUCH_EVENT_CONTACT       = 0x03, /*!< Touch Event Contact */
  TOUCH_EVENT_NB_MAX        = 0x04  /*!< max number of touch events kind */

} TS_TouchEventTypeDef;

static TS_DrvTypeDef *tsDriver;
static uint16_t tsXBoundary, tsYBoundary;
static uint8_t  tsOrientation;
static uint8_t  I2cAddress;

static TS_StateTypeDef  TS_State;

int init_touchscreen = TS_ERROR;

//---------------------------------------------------------------------------------------------------------

uint8_t BSP_TS_Init(uint16_t ts_SizeX, uint16_t ts_SizeY);

void init_touch_screen(void)
{
	uint8_t status = 0;
	status = BSP_TS_Init(BSP_LCD_GetXSize(), BSP_LCD_GetYSize());
	if (status != TS_OK)
	{
		printf("[init error]Touchscreen cannot be initialized\r\n");
	}
	init_touchscreen = status;
}

/**
  * @brief  Initializes and configures the touch screen functionalities and
  *         configures all necessary hardware resources (GPIOs, I2C, clocks..).
  * @param  ts_SizeX: Maximum X size of the TS area on LCD
  * @param  ts_SizeY: Maximum Y size of the TS area on LCD
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_Init(uint16_t ts_SizeX, uint16_t ts_SizeY)
{
	uint8_t status = TS_OK;
	tsXBoundary = ts_SizeX;
	tsYBoundary = ts_SizeY;

	/* Read ID and verify if the touch screen driver is ready */
	ft5336_ts_drv.Init(TS_I2C_ADDRESS);
	if(ft5336_ts_drv.ReadID(TS_I2C_ADDRESS) == FT5336_ID_VALUE)
	{
		/* Initialize the TS driver structure */
		tsDriver = &ft5336_ts_drv;
		I2cAddress = TS_I2C_ADDRESS;
		tsOrientation = TS_SWAP_XY;

		/* Initialize the TS driver */
		tsDriver->Start(I2cAddress);
 	}
	else
	{
		status = TS_DEVICE_NOT_FOUND;
	}

	return status;
}

/**
  * @brief  Update gesture Id following a touch detected.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_Get_GestureId(TS_StateTypeDef *TS_State)
{
	uint32_t gestureId = 0;
	uint8_t  ts_status = TS_OK;

	/* Get gesture Id */
	ft5336_TS_GetGestureID(I2cAddress, &gestureId);

	/* Remap gesture Id to a TS_GestureIdTypeDef value */
	switch(gestureId)
	{
    case FT5336_GEST_ID_NO_GESTURE :
    	TS_State->gestureId = GEST_ID_NO_GESTURE;
    	break;
    case FT5336_GEST_ID_MOVE_UP :
    	TS_State->gestureId = GEST_ID_MOVE_UP;
    	break;
    case FT5336_GEST_ID_MOVE_RIGHT :
    	TS_State->gestureId = GEST_ID_MOVE_RIGHT;
    	break;
    case FT5336_GEST_ID_MOVE_DOWN :
    	TS_State->gestureId = GEST_ID_MOVE_DOWN;
    	break;
    case FT5336_GEST_ID_MOVE_LEFT :
    	TS_State->gestureId = GEST_ID_MOVE_LEFT;
    	break;
    case FT5336_GEST_ID_ZOOM_IN :
    	TS_State->gestureId = GEST_ID_ZOOM_IN;
    	break;
    case FT5336_GEST_ID_ZOOM_OUT :
    	TS_State->gestureId = GEST_ID_ZOOM_OUT;
    	break;
    default :
    	ts_status = TS_ERROR;
    	break;
	} /* of switch(gestureId) */

	return(ts_status);
}

/**
  * @brief  Returns status and positions of the touch screen.
  * @param  TS_State: Pointer to touch screen current state structure
  * @retval TS_OK if all initializations are OK. Other value if error.
  */
uint8_t BSP_TS_GetState(TS_StateTypeDef *TS_State)
{
	static uint32_t _x[TS_MAX_NB_TOUCH] = {0, 0};
	static uint32_t _y[TS_MAX_NB_TOUCH] = {0, 0};
	uint8_t ts_status = TS_OK;
	uint16_t x[TS_MAX_NB_TOUCH];
	uint16_t y[TS_MAX_NB_TOUCH];
	uint16_t brute_x[TS_MAX_NB_TOUCH];
	uint16_t brute_y[TS_MAX_NB_TOUCH];
	uint16_t x_diff;
	uint16_t y_diff;
	uint32_t index;
#if (TS_MULTI_TOUCH_SUPPORTED == 1)
	uint32_t weight = 0;
	uint32_t area = 0;
	uint32_t event = 0;
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

	/* Check and update the number of touches active detected */
	TS_State->touchDetected = tsDriver->DetectTouch(I2cAddress);

	if(TS_State->touchDetected)
	{
		for(index=0; index < TS_State->touchDetected; index++)
		{
			/* Get each touch coordinates */
			tsDriver->GetXY(I2cAddress, &(brute_x[index]), &(brute_y[index]));

			if(tsOrientation == TS_SWAP_NONE)
			{
				x[index] = brute_x[index];
				y[index] = brute_y[index];
			}

			if(tsOrientation & TS_SWAP_X)
			{
				x[index] = 4096 - brute_x[index];
			}

			if(tsOrientation & TS_SWAP_Y)
			{
				y[index] = 4096 - brute_y[index];
			}

			if(tsOrientation & TS_SWAP_XY)
			{
				y[index] = brute_x[index];
				x[index] = brute_y[index];
			}

			x_diff = x[index] > _x[index]? (x[index] - _x[index]): (_x[index] - x[index]);
			y_diff = y[index] > _y[index]? (y[index] - _y[index]): (_y[index] - y[index]);

			if ((x_diff + y_diff) > 5)
			{
				_x[index] = x[index];
				_y[index] = y[index];
			}

			if(I2cAddress == FT5336_I2C_SLAVE_ADDRESS)
			{
				TS_State->touchX[index] = x[index];
				TS_State->touchY[index] = y[index];
			}
			else
			{
				/* 2^12 = 4096 : indexes are expressed on a dynamic of 4096 */
				TS_State->touchX[index] = (tsXBoundary * _x[index]) >> 12;
				TS_State->touchY[index] = (tsYBoundary * _y[index]) >> 12;
			}

#if (TS_MULTI_TOUCH_SUPPORTED == 1)

			/* Get touch info related to the current touch */
			ft5336_TS_GetTouchInfo(I2cAddress, index, &weight, &area, &event);

			/* Update TS_State structure */
			TS_State->touchWeight[index] = weight;
			TS_State->touchArea[index]   = area;

			/* Remap touch event */
			switch(event)
			{
			case FT5336_TOUCH_EVT_FLAG_PRESS_DOWN	:
				TS_State->touchEventId[index] = TOUCH_EVENT_PRESS_DOWN;
				break;
			case FT5336_TOUCH_EVT_FLAG_LIFT_UP :
				TS_State->touchEventId[index] = TOUCH_EVENT_LIFT_UP;
				break;
			case FT5336_TOUCH_EVT_FLAG_CONTACT :
				TS_State->touchEventId[index] = TOUCH_EVENT_CONTACT;
				break;
			case FT5336_TOUCH_EVT_FLAG_NO_EVENT :
				TS_State->touchEventId[index] = TOUCH_EVENT_NO_EVT;
				break;
			default :
				ts_status = TS_ERROR;
				break;
			} /* of switch(event) */

#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

		} /* of for(index=0; index < TS_State->touchDetected; index++) */

#if (TS_MULTI_TOUCH_SUPPORTED == 1)
    	/* Get gesture Id */
		ts_status = BSP_TS_Get_GestureId(TS_State);
#endif /* TS_MULTI_TOUCH_SUPPORTED == 1 */

	} /* end of if(TS_State->touchDetected != 0) */

	return (ts_status);
}

void get_touch_location(int *location)
{
	BSP_TS_GetState(&TS_State);
	if(TS_State.touchDetected){
		/* Get X and Y position of the touch post calibrated */
		location[0] = TS_State.touchX[0];
		location[1] = TS_State.touchY[0];
	}
}




