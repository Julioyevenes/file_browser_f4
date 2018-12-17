/**
  ******************************************************************************
  * @file    ${file_name} 
  * @author  ${user}
  * @version 
  * @date    ${date}
  * @brief   
  ******************************************************************************
  * @attention
  *
  * This program is free software: you can redistribute it and/or modify
  * it under the terms of the GNU General Public License as published by
  * the Free Software Foundation, either version 3 of the License, or
  * (at your option) any later version.
  *
  * This program is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  * GNU General Public License for more details.
  *
  * You should have received a copy of the GNU General Public License
  * along with this program.  If not, see <http://www.gnu.org/licenses/>.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private types ------------------------------------------------------------*/
/* Private constants --------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/**
  * @brief  Base address of the Flash sectors
  */
#define ADDR_FLASH_SECTOR_11    ((uint32_t)0x080E0000) /* Base @ of Sector 11, 128 Kbytes */

/* Private variables --------------------------------------------------------*/
__IO uint32_t TimingDelay = 0;
RCC_ClocksTypeDef RCC_Clocks;
__IO DWORD tick = 0;

#if defined ENABLE_USB_MSD
	USB_OTG_CORE_HANDLE          USB_OTG_Core;
	USBH_HOST                    USB_Host;
	BOOL EnumDone = FALSE;
#endif

/**
  * @brief   LCD & Touch variables
  */
uint8_t calibrateDone = RESET;
BOOL lockScreen = FALSE;

/**
  * @brief   Back light variables 
  */
uint16_t TimerPeriod    = 0;
uint16_t Channel1Pulse  = 0;
TIM_OCInitTypeDef TIM_OCInitStructure;
UINT BacklightLevel = 100;

/**
  * @brief   Audio codec variables
  */
FlagStatus volTask = RESET;

/**
  * @brief   GOL variables
  */
SCREEN_STATES screenState = CREATE_FILEBROWSER; 	// current state of main state machine
GOL_SCHEME *altScheme;                         		// alternative style scheme

/* Private function prototypes ----------------------------------------------*/
void TouchGetMsg(GOL_MSG *pMsg);
void FLASH_TouchCalibrationSave(Matrix *matrixPtr);
BOOL FLASH_TouchCalibrationGet(Matrix *matrixPtr);

/**
  * @brief   Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	/*!< At this stage the microcontroller clock setting is already configured, 
		this is done through SystemInit() function which is called from startup
		files (startup_stm32f429_439xx.s) before to branch to application main. 
		To reconfigure the default setting of SystemInit() function, refer to
		system_stm32f4xx.c file
		*/  
  
	/* SysTick end of count event each 10ms */
	RCC_GetClocksFreq(&RCC_Clocks);
	SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000000);
	
	/* Init routines */
	TP_Init();
	GOLInit(); // initialize graphics library
	
	RTCCInit(); // Setup the RTCC
    RTCCProcessEvents();
	
    if(!FLASH_TouchCalibrationGet(&matrix)) {

    	/* Calibrate touch sensor */
    	TouchPanel_Calibrate();
    	FLASH_TouchCalibrationSave(&matrix);
    	calibrateDone = SET;

    } else {
    	calibrateDone = SET;
    }
	
	/* Initialize User Button */
	STM_EVAL_PBInit(BUTTON_USER, BUTTON_MODE_EXTI);
	
	/* Initialize LEDS */
	STM_EVAL_LEDInit(LED3);
	STM_EVAL_LEDInit(LED4);
	STM_EVAL_LEDInit(LED5);
	STM_EVAL_LEDInit(LED6);

#if defined ENABLE_USB_MSD
	/* Init Host Library */
	USBH_Init(&USB_OTG_Core, USB_OTG_FS_CORE_ID, &USB_Host, &USBH_MSC_cb, &USR_Callbacks);
#endif

	altScheme = GOLCreateScheme();
	
#ifdef USE_TRANSPARENT_COLOR
	TransparentColorEnable(WHITE); // Define transparent color to be used
#endif
    
	/* Infinite loop */
	while (1)
	{
#if defined(ENABLE_USB_MSD)
      /* Host Task handler */
      USBH_Process(&USB_OTG_Core, &USB_Host);
#endif

      /* Graphic user interface */
      GOL_Procedures();
	}
}

/**
  * @brief  This function must be called periodically to manage
  *         graphic interface and user interactions.
  * @param  None
  * @retval None
  */
void GOL_Procedures(void)
{
	GOL_MSG msg; // GOL message structure to interact with GOL

	if(GOLDraw())
	{                               // Draw GOL objects
		// Drawing is finished, we can now process new message
		TouchGetMsg(&msg);          // Get message from touch screen

		GOLMsg(&msg);               // Process message
	}
}

/**
  * @brief  The user MUST implement this function. GOLMsg() calls
  *         this function when a valid message for an object in the
  *         active list is received. User action for the message should
  *         be implemented here. If this function returns non-zero,
  *         the message for the object will be processed by default.
  *         If zero is returned, GOL will not perform any action.
  * @param  objMsg - Translated message for the object or the action ID response from the object.
  * @param  pObj   - Pointer to the object that processed the message.
  * @param  pMsg   - Pointer to the GOL message from user.
  * @retval Return a non-zero if the message will be processed by default.
  *         If a zero is returned, the message will not be processed by GOL.
  */
WORD GOLMsgCallback(WORD objMsg, OBJ_HEADER* pObj, GOL_MSG* pMsg)
{
	switch(screenState) {
		case DISPLAY_FILEBROWSER:
			return (fileBrowser_MsgCallback(objMsg, pObj, pMsg));
		default:
			return (1); // process message by default
	}
}

/**
  * @brief  GOLDrawCallback() function MUST BE implemented by
  *         the user. This is called inside the GOLDraw()
  *         function when the drawing of objects in the active
  *         list is completed. User drawing must be done here.
  *         Drawing color, line type, clipping region, graphic
  *         cursor position and current font will not be changed
  *         by GOL if this function returns a zero. To pass
  *         drawing control to GOL this function must return
  *         a non-zero value. If GOL messaging is not using
  *         the active link list, it is safe to modify the
  *         list here.
  * @param  None
  * @retval Return a one if GOLDraw() will have drawing control
  *         on the active list. Return a zero if user wants to
  *         keep the drawing control.
  */
WORD GOLDrawCallback()
{
	static DWORD    prevTick = 0; // keeps previous value of tick

	switch(screenState) {
		case CREATE_FILEBROWSER:
			if(EnumDone) {
				if(Create_fileBrowser() == 1)
					screenState = DISPLAY_FILEBROWSER; // switch to next state
				else
					screenState = CREATE_FILEBROWSER;
			}
			return (1);

		case DISPLAY_FILEBROWSER:
			fileBrowser_DrawCallback();                     
			return (1);
		default:
			break;
	}

	return (1); // release drawing control to GOL
}

/**
  * @brief  Manages the interactions with touch screen
  *         and populates GOL message structure.
  * @param  GOL_MSG* pMsg: Pointer to the message structure to be populated.
  * @retval None
  */
void TouchGetMsg(GOL_MSG* pMsg)
{
   static SHORT prevX = -1;
   static SHORT prevY = -1;
   SHORT x, y;

   if(!lockScreen)
	   getDisplayPoint(&display, Read_Ads7846(), &matrix);

   if ((display.x <= 0) || (display.x > GetMaxX()) || (display.y <= 0)
         || (display.y > GetMaxY()))
   {
      display.x = -1;
      display.y = -1;
   }

   x = display.x;
   y = display.y;

   pMsg->type = TYPE_TOUCHSCREEN;
   pMsg->uiEvent = EVENT_INVALID;

   if ((prevX == x) && (prevY == y) && (x != -1) && (y != -1))
   {
      pMsg->uiEvent = EVENT_STILLPRESS;
      pMsg->param1 = x;
      pMsg->param2 = y;
      return;
   }

   if ((prevX != -1) || (prevY != -1))
   {

      if ((x != -1) && (y != -1))
      {
         pMsg->uiEvent = EVENT_MOVE;
      }
      else
      {
         pMsg->uiEvent = EVENT_RELEASE;
         pMsg->param1 = prevX;
         pMsg->param2 = prevY;
         prevX = x;
         prevY = y;

         return;
      }
   }
   else
   {
      if ((x != -1) && (y != -1))
      {
         pMsg->uiEvent = EVENT_PRESS;
      }
      else
      {
         pMsg->uiEvent = EVENT_INVALID;
      }

   }

   pMsg->param1 = x;
   pMsg->param2 = y;
   prevX = x;
   prevY = y;
}

/**
  * @brief  LCD Back light control function.
  * @param  uint16_t percent (0 to 100).
  * @retval None
  */
void LCD_SetBacklight(uint16_t intensity)
{
  if (intensity > 100)
  {
     intensity = 100;
  }
  else if (intensity < 0)
  {
     intensity = 0;
  }

  Channel1Pulse = (uint16_t) (((uint32_t) (intensity) * (TimerPeriod - 1)) / 100);

  TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);
}

/**
  * @brief  Configures LCD Back light line (TIM1) in alternate function mode.
  * @param  None
  * @retval None
  */
void TIM1_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;

  /* Enable clocks */
  RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA, ENABLE);
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);

  /* Configure GPIO */
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;

  /* Connect GPIO to TIM1 */
  GPIO_PinAFConfig(GPIOA, GPIO_PinSource8, GPIO_AF_TIM1);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  /* Configure TIM1 */
  TimerPeriod = (SystemCoreClock / 17570) - 1;

  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  TIM_TimeBaseStructure.TIM_Period = TimerPeriod;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
  TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

  /* Configure TIM1 in OC mode */
  Channel1Pulse = (uint16_t) (((uint32_t) 99 * (TimerPeriod - 1)) / 100);

  TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM2;
  TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;
  TIM_OCInitStructure.TIM_OutputNState = TIM_OutputNState_Enable;
  TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_Low;
  TIM_OCInitStructure.TIM_OCNPolarity = TIM_OCNPolarity_High;
  TIM_OCInitStructure.TIM_OCIdleState = TIM_OCIdleState_Set;
  TIM_OCInitStructure.TIM_OCNIdleState = TIM_OCIdleState_Reset;
  TIM_OCInitStructure.TIM_Pulse = Channel1Pulse;
  TIM_OC1Init(TIM1, &TIM_OCInitStructure);

  /* Enable TIM & OC outputs */
  TIM_Cmd(TIM1, ENABLE);
  TIM_CtrlPWMOutputs(TIM1, ENABLE);
}

/**
  * @brief  Stores the calibration constants obtained from lcd touch module.
  * @param  matrixPtr: pointer to the constants structure.
  * @retval None
  */
void FLASH_TouchCalibrationSave(Matrix *matrixPtr) {
	uint8_t i, HL = 0xFF;
	uint32_t AddressToSave, DataToSave;

	/* Unlock the Flash to enable the flash control register access */
	FLASH_Unlock();

	/* Erase the user Flash area */

	/* Clear pending flags (if any) */
	FLASH_ClearFlag(FLASH_FLAG_EOP    | FLASH_FLAG_OPERR  | FLASH_FLAG_WRPERR |
					FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);

	/* Device voltage range supposed to be [2.7V to 3.6V], the operation will
	 be done by word */
	if (FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3) != FLASH_COMPLETE) {
		/* Error occurred while sector erase.
		 User can add here some code to deal with this error  */
		while (1) {
		}
	}

	/* Program the user Flash area word by word */

	AddressToSave = ADDR_FLASH_SECTOR_11;

	for (i = 0; i < 7; i++) {
		switch (i) {
		case 0:
			DataToSave = matrixPtr->An;
			break;
		case 1:
			DataToSave = matrixPtr->Bn;
			break;
		case 2:
			DataToSave = matrixPtr->Cn;
			break;
		case 3:
			DataToSave = matrixPtr->Dn;
			break;
		case 4:
			DataToSave = matrixPtr->En;
			break;
		case 5:
			DataToSave = matrixPtr->Fn;
			break;
		case 6:
			DataToSave = matrixPtr->Divider;
			break;
		}

		if (FLASH_ProgramWord(AddressToSave, DataToSave) == FLASH_COMPLETE) {
			AddressToSave += 4;
		} else {
			/* Error occurred while writing data in Flash memory.
			 User can add here some code to deal with this error */
			while (1) {
			}
		}
	}

	/* Lock the Flash to disable the flash control register access (recommended
	 to protect the FLASH memory against possible unwanted operation) */
	FLASH_Lock();
}

/**
  * @brief  Get the calibration constants obtained from lcd touch module.
  * @param  matrixPtr: pointer to the constants structure.
  * @retval BOOL: TRUE if data is present, else FALSE.
  */
BOOL FLASH_TouchCalibrationGet(Matrix *matrixPtr) {
	uint8_t i;
	uint32_t AddressToGet, DataToGet;

	AddressToGet = ADDR_FLASH_SECTOR_11;

	for (i = 0; i < 7; i++) {
		DataToGet = *(__IO uint32_t*)AddressToGet;

		switch (i) {
		case 0:
			matrixPtr->An = DataToGet;
			break;
		case 1:
			matrixPtr->Bn = DataToGet;
			break;
		case 2:
			matrixPtr->Cn = DataToGet;
			break;
		case 3:
			matrixPtr->Dn = DataToGet;
			break;
		case 4:
			matrixPtr->En = DataToGet;
			break;
		case 5:
			matrixPtr->Fn = DataToGet;
			break;
		case 6:
			matrixPtr->Divider = DataToGet;
			break;
		}

		AddressToGet += 4;
	}

	if (matrixPtr->An != 0xffffffff && matrixPtr->Bn != 0xffffffff &&
	    matrixPtr->Cn != 0xffffffff && matrixPtr->Dn != 0xffffffff &&
	    matrixPtr->En != 0xffffffff && matrixPtr->Fn != 0xffffffff &&
	    matrixPtr->Divider != 0xffffffff)
		return (TRUE);
	else
		return (FALSE);
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{
  TimingDelay = nTime;

  while(TimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
}
