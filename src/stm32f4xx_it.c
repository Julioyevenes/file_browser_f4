/**
  ******************************************************************************
  * @file    Template/stm32f4xx_it.c 
  * @author  MCD Application Team
  * @version V1.0.0
  * @date    20-September-2013
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT 2013 STMicroelectronics</center></h2>
  *
  * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
  * You may not use this file except in compliance with the License.
  * You may obtain a copy of the License at:
  *
  *        http://www.st.com/software_license_agreement_liberty_v2
  *
  * Unless required by applicable law or agreed to in writing, software 
  * distributed under the License is distributed on an "AS IS" BASIS, 
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_it.h"
#include "main.h"

/** @addtogroup Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
__IO uint32_t DisableTime = 0;
extern __IO uint32_t TimingDelay;
extern FlagStatus volTask;
extern BOOL lockScreen;
extern __IO uint8_t volumeAudio;
extern __IO DWORD  tick;
__IO uint16_t count = 0;
extern UINT BacklightLevel;

#if defined ENABLE_USB_MSD
	extern USB_OTG_CORE_HANDLE          USB_OTG_Core;
#endif

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M4 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief   This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	/* Debounce time decrement */
	if (DisableTime != 0x00)
		DisableTime--;
		
	/* Counter variable for Graphics */
	count++;
	if (count > 520)
	{
		count = 0;
		tick++;
	}

	/* Delay routines */
	TimingDelay_Decrement();
	
	if(volTask) {
		Audio_MAL_VolumeCtl(volumeAudio);
        volTask = RESET;
	}
}

/******************************************************************************/
/*                 STM32F4xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f429_439xx.s).                                               */
/******************************************************************************/

/**
  * @brief  EXTI0_IRQHandler
  *         This function handles External line 0 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI0_IRQHandler(void)
{
  /* Checks whether the User Button EXTI line is asserted */
  if (EXTI_GetITStatus(EXTI_Line0) != RESET) 
  {
	  /* Debouncing technique */
	  if (DisableTime == 0x0)
	  {
	  	DisableTime = 500000; //500ms debouncing time
	  	if (GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
	  	{
	  		if(!lockScreen) {
	  			LCD_SetBacklight(0); //Desactivamos brillo de lcd.
	  			lockScreen = TRUE;
	  		} else {
	  			LCD_SetBacklight(BacklightLevel); //Activamos brillo de lcd a su valor anterior.
	  			lockScreen = FALSE;
	  		}
	  	}
	  }
  } 
  /* Clears the EXTI's line pending bit.*/ 
  EXTI_ClearITPendingBit(EXTI_Line0);
}

#if defined ENABLE_USB_MSD
/**
  * @brief  This function handles TIM2 global interrupt request.
  * @param  None
  * @retval None
  */
void TIM2_IRQHandler(void)
{
  USB_OTG_BSP_TimerIRQ();
}


/**
  * @brief  This function handles USB-On-The-Go FS global interrupt request.
  * @param  None
  * @retval None
  */
void OTG_FS_IRQHandler(void)
{
  USBH_OTG_ISR_Handler(&USB_OTG_Core);
}
#endif /* ENABLE_USB_MSD */

/**
  * @}
  */ 


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
