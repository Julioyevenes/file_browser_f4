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

#ifdef SUPPORT_MP3  
  
/* Includes ------------------------------------------------------------------*/
#include "mp3player.h"
#include "coder.h"
#include "mp3dec.h"
#include "AudioDecoders.h"
#include "AudioHardware.h"

/* Private types ------------------------------------------------------------*/
 enum {
 	AUDIOBUF0 = 0, AUDIOBUF1 = 1, AUDIOBUF2
 };
 
/* Private constants --------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/
FSFILE *fileR;
HMP3Decoder hMP3Decoder;
MP3FrameInfo mp3FrameInfo;
uint8_t *readBuf, *readPtr;
int bytesLeft, outOfData, eofReached;
uint32_t pcmframe_size;

/**
  * @brief  Play from audio buf 0, then move to the next buf 
  */
volatile uint8_t PlayingBufIdx = AUDIOBUF0;
volatile uint8_t DecodeBufIdx = AUDIOBUF0;

AUDIOBUF audiobuf[MAX_AUDIOBUF_NUM] = { { AUDIOBUF_BASEADDR, -1, 1 }, { AUDIOBUF_BASEADDR + AUDIOBUF_SIZE, -1, 1 },
#if MAX_AUDIOBUF_NUM==3
	{ AUDIOBUF_BASEADDR + AUDIOBUF_SIZE * 2, -1, 1 },
#endif
};

extern BYTE blAudioPlaying;
extern __IO uint8_t volumeAudio;
extern __IO uint32_t XferCplt;
extern __IO uint8_t PauseResumeStatus;
extern FlagStatus PauseFlag;
extern uint32_t AudioTimeOut;

/* Private function prototypes ----------------------------------------------*/
uint8_t MP3_DecodeFrame();
int MP3_FillReadBuffer(uint8_t *readBuf, uint8_t *readPtr, uint32_t bytesLeft, uint32_t bytesAlign);
void MP3_AddAudioBuf (uint32_t len);

 /**
  * @brief  Decode a frame.
  *
  * @param  None.
  * @retval 0: success, 1: terminated.
  *
  */
 uint8_t MP3_DecodeFrame() {
 	uint8_t word_align, frame_decoded;
 	int nRead, offset, err;

 	frame_decoded = 0;
 	word_align = 0;
 	nRead = 0;
 	offset = 0;

 	do {
 		/* somewhat arbitrary trigger to refill buffer - should always be enough for a full frame */
 		if (bytesLeft < 2 * MAINBUF_SIZE && !eofReached) {
 			/* Align to 4 bytes */
 			word_align = (4 - (bytesLeft & 3)) & 3;

 			/* Fill read buffer */
 			nRead = MP3_FillReadBuffer(readBuf, readPtr, bytesLeft, word_align);

 			bytesLeft += nRead;
 			readPtr = readBuf + word_align;
 			if (nRead == 0) {
 				eofReached = 1; /* end of file */
 				outOfData = 1;
 			}
 		}

 		/* find start of next MP3 frame - assume EOF if no sync found */
 		offset = MP3FindSyncWord(readPtr, bytesLeft);
 		if (offset < 0) {
 			readPtr = readBuf;
 			bytesLeft = 0;
 			continue;
 		}
 		readPtr += offset;
 		bytesLeft -= offset;

 		//simple check for valid header
 		if (((*(readPtr + 1) & 24) == 8) || ((*(readPtr + 1) & 6) != 2) || ((*(readPtr + 2) & 240) == 240) || ((*(readPtr + 2) & 12) == 12)
 				|| ((*(readPtr + 3) & 3) == 2)) {
 			readPtr += 1; //header not valid, try next one
 			bytesLeft -= 1;
 			continue;
 		}

 		err = MP3Decode(hMP3Decoder, &readPtr, &bytesLeft, (short *) audiobuf[DecodeBufIdx].BaseAddr, 0);
 		if (err == -6) {
 			readPtr += 1;
 			bytesLeft -= 1;
 			continue;
 		}

 		if (err) {
 			/* error occurred */
 			switch (err) {
 				case ERR_MP3_INDATA_UNDERFLOW:
 					outOfData = 1;
 					break;
 				case ERR_MP3_MAINDATA_UNDERFLOW:
 					/* do nothing - next call to decode will provide more mainData */
 					break;
 				case ERR_MP3_FREE_BITRATE_SYNC:
 				default:
 					outOfData = 1;
 					break;
 			}
 		} else {
 			/* no error */
 			MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
 			frame_decoded = 1;
 		}

 	} while (!frame_decoded && !outOfData);

 	if (outOfData == 1)
 		return 0x1; /* Decoder terminated */
 	else return 0x0; /* Decoder success. */

 }

 /**
  * @brief  Read data from MP3 file and fill in the Read Buffer.
  *
  * @param  readBuf: Pointer to the start of the Read Buffer
  * @param  readPtr: Pointer to the start of the left bytes in Read Buffer
  * @param  bytesLeft: Specifies the left bytes number in Read Buffer
  * @param  bytesAlign: Specifies the pad number to make sure it is aligned to 4 bytes
  * @retval Actual number of bytes read from file.
  *
  */
 int MP3_FillReadBuffer(uint8_t *readBuf, uint8_t *readPtr, uint32_t bytesLeft, uint32_t bytesAlign) {
 	uint32_t nRead;

 	/* Move the left bytes from the end to the front */
 	memmove(readBuf + bytesAlign, readPtr, bytesLeft);

 	    nRead = FSfread((void *) (readBuf + bytesLeft + bytesAlign), 1, READBUF_SIZE - bytesLeft - bytesAlign, fileR);
 		/* zero-pad to avoid finding false sync word after last frame (from old data in readBuf) */
 		if (nRead < READBUF_SIZE - bytesLeft - bytesAlign)
 			memset(readBuf + bytesAlign + bytesLeft + nRead, 0, READBUF_SIZE - bytesAlign - bytesLeft - nRead);

 	return nRead;
 }

 /**
   * @brief  Add an PCM frame to audio buf after decoding.
   *
   * @param  len: Specifies the frame size in bytes.
   * @retval None.
   *
   */
 void MP3_AddAudioBuf (uint32_t len)
 {
     /* Mark the status to not-empty which means it is available to playback. */
     audiobuf[DecodeBufIdx].Empty = 0;
     audiobuf[DecodeBufIdx].Size = len;

     /* Point to the next buffer */
     DecodeBufIdx ++;
     if (DecodeBufIdx == MAX_AUDIOBUF_NUM)
         DecodeBufIdx = 0;
 }
 
 /**
  * @brief  Play mp3 from file
  * @param  pFile: Pointer to file object
  * @retval error: 0 no error
  *                1 error
  */
 BYTE MP3PlayBack(FSFILE *pFile) {
 
	fileR = pFile;
 
	hMP3Decoder = MP3InitDecoder();
	if(hMP3Decoder == 0)
	{
		/* The MP3 Decoder could not be initialized
		 * Stop here. */

		return(200);
	}
	
	/* Set playing flag */
    blAudioPlaying = TRUE;
	
 reinitPlayback:
 
	/* Init variables */
	bytesLeft = 0;
	outOfData = 0;
	eofReached = 0;
	readPtr = readBuf = (uint8_t *)(READBUF_BASEADDR);
	pcmframe_size = 0;
	AudioTimeOut = 0;
	
	/* Decode and playback from audio buf 0 */
    DecodeBufIdx = PlayingBufIdx = AUDIOBUF0;
    audiobuf[AUDIOBUF0].Empty = audiobuf[AUDIOBUF1].Empty = 1;

    /* Decode the first frame to get the frame format */
    if (MP3_DecodeFrame() == 0)
    {
        pcmframe_size = (mp3FrameInfo.bitsPerSample / 8) * mp3FrameInfo.outputSamps;
#ifdef USE_STM32_DAC
        PCMBuffConv((uint32_t *)audiobuf[DecodeBufIdx].BaseAddr, pcmframe_size, (uint32_t *)audiobuf[DecodeBufIdx].BaseAddr, S16_TO_U16);
#endif /* USE_STM32_DAC */
        MP3_AddAudioBuf (pcmframe_size);
    }
    else
    {
    	/* Deallocate memory used by decoder */
    	MP3FreeDecoder(hMP3Decoder);

    	return(100); /* fail to decode the first frame */
    }

    if(mp3FrameInfo.samprate != 44100 || mp3FrameInfo.nChans != 2)
    {
        /* This means that the MP3 file is either a
         * mono audio stream or not sampled at 44100Hz.
         * We currently dont want to handle these type
         * of files even if the decoder can decode
         * them */
    }
	
#ifdef USE_STM32_DAC
	Audio_MAL_Init(0, 0, mp3FrameInfo.samprate);
#endif /* USE_STM32_DAC */

#ifdef USE_CS43L22
	Audio_MAL_Init(OUTPUT_DEVICE_AUTO, volumeAudio, mp3FrameInfo.samprate);
#endif /* USE_CS43L22 */

	Audio_MAL_Play((uint32_t *)audiobuf[AUDIOBUF0].BaseAddr, audiobuf[AUDIOBUF0].Size);
	
	XferCplt = 0;
	PauseResumeStatus = 1;
	
	while((outOfData == 0) && (eofReached ==0) &&  MDD_MediaDetect()) {
	
#ifdef GFX_LIB
		/* Graphic user interface */
		GOL_Procedures(); //Polling mode
#endif

		/* Playing */
		if (blAudioPlaying) {
		    /* wait for DMA transfer complete */
			while((XferCplt == 0) &&  MDD_MediaDetect()) {
				/* wait for playback is complete to release buf */
				while ((audiobuf[DecodeBufIdx].Empty == 0) && (outOfData == 0)) {
					/* Decoder is faster than play, all buf is full */
					
					if (PauseResumeStatus == 0)
					{
						/* Pause Playing wave */
						Audio_MAL_PauseResume(PauseResumeStatus);
						PauseResumeStatus = 2;
						PauseFlag = SET;
					}
					else if (PauseResumeStatus == 1)
					{
						/* Resume Playing wave */
						Audio_MAL_PauseResume(PauseResumeStatus);
						PauseResumeStatus = 2;
						PauseFlag = RESET;
					}
					else if (PauseFlag == SET)
					{
#ifdef GFX_LIB
						/* Graphic user interface */
						GOL_Procedures(); //Polling mode
#endif
					}
					
					if(PauseResumeStatus > 1 && PauseFlag == RESET) {
						AudioTimeOut++;
						if(AudioTimeOut > AUDIO_WAIT_TOL) {
							Audio_MAL_Stop();
							goto reinitPlayback;
						}
					}
				}
				if (blAudioPlaying) {
					/* Decoder one frame */
					if (MP3_DecodeFrame() == 0)
					{
						pcmframe_size = (mp3FrameInfo.bitsPerSample / 8) * mp3FrameInfo.outputSamps;
#ifdef USE_STM32_DAC
						PCMBuffConv((uint32_t *)audiobuf[DecodeBufIdx].BaseAddr, pcmframe_size, (uint32_t *)audiobuf[DecodeBufIdx].BaseAddr, S16_TO_U16);
#endif /* USE_STM32_DAC */
						MP3_AddAudioBuf (pcmframe_size);
					}
				}
				AudioTimeOut = 0;
			}
			XferCplt = 0;
		} else {
			break;
		}
	}
	Audio_MAL_Stop();
	
	// Close the decoder.
	MP3FreeDecoder(hMP3Decoder);
	
    /* Clear playing flag */
    blAudioPlaying = FALSE;
    return(0);
 }

#ifndef USE_AUDIO_DECODERS 
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
#endif /* USE_AUDIO_DECODERS */
 
 /**
  * @brief  Reset the mp3 player
  * @param  None
  * @retval None
  */
 void MP3Player_CallBack(void) {
	/* Clear playing flag */
	blAudioPlaying = FALSE;
	  
	/* Reset the mp3 player variables */
	PauseResumeStatus = 1;
	bytesLeft = 0;
	outOfData = 0;
	eofReached = 1;
	readPtr = readBuf = (uint8_t *)(READBUF_BASEADDR);
	pcmframe_size = 0;
 }
 
#endif /* SUPPORT_MP3 */
