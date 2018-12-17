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
#ifndef __MP3_PLAYER_H
#define __MP3_PLAYER_H

#ifdef __cplusplus
 extern "C" {
#endif 

#ifdef SUPPORT_MP3

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx.h"
#include "FSIO.h"

/* Exported types ------------------------------------------------------------*/
 typedef struct {
 	uint32_t BaseAddr; /* base address for the audio buf */
 	int32_t Size; /* size of the audio buf */
 	uint8_t Empty; /* buf status, 1-> empty */
 } AUDIOBUF;

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define READBUF_BASEADDR	0x2001BC00
#define AUDIOBUF_BASEADDR	0x2001CC00

#define READBUF_SIZE      	(0x1000)  	/* 4096 byte */
#define AUDIOBUF_SIZE     	(0x1200)	/* 4608 byte */

#define MAX_AUDIOBUF_NUM	2
#if (MAX_AUDIOBUF_NUM < 2) || (MAX_AUDIOBUF_NUM > 3)
	#error Invalid MAX_AUDIOBUF_NUM setting.
#endif

/* Exported variables --------------------------------------------------------*/
extern uint8_t *readBuf, *readPtr;
extern int bytesLeft, outOfData, eofReached;
extern uint32_t pcmframe_size;
extern volatile uint8_t PlayingBufIdx;
extern volatile uint8_t DecodeBufIdx;
extern AUDIOBUF audiobuf[];

/* Exported functions ------------------------------------------------------- */
BYTE MP3PlayBack(FSFILE *pFile);
void MP3Player_CallBack(void);

#endif /* SUPPORT_MP3 */

#ifdef __cplusplus
}
#endif

#endif /* __MP3_PLAYER_H */
