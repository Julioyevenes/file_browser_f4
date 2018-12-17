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
#ifndef __AUDIO_DECODERS_H
#define __AUDIO_DECODERS_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "FSIO.h"
#include "fileBrowser.h"
#include "AudioHardware.h"

/* Exported types ------------------------------------------------------------*/
/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define AUDIO_WAIT_TOL 1000000

/* Exported variables --------------------------------------------------------*/
extern BYTE                 		blAudioPlaying;
extern __IO uint8_t 				volumeAudio;
extern __IO uint32_t 				XferCplt;
extern __IO uint8_t 				PauseResumeStatus;
extern FlagStatus 					PauseFlag;
extern uint32_t 					AudioTimeOut;

/* Exported functions ------------------------------------------------------- */
BYTE AudioPlayBack(FSFILE *pFile, FILE_FORMAT fileFormat);
void AudioPlayer_CallBack(void);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIO_DECODERS_H */
