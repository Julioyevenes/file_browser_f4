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
#ifndef __WAVE_PLAYER_H
#define __WAVE_PLAYER_H

#ifdef __cplusplus
 extern "C" {
#endif 

#ifdef SUPPORT_WAV

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "FSIO.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/* Exported variables --------------------------------------------------------*/
extern __IO uint32_t WaveDataLength;

/* Exported functions ------------------------------------------------------- */
BYTE WavePlayBack(FSFILE *pFile);
void WavePlayer_CallBack(void);

#endif /* SUPPORT_WAV */

#ifdef __cplusplus
}
#endif

#endif /* __WAVE_PLAYER_H */