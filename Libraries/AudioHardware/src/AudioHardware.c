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
#include "AudioHardware.h"
#ifdef USE_STM32_DAC
	#include "stm32f4xx_dac.h"
#endif /* USE_STM32_DAC */
#ifdef USE_CS43L22
	#include "stm32f4xx_i2c.h"
	#include "stm32f4xx_spi.h"
#endif /* USE_CS43L22 */

/* Private types ------------------------------------------------------------*/
/* Private constants --------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
#ifdef USE_CS43L22
/**
  * @brief  Codec Hardware parameters
  */
/* I2C clock speed configuration (in Hz) 
  WARNING: 
   Make sure that this define is not already declared in other files (ie. 
  stm322xg_eval.h file). It can be used in parallel by other modules. */
#ifndef I2C_SPEED
 #define I2C_SPEED                        100000
#endif /* I2C_SPEED */

/* The 7 bits Codec address (sent through I2C interface) */
#define CODEC_ADDRESS                   0x94  /* b00100111 */

/* Uncomment defines below to select standard for audio communication between 
  Codec and I2S peripheral */
#define I2S_STANDARD_PHILLIPS
/* #define I2S_STANDARD_MSB */
/* #define I2S_STANDARD_LSB */

/* Codec audio Standards */
#ifdef I2S_STANDARD_PHILLIPS
 #define  CODEC_STANDARD                0x04
 #define I2S_STANDARD                   I2S_Standard_Phillips         
#elif defined(I2S_STANDARD_MSB)
 #define  CODEC_STANDARD                0x00
 #define I2S_STANDARD                   I2S_Standard_MSB    
#elif defined(I2S_STANDARD_LSB)
 #define  CODEC_STANDARD                0x08
 #define I2S_STANDARD                   I2S_Standard_LSB    
#else 
 #error "Error: No audio communication standard selected !"
#endif /* I2S_STANDARD */

/* Uncomment the defines below to select if the Master clock mode should be 
  enabled or not */
#define CODEC_MCLK_ENABLED
/* #define CODEC_MCLK_DISABLED */

/* Mask for the bit EN of the I2S CFGR register */
#define I2S_ENABLE_MASK                 0x0400

/* Delay for the Codec to be correctly reset */
#define CODEC_RESET_DELAY               0x4FFF

/* Maximum Timeout values for flags and events waiting loops. These timeouts are
   not based on accurate values, they just guarantee that the application will 
   not remain stuck if the I2C communication is corrupted.
   You may modify these timeout values depending on CPU frequency and application
   conditions (interrupts routines ...). */   
#define CODEC_FLAG_TIMEOUT             ((uint32_t)0x1000)
#define CODEC_LONG_TIMEOUT             ((uint32_t)(300 * CODEC_FLAG_TIMEOUT))

/* Uncomment this line to enable verifying data sent to codec after each write 
  operation */
#define VERIFY_WRITTENDATA

/* Audio Reset Pin definition */
#define AUDIO_RESET_GPIO_CLK           RCC_AHB1Periph_GPIOD  
#define AUDIO_RESET_PIN                GPIO_Pin_4    
#define AUDIO_RESET_GPIO               GPIOD 
                 
/* I2S peripheral configuration defines */
#define CODEC_I2S                      SPI3
#define CODEC_I2S_CLK                  RCC_APB1Periph_SPI3
#define CODEC_I2S_ADDRESS              0x40003C0C
#define CODEC_I2S_GPIO_AF              GPIO_AF_SPI3
#define CODEC_I2S_IRQ                  SPI3_IRQn
#define CODEC_I2S_GPIO_CLOCK           (RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOA)
#define CODEC_I2S_WS_PIN               GPIO_Pin_4
#define CODEC_I2S_SCK_PIN              GPIO_Pin_10
#define CODEC_I2S_SD_PIN               GPIO_Pin_12
#define CODEC_I2S_MCK_PIN              GPIO_Pin_7
#define CODEC_I2S_WS_PINSRC            GPIO_PinSource4
#define CODEC_I2S_SCK_PINSRC           GPIO_PinSource10
#define CODEC_I2S_SD_PINSRC            GPIO_PinSource12
#define CODEC_I2S_MCK_PINSRC           GPIO_PinSource7
#define CODEC_I2S_GPIO                 GPIOC
#define CODEC_I2S_WS_GPIO              GPIOA
#define CODEC_I2S_MCK_GPIO             GPIOC
#define Audio_I2S_IRQHandler           SPI3_IRQHandler

/* MAL DMA Stream definitions */
#define AUDIO_MAL_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_MAL_DMA_STREAM           DMA1_Stream7
#define AUDIO_MAL_DMA_DREG             CODEC_I2S_ADDRESS
#define AUDIO_MAL_DMA_CHANNEL          DMA_Channel_0
#define AUDIO_MAL_DMA_IRQ              DMA1_Stream7_IRQn
#define AUDIO_MAL_DMA_FLAG_TC          DMA_FLAG_TCIF7
#define AUDIO_MAL_DMA_FLAG_HT          DMA_FLAG_HTIF7
#define AUDIO_MAL_DMA_FLAG_FE          DMA_FLAG_FEIF7
#define AUDIO_MAL_DMA_FLAG_TE          DMA_FLAG_TEIF7
#define AUDIO_MAL_DMA_FLAG_DME         DMA_FLAG_DMEIF7

#define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord
#define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord

#define Audio_MAL_I2S_IRQHandler       DMA1_Stream7_IRQHandler

/* I2C peripheral configuration defines (control interface of the audio codec) */
#define CODEC_I2C                      I2C1
#define CODEC_I2C_CLK                  RCC_APB1Periph_I2C1
#define CODEC_I2C_GPIO_CLOCK           RCC_AHB1Periph_GPIOB
#define CODEC_I2C_GPIO_AF              GPIO_AF_I2C1
#define CODEC_I2C_GPIO                 GPIOB
#define CODEC_I2C_SCL_PIN              GPIO_Pin_6
#define CODEC_I2C_SDA_PIN              GPIO_Pin_9
#define CODEC_I2S_SCL_PINSRC           GPIO_PinSource6
#define CODEC_I2S_SDA_PINSRC           GPIO_PinSource9

/* Codec POWER DOWN modes */
#define CODEC_PDWN_HW                 1
#define CODEC_PDWN_SW                 2

/**
  * @brief  Codec macros
  */
#define VOLUME_CONVERT(x)    ((Volume > 100)? 100:((uint8_t)((Volume * 255) / 100)))
#endif /* USE_CS43L22 */

#ifdef USE_STM32_DAC
/**
  * @brief  DAC Hardware parameters
  */
/* Registers address */
#define DAC_DHR8R1_ADDRESS  		   0x40007410
#define DAC_DHR8R2_ADDRESS  		   0x4000741C
#define DAC_DHR12RD_ADDRESS            0x40007420
#define DAC_DHR12LD_ADDRESS            0x40007424
#define DAC_DHR8RD_ADDRESS 		       0x40007428

/* MAL DMA Stream definitions */
#define AUDIO_MAL_DMA_CLOCK            RCC_AHB1Periph_DMA1
#define AUDIO_MAL_DMA_STREAM           DMA1_Stream5
#define AUDIO_MAL_DMA_DREG             DAC_DHR12LD_ADDRESS
#define AUDIO_MAL_DMA_CHANNEL          DMA_Channel_7
#define AUDIO_MAL_DMA_IRQ              DMA1_Stream5_IRQn
#define AUDIO_MAL_DMA_FLAG_TC          DMA_FLAG_TCIF5
#define AUDIO_MAL_DMA_FLAG_HT          DMA_FLAG_HTIF5
#define AUDIO_MAL_DMA_FLAG_FE          DMA_FLAG_FEIF5
#define AUDIO_MAL_DMA_FLAG_TE          DMA_FLAG_TEIF5
#define AUDIO_MAL_DMA_FLAG_DME         DMA_FLAG_DMEIF5

#define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_Word
/* #define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_HalfWord */
/* #define AUDIO_MAL_DMA_PERIPH_DATA_SIZE DMA_PeripheralDataSize_Byte */
#define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_Word
/* #define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_HalfWord */
/* #define AUDIO_MAL_DMA_MEM_DATA_SIZE    DMA_MemoryDataSize_Byte */

#define Audio_MAL_DAC_IRQHandler       DMA1_Stream5_IRQHandler
#endif /* USE_STM32_DAC */

/**
  * @brief  Common DMA Configuration parameters
  */
/* For the DMA modes select the interrupt that will be used */
#define AUDIO_MAL_DMA_IT_TC_EN        /* Uncomment this line to enable DMA Transfer Complete interrupt */
/* #define AUDIO_MAL_DMA_IT_HT_EN */  /* Uncomment this line to enable DMA Half Transfer Complete interrupt */
/* #define AUDIO_MAL_DMA_IT_TE_EN */  /* Uncomment this line to enable DMA Transfer Error interrupt */

/* Select the interrupt preemption priority and subpriority for the DMA interrupt */
#define EVAL_AUDIO_IRQ_PREPRIO           0   /* Select the preemption priority level(0 is the highest) */
#define EVAL_AUDIO_IRQ_SUBRIO            0   /* Select the sub-priority level (0 is the highest) */

/**
  * @brief  Common function defines
  */
/* PLAYBACK commands */
#define AUDIO_PAUSE                   0
#define AUDIO_RESUME                  1

/* MUTE commands */
#define AUDIO_MUTE_ON                 1
#define AUDIO_MUTE_OFF                0

/* Private variables --------------------------------------------------------*/
DMA_InitTypeDef DMA_InitStructure;
#ifdef USE_CS43L22
	__IO uint32_t  CODECTimeout = CODEC_LONG_TIMEOUT;   
	__IO uint8_t OutputDev = 0;
#endif /* USE_CS43L22 */

/* Private function prototypes ----------------------------------------------*/
static void Audio_MAL_IRQHandler(void);
static void DMA_Config(void);
#ifdef USE_STM32_DAC
	static void TIM6_Config(uint32_t SampleRate);
	static void DAC_Config(void);
#endif /* USE_STM32_DAC */
#ifdef USE_CS43L22
	static uint32_t Codec_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq);
	static uint32_t Codec_DeInit(void);
	static uint32_t Codec_PauseResume(uint32_t Cmd);
	static uint32_t Codec_Stop(uint32_t Cmd);
	static uint32_t Codec_VolumeCtrl(uint8_t Volume);
	static uint32_t Codec_Mute(uint32_t Cmd);
	static void     Codec_CtrlInterface_Init(void);
	static void     Codec_CtrlInterface_DeInit(void);
	static void     Codec_AudioInterface_Init(uint32_t AudioFreq);
	static void     Codec_AudioInterface_DeInit(void);
	static void     Codec_Reset(void);
	static uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue);
	static uint32_t Codec_ReadRegister(uint8_t RegisterAddr);
	static void     Codec_GPIO_Init(void);
	static void     Codec_GPIO_DeInit(void);
	static void     Delay(__IO uint32_t nCount);
#endif /* USE_CS43L22 */

/**
  * @brief  
  * @param  
  * @retval 
  */
void Audio_MAL_IRQHandler(void)
{
#ifndef AUDIO_MAL_MODE_NORMAL
#endif /* AUDIO_MAL_MODE_NORMAL */
  
#ifdef AUDIO_MAL_DMA_IT_TC_EN
   /* Transfer complete interrupt */
   if (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC) != RESET)
   {        
#ifdef AUDIO_MAL_MODE_NORMAL
      /* Disable the I2S DMA Stream */
      DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);   
      
      /* Clear the Interrupt flag */
      DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC);       
      
      /* Manage the remaining file size and new address offset: This function 
      should be coded by user (its prototype is already declared in stm32f4_discovery_audio_codec.h) */  
      EVAL_AUDIO_TransferComplete_CallBack(0, 0);
    
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
      /* Manage the remaining file size and new address offset: This function 
         should be coded by user (its prototype is already declared in stm32f4_discovery_audio_codec.h) */  
      EVAL_AUDIO_TransferComplete_CallBack(0, 0);    
    
      /* Clear the Interrupt flag */
      DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC);
#endif /* AUDIO_MAL_MODE_NORMAL */  
   }
#endif /* AUDIO_MAL_DMA_IT_TC_EN */

#ifdef AUDIO_MAL_DMA_IT_HT_EN  
   /* Half Transfer complete interrupt */
   if (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_HT) != RESET)
   {
      /* Manage the remaining file size and new address offset: This function 
         should be coded by user (its prototype is already declared in stm32f4_discovery_audio_codec.h) */  
      EVAL_AUDIO_HalfTransfer_CallBack(0, 0);    
   
      /* Clear the Interrupt flag */
      DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_HT);    
   }
#endif /* AUDIO_MAL_DMA_IT_HT_EN */
  
#ifdef AUDIO_MAL_DMA_IT_TE_EN  
   /* FIFO Error interrupt */
   if ((DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TE) != RESET) || \
      (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_FE) != RESET) || \
      (DMA_GetFlagStatus(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_DME) != RESET))
    
   {
      /* Manage the error generated on DMA FIFO: This function 
         should be coded by user (its prototype is already declared in stm32f4_discovery_audio_codec.h) */  
      EVAL_AUDIO_Error_CallBack(0);    
    
      /* Clear the Interrupt flag */
      DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TE | AUDIO_MAL_DMA_FLAG_FE | \
                                        AUDIO_MAL_DMA_FLAG_DME);
   }  
#endif /* AUDIO_MAL_DMA_IT_TE_EN */
}

#ifdef USE_CS43L22
/**
  * @brief  This function handles main I2S interrupt. 
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
void Audio_MAL_I2S_IRQHandler(void)
{ 
  Audio_MAL_IRQHandler();
}
#endif /* USE_CS43L22 */

#ifdef USE_STM32_DAC
/**
  * @brief  This function handles main DAC interrupt. 
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
void Audio_MAL_DAC_IRQHandler(void)
{ 
  Audio_MAL_IRQHandler();
}
#endif /* USE_STM32_DAC */

/**
  * @brief  
  * @param  
  * @retval 
  */
void Audio_MAL_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t SampleRate)
{
#ifdef USE_CS43L22
   /* Perform low layer Codec initialization */
   Codec_Init(OutputDevice, VOLUME_CONVERT(Volume), SampleRate);
   
   /* Configure DMA to handle data flow to I2S */
   DMA_Config();
#endif /* USE_CS43L22 */

#ifdef USE_STM32_DAC
   /* Configure audio frequency */
   TIM6_Config(SampleRate);

   /* Configure the STM32 DAC to generate audio analog signal */
   DAC_Config();

   /* Configure DMA to handle data flow to DAC */
   DMA_Config();

   /* Enable DMA request for both DAC Channels */
   DAC_DMACmd(DAC_Channel_1, ENABLE);
   DAC_DMACmd(DAC_Channel_2, ENABLE);
#endif /* USE_STM32_DAC */
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void Audio_MAL_DeInit(void)
{
#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
   NVIC_InitTypeDef NVIC_InitStructure;  
  
   /* Deinitialize the NVIC interrupt for the DMA Stream */
   NVIC_InitStructure.NVIC_IRQChannel = AUDIO_MAL_DMA_IRQ;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
   NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
   NVIC_Init(&NVIC_InitStructure);  
#endif 
  
   /* Disable the DMA stream before the deinit */
   DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
  
   /* Deinitialize the DMA Stream */
   DMA_DeInit(AUDIO_MAL_DMA_STREAM);
  
   /* 
      The DMA clock is not disabled, since it can be used by other streams 
                                                                           */

#ifdef USE_CS43L22
   /* DeInitialize Codec */  
   Codec_DeInit();
#endif /* USE_CS43L22 */
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void Audio_MAL_Play(uint32_t Addr, uint32_t Size)
{
   /* Configure the buffer address and size */
   DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)Addr;
   if(AUDIO_MAL_DMA_MEM_DATA_SIZE == DMA_MemoryDataSize_Word)
   	DMA_InitStructure.DMA_BufferSize = (uint32_t)(Size / 4);
   if(AUDIO_MAL_DMA_MEM_DATA_SIZE == DMA_MemoryDataSize_HalfWord)
   	DMA_InitStructure.DMA_BufferSize = (uint32_t)(Size / 2);
   if(AUDIO_MAL_DMA_MEM_DATA_SIZE == DMA_MemoryDataSize_Byte)
   	DMA_InitStructure.DMA_BufferSize = (uint32_t)Size;
    
   /* Configure the DMA Stream with the new parameters */
   DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);
    
   /* Enable the DMA Stream*/
   DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
   
#ifdef USE_CS43L22
   /* If the I2S peripheral is still not enabled, enable it */
   if ((CODEC_I2S->I2SCFGR & I2S_ENABLE_MASK) == 0)
   {
     I2S_Cmd(CODEC_I2S, ENABLE);
   }
#endif /* USE_CS43L22 */
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void Audio_MAL_PauseResume(uint32_t Cmd)
{
#ifdef USE_CS43L22
	/* Call the Audio Codec Pause/Resume function */
	Codec_PauseResume(Cmd);
#endif /* USE_CS43L22 */

   /* Pause the audio file playing */
   if (Cmd == AUDIO_PAUSE)
   {
      /* Disable the DMA request */
#ifdef USE_STM32_DAC
      DAC_DMACmd(DAC_Channel_1, DISABLE);
      DAC_DMACmd(DAC_Channel_2, DISABLE);
#endif /* USE_STM32_DAC */

#ifdef USE_CS43L22
	  SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, DISABLE);
#endif /* USE_CS43L22 */

      /* Pause the DMA Stream 
          Note. For the STM32F40x devices, the DMA implements a pause feature, 
                by disabling the stream, all configuration is preserved and data 
                transfer is paused till the next enable of the stream.
                This feature is not available on STM32F40x devices. */
      DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
   }
   else /* AUDIO_RESUME */
   {
      /* Enable the DMA request */
#ifdef USE_STM32_DAC
      DAC_DMACmd(DAC_Channel_1, ENABLE);
      DAC_DMACmd(DAC_Channel_2, ENABLE);
#endif /* USE_STM32_DAC */

#ifdef USE_CS43L22
	  SPI_I2S_DMACmd(CODEC_I2S, SPI_I2S_DMAReq_Tx, ENABLE);
#endif /* USE_CS43L22 */
  
      /* Resume the DMA Stream 
          Note. For the STM32F40x devices, the DMA implements a pause feature, 
                by disabling the stream, all configuration is preserved and data 
                transfer is paused till the next enable of the stream.
                This feature is not available on STM32F40x devices. */
      DMA_Cmd(AUDIO_MAL_DMA_STREAM, ENABLE);
	  
#ifdef USE_CS43L22
      /* If the I2S peripheral is still not enabled, enable it */
      if ((CODEC_I2S->I2SCFGR & I2S_ENABLE_MASK) == 0)
      {
        I2S_Cmd(CODEC_I2S, ENABLE);
      }
#endif /* USE_CS43L22 */
   } 
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void Audio_MAL_Stop(void)
{
#ifdef USE_CS43L22
   /* Call Audio Codec Stop function */
   Codec_Stop(CODEC_PDWN_HW);
#endif /* USE_CS43L22 */

   /* Stop and disable the DMA stream */
   DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);

   /* Clear all the DMA flags for the next transfer */
   DMA_ClearFlag(AUDIO_MAL_DMA_STREAM, AUDIO_MAL_DMA_FLAG_TC |AUDIO_MAL_DMA_FLAG_HT | \
                 AUDIO_MAL_DMA_FLAG_FE | AUDIO_MAL_DMA_FLAG_TE);

#ifdef USE_STM32_DAC				 
   /* Disable the DMA request */
   DAC_DMACmd(DAC_Channel_1, DISABLE);
   DAC_DMACmd(DAC_Channel_2, DISABLE);

   /* Disable DAC */
   DAC_Cmd(DAC_Channel_1, DISABLE);
   DAC_Cmd(DAC_Channel_2, DISABLE);
#endif /* USE_STM32_DAC */

#ifdef USE_CS43L22
   /* In all modes, disable the I2S peripheral */
   I2S_Cmd(CODEC_I2S, DISABLE);
#endif /* USE_CS43L22 */
}

#ifdef USE_CS43L22
/**
  * @brief  Controls the current audio volume level.
  * @param  Volume: Volume level to be set in percentage from 0% to 100% (0 for 
  *         Mute and 100 for Max volume level).
  * @retval 
  */
void Audio_MAL_VolumeCtl(uint8_t Volume)
{
  /* Call the codec volume control function with converted volume value */
  Codec_VolumeCtrl(VOLUME_CONVERT(Volume));
}

/**
  * @brief  Enables or disables the MUTE mode by software.
  * @param  Command: could be AUDIO_MUTE_ON to mute sound or AUDIO_MUTE_OFF to 
  *         unmute the codec and restore previous volume level.
  * @retval 
  */
void Audio_MAL_Mute(uint32_t Cmd)
{
  /* Call the Codec Mute function */
  Codec_Mute(Cmd);
}
#endif /* USE_CS43L22 */

#ifdef USE_STM32_DAC
/**
  * @brief  
  * @param  
  * @retval 
  */
static void TIM6_Config(uint32_t SampleRate)
{ 
   TIM_TimeBaseInitTypeDef    TIM_TimeBaseStructure;
   RCC_ClocksTypeDef 		  RCC_Freqs;

   /* TIM6 Periph clock enable */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

   /* Get the core clock */
   RCC_GetClocksFreq(&RCC_Freqs);

   /* Time base configuration */
   TIM_TimeBaseStructInit(&TIM_TimeBaseStructure);
   TIM_TimeBaseStructure.TIM_Period = ((RCC_Freqs.HCLK_Frequency / 4) * 2) / SampleRate - 1;
   TIM_TimeBaseStructure.TIM_Prescaler = 0;
   TIM_TimeBaseStructure.TIM_ClockDivision = 0;
   TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up; 
   TIM_TimeBaseInit(TIM6, &TIM_TimeBaseStructure);

   /* TIM6 TRGO selection */
   TIM_SelectOutputTrigger(TIM6, TIM_TRGOSource_Update);
  
   /* TIM6 enable counter */
   TIM_Cmd(TIM6, ENABLE);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
static void DAC_Config(void)
{
   GPIO_InitTypeDef GPIO_InitStructure;
   DAC_InitTypeDef  DAC_InitStructure;

   /* GPIOA clock enable (to be used with DAC) */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

   /* DAC Periph clock enable */
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_DAC, ENABLE);

   /* DAC output configuration */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AN;
   GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_Init(GPIOA, &GPIO_InitStructure);
   
   /* DAC Configuration */
   DAC_InitStructure.DAC_Trigger = DAC_Trigger_T6_TRGO;
   DAC_InitStructure.DAC_WaveGeneration = DAC_WaveGeneration_None;
   DAC_InitStructure.DAC_OutputBuffer = DAC_OutputBuffer_Enable;
   DAC_Init(DAC_Channel_1, &DAC_InitStructure);
   DAC_Init(DAC_Channel_2, &DAC_InitStructure);

   /* Enable DAC */
   DAC_Cmd(DAC_Channel_1, ENABLE);
   DAC_Cmd(DAC_Channel_2, ENABLE);
}
#endif /* USE_STM32_DAC */

#ifdef USE_CS43L22
/**
  * @brief  Initializes the audio codec and all related interfaces (control 
  *         interface: I2C and audio interface: I2S)
  * @param  OutputDevice: can be OUTPUT_DEVICE_SPEAKER, OUTPUT_DEVICE_HEADPHONE,
  *                       OUTPUT_DEVICE_BOTH or OUTPUT_DEVICE_AUTO .
  * @param  Volume: Initial volume level (from 0 (Mute) to 100 (Max))
  * @param  AudioFreq: Audio frequency used to play the audio stream.
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_Init(uint16_t OutputDevice, uint8_t Volume, uint32_t AudioFreq)
{
  uint32_t counter = 0; 

  /* Configure the Codec related IOs */
  Codec_GPIO_Init();   
  
  /* Reset the Codec Registers */
  Codec_Reset();

  /* Initialize the Control interface of the Audio Codec */
  Codec_CtrlInterface_Init();     
  
  /* Keep Codec powered OFF */
  counter += Codec_WriteRegister(0x02, 0x01);  
      
  counter += Codec_WriteRegister(0x04, 0xAF); /* SPK always OFF & HP always ON */
  OutputDev = 0xAF;
  
  /* Clock configuration: Auto detection */  
  counter += Codec_WriteRegister(0x05, 0x81);
  
  /* Set the Slave Mode and the audio Standard */  
  counter += Codec_WriteRegister(0x06, CODEC_STANDARD);
      
  /* Set the Master volume */
  Codec_VolumeCtrl(Volume);

  /* Power on the Codec */
  counter += Codec_WriteRegister(0x02, 0x9E);  
  
  /* Additional configuration for the CODEC. These configurations are done to reduce
      the time needed for the Codec to power off. If these configurations are removed, 
      then a long delay should be added between powering off the Codec and switching 
      off the I2S peripheral MCLK clock (which is the operating clock for Codec).
      If this delay is not inserted, then the codec will not shut down properly and
      it results in high noise after shut down. */
  
  /* Disable the analog soft ramp */
  counter += Codec_WriteRegister(0x0A, 0x00);
 
  /* Disable the digital soft ramp */
  counter += Codec_WriteRegister(0x0E, 0x04);

  /* Disable the limiter attack level */
  counter += Codec_WriteRegister(0x27, 0x00);
  /* Adjust Bass and Treble levels */
  counter += Codec_WriteRegister(0x1F, 0x0F);
  /* Adjust PCM volume level */
  counter += Codec_WriteRegister(0x1A, 0x0A);
  counter += Codec_WriteRegister(0x1B, 0x0A);

  /* Configure the I2S peripheral */
  Codec_AudioInterface_Init(AudioFreq);  
  
  /* Return communication control value */
  return counter;  
}

/**
  * @brief  Restore the audio codec state to default state and free all used 
  *         resources.
  * @param  None
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_DeInit(void)
{
  uint32_t counter = 0; 

  /* Reset the Codec Registers */
  Codec_Reset();  
  
  /* Keep Codec powered OFF */
  counter += Codec_WriteRegister(0x02, 0x01);    
  
  /* Deinitialize all use GPIOs */
  Codec_GPIO_DeInit();

  /* Disable the Codec control interface */
  Codec_CtrlInterface_DeInit();
  
  /* Deinitialize the Codec audio interface (I2S) */
  Codec_AudioInterface_DeInit(); 
  
  /* Return communication control value */
  return counter;  
}

/**
  * @brief  Pauses and resumes playing on the audio codec.
  * @param  Cmd: AUDIO_PAUSE (or 0) to pause, AUDIO_RESUME (or any value different
  *         from 0) to resume. 
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_PauseResume(uint32_t Cmd)
{
  uint32_t counter = 0;   
  
  /* Pause the audio file playing */
  if (Cmd == AUDIO_PAUSE)
  { 
    /* Mute the output first */
    counter += Codec_Mute(AUDIO_MUTE_ON);

    /* Put the Codec in Power save mode */    
    counter += Codec_WriteRegister(0x02, 0x01);    
  }
  else /* AUDIO_RESUME */
  {
    /* Unmute the output first */
    counter += Codec_Mute(AUDIO_MUTE_OFF);
    
    counter += Codec_WriteRegister(0x04, OutputDev);
    
    /* Exit the Power save mode */
    counter += Codec_WriteRegister(0x02, 0x9E); 
  }

  return counter;
}

/**
  * @brief  Stops audio Codec playing. It powers down the codec.
  * @param  CodecPdwnMode: selects the  power down mode.
  *          - CODEC_PDWN_SW: only mutes the audio codec. When resuming from this 
  *                           mode the codec keeps the previous initialization
  *                           (no need to re-Initialize the codec registers).
  *          - CODEC_PDWN_HW: Physically power down the codec. When resuming from this
  *                           mode, the codec is set to default configuration 
  *                           (user should re-Initialize the codec in order to 
  *                            play again the audio stream).
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_Stop(uint32_t CodecPdwnMode)
{
  uint32_t counter = 0;   

  /* Mute the output first */
  Codec_Mute(AUDIO_MUTE_ON);
  
  if (CodecPdwnMode == CODEC_PDWN_SW)
  {    
    /* Power down the DAC and the speaker (PMDAC and PMSPK bits)*/
    counter += Codec_WriteRegister(0x02, 0x9F);
  }
  else /* CODEC_PDWN_HW */
  { 
    /* Power down the DAC components */
    counter += Codec_WriteRegister(0x02, 0x9F);
    
    /* Wait at least 100us */
    Delay(0xFFF);
    
    /* Reset The pin */
    GPIO_WriteBit(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, Bit_RESET);
  }
  
  return counter;    
}

/**
  * @brief  Sets higher or lower the codec volume level.
  * @param  Volume: a byte value from 0 to 255 (refer to codec registers 
  *         description for more details).
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_VolumeCtrl(uint8_t Volume)
{
  uint32_t counter = 0;
  
  if (Volume > 0xE6)
  {
    /* Set the Master volume */
    counter += Codec_WriteRegister(0x20, Volume - 0xE7); 
    counter += Codec_WriteRegister(0x21, Volume - 0xE7);     
  }
  else
  {
    /* Set the Master volume */
    counter += Codec_WriteRegister(0x20, Volume + 0x19); 
    counter += Codec_WriteRegister(0x21, Volume + 0x19); 
  }

  return counter;  
}

/**
  * @brief  Enables or disables the mute feature on the audio codec.
  * @param  Cmd: AUDIO_MUTE_ON to enable the mute or AUDIO_MUTE_OFF to disable the
  *             mute mode.
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_Mute(uint32_t Cmd)
{
  uint32_t counter = 0;  
  
  /* Set the Mute mode */
  if (Cmd == AUDIO_MUTE_ON)
  {
    counter += Codec_WriteRegister(0x04, 0xFF);
  }
  else /* AUDIO_MUTE_OFF Disable the Mute */
  {
    counter += Codec_WriteRegister(0x04, OutputDev);
  }
  
  return counter; 
}

/**
  * @brief  Resets the audio codec. It restores the default configuration of the 
  *         codec (this function shall be called before initializing the codec).
  * @note   This function calls an external driver function: The IO Expander driver.
  * @param  None
  * @retval None
  */
static void Codec_Reset(void)
{
  /* Power Down the codec */
  GPIO_WriteBit(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, Bit_RESET);

  /* wait for a delay to insure registers erasing */
  Delay(CODEC_RESET_DELAY); 
  
  /* Power on the codec */
  GPIO_WriteBit(AUDIO_RESET_GPIO, AUDIO_RESET_PIN, Bit_SET);
}

/**
  * @brief  Writes a Byte to a given register into the audio codec through the 
            control interface (I2C)
  * @param  RegisterAddr: The address (location) of the register to be written.
  * @param  RegisterValue: the Byte value to be written into destination register.
  * @retval 0 if correct communication, else wrong communication
  */
static uint32_t Codec_WriteRegister(uint8_t RegisterAddr, uint8_t RegisterValue)
{
  uint32_t result = 0;

  /*!< While the bus is busy */
  CODECTimeout = CODEC_LONG_TIMEOUT;
  while(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BUSY))
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }
  
  /* Start the config sequence */
  I2C_GenerateSTART(CODEC_I2C, ENABLE);

  /* Test on EV5 and clear it */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while (!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }
  
  /* Transmit the slave address and enable writing operation */
  I2C_Send7bitAddress(CODEC_I2C, CODEC_ADDRESS, I2C_Direction_Transmitter);

  /* Test on EV6 and clear it */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while (!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }

  /* Transmit the first address for write operation */
  I2C_SendData(CODEC_I2C, RegisterAddr);

  /* Test on EV8 and clear it */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while (!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_BYTE_TRANSMITTING))
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }
  
  /* Prepare the register value to be sent */
  I2C_SendData(CODEC_I2C, RegisterValue);
  
  /*!< Wait till all data have been physically transferred on the bus */
  CODECTimeout = CODEC_LONG_TIMEOUT;
  while(!I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BTF))
  {
    if((CODECTimeout--) == 0) Codec_TIMEOUT_UserCallback();
  }
  
  /* End the configuration sequence */
  I2C_GenerateSTOP(CODEC_I2C, ENABLE);  
  
#ifdef VERIFY_WRITTENDATA
  /* Verify that the data has been correctly written */  
  result = (Codec_ReadRegister(RegisterAddr) == RegisterValue)? 0:1;
#endif /* VERIFY_WRITTENDATA */

  /* Return the verifying value: 0 (Passed) or 1 (Failed) */
  return result;  
}

/**
  * @brief  Reads and returns the value of an audio codec register through the
  *         control interface (I2C).
  * @param  RegisterAddr: Address of the register to be read.
  * @retval Value of the register to be read or dummy value if the communication
  *         fails.
  */
static uint32_t Codec_ReadRegister(uint8_t RegisterAddr)
{
  uint32_t result = 0;

  /*!< While the bus is busy */
  CODECTimeout = CODEC_LONG_TIMEOUT;
  while(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BUSY))
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }
  
  /* Start the config sequence */
  I2C_GenerateSTART(CODEC_I2C, ENABLE);

  /* Test on EV5 and clear it */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while (!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }
  
  /* Transmit the slave address and enable writing operation */
  I2C_Send7bitAddress(CODEC_I2C, CODEC_ADDRESS, I2C_Direction_Transmitter);

  /* Test on EV6 and clear it */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while (!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }

  /* Transmit the register address to be read */
  I2C_SendData(CODEC_I2C, RegisterAddr);

  /* Test on EV8 and clear it */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while (I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_BTF) == RESET)
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }
  
  /*!< Send START condition a second time */  
  I2C_GenerateSTART(CODEC_I2C, ENABLE);
  
  /*!< Test on EV5 and clear it (cleared by reading SR1 then writing to DR) */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while(!I2C_CheckEvent(CODEC_I2C, I2C_EVENT_MASTER_MODE_SELECT))
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  } 
  
  /*!< Send Codec address for read */
  I2C_Send7bitAddress(CODEC_I2C, CODEC_ADDRESS, I2C_Direction_Receiver);  
  
  /* Wait on ADDR flag to be set (ADDR is still not cleared at this level */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_ADDR) == RESET)
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }     
  
  /*!< Disable Acknowledgment */
  I2C_AcknowledgeConfig(CODEC_I2C, DISABLE);   
  
  /* Clear ADDR register by reading SR1 then SR2 register (SR1 has already been read) */
  (void)CODEC_I2C->SR2;
  
  /*!< Send STOP Condition */
  I2C_GenerateSTOP(CODEC_I2C, ENABLE);
  
  /* Wait for the byte to be received */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while(I2C_GetFlagStatus(CODEC_I2C, I2C_FLAG_RXNE) == RESET)
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }
  
  /*!< Read the byte received from the Codec */
  result = I2C_ReceiveData(CODEC_I2C);
  
  /* Wait to make sure that STOP flag has been cleared */
  CODECTimeout = CODEC_FLAG_TIMEOUT;
  while(CODEC_I2C->CR1 & I2C_CR1_STOP)
  {
    if((CODECTimeout--) == 0) return Codec_TIMEOUT_UserCallback();
  }  
  
  /*!< Re-Enable Acknowledgment to be ready for another reception */
  I2C_AcknowledgeConfig(CODEC_I2C, ENABLE);  
  
  /* Clear AF flag for next communication */
  I2C_ClearFlag(CODEC_I2C, I2C_FLAG_AF); 
  
  /* Return the byte read from Codec */
  return result;
}

/**
  * @brief  Initializes the Audio Codec control interface (I2C).
  * @param  None
  * @retval None
  */
static void Codec_CtrlInterface_Init(void)
{
  I2C_InitTypeDef I2C_InitStructure;
  
  /* Enable the CODEC_I2C peripheral clock */
  RCC_APB1PeriphClockCmd(CODEC_I2C_CLK, ENABLE);
  
  /* CODEC_I2C peripheral configuration */
  I2C_DeInit(CODEC_I2C);
  I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
  I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
  I2C_InitStructure.I2C_OwnAddress1 = 0x33;
  I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
  I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;
  I2C_InitStructure.I2C_ClockSpeed = I2C_SPEED;
  /* Enable the I2C peripheral */
  I2C_Cmd(CODEC_I2C, ENABLE);  
  I2C_Init(CODEC_I2C, &I2C_InitStructure);
}

/**
  * @brief  Restore the Audio Codec control interface to its default state.
  *         This function doesn't de-initialize the I2C because the I2C peripheral
  *         may be used by other modules.
  * @param  None
  * @retval None
  */
static void Codec_CtrlInterface_DeInit(void)
{
  /* Disable the I2C peripheral */ /* This step is not done here because 
     the I2C interface can be used by other modules */
  /* I2C_DeInit(CODEC_I2C); */
}

/**
  * @brief  Initializes the Audio Codec audio interface (I2S)
  * @note   This function assumes that the I2S input clock (through PLL_R in 
  *         Devices RevA/Z and through dedicated PLLI2S_R in Devices RevB/Y)
  *         is already configured and ready to be used.    
  * @param  AudioFreq: Audio frequency to be configured for the I2S peripheral. 
  * @retval None
  */
static void Codec_AudioInterface_Init(uint32_t AudioFreq)
{
  I2S_InitTypeDef I2S_InitStructure;

  /* Enable the CODEC_I2S peripheral clock */
  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, ENABLE);
  
  /* CODEC_I2S peripheral configuration */
  SPI_I2S_DeInit(CODEC_I2S);
  I2S_InitStructure.I2S_AudioFreq = AudioFreq;
  I2S_InitStructure.I2S_Standard = I2S_STANDARD;
  I2S_InitStructure.I2S_DataFormat = I2S_DataFormat_16b;
  I2S_InitStructure.I2S_CPOL = I2S_CPOL_Low;
  I2S_InitStructure.I2S_Mode = I2S_Mode_MasterTx;
#ifdef CODEC_MCLK_ENABLED
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Enable;
#elif defined(CODEC_MCLK_DISABLED)
  I2S_InitStructure.I2S_MCLKOutput = I2S_MCLKOutput_Disable;
#else
  #error "No selection for the MCLK output has been defined !"
#endif /* CODEC_MCLK_ENABLED */
  
  /* Initialize the I2S peripheral with the structure above */
  I2S_Init(CODEC_I2S, &I2S_InitStructure);  
}

/**
  * @brief  Restores the Audio Codec audio interface to its default state.
  * @param  None
  * @retval None
  */
static void Codec_AudioInterface_DeInit(void)
{
  /* Disable the CODEC_I2S peripheral (in case it hasn't already been disabled) */
  I2S_Cmd(CODEC_I2S, DISABLE);
  
  /* Deinitialize the CODEC_I2S peripheral */
  SPI_I2S_DeInit(CODEC_I2S);
  
  /* Disable the CODEC_I2S peripheral clock */
  RCC_APB1PeriphClockCmd(CODEC_I2S_CLK, DISABLE); 
}

/**
  * @brief Initializes IOs used by the Audio Codec (on the control and audio 
  *        interfaces).
  * @param  None
  * @retval None
  */
static void Codec_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Enable Reset GPIO Clock */
  RCC_AHB1PeriphClockCmd(AUDIO_RESET_GPIO_CLK,ENABLE);
  
  /* Audio reset pin configuration -------------------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = AUDIO_RESET_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(AUDIO_RESET_GPIO, &GPIO_InitStructure);    
  
  /* Enable I2S and I2C GPIO clocks */
  RCC_AHB1PeriphClockCmd(CODEC_I2C_GPIO_CLOCK | CODEC_I2S_GPIO_CLOCK, ENABLE);

  /* CODEC_I2C SCL and SDA pins configuration -------------------------------------*/
  GPIO_InitStructure.GPIO_Pin = CODEC_I2C_SCL_PIN | CODEC_I2C_SDA_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2C_GPIO, &GPIO_InitStructure);     
  /* Connect pins to I2C peripheral */
  GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2S_SCL_PINSRC, CODEC_I2C_GPIO_AF);  
  GPIO_PinAFConfig(CODEC_I2C_GPIO, CODEC_I2S_SDA_PINSRC, CODEC_I2C_GPIO_AF);  

  /* CODEC_I2S pins configuration: WS, SCK and SD pins -----------------------------*/
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_SCK_PIN | CODEC_I2S_SD_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2S_GPIO, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_WS_PIN ;
  GPIO_Init(CODEC_I2S_WS_GPIO, &GPIO_InitStructure);
  
  /* Connect pins to I2S peripheral  */
  GPIO_PinAFConfig(CODEC_I2S_WS_GPIO, CODEC_I2S_WS_PINSRC, CODEC_I2S_GPIO_AF);  
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SCK_PINSRC, CODEC_I2S_GPIO_AF);
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SD_PINSRC, CODEC_I2S_GPIO_AF);

#ifdef CODEC_MCLK_ENABLED
  /* CODEC_I2S pins configuration: MCK pin */
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_MCK_PIN; 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2S_MCK_GPIO, &GPIO_InitStructure);   
  /* Connect pins to I2S peripheral  */
  GPIO_PinAFConfig(CODEC_I2S_MCK_GPIO, CODEC_I2S_MCK_PINSRC, CODEC_I2S_GPIO_AF); 
#endif /* CODEC_MCLK_ENABLED */ 
}

/**
  * @brief  Restores the IOs used by the Audio Codec interface to their default state.
  * @param  None
  * @retval None
  */
static void Codec_GPIO_DeInit(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
  /* Deinitialize all the GPIOs used by the driver */
  GPIO_InitStructure.GPIO_Pin =  CODEC_I2S_SCK_PIN | CODEC_I2S_SD_PIN;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(CODEC_I2S_GPIO, &GPIO_InitStructure);  
  
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_WS_PIN ;
  GPIO_Init(CODEC_I2S_WS_GPIO, &GPIO_InitStructure); 
     
  /* Disconnect pins from I2S peripheral  */
  GPIO_PinAFConfig(CODEC_I2S_WS_GPIO, CODEC_I2S_WS_PINSRC, 0x00);  
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SCK_PINSRC, 0x00);
  GPIO_PinAFConfig(CODEC_I2S_GPIO, CODEC_I2S_SD_PINSRC, 0x00);  
  
#ifdef CODEC_MCLK_ENABLED
  /* CODEC_I2S pins deinitialization: MCK pin */
  GPIO_InitStructure.GPIO_Pin = CODEC_I2S_MCK_PIN; 
  GPIO_Init(CODEC_I2S_MCK_GPIO, &GPIO_InitStructure);   
  /* Disconnect pins from I2S peripheral  */
  GPIO_PinAFConfig(CODEC_I2S_MCK_GPIO, CODEC_I2S_MCK_PINSRC, CODEC_I2S_GPIO_AF); 
#endif /* CODEC_MCLK_ENABLED */    
}

/**
  * @brief  Inserts a delay time (not accurate timing).
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void Delay( __IO uint32_t nCount)
{
  for (; nCount != 0; nCount--);
}

#ifdef USE_DEFAULT_TIMEOUT_CALLBACK
/**
  * @brief  Basic management of the timeout situation.
  * @param  None
  * @retval None
  */
uint32_t Codec_TIMEOUT_UserCallback(void)
{
  /* Block communication and all processes */
  while (1)
  {   
  }
}
#endif /* USE_DEFAULT_TIMEOUT_CALLBACK */
#endif /* USE_CS43L22 */

/**
  * @brief  
  * @param  
  * @retval 
  */
static void DMA_Config(void)
{
#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
   NVIC_InitTypeDef NVIC_InitStructure;
#endif

   /* Enable the DMA clock */
   RCC_AHB1PeriphClockCmd(AUDIO_MAL_DMA_CLOCK, ENABLE);

   /* Configure the DMA Stream */
   DMA_Cmd(AUDIO_MAL_DMA_STREAM, DISABLE);
   DMA_DeInit(AUDIO_MAL_DMA_STREAM);

   /* Set the parameters to be configured */
   DMA_InitStructure.DMA_Channel = AUDIO_MAL_DMA_CHANNEL;  
   DMA_InitStructure.DMA_PeripheralBaseAddr = AUDIO_MAL_DMA_DREG;
   DMA_InitStructure.DMA_Memory0BaseAddr = (uint32_t)0;      /* This field will be configured in play function */
   DMA_InitStructure.DMA_DIR = DMA_DIR_MemoryToPeripheral;
   DMA_InitStructure.DMA_BufferSize = (uint32_t)0xFFFE;      /* This field will be configured in play function */
   DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
   DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
   DMA_InitStructure.DMA_PeripheralDataSize = AUDIO_MAL_DMA_PERIPH_DATA_SIZE;
   DMA_InitStructure.DMA_MemoryDataSize = AUDIO_MAL_DMA_MEM_DATA_SIZE; 
#ifdef AUDIO_MAL_MODE_NORMAL
   DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
#elif defined(AUDIO_MAL_MODE_CIRCULAR)
   DMA_InitStructure.DMA_Mode = DMA_Mode_Circular;
#else
   #error "AUDIO_MAL_MODE_NORMAL or AUDIO_MAL_MODE_CIRCULAR should be selected !!"
#endif /* AUDIO_MAL_MODE_NORMAL */  
   DMA_InitStructure.DMA_Priority = DMA_Priority_High;
   DMA_InitStructure.DMA_FIFOMode = DMA_FIFOMode_Enable;
   DMA_InitStructure.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
   DMA_InitStructure.DMA_MemoryBurst = DMA_MemoryBurst_Single;
   DMA_InitStructure.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;  
   DMA_Init(AUDIO_MAL_DMA_STREAM, &DMA_InitStructure);  
    
   /* Enable the selected DMA interrupts */
#ifdef AUDIO_MAL_DMA_IT_TC_EN
   DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_TC, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_TC_EN */
#ifdef AUDIO_MAL_DMA_IT_HT_EN
   DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_HT, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_HT_EN */
#ifdef AUDIO_MAL_DMA_IT_TE_EN
   DMA_ITConfig(AUDIO_MAL_DMA_STREAM, DMA_IT_TE | DMA_IT_FE | DMA_IT_DME, ENABLE);
#endif /* AUDIO_MAL_DMA_IT_TE_EN */

#if defined(AUDIO_MAL_DMA_IT_TC_EN) || defined(AUDIO_MAL_DMA_IT_HT_EN) || defined(AUDIO_MAL_DMA_IT_TE_EN)
   /* DMA IRQ Channel configuration */
   NVIC_InitStructure.NVIC_IRQChannel = AUDIO_MAL_DMA_IRQ;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = EVAL_AUDIO_IRQ_PREPRIO;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = EVAL_AUDIO_IRQ_SUBRIO;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
   NVIC_Init(&NVIC_InitStructure);
#endif
}

/**
  * @brief
  * @param
  * @retval
  */
void PCMBuffConv(uint32_t *inbuff, uint16_t inbuffSize, uint32_t *outbuff, BitConv bitconv)
{
	uint16_t i;

	switch(bitconv)
	{
		case S16_TO_U8:
			for(i = 0 ; i < (inbuffSize / 8) ; i++)
			{
				*outbuff = ( ((*inbuff >> 24) & 0xFF) + 0x80 ) << 24;
				*outbuff |= ( ((*inbuff >> 8) & 0xFF) + 0x80 ) << 16;
				inbuff++;
				*outbuff |= ( ((*inbuff >> 24) & 0xFF) + 0x80 ) << 8;
				*outbuff |= ( ((*inbuff >> 8) & 0xFF) + 0x80 );
				inbuff++;
				outbuff++;
			}

			break;

		case S16_TO_U16:
			for(i = 0 ; i < (inbuffSize / 4) ; i++)
			{
				*outbuff = *inbuff + 0x80008000;
				inbuff++;
				outbuff++;
			}

			break;

		case U8_TO_U16:
			for(i = 0 ; i < (inbuffSize / 4) ; i++)
			{
				*outbuff = ( ((*inbuff >> 24) & 0xFF) << 24 ) + ( ((*inbuff >> 24) & 0xFF) << 16 );
				*outbuff |= ( ((*inbuff >> 16) & 0xFF) << 8 ) + ((*inbuff >> 16) & 0xFF);
				outbuff++;
				*outbuff = ( ((*inbuff >> 8) & 0xFF) << 24 ) + ( ((*inbuff >> 8) & 0xFF) << 16 );
				*outbuff |= ( (*inbuff & 0xFF) << 8 ) + (*inbuff & 0xFF);
				outbuff++;
				inbuff++;
			}

			break;

		default:
			if(inbuff != outbuff)
			{
				for(i = 0 ; i < (inbuffSize / 4) ; i++)
				{
					*outbuff = *inbuff;
					inbuff++;
					outbuff++;
				}
			}

			break;
	}
}
