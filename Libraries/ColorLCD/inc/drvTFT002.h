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
#ifndef _DRVTFT002_H
#define _DRVTFT002_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "HardwareProfile.h"
#include "GraphicsConfig.h"
#include "GenericTypeDefs.h"

/**
  * @brief   Error Checking
  */
#ifndef DISP_HOR_RESOLUTION
	#error DISP_HOR_RESOLUTION must be defined in HardwareProfile.h
#endif
#ifndef DISP_VER_RESOLUTION
	#error DISP_VER_RESOLUTION must be defined in HardwareProfile.h
#endif
#ifndef COLOR_DEPTH
	#error COLOR_DEPTH must be defined in GraphicsConfig.h
#endif
#ifndef DISP_ORIENTATION
	#error DISP_ORIENTATION must be defined in HardwareProfile.h
#endif

/**
  * @brief   Horizontal and vertical screen size.
  */	
#if defined (USE_DISPLAY_CONTROLLER_SSD1289)
	#if (DISP_HOR_RESOLUTION != 240)
		#error This driver doesn't supports this resolution. Horisontal resolution must be 240 pixels.
	#endif
	#if (DISP_VER_RESOLUTION != 320)
		#error This driver doesn't supports this resolution. Vertical resolution must be 320 pixels.
	#endif
#elif defined (USE_DISPLAY_CONTROLLER_SSD2119)
	#if (DISP_HOR_RESOLUTION != 320)
		#error This driver doesn't supports this resolution. Horisontal resolution must be 320 pixels.
	#endif
	#if (DISP_VER_RESOLUTION != 240)
		#error This driver doesn't supports this resolution. Vertical resolution must be 240 pixels.
	#endif
#elif defined (USE_DISPLAY_CONTROLLER_ILI9481)
	#if (DISP_HOR_RESOLUTION != 320)
		#error This driver doesn't supports this resolution. Horisontal resolution must be 320 pixels.
	#endif
	#if (DISP_VER_RESOLUTION != 480)
		#error This driver doesn't supports this resolution. Vertical resolution must be 240 pixels.
	#endif
#endif

/**
  * @brief   Display orientation.
  */	
#if (DISP_ORIENTATION != 0) && (DISP_ORIENTATION != 90)
	#error This driver doesn't support this orientation.
#endif

/**
  * @brief   Color depth.
  */	
#if (COLOR_DEPTH != 16)
	#error This driver support 16 BPP color depth only.
#endif

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */

#ifdef __cplusplus
}
#endif

#endif /* _DRVTFT002_H */
