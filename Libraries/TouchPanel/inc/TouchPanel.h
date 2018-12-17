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
#ifndef _TOUCHPANEL_H_
#define _TOUCHPANEL_H_

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/
typedef	struct POINT 
{
   uint16_t x;
   uint16_t y;
} Coordinate;

typedef struct Matrix 
{						
   int32_t  An,
            Bn,     
            Cn,   
            Dn,    
            En,    
            Fn,     
            Divider ;
} Matrix;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/**
 * @brief Uncomment the line below if you want to use user defined Delay function
 *        (for precise timing), otherwise default _delay_ function defined within
 *         this driver is used (less precise timing).  
 */
/* #define USE_Delay */

#ifdef USE_Delay
#include "main.h" 
  #define _delay_     Delay  /* !< User can provide more timing precise _delay_ function
                                   (with 10ms time base), using SysTick for example */
#else
  #define _delay_     delay      /* !< Default _delay_ function with less precise timing */
#endif

/* Exported variables --------------------------------------------------------*/
extern Coordinate ScreenSample[3];
extern Coordinate DisplaySample[3];
extern Matrix matrix;
extern Coordinate display;

/* Exported functions ------------------------------------------------------- */
void TP_Init(void);	
Coordinate *Read_Ads7846(void);
void TouchPanel_Calibrate(void);
FunctionalState setCalibrationMatrix(Coordinate *displayPtr, Coordinate *screenPtr, Matrix *matrixPtr);
FunctionalState getDisplayPoint(Coordinate *displayPtr, Coordinate *screenPtr, Matrix *matrixPtr);

#ifdef __cplusplus
}
#endif

#endif /* _TOUCHPANEL_H_ */