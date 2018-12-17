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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include <stdlib.h>
#include "stm32f4xx.h"
#include "GenericTypeDefs.h"

/**
  * @brief   General hardware
  */
#include "stm32f4_discovery.h"
#include "HardwareProfile.h"
#include "TimeDelay.h"

/**
  * @brief   LCD with Touch screen
  */
#include "Graphics.h"
#include "TouchPanel.h"

/**
  * @brief   USB Peripheral
  */
#ifdef ENABLE_USB_MSD
	#include "usb_hcd_int.h"
	#include "usbh_usr.h"
	#include "usbh_core.h"
	#include "usbh_msc_core.h"
#endif

/**
  * @brief   Audio Codec
  */
#include "AudioDecoders.h"

/**
  * @brief   RTC for FSIO
  */
#include "rtcc.h"
  
/**
  * @brief   Application
  */
#include "fileBrowser.h"

/* Exported types ------------------------------------------------------------*/
 typedef enum
 {
     CREATE_FILEBROWSER = 0,
     DISPLAY_FILEBROWSER
 } SCREEN_STATES;

/* Exported constants --------------------------------------------------------*/
extern const FONT_FLASH GOLFontDefault; // default GOL font

/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern __IO uint32_t TimingDelay;
extern FlagStatus volTask;
extern BOOL lockScreen;
extern __IO uint8_t volumeAudio;
extern __IO DWORD  tick;
extern UINT BacklightLevel;
extern USB_OTG_CORE_HANDLE USB_OTG_Core;
extern GOL_SCHEME *altScheme; // alternative style scheme
extern SCREEN_STATES screenState; 	// current state of main state machine

/* Exported functions ------------------------------------------------------- */
void GOL_Procedures(void);
void LCD_SetBacklight(uint16_t intensity);
void TIM1_Config(void);
void TimingDelay_Decrement(void);
void Delay(__IO uint32_t nTime);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
