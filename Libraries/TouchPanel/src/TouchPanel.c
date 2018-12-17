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
#include "TouchPanel.h"
#include "Graphics.h"

/* Private types ------------------------------------------------------------*/
/* Private constants --------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
#define	CHX 	0x90
#define	CHY 	0xd0

#define TP_CS(x)	x ? GPIO_SetBits(GPIOB,GPIO_Pin_12): GPIO_ResetBits(GPIOB,GPIO_Pin_12)
#define TP_INT_IN   GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_6)

#define NORMAL_THRESHOLD 1000

/* Private variables --------------------------------------------------------*/
Coordinate DisplaySample[3] = { { GetMaxX()/2, GetMaxY()/4 }, 		
								{ GetMaxX()/4, 0.75*GetMaxY() }, 
								{ 0.75*GetMaxX(), 0.75*GetMaxY() } };

Matrix 			matrix;
Coordinate 		display;
Coordinate 		ScreenSample[3];
extern uint8_t 	calibrateDone;
uint16_t 		Threshold = NORMAL_THRESHOLD;

/* Private function prototypes ----------------------------------------------*/
#ifndef USE_Delay
	static void delay(__IO uint32_t nCount);
#endif /* USE_Delay*/

/**
  * @brief  Initializes the Touch controller.
  * @param  None
  * @retval None
  */
void TP_Init(void) 
{ 
   SPI_InitTypeDef  SPI_InitStructure;
   GPIO_InitTypeDef GPIO_InitStruct;
   NVIC_InitTypeDef NVIC_InitStructure;
   EXTI_InitTypeDef EXTI_InitStructure;

   /* Enable GPIOB, GPIOD and AFIO clocks */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOD, ENABLE);
   RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2, ENABLE);

   /* SPI2 GPIO Configuration */
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;

   GPIO_PinAFConfig(GPIOB, GPIO_PinSource13, GPIO_AF_SPI2);
   GPIO_PinAFConfig(GPIOB, GPIO_PinSource14, GPIO_AF_SPI2);
   GPIO_PinAFConfig(GPIOB, GPIO_PinSource15, GPIO_AF_SPI2);
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_15 | GPIO_Pin_13 | GPIO_Pin_14;
   GPIO_Init(GPIOB, &GPIO_InitStruct);

   /* SPI2 Configuration */
   SPI_I2S_DeInit(SPI2);
   SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
   SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
   SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
   SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
   SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
   SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
   SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;
   SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
   SPI_InitStructure.SPI_CRCPolynomial = 7;
   SPI_Init(SPI2, &SPI_InitStructure);
   SPI_Cmd(SPI2, ENABLE);

   /* CS Pin Configuration */
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
   GPIO_Init(GPIOB, &GPIO_InitStruct);

   /* TP_IRQ Pin Configuration */
   GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN;
   GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
   GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6;
   GPIO_Init(GPIOD, &GPIO_InitStruct);

#ifdef ENABLE_TP_INTERRUPT
   RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

   /* Connect Button EXTI Line to Button GPIO Pin */
   SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOD, EXTI_PinSource6);

   /* Configure Button EXTI line */
   EXTI_InitStructure.EXTI_Line = EXTI_Line6;
   EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
   EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
   EXTI_InitStructure.EXTI_LineCmd = ENABLE;
   EXTI_Init(&EXTI_InitStructure);

   /* Enable and set Button EXTI Interrupt to the lowest priority */
   NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
   NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
   NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
   NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;

   NVIC_Init(&NVIC_InitStructure);
#endif /* ENABLE_TP_INTERRUPT */
}

/**
  * @brief  Write command to touch controller.
  * @param  cmd.
  * @retval None
  */
static void WR_CMD (uint8_t cmd)  
{ 
  /* Wait for SPI2 Tx buffer empty */ 
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  
  /* Send SPI2 data */ 
  SPI_I2S_SendData(SPI2,cmd);
  
  /* Wait for SPI2 data reception */ 
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  
  /* Read SPI2 received data */ 
  SPI_I2S_ReceiveData(SPI2); 
} 

/**
  * @brief  Read address in touch controller.
  * @param  None
  * @retval Register value.
  */
static int RD_AD(void)  
{ 
  unsigned short buf, temp;
  
  /* Wait for SPI2 Tx buffer empty */ 
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  
  /* Send SPI2 data */ 
  SPI_I2S_SendData(SPI2, 0x0000);
  
  /* Wait for SPI2 data reception */ 
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  
  /* Read SPI2 received data */ 
  temp = SPI_I2S_ReceiveData(SPI2); 
  buf = temp << 8;
  
  _delay_(1);
  
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET);
  
  /* Send SPI2 data */ 
  SPI_I2S_SendData(SPI2, 0x0000);
  
  /* Wait for SPI2 data reception */ 
  while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
  
  /* Read SPI2 received data */ 
  temp = SPI_I2S_ReceiveData(SPI2); 
  buf |= temp; 
  buf >>= 3; 
  buf &= 0xfff;
  
  return buf; 
}

/**
  * @brief  Read xpos in touch controller.
  * @param  None
  * @retval xpos value.
  */
int Read_X(void)  
{  
  int i;
  
  TP_CS(0); 
  _delay_(1); 
  WR_CMD(CHX); 
  _delay_(1); 
  i = RD_AD(); 
  TP_CS(1);
  
  return i;    
} 

/**
  * @brief  Read ypos in touch controller.
  * @param  None
  * @retval ypos value.
  */
int Read_Y(void)  
{  
  int i;
  
  TP_CS(0); 
  _delay_(1); 
  WR_CMD(CHY); 
  _delay_(1); 
  i = RD_AD(); 
  TP_CS(1);
  
  return i;     
} 

/**
  * @brief  Copy xpos & ypos in argument variables.
  * @param  xpos buffer.
  * @param  ypos buffer.  
  * @retval None
  */
void TP_GetAdXY(int *x,int *y)
{
  int adx,ady;
  adx=Read_X();
  _delay_(1); 
  ady=Read_Y();
  *x=adx;
  *y=ady;
}

/**
  * @brief  Gets xpos & ypos.
  * @param  None
  * @param  None  
  * @retval Coordinate *
  */
Coordinate *Read_Ads7846(void)
{
  static Coordinate screen;
  int m0, m1, m2, TP_X[1], TP_Y[1], temp[3];
  uint8_t count = 0;
  int buffer[2][9] = {{0},{0}};
  
  /* Mask touch interrupt flag */
  calibrateDone = RESET;

  do
  {		   
    TP_GetAdXY(TP_X, TP_Y);  
	buffer[0][count] = TP_X[0];  
	buffer[1][count] = TP_Y[0];
	count++;  
  }
  while( (!TP_INT_IN) && (count < 9) );  /* TP_INT_IN */

  /* Unmask touch interrupt flag */
  calibrateDone = SET;

  if(count == 9)   /* Average X Y */ 
  {
	/* Average X */
	temp[0] = (buffer[0][0] + buffer[0][1] + buffer[0][2])/3;
	temp[1] = (buffer[0][3] + buffer[0][4] + buffer[0][5])/3;
	temp[2] = (buffer[0][6] + buffer[0][7] + buffer[0][8])/3;

	m0 = temp[0] - temp[1];
	m1 = temp[1] - temp[2];
	m2 = temp[2] - temp[0];

	m0 = m0 > 0 ? m0 : (-m0);
	m1 = m1 > 0 ? m1 : (-m1);
	m2 = m2 > 0 ? m2 : (-m2);

	if( (m0 > Threshold) && (m1 > Threshold) && (m2 > Threshold) ) return 0;

	if(m0 < m1)
	{
	  if(m2 < m0) 
	    screen.x = (temp[0] + temp[2])/2;
	  else 
	    screen.x = (temp[0] + temp[1])/2;	
	}
	else if(m2 < m1) 
	  screen.x = (temp[0] + temp[2])/2;
	else 
	  screen.x = (temp[1] + temp[2])/2;

	/* Average Y  */
	temp[0] = (buffer[1][0] + buffer[1][1] + buffer[1][2])/3;
	temp[1] = (buffer[1][3] + buffer[1][4] + buffer[1][5])/3;
	temp[2] = (buffer[1][6] + buffer[1][7] + buffer[1][8])/3;
	
	m0 = temp[0] - temp[1];
	m1 = temp[1] - temp[2];
	m2 = temp[2] - temp[0];
	
	m0 = m0 > 0 ? m0 : (-m0);
	m1 = m1 > 0 ? m1 : (-m1);
	m2 = m2 > 0 ? m2 : (-m2);
	
	if( (m0 > Threshold) && (m1 > Threshold) && (m2 > Threshold) ) return 0;

	if(m0 < m1)
	{
	  if(m2 < m0) 
	    screen.y = (temp[0] + temp[2])/2;
	  else 
	    screen.y = (temp[0] + temp[1])/2;	
    }
	else if(m2 < m1) 
	   screen.y = (temp[0] + temp[2])/2;
	else
	   screen.y = (temp[1] + temp[2])/2;

	return &screen;
  }  
  return 0; 
}

/**
  * @brief  Calibration matrix calc for touch controller (Calculate K A B C D E F constants).
  * @param  None
  * @param  None  
  * @retval FunctionalState bool
  */
FunctionalState setCalibrationMatrix(Coordinate *displayPtr,
									 Coordinate *screenPtr,
									 Matrix *matrixPtr)
{

  FunctionalState retTHRESHOLD = ENABLE ;
  /* K = (X0 ­ X2)(Y1 ­ Y2) ­ (X1 ­ X2)(Y0 ­ Y2) */
  matrixPtr->Divider = ((screenPtr[0].x - screenPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) - 
                       ((screenPtr[1].x - screenPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
  if( matrixPtr->Divider == 0 )
  {
    retTHRESHOLD = DISABLE;
  }
  else
  {
    /* A = (XD0 ­ XD2)(Y1 ­ Y2) ­ (XD1 ­ XD2)(Y0 ­ Y2) */
    matrixPtr->An = ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].y - screenPtr[2].y)) - 
                    ((displayPtr[1].x - displayPtr[2].x) * (screenPtr[0].y - screenPtr[2].y)) ;
	/* B = (X0 ­ X2)(XD1 ­ XD2) ­ (XD0 ­ XD2)(X1 ­ X2) */
    matrixPtr->Bn = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].x - displayPtr[2].x)) - 
                    ((displayPtr[0].x - displayPtr[2].x) * (screenPtr[1].x - screenPtr[2].x)) ;
    /* C = Y0(X2XD1 ­ X1XD2) + Y1(X0XD2 ­ X2XD0) + Y2(X1XD0 ­ X0XD1) */
    matrixPtr->Cn = (screenPtr[2].x * displayPtr[1].x - screenPtr[1].x * displayPtr[2].x) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].x - screenPtr[2].x * displayPtr[0].x) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].x - screenPtr[0].x * displayPtr[1].x) * screenPtr[2].y ;
    /* D = (YD0 ­ YD2)(Y1 ­ Y2) ­ (YD1 ­ YD2)(Y0 ­ Y2) */
    matrixPtr->Dn = ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].y - screenPtr[2].y)) - 
                    ((displayPtr[1].y - displayPtr[2].y) * (screenPtr[0].y - screenPtr[2].y)) ;
    /* E = (X0 ­ X2)(YD1 ­ YD2) ­ (YD0 ­ YD2)(X1 ­ X2) */
    matrixPtr->En = ((screenPtr[0].x - screenPtr[2].x) * (displayPtr[1].y - displayPtr[2].y)) - 
                    ((displayPtr[0].y - displayPtr[2].y) * (screenPtr[1].x - screenPtr[2].x)) ;
    /* F = Y0(X2YD1 ­ X1YD2) + Y1(X0YD2 ­ X2YD0) + Y2(X1YD0 ­ X0YD1) */
    matrixPtr->Fn = (screenPtr[2].x * displayPtr[1].y - screenPtr[1].x * displayPtr[2].y) * screenPtr[0].y +
                    (screenPtr[0].x * displayPtr[2].y - screenPtr[2].x * displayPtr[0].y) * screenPtr[1].y +
                    (screenPtr[1].x * displayPtr[0].y - screenPtr[0].x * displayPtr[1].y) * screenPtr[2].y ;
  }
  return( retTHRESHOLD ) ;
}

/**
  * @brief  Gets xpos & ypos (calibrated position).
  * @param  None
  * @param  None  
  * @retval FunctionalState bool
  */
FunctionalState getDisplayPoint(Coordinate *displayPtr,
								Coordinate *screenPtr,
								Matrix *matrixPtr)
{
  FunctionalState retTHRESHOLD = ENABLE;

  if( matrixPtr->Divider != 0 )
  {
    /* XD = AX + BY + C */        
    displayPtr->x = ( (matrixPtr->An * screenPtr->x) + 
                      (matrixPtr->Bn * screenPtr->y) + 
                       matrixPtr->Cn 
                    ) / matrixPtr->Divider ;
	/* YD = DX + EY+ F */        
    displayPtr->y = ( (matrixPtr->Dn * screenPtr->x) + 
                      (matrixPtr->En * screenPtr->y) + 
                       matrixPtr->Fn 
                    ) / matrixPtr->Divider ;
  }
  else
  {
    retTHRESHOLD = DISABLE;
  }
  return(retTHRESHOLD);
}

/**
  * @brief  Init the calibration routine.
  * @param  None
  * @param  None  
  * @retval None
  */
void TouchPanel_Calibrate(void)
{
  uint8_t i;
  Coordinate *Ptr;

#if defined(GFX_LIB)
  SHORT width, height;
#if defined (USE_MULTIBYTECHAR)
  XCHAR text_1[] = {'T','o','u','c','h',' ','i','n','s','i','d','e',' ','t','h','e',' ','c','i','r','c','l','e','s',0x0000},
        text_2[] = {'D','o','n','e','.',' ','T','o','u','c','h',' ','a','g','a','i','n',' ','t','o',' ','c','o','n','t','i','n','u','e',0x0000};
#else
  XCHAR text_1[] = "Touch inside the circles",
        text_2[] = "Done. Touch again to continue";
#endif
  // maximum precision for calibration
  Threshold = 1;

  SetColor(WHITE);
  ClearDevice();
  
  for(i = 0; i < 3; i++)
  {
	SetColor(BLACK); //set color
	SetFontFlash(&GOLFontDefault); //set font

	// get string width
	width = GetTextWidth(text_1, &GOLFontDefault);

	// place string in the middle of the screen
	OutTextXY((GetMaxX() - width) >> 1, 0, (char*)text_1);

	_delay_(500000); //500ms time delay.
	
	Circle(DisplaySample[i].x, DisplaySample[i].y, 10);
	
	do
	{
		Ptr=Read_Ads7846();
	}
	while( Ptr == (void*)0 );
	
	FillCircle(DisplaySample[i].x, DisplaySample[i].y, 10);
	
	ScreenSample[i].x = Ptr->x; 
	ScreenSample[i].y = Ptr->y;
  }
  
  setCalibrationMatrix(&DisplaySample[0], &ScreenSample[0], &matrix);
  
  _delay_(500000); //500ms time delay.

  // get string width & height
  width = GetTextWidth(text_2, &GOLFontDefault);
  height = GetTextHeight(&GOLFontDefault);

  // place string in the middle of the screen
  OutTextXY((GetMaxX() - width) >> 1, (GetMaxY() - height) >> 1, (char*)text_2);
  
  do
  {
	Ptr=Read_Ads7846();
  }
  while( Ptr == (void*)0 );
  
  SetColor(WHITE);
  ClearDevice();

  // return to the normal threshold
  Threshold = NORMAL_THRESHOLD;
#endif /* GFX_LIB */
} 

#ifndef USE_Delay
/**
  * @brief  Inserts a delay time.
  * @param  nCount: specifies the delay time length.
  * @retval None
  */
static void delay(__IO uint32_t nCount)
{
  __IO uint32_t index = 0; 
  for(index = nCount; index != 0; index--)
  {
  }
}
#endif /* USE_Delay*/ 
