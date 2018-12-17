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
#ifndef __HARDWARE_PROFILE_H
#define __HARDWARE_PROFILE_H

#ifdef __cplusplus
 extern "C" {
#endif 

#if defined(__GNUC__) && defined(STM32F4XX)

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/**
  * @brief   HARDWARE DEVICES SELECTION
  */
//#define USE_DISPLAY_CONTROLLER_SSD1289
#define USE_DISPLAY_CONTROLLER_ILI9481
//#define USE_DISPLAY_CONTROLLER_STM32_LTDC
#define USE_USB_INTERFACE
#define ENABLE_USB_MSD
//#define USE_SD_INTERFACE_WITH_SPI
//#define USE_SD_INTERFACE_WITH_SDIO
//#define ENABLE_SD_MSD

/**
  * @brief   DISPLAY PARAMETERS
  */
// Using SSD1289 Display Controller
#if defined(USE_DISPLAY_CONTROLLER_SSD1289)
    #define DISP_ORIENTATION    90
    #define DISP_HOR_RESOLUTION 240
    #define DISP_VER_RESOLUTION 320

// Using ILI9481 Display Controller
#elif defined(USE_DISPLAY_CONTROLLER_ILI9481)
    #define DISP_ORIENTATION    90
    #define DISP_HOR_RESOLUTION 320
    #define DISP_VER_RESOLUTION 480

// Using STM32 LTDC Display Controller
#elif defined(USE_DISPLAY_CONTROLLER_STM32_LTDC)

/**
  * @brief  Display memory
  */
#define LCD_FRAME_BUFFER       ((uint32_t)0xD0000000)
#define BUFFER_OFFSET          ((uint32_t)0x50000)

/**
  * @brief  LCD panel config
  */
#define TY430TFT480272 		1
#define TY500TFT800480 		2
#define TY700TFT800480 		3
#define LVC75Z779V1S   		4

#define DISPLAY_PANEL TY430TFT480272
//#define DISPLAY_PANEL TY500TFT800480
//#define DISPLAY_PANEL TY700TFT800480
//#define DISPLAY_PANEL LVC75Z779V1S

#if (DISPLAY_PANEL == TY430TFT480272)
	
	/* Horizontal and vertical display resolution */
	#define DISP_HOR_RESOLUTION				480
	#define DISP_VER_RESOLUTION				272

	/* Image orientation (can be 0, 90, 180, 270 degrees). */
	#define DISP_ORIENTATION				0

	/* Panel Data Width */
	#define DISP_DATA_WIDTH                 24

	/* LSHIFT Polarity Swap */
	/* If defined LSHIFT is a falling trigger */
	//#define DISP_INV_LSHIFT

	/* Horizontal synchronization timing in pixels */
	#define DISP_HOR_PULSE_WIDTH		41  
	#define DISP_HOR_BACK_PORCH			2
	#define DISP_HOR_FRONT_PORCH		2	

	/* Vertical synchronization timing in lines */
	#define DISP_VER_PULSE_WIDTH		10
	#define DISP_VER_BACK_PORCH			2	
	#define DISP_VER_FRONT_PORCH		2	
	
	/* End of definition for DISPLAY_PANEL == TY430TFT480272 */

#elif (DISPLAY_PANEL == TY500TFT800480)

	/* Horizontal and vertical display resolution */
	#define DISP_HOR_RESOLUTION				800
	#define DISP_VER_RESOLUTION				480
	
	/* Image orientation (can be 0, 90, 180, 270 degrees). */
	#define DISP_ORIENTATION				0
	
	/* Panel Data Width */
	#define DISP_DATA_WIDTH                 24
	
	/* LSHIFT Polarity Swap */
	/* If defined LSHIFT is a falling trigger */
	//#define DISP_INV_LSHIFT
	
	/* Horizontal synchronization timing in pixels */
	#define DISP_HOR_PULSE_WIDTH		128  
	#define DISP_HOR_BACK_PORCH			88
	#define DISP_HOR_FRONT_PORCH		40	
	
	/* Vertical synchronization timing in lines */
	#define DISP_VER_PULSE_WIDTH		2
	#define DISP_VER_BACK_PORCH			25	
	#define DISP_VER_FRONT_PORCH		18
	
	/* End of definition for DISPLAY_PANEL == TY500TFT800480 */

#elif (DISPLAY_PANEL == TY700TFT800480)

	/* Horizontal and vertical display resolution */
	#define DISP_HOR_RESOLUTION 800
	#define DISP_VER_RESOLUTION 480
	
	/* Image orientation (can be 0, 90, 180, 270 degrees). */	
  	#define DISP_ORIENTATION    0
	
	/* Panel Data Width */
	#define DISP_DATA_WIDTH                 18
	
	/* Horizontal synchronization timing in pixels */
	#define DISP_HOR_PULSE_WIDTH		1
	#define DISP_HOR_BACK_PORCH			210
	#define DISP_HOR_FRONT_PORCH		45	
	
	/* Vertical synchronization timing in lines */
	#define DISP_VER_PULSE_WIDTH		1
	#define DISP_VER_BACK_PORCH			34	
	#define DISP_VER_FRONT_PORCH		10
	
	/* End of definition for DISPLAY_PANEL == TY700TFT800480 */	
	
#elif (DISPLAY_PANEL == LVC75Z779V1S)

	/* Horizontal and vertical display resolution */
	#define DISP_HOR_RESOLUTION				320
	#define DISP_VER_RESOLUTION				240
	
	/* Image orientation (can be 0, 90, 180, 270 degrees). */
	#define DISP_ORIENTATION				0
	
	/* Panel Data Width */
	#define DISP_DATA_WIDTH                 24
	
	/* LSHIFT Polarity Swap */
	/* If defined LSHIFT is a falling trigger */
	//#define DISP_INV_LSHIFT
	
	/* Horizontal synchronization timing in pixels */
	#define DISP_HOR_PULSE_WIDTH		20  
	#define DISP_HOR_BACK_PORCH			48
	#define DISP_HOR_FRONT_PORCH		20	
	
	/* Vertical synchronization timing in lines */
	#define DISP_VER_PULSE_WIDTH		2
	#define DISP_VER_BACK_PORCH			16
	#define DISP_VER_FRONT_PORCH		4
	
	/* End of definition for DISPLAY_PANEL == LVC75Z779V1S */

#else
        #error "Error: DISPLAY_PANEL not defined"	
#endif /* __DISPLAY_PANEL */

/**
  * @brief DMA2D settings 
  */
#define DMA2D_GRAPHIC_LCD_WIDTH     DISP_HOR_RESOLUTION
#define DMA2D_GRAPHIC_LCD_HEIGHT    DISP_VER_RESOLUTION
#define DMA2D_GRAPHIC_RAM_ADDR      LCD_FRAME_BUFFER

#else
        #error "Graphics controller is not defined"
#endif /* __USE_DISPLAY_CONTROLLER */

/**
  * @brief   DISPLAY PARALLEL INTERFACE
  */

/**
  * @brief   HARDWARE PROFILE FOR DISPLAY CONTROLLER INTERFACE
  */
#if defined(USE_DISPLAY_CONTROLLER_SSD1289) || \
    defined(USE_DISPLAY_CONTROLLER_ILI9481)
	
// Definitions for backlight control pin
#define DisplayBacklightOn()        GPIOA->BSRRL = GPIO_Pin_8
#define DisplayBacklightOff()       GPIOA->BSRRH = GPIO_Pin_8

// Definitions for CS pin
#define SET_CS()        			GPIOD->BSRRL = GPIO_Pin_0
#define CLR_CS()        			GPIOD->BSRRH = GPIO_Pin_0

// Definitions for RS pin
#define SET_RS()        			GPIOD->BSRRL = GPIO_Pin_1
#define CLR_RS()        			GPIOD->BSRRH = GPIO_Pin_1

// Definitions for NWR pin
#define SET_NWR()       			GPIOD->BSRRL = GPIO_Pin_2
#define CLR_NWR()       			GPIOD->BSRRH = GPIO_Pin_2

// Definitions for NRD pin
#define SET_NRD()       			GPIOD->BSRRL = GPIO_Pin_3
#define CLR_NRD()       			GPIOD->BSRRH = GPIO_Pin_3

// Definitions for RST pin
#define SET_RST()       			GPIOD->BSRRL = GPIO_Pin_7
#define CLR_RST()       			GPIOD->BSRRH = GPIO_Pin_7

#endif /* USE_DISPLAY_CONTROLLER */

/**
  * @brief   HARDWARE PROFILE FOR THE RESISTIVE TOUCHSCREEN
  */

/**
  * @brief   HARDWARE PROFILE FOR SD-SPI
  */
#if defined(USE_SD_INTERFACE_WITH_SPI)

#define INPUT_PIN  1
#define OUTPUT_PIN 0

//Chip Select Signal.
#define SET_SD_CS GPIOB->BSRRL = GPIO_Pin_0
#define CLR_SD_CS GPIOB->BSRRH = GPIO_Pin_0

//Card detect signal.
#define READ_SD_CD GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_1)

//Write protect signal.
#define READ_SD_WE GPIO_ReadInputDataBit(GPIOB,GPIO_Pin_2)

/**
  * @brief   HARDWARE PROFILE FOR SD-SDIO
  */
#elif defined(USE_SD_INTERFACE_WITH_SDIO)

#ifndef USE_DETECT_PIN
#define USE_DETECT_PIN				    1
#endif

#ifndef USE_WRITEPROTECT_PIN
#define USE_WRITEPROTECT_PIN			1
#endif

#ifndef SDIO_4BIT
#define SDIO_4BIT						1
#endif

#if USE_DETECT_PIN > 0
#ifndef DETECT_PIN_PIN
#define DETECT_PIN_RCC			RCC_AHB1Periph_GPIOB
#define DETECT_PIN_PORT			GPIOB
#define DETECT_PIN_PIN			GPIO_Pin_1
#endif
#endif

#if USE_WRITEPROTECT_PIN > 0
#ifndef WRITEPROTECT_PIN_PIN
#define WRITEPROTECT_PIN_RCC		RCC_AHB1Periph_GPIOB
#define WRITEPROTECT_PIN_PORT		GPIOB
#define WRITEPROTECT_PIN_PIN		GPIO_Pin_2
#endif
#endif

#endif /* USE_SD_INTERFACE */

/**
  * @brief   RTCC DEFAULT INITIALIZATION (these are values to initialize the RTCC)
  */
#define RTCC_DEFAULT_DAY        15      // 15th
#define RTCC_DEFAULT_MONTH      10      // October
#define RTCC_DEFAULT_YEAR       10      // 2010
#define RTCC_DEFAULT_WEEKDAY    05      // Friday
#define RTCC_DEFAULT_HOUR       10      // 10:10:01
#define RTCC_DEFAULT_MINUTE     10
#define RTCC_DEFAULT_SECOND     01

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#endif /* __GNUC__ && STM32F4XX */

#ifdef __cplusplus
}
#endif

#endif /* __HARDWARE_PROFILE_H */