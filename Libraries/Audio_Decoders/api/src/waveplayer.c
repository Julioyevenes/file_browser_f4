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
  
#ifdef SUPPORT_WAV

/* Includes ------------------------------------------------------------------*/
#include "waveplayer.h"
#include "AudioDecoders.h"
#include "AudioHardware.h"

/* Private types ------------------------------------------------------------*/
typedef enum
{
  LittleEndian,
  BigEndian
}Endianness;

typedef struct
{
  uint32_t  RIFFchunksize;
  uint16_t  FormatTag;
  uint16_t  NumChannels;
  uint32_t  SampleRate;
  uint32_t  ByteRate;
  uint16_t  BlockAlign;
  uint16_t  BitsPerSample;
  uint32_t  DataSize;
}
WAVE_FormatTypeDef;

typedef enum
{
  Valid_WAVE_File = 0,
  Unvalid_RIFF_ID,
  Unvalid_WAVE_Format,
  Unvalid_FormatChunk_ID,
  Unsupporetd_FormatTag,
  Unsupporetd_Number_Of_Channel,
  Unsupporetd_Sample_Rate,
  Unsupporetd_Bits_Per_Sample,
  Unvalid_DataChunk_ID,
  Unsupporetd_ExtraFormatBytes,
  Unvalid_FactChunk_ID
} ErrorCode;

/* Private constants --------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
#define  CHUNK_ID                            0x52494646  /* correspond to the letters 'RIFF' */
#define  FILE_FORMAT                         0x57415645  /* correspond to the letters 'WAVE' */
#define  FORMAT_ID                           0x666D7420  /* correspond to the letters 'fmt ' */
#define  DATA_ID                             0x64617461  /* correspond to the letters 'data' */
#define  FACT_ID                             0x66616374  /* correspond to the letters 'fact' */
#define  WAVE_FORMAT_PCM                     0x01
#define  FORMAT_CHNUK_SIZE                   0x10
#define  CHANNEL_MONO                        0x01
#define  CHANNEL_STEREO                      0x02
#define  SAMPLE_RATE_8000                    8000
#define  SAMPLE_RATE_11025                   11025
#define  SAMPLE_RATE_22050                   22050
#define  SAMPLE_RATE_44100                   44100
#define  BITS_PER_SAMPLE_8                   8
#define  BITS_PER_SAMPLE_16                  16

/* Private variables --------------------------------------------------------*/
static uint32_t wavelen = 0;
__IO ErrorCode WaveFileStatus = Unvalid_RIFF_ID;
__IO uint32_t WaveDataLength = 0;
WAVE_FormatTypeDef WAVE_Format;
extern BYTE blAudioPlaying;
extern __IO uint8_t volumeAudio;
__IO uint32_t WaveCounter;
uint8_t buffer_switch = 1;
extern __IO uint32_t XferCplt;
extern __IO uint8_t PauseResumeStatus;
extern FlagStatus PauseFlag;
static __IO uint32_t SpeechDataOffset = 0x00;
extern uint32_t AudioTimeOut;
uint32_t *buffer1 = NULL, *buffer2 = NULL;
 
/* Private function prototypes ----------------------------------------------*/
static ErrorCode WavePlayer_WaveParsing(uint32_t *FileLen);
uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat);

/**
  * @brief  Play wave from file
  * @param  pFile: Pointer to file object
  * @retval error: 0 no error
  *                1 error
  */
BYTE WavePlayBack(FSFILE *pFile) {
  /* Allocate memory for audio buffers */
  buffer1 = (uint32_t *) malloc(MEDIA_SECTOR_SIZE);
  buffer2 = (uint32_t *) malloc(MEDIA_SECTOR_SIZE);

  if(!buffer1 || !buffer2) {
     if(buffer1)
	    free(buffer1);
     if(buffer2)
	    free(buffer2);
     return(200); /* Memory allocation error */
  }

  /* Read data from the selected file */
  FSfread(buffer1, MEDIA_SECTOR_SIZE, 1, pFile);

  WaveFileStatus = WavePlayer_WaveParsing(&wavelen);

  if (WaveFileStatus == Valid_WAVE_File) /* the .WAV file is valid */
  {
	 /* Set WaveDataLenght to the Speech wave length */
	 WaveDataLength = WAVE_Format.DataSize;
  }
  else /* Unvalid wave file */
  {
	 /* Free memory used for audio buffers */
	 free(buffer1);
	 free(buffer2);

	 return(100);
  }

  /* Set playing flag */
  blAudioPlaying = TRUE;

reinitPlayback:

  AudioTimeOut = 0;

#ifdef USE_STM32_DAC
  Audio_MAL_Init(0, 0, WAVE_Format.SampleRate);
#endif /* USE_STM32_DAC */

#ifdef USE_CS43L22
  Audio_MAL_Init(OUTPUT_DEVICE_AUTO, volumeAudio, WAVE_Format.SampleRate);
#endif /* USE_CS43L22 */
  
  /* Get Data */
  FSfseek(pFile, WaveCounter, SEEK_SET);
  FSfread(buffer1, MEDIA_SECTOR_SIZE, 1, pFile);
  FSfread(buffer2, MEDIA_SECTOR_SIZE, 1, pFile);

#ifdef USE_STM32_DAC
  PCMBuffConv(buffer1, MEDIA_SECTOR_SIZE, buffer1, S16_TO_U16);
  PCMBuffConv(buffer2, MEDIA_SECTOR_SIZE, buffer2, S16_TO_U16);
#endif /* USE_STM32_DAC */

  /* Start playing wave */
  Audio_MAL_Play((uint32_t)buffer1, MEDIA_SECTOR_SIZE);
  
  buffer_switch = 1;
  XferCplt = 0;
  PauseResumeStatus = 1;
  
  while((WaveDataLength != 0) &&  MDD_MediaDetect()) {
  
#ifdef GFX_LIB
	/* Graphic user interface */
    GOL_Procedures(); //Polling mode
#endif

	/* Playing */
    if (blAudioPlaying) {
	
	  /* wait for DMA transfert complete */
      while((XferCplt == 0) &&  MDD_MediaDetect()) {
	  
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
	  
      AudioTimeOut = 0;
	  XferCplt = 0;
	  
	  if(buffer_switch == 0)
      {
        /* Play data from buffer1 */
        Audio_MAL_Play((uint32_t)buffer1, MEDIA_SECTOR_SIZE);
        /* Store data in buffer2 */
        FSfread(buffer2, MEDIA_SECTOR_SIZE, 1, pFile);

#ifdef USE_STM32_DAC
        PCMBuffConv(buffer2, MEDIA_SECTOR_SIZE, buffer2, S16_TO_U16);
#endif /* USE_STM32_DAC */

        buffer_switch = 1;
      }
      else
      {   
        /* Play data from buffer2 */
        Audio_MAL_Play((uint32_t)buffer2, MEDIA_SECTOR_SIZE);
        /* Store data in buffer1 */
        FSfread(buffer1, MEDIA_SECTOR_SIZE, 1, pFile);

#ifdef USE_STM32_DAC
        PCMBuffConv(buffer1, MEDIA_SECTOR_SIZE, buffer1, S16_TO_U16);
#endif /* USE_STM32_DAC */

        buffer_switch = 0;
      } 
	  
	}
	else 
    {
      WaveDataLength = 0;
      break;
    }	
  }
  
  /* Stop playback */
  Audio_MAL_Stop();
  
  /* Free memory used for audio buffers */
  free(buffer1);
  free(buffer2);

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
	if (WaveDataLength) WaveDataLength -= MEDIA_SECTOR_SIZE;
	if (WaveDataLength < MEDIA_SECTOR_SIZE) WaveDataLength = 0;
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
  * @brief  Reset the wave player
  * @param  None
  * @retval None
  */
void WavePlayer_CallBack(void) {
	  /* Clear playing flag */
      blAudioPlaying = FALSE;
	  
	  /* Reset the wave player variables */
	  PauseResumeStatus = 1;
	  WaveDataLength = 0;
}

/**
  * @brief  Checks the format of the .WAV file and gets information about
  *   the audio format. This is done by reading the value of a
  *   number of parameters stored in the file header and comparing
  *   these to the values expected authenticates the format of a
  *   standard .WAV  file (44 bytes will be read). If  it is a valid
  *   .WAV file format, it continues reading the header to determine
  *   the  audio format such as the sample rate and the sampled data
  *   size. If the audio format is supported by this application,
  *   it retrieves the audio format in WAVE_Format structure and
  *   returns a zero value. Otherwise the function fails and the
  *   return value is nonzero.In this case, the return value specifies
  *   the cause of  the function fails. The error codes that can be
  *   returned by this function are declared in the header file.
  * @param  None
  * @retval Zero value if the function succeed, otherwise it return
  *         a nonzero value which specifies the error code.
  */
static ErrorCode WavePlayer_WaveParsing(uint32_t *FileLen)
{
  uint32_t temp = 0x00;
  uint32_t extraformatbytes = 0;
  
  /* Read chunkID, must be 'RIFF' */
  temp = ReadUnit((uint8_t*)buffer1, 0, 4, BigEndian);
  if (temp != CHUNK_ID)
  {
    return(Unvalid_RIFF_ID);
  }
  
  /* Read the file length */
  WAVE_Format.RIFFchunksize = ReadUnit((uint8_t*)buffer1, 4, 4, LittleEndian);
  
  /* Read the file format, must be 'WAVE' */
  temp = ReadUnit((uint8_t*)buffer1, 8, 4, BigEndian);
  if (temp != FILE_FORMAT)
  {
    return(Unvalid_WAVE_Format);
  }
  
  /* Read the format chunk, must be'fmt ' */
  temp = ReadUnit((uint8_t*)buffer1, 12, 4, BigEndian);
  if (temp != FORMAT_ID)
  {
    return(Unvalid_FormatChunk_ID);
  }
  /* Read the length of the 'fmt' data, must be 0x10 -------------------------*/
  temp = ReadUnit((uint8_t*)buffer1, 16, 4, LittleEndian);
  if (temp != 0x10)
  {
    extraformatbytes = 1;
  }
  /* Read the audio format, must be 0x01 (PCM) */
  WAVE_Format.FormatTag = ReadUnit((uint8_t*)buffer1, 20, 2, LittleEndian);
  if (WAVE_Format.FormatTag != WAVE_FORMAT_PCM)
  {
    return(Unsupporetd_FormatTag);
  }
  
  /* Read the number of channels, must be 0x01 (Mono) or 0x02 (Stereo) */
  WAVE_Format.NumChannels = ReadUnit((uint8_t*)buffer1, 22, 2, LittleEndian);
  
  /* Read the Sample Rate */
  WAVE_Format.SampleRate = ReadUnit((uint8_t*)buffer1, 24, 4, LittleEndian);

  /* Read the Byte Rate */
  WAVE_Format.ByteRate = ReadUnit((uint8_t*)buffer1, 28, 4, LittleEndian);
  
  /* Read the block alignment */
  WAVE_Format.BlockAlign = ReadUnit((uint8_t*)buffer1, 32, 2, LittleEndian);
  
  /* Read the number of bits per sample */
  WAVE_Format.BitsPerSample = ReadUnit((uint8_t*)buffer1, 34, 2, LittleEndian);
  if (WAVE_Format.BitsPerSample != BITS_PER_SAMPLE_16)
  {
    return(Unsupporetd_Bits_Per_Sample);
  }
  SpeechDataOffset = 36;
  /* If there is Extra format bytes, these bytes will be defined in "Fact Chunk" */
  if (extraformatbytes == 1)
  {
    /* Read th Extra format bytes, must be 0x00 */
    temp = ReadUnit((uint8_t*)buffer1, 36, 2, LittleEndian);
    if (temp != 0x00)
    {
      return(Unsupporetd_ExtraFormatBytes);
    }
    /* Read the Fact chunk, must be 'fact' */
    temp = ReadUnit((uint8_t*)buffer1, 38, 4, BigEndian);
    if (temp != FACT_ID)
    {
      return(Unvalid_FactChunk_ID);
    }
    /* Read Fact chunk data Size */
    temp = ReadUnit((uint8_t*)buffer1, 42, 4, LittleEndian);
    
    SpeechDataOffset += 10 + temp;
  }
  /* Read the Data chunk, must be 'data' */
  temp = ReadUnit((uint8_t*)buffer1, SpeechDataOffset, 4, BigEndian);
  SpeechDataOffset += 4;
  if (temp != DATA_ID)
  {
    return(Unvalid_DataChunk_ID);
  }
  
  /* Read the number of sample data */
  WAVE_Format.DataSize = ReadUnit((uint8_t*)buffer1, SpeechDataOffset, 4, LittleEndian);
  SpeechDataOffset += 4;
  WaveCounter =  SpeechDataOffset;
  return(Valid_WAVE_File);
}

/**
* @brief  Reads a number of bytes and reorder them in Big
*         or little endian.
* @param  NbrOfBytes: number of bytes to read.
*         This parameter must be a number between 1 and 4.
* @param  ReadAddr: external memory address to read from.
* @param  Endians: specifies the bytes endianness.
*         This parameter can be one of the following values:
*             - LittleEndian
*             - BigEndian
* @retval Bytes read from the SPI Flash.
*/
uint32_t ReadUnit(uint8_t *buffer, uint8_t idx, uint8_t NbrOfBytes, Endianness BytesFormat)
{
  uint32_t index = 0;
  uint32_t temp = 0;
  
  for (index = 0; index < NbrOfBytes; index++)
  {
    temp |= buffer[idx + index] << (index * 8);
  }
  
  if (BytesFormat == BigEndian)
  {
    temp = __REV(temp);
  }
  return temp;
}

#endif /* SUPPORT_WAV */
