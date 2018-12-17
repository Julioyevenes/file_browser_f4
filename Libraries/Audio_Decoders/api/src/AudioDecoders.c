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
#include "AudioDecoders.h"
#ifdef SUPPORT_WAV
	#include "waveplayer.h"
#endif
#ifdef SUPPORT_MP3
	#include "mp3player.h"
#endif

/* Private types ------------------------------------------------------------*/
/* Private constants --------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
/**
  * @brief   Audio player variables
  */
BYTE                        blAudioPlaying = FALSE;
__IO uint8_t 				volumeAudio = 70;
__IO uint32_t 				XferCplt = 0;
__IO uint8_t 				PauseResumeStatus;
FlagStatus 					PauseFlag = RESET;
FILE_FORMAT					bCallback;
uint32_t 					AudioTimeOut = 0;

/* Private function prototypes ----------------------------------------------*/

/**
  * @brief  Play audio from file
  * @param  pFile	  : Pointer to file object
  * @param  fileFormat: File extension
  * @retval error: 0 no error
  *                1 error
  */
BYTE AudioPlayBack(FSFILE *pFile, FILE_FORMAT fileFormat) {
	BYTE bRetVal = 0;

	bCallback = fileFormat;

	switch(fileFormat)
	{
#ifdef SUPPORT_WAV
		case WAV: 	bRetVal = WavePlayBack(pFile); break;
#endif
#ifdef SUPPORT_MP3
		case MP3: 	bRetVal = MP3PlayBack(pFile); break;
#endif
		default:    bRetVal = 0xFF;
	}

     return(bRetVal);
}

 /**
   * @brief  Calculates the remaining file size and new position of the pointer.
   * @param  None
   * @retval None
   */
 void EVAL_AUDIO_TransferComplete_CallBack(uint32_t pBuffer, uint32_t Size) {
 
	/* Calculate the remaining audio data in the file and the new size 
	for the DMA transfer. If the Audio files size is less than the DMA max 
	data transfer size, so there is no calculation to be done, just restart 
	from the beginning of the file ... */
	/* Check if the end of file has been reached */
	
#ifdef AUDIO_MAL_MODE_NORMAL
	XferCplt = 1;

#ifdef SUPPORT_WAV
	if(bCallback == WAV) {
		if (WaveDataLength) WaveDataLength -= MEDIA_SECTOR_SIZE;
		if (WaveDataLength < MEDIA_SECTOR_SIZE) WaveDataLength = 0;
	}
#endif /* SUPPORT_WAV */

#ifdef SUPPORT_MP3
	if(bCallback == MP3) {
		/* Data in audiobuf[PlayingBufIdx] has been sent out */
		audiobuf[PlayingBufIdx].Empty = 1;
		audiobuf[PlayingBufIdx].Size = -1;

		/* Send the data in next audio buffer */
		PlayingBufIdx++;
		if (PlayingBufIdx == MAX_AUDIOBUF_NUM)
			PlayingBufIdx = 0;

		if (audiobuf[PlayingBufIdx].Empty == 1) {
			/* If empty==1, it means read file+decoder is slower than playback
		 	 (it will cause noise) or playback is over (it is ok). */;

		 	 return;
		}
		Audio_MAL_Play((uint32_t *) audiobuf[PlayingBufIdx].BaseAddr, audiobuf[PlayingBufIdx].Size);
	}
#endif /* SUPPORT_MP3 */

#else /* #ifdef AUDIO_MAL_MODE_CIRCULAR */  
#endif /* AUDIO_MAL_MODE_CIRCULAR */
 }
 
/**
* @brief  Manages the DMA Half Transfer complete interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_HalfTransfer_CallBack(uint32_t pBuffer, uint32_t Size)
{  
#ifdef AUDIO_MAL_MODE_CIRCULAR
    
#endif /* AUDIO_MAL_MODE_CIRCULAR */
  
  /* Generally this interrupt routine is used to load the buffer when 
  a streaming scheme is used: When first Half buffer is already transferred load 
  the new data to the first half of buffer while DMA is transferring data from 
  the second half. And when Transfer complete occurs, load the second half of 
  the buffer while the DMA is transferring from the first half ... */
  /* 
  ...........
  */
}

/**
* @brief  Manages the DMA FIFO error interrupt.
* @param  None
* @retval None
*/
void EVAL_AUDIO_Error_CallBack(void* pData)
{
  /* Stop the program with an infinite loop */
  while (1)
  {}
  
  /* could also generate a system reset to recover from the error */
  /* .... */
}

/**
* @brief  Get next data sample callback
* @param  None
* @retval Next data sample to be sent
*/
uint16_t EVAL_AUDIO_GetSampleCallBack(void)
{
  return 0;
}


#ifndef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None.
  * @retval None.
  */
uint32_t Codec_TIMEOUT_UserCallback(void)
{   
  return (0);
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */

 /**
  * @brief  Reset the audio player
  * @param  None
  * @retval None
  */
 void AudioPlayer_CallBack(void) {
	/* Clear playing flag */
	blAudioPlaying = FALSE;

	/* Reset common variables */
	PauseResumeStatus = 1;
	
#ifdef SUPPORT_WAV
	/* Reset the wave player variables */
	WaveDataLength = 0;
#endif

#ifdef SUPPORT_MP3
	/* Reset the mp3 player variables */
	bytesLeft = 0;
	outOfData = 0;
	eofReached = 1;
	readPtr = readBuf = (uint8_t *)(READBUF_BASEADDR);
	pcmframe_size = 0;
#endif
 }
