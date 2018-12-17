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
#ifndef __AUDIOHARDWARE_H
#define __AUDIOHARDWARE_H

#ifdef __cplusplus
 extern "C" {
#endif 

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"

/* Exported types ------------------------------------------------------------*/
typedef enum
{
	NONE = 0,
	S16_TO_U8,
	S16_TO_U16,
	U8_TO_U16
} BitConv;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
/**
  * @brief  Default device
  */
#if !defined(USE_STM32_DAC) && !defined(USE_CS43L22)
	#define USE_CS43L22
#endif

#if defined(USE_STM32_DAC) && defined(USE_CS43L22)
	#error "Only one device may be defined at a time"
#endif

/**
  * @brief  Driver behaviour
  */
/* Audio Transfer mode (DMA, Interrupt or Polling) */
#define AUDIO_MAL_MODE_NORMAL         /* Uncomment this line to enable the audio
                                         Transfer using DMA */
/* #define AUDIO_MAL_MODE_CIRCULAR */ /* Uncomment this line to enable the audio
                                         Transfer using DMA */

#ifdef USE_CS43L22
	/* Codec output DEVICE */
	#define OUTPUT_DEVICE_SPEAKER         1
	#define OUTPUT_DEVICE_HEADPHONE       2
	#define OUTPUT_DEVICE_BOTH            3
	#define OUTPUT_DEVICE_AUTO            4
#endif /* USE_CS43L22 */

/* Exported variables --------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
void Audio_MAL_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t SampleRate);
void Audio_MAL_DeInit(void);
void Audio_MAL_Play(uint32_t Addr, uint32_t Size);
void Audio_MAL_PauseResume(uint32_t Cmd);
void Audio_MAL_Stop(void);
#ifdef USE_CS43L22
	void Audio_MAL_VolumeCtl(uint8_t Volume);
	void Audio_MAL_Mute(uint32_t Cmd);
#endif /* USE_CS43L22 */
void PCMBuffConv(uint32_t *inbuff, uint16_t inbuffSize, uint32_t *outbuff, BitConv bitconv);

/**
  * @brief  User Callbacks: user has to implement these functions in his code
  */
uint16_t EVAL_AUDIO_GetSampleCallBack(void);
void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size);
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size);
void EVAL_AUDIO_Error_CallBack(void* pData);
uint32_t Codec_TIMEOUT_UserCallback(void);

#ifdef __cplusplus
}
#endif

#endif /* __AUDIOHARDWARE_H */
