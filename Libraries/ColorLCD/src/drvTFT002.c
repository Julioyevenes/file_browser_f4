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
#include "HardwareProfile.h"

#if defined (USE_DISPLAY_CONTROLLER_SSD1289) || \
	defined (USE_DISPLAY_CONTROLLER_SSD2119) || \
	defined (USE_DISPLAY_CONTROLLER_ILI9481)

#include "stm32f4xx.h"
#include "TimeDelay.h"
#include "DisplayDriver.h"
#include "drvTFT002.h"
#include "Primitive.h"

// Unsupported Graphics Library Features
#ifdef USE_TRANSPARENT_COLOR
    #warning "This driver does not support the transparent feature on PutImage(). Build will use the PutImage() functions defined in the Primitive.c"
#endif

#define USE_PRIMITIVE_PUTIMAGE
#ifndef USE_PRIMITIVE_PUTIMAGE
    #warning "This driver does not support partial putImage feature. To enable partial putimage feature, uncomment the macro USE_PRIMITIVE_PUTIMAGE in this file. This will enable the PutImageXBPPYYY() in the Primitive.c implementation."
#endif 
	
/* Private types ------------------------------------------------------------*/
/* Private constants --------------------------------------------------------*/
/* Private macro ------------------------------------------------------------*/
/* Private variables --------------------------------------------------------*/

// Clipping region control
SHORT       _clipRgn;

// Clipping region borders
SHORT       _clipLeft;
SHORT       _clipTop;
SHORT       _clipRight;
SHORT       _clipBottom;

// Color
GFX_COLOR   _color;
#ifdef USE_TRANSPARENT_COLOR
GFX_COLOR   _colorTransparent;
SHORT       _colorTransparentEnable;
#endif

/* Private function prototypes ----------------------------------------------*/
void	 LCD_SetReg(UINT16 LCD_Reg);
void	 LCD_WriteReg(UINT16 LCD_RegValue);
void     LCD_SetRAM(void);
void     LCD_WriteRAM(UINT16 RGB_Code);
UINT16   LCD_Read(UINT16 LCD_Reg);
UINT16   LCD_ReadRAM(void);
void DriverInterfaceInit(void);

/**
  * @brief  
  * @param  
  * @retval 
  */
#define LCD_Write(LCD_Reg, LCD_RegValue) {LCD_SetReg(LCD_Reg); LCD_WriteReg(LCD_RegValue);}

/**
  * @brief  
  * @param  
  * @retval 
  */
#if defined (USE_DISPLAY_CONTROLLER_ILI9481)
	#define WritePixel(color)	LCD_WriteReg(color)
#else
	#define WritePixel(color)	LCD_Write(0x22,color)
#endif

/**
  * @brief  
  * @param  
  * @retval 
  */
inline void SetAddress(WORD x, WORD y) {
#if (DISP_ORIENTATION == 0)
	#if defined (USE_DISPLAY_CONTROLLER_ILI9481)
		LCD_SetReg(0x2A);                 //Set column address
		LCD_WriteReg(((WORD_VAL)x).v[1]); //x1
		LCD_WriteReg(((WORD_VAL)x).v[0]);
		LCD_WriteReg(1);                  //x2
		LCD_WriteReg(0x3F);
		LCD_SetReg(0x2B);                 //Set page address
		LCD_WriteReg(((WORD_VAL)y).v[1]); //y1
		LCD_WriteReg(((WORD_VAL)y).v[0]);
		LCD_WriteReg(1);                  //y2
		LCD_WriteReg(0xE0);
	#else
		LCD_Write(0x4E, x);
		LCD_Write(0x4F, y);
	#endif
#else
	#if defined (USE_DISPLAY_CONTROLLER_ILI9481)
		LCD_SetReg(0x2A);                 //Set column address
		LCD_WriteReg(((WORD_VAL)x).v[1]); //x1
		LCD_WriteReg(((WORD_VAL)x).v[0]);
		LCD_WriteReg(1);                  //x2
		LCD_WriteReg(0xE0);
		LCD_SetReg(0x2B);                 //Set page address
		LCD_WriteReg(((WORD_VAL)y).v[1]); //y1
		LCD_WriteReg(((WORD_VAL)y).v[0]);
		LCD_WriteReg(1);                  //y2
		LCD_WriteReg(0x3F);
	#else
		LCD_Write(0x4F, x);
		LCD_Write(0x4E, y);
	#endif
#endif
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void LCD_SetReg(UINT16 LCD_Reg) {
  CLR_CS();

  /* Write 16-bit Index */
  CLR_RS();
  SET_NRD();
  GPIOE->ODR = LCD_Reg;
  CLR_NWR();
  SET_NWR();

  SET_CS();
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void LCD_WriteReg(UINT16 LCD_RegValue) {
  CLR_CS();

  /* Write 16-bit Reg */
  SET_RS();
  SET_NRD();
  GPIOE->ODR = LCD_RegValue;
  CLR_NWR();
  SET_NWR();

  SET_CS();
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void LCD_SetRAM(void) {
  /* Write 16-bit Index */
  CLR_RS();
  SET_NRD();
  GPIOE->ODR = 0x22;
  CLR_NWR();
  SET_NWR();
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void LCD_WriteRAM(UINT16 RGB_Code) {
  /* Write 16-bit GRAM Reg */
  SET_RS();
  GPIOE->ODR = RGB_Code;
  CLR_NWR();
  SET_NWR();
}

/**
  * @brief  
  * @param  
  * @retval 
  */
UINT16 LCD_Read(UINT16 LCD_Reg) {
  UINT16 LCD_RAM;
  GPIO_InitTypeDef GPIO_InitStructure;

  CLR_CS();

  /* Write 16-bit Index (then Read Reg) */
  CLR_RS();
  SET_NRD();
  GPIOE->ODR = LCD_Reg;
  CLR_NWR();
  SET_NWR();

  /* Read 16-bit Reg */
  SET_RS();
  SET_NWR();
  CLR_NRD();
  /* Configure Data Port as Input */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  /* Read reg value */
  LCD_RAM = GPIOE->IDR;
  /* Configure Data Port as Output */
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOE, &GPIO_InitStructure);
  SET_NRD();

  SET_CS();
  return(LCD_RAM);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
UINT16 LCD_ReadRAM(void) {
  UINT16 LCD_RAM;

  /* Read 16-bit Reg */
  LCD_RAM = LCD_Read(0x22); /* Select GRAM Reg */
  return(LCD_RAM);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void ResetDevice(void) {
    // Initialize the device
	DriverInterfaceInit();

    // Configure the TIM Peripheral
	TIM1_Config();
	LCD_SetBacklight(100);

    // Setup display
#if defined (USE_DISPLAY_CONTROLLER_SSD1289)
    
    /////////////////////////////////////////////////////////
    LCD_Write(0x00, 0x0001);
    LCD_Write(0x03, 0xAAAC);
    LCD_Write(0x0C, 0x0002);
    DelayMs(15);
    LCD_Write(0x0D, 0x000A);
    LCD_Write(0x0E, 0x2D00);
    LCD_Write(0x1E, 0x00BC);

    LCD_Write(0x01, 0x1A0C);

    DelayMs(15);
#if (DISP_ORIENTATION == 0)
    LCD_Write(0x01, 0x2B3F);
#else
    LCD_Write(0x01, 0x293F);
#endif
    LCD_Write(0x02, 0x0600);
    LCD_Write(0x10, 0x0000);

#if (DISP_ORIENTATION == 0)
    LCD_Write(0x11, 0x60B0);
#else
    LCD_Write(0x11, 0x60B8);
#endif
    LCD_Write(0x05, 0x0000);
    LCD_Write(0x06, 0x0000);
    DelayMs(100);
    LCD_Write(0x16, 0xEF1C);
    LCD_Write(0x17, 0x0003);
    LCD_Write(0x07, 0x0233);
    LCD_Write(0x0B, 0x0000);
    LCD_Write(0x0F, 0x0000);
    LCD_Write(0x41, 0x0000);
    LCD_Write(0x42, 0x0000);
    LCD_Write(0x48, 0x0000);
    LCD_Write(0x49, 0x013F);
    LCD_Write(0x44, 0xEF00);
    LCD_Write(0x45, 0x0000);
    LCD_Write(0x46, 0x013F);
    LCD_Write(0x4A, 0x0000);
    LCD_Write(0x4B, 0x0000);
    LCD_Write(0x30, 0x0707);
    LCD_Write(0x31, 0x0704);
    LCD_Write(0x32, 0x0204);
    LCD_Write(0x33, 0x0502);
    LCD_Write(0x34, 0x0507);
    LCD_Write(0x35, 0x0204);
    LCD_Write(0x36, 0x0204);
    LCD_Write(0x37, 0x0502);
    LCD_Write(0x3A, 0x0302);
    LCD_Write(0x3B, 0x1f00);
    LCD_Write(0x23, 0x0000);
    LCD_Write(0x24, 0x0000);

#elif defined (USE_DISPLAY_CONTROLLER_SSD2119)
    
    LCD_Write(0x0028, 0x0006); // VCOM OTP, page 55-56 of datasheet
    LCD_Write(0x0000, 0x0001); // start Oscillator, page 36 of datasheet
    LCD_Write(0x0010, 0x0000); // Sleep mode, page 49 of datasheet
#if (DISP_ORIENTATION == 0)
    LCD_Write(0x0001, 0x72EF); // Driver Output Control, page 36-39 of datasheet
#else
    LCD_Write(0x0001, 0x32EF); // Driver Output Control, page 36-39 of datasheet
#endif
    LCD_Write(0x0002, 0x0600); // LCD Driving Waveform Control, page 40-42 of datasheet
    LCD_Write(0x0003, 0x6A38); // Power Control 1, page 43-44 of datasheet
#if (DISP_ORIENTATION == 0)
    LCD_Write(0x0011, 0x6870); // Entry Mode, page 50-52 of datasheet
#else
    LCD_Write(0x0011, 0x6878); // Entry Mode, page 50-52 of datasheet
#endif
    LCD_Write(0x000F, 0x0000); // Gate Scan Position, page 49 of datasheet
    LCD_Write(0x000B, 0x5308); // Frame Cycle Control, page 45 of datasheet
    LCD_Write(0x000C, 0x0003); // Power Control 2, page 47 of datasheet
    LCD_Write(0x000D, 0x000A); // Power Control 3, page 48 of datasheet
    LCD_Write(0x000E, 0x2E00); // Power Control 4, page 48 of datasheet
    LCD_Write(0x001E, 0x00BE); // Power Control 5, page 53 of datasheet
    LCD_Write(0x0025, 0x8000); // Frame Frequency Control, page 53 of datasheet
    LCD_Write(0x0026, 0x7800); // Analog setting, page 54 of datasheet
    LCD_Write(0x004E, 0x0000); // Ram Address Set, page 58 of datasheet
    LCD_Write(0x004F, 0x0000); // Ram Address Set, page 58 of datasheet
    LCD_Write(0x0012, 0x08D9); // Sleep mode, page 49 of datasheet

    // Gamma Control (R30h to R3Bh) -- page 56 of datasheet
    LCD_Write(0x0030, 0x0000);
    LCD_Write(0x0031, 0x0104);
    LCD_Write(0x0032, 0x0100);
    LCD_Write(0x0033, 0x0305);
    LCD_Write(0x0034, 0x0505);
    LCD_Write(0x0035, 0x0305);
    LCD_Write(0x0036, 0x0707);
    LCD_Write(0x0037, 0x0300);
    LCD_Write(0x003A, 0x1200);
    LCD_Write(0x003B, 0x0800);

    LCD_Write(0x0007, 0x0033); // Display Control, page 45 of datasheet

#elif defined (USE_DISPLAY_CONTROLLER_ILI9481)

    CLR_RST();
    DelayMs(100);
    SET_RST();
    DelayMs(100);

    LCD_SetReg(0x11); //Exit sleep mode
    DelayMs(50);
    LCD_SetReg(0x13); //Enter normal mode
    DelayMs(50);

    LCD_SetReg(0xD0); //Power Setting
    LCD_WriteReg(0x07);
    LCD_WriteReg(0x42);
    LCD_WriteReg(0x18);

    LCD_SetReg(0xD1); //VCOM Control
    LCD_WriteReg(0x00);
    LCD_WriteReg(0x07);
    LCD_WriteReg(0x10);

    LCD_SetReg(0xD2); //Power Setting for Normal Mode
    LCD_WriteReg(0x01);
    LCD_WriteReg(0x02);

    LCD_SetReg(0xC0); //Panel Driving Setting
    LCD_WriteReg(0x10);
    LCD_WriteReg(0x3B);
    LCD_WriteReg(0x00);
    LCD_WriteReg(0x02);
    LCD_WriteReg(0x11);

    LCD_SetReg(0xC5); //Frame Rate and Inversion Control
    LCD_WriteReg(0x02);

    LCD_SetReg(0xC8); //Gamma Setting
    LCD_WriteReg(0x00);
    LCD_WriteReg(0x32);
    LCD_WriteReg(0x36);
    LCD_WriteReg(0x45);
    LCD_WriteReg(0x06);
    LCD_WriteReg(0x16);
    LCD_WriteReg(0x37);
    LCD_WriteReg(0x75);
    LCD_WriteReg(0x77);
    LCD_WriteReg(0x54);
    LCD_WriteReg(0x0C);
    LCD_WriteReg(0x00);

    LCD_SetReg(0x36); //Set address mode
#if (DISP_ORIENTATION == 0)
    LCD_WriteReg(0x18);
#else
    LCD_WriteReg(0x28);
#endif

    LCD_SetReg(0x3A); //Set pixel format
    LCD_WriteReg(0x55);

    LCD_SetReg(0x11); //Exit sleep mode
    DelayMs(50);
    LCD_SetReg(0x29); //Set display on

#endif /* GFX_USE_DISPLAY_CONTROLLER */
    DelayMs(50);
}

#ifdef USE_TRANSPARENT_COLOR
/**
  * @brief  
  * @param  
  * @retval 
  */
void TransparentColorEnable(GFX_COLOR color)
{
    _colorTransparent = color;    
    _colorTransparentEnable = TRANSPARENT_COLOR_ENABLE;

}
#endif

/**
  * @brief  
  * @param  
  * @retval 
  */
void PutPixel(SHORT x, SHORT y) {
    if(_clipRgn)
    {
        if(x < _clipLeft)
            return;
        if(x > _clipRight)
            return;
        if(y < _clipTop)
            return;
        if(y > _clipBottom)
            return;
    }
	
#if defined (USE_DISPLAY_CONTROLLER_ILI9481)
    SetAddress(x, y);
    LCD_SetReg(0x2C); //Write memory start
    WritePixel(_color);
#else
    SetAddress(x, y);
    CLR_CS();
    LCD_SetRAM();
    LCD_WriteRAM(_color);
    SET_CS();
#endif
}

/**
  * @brief  
  * @param  
  * @retval 
  */
WORD GetPixel(SHORT x, SHORT y)
{
	SetAddress(x, y);
#if defined (USE_DISPLAY_CONTROLLER_ILI9481)
	LCD_Read(0x2E); /* dummy read */
	return (LCD_Read(0x2E));
#else
	LCD_ReadRAM(); /* dummy read */
	return (LCD_ReadRAM());
#endif
}

/**
  * @brief  
  * @param  
  * @retval 
  */
WORD IsDeviceBusy(void)
{  
    return (0);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
WORD Bar(SHORT left, SHORT top, SHORT right, SHORT bottom) {
    register SHORT  x, y;

    #ifndef USE_NONBLOCKING_CONFIG
    while(IsDeviceBusy() != 0);

    /* Ready */
    #else
    if(IsDeviceBusy() != 0)
        return (0);
    #endif
    if(_clipRgn)
    {
        if(left < _clipLeft)
            left = _clipLeft;
        if(right > _clipRight)
            right = _clipRight;
        if(top < _clipTop)
            top = _clipTop;
        if(bottom > _clipBottom)
            bottom = _clipBottom;
    }
	
	for(y = top; y < bottom + 1; y++)
    {
        SetAddress(left, y);
        LCD_SetReg(0x2C); //Write memory start
#if !defined (USE_DISPLAY_CONTROLLER_ILI9481)
        CLR_CS();
        LCD_SetRAM(); /* Prepare to write GRAM */
#endif
        for(x = left; x < right + 1; x++)
        {
            WritePixel(_color);
        }
#if !defined (USE_DISPLAY_CONTROLLER_ILI9481)
        SET_CS();
#endif
    }
	
	return (1);
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void ClearDevice(void) {
    DWORD   counter;
	
	SetAddress(0, 0);
    LCD_SetReg(0x2C); //Write memory start
#if !defined (USE_DISPLAY_CONTROLLER_ILI9481)
    CLR_CS();
    LCD_SetRAM(); /* Prepare to write GRAM */
#endif
    for(counter = 0; counter < (DWORD) (GetMaxX() + 1) * (GetMaxY() + 1); counter++)
    {
#if defined (USE_DISPLAY_CONTROLLER_ILI9481)
       WritePixel(_color);
#else
       LCD_WriteRAM(_color);
#endif
    }
#if !defined (USE_DISPLAY_CONTROLLER_ILI9481)
    SET_CS();
#endif
}

/**
  * @brief  
  * @param  
  * @retval 
  */
void DriverInterfaceInit(void) {
   GPIO_InitTypeDef GPIO_InitStructure;

   /* Enable GPIOD, GPIOE, and AFIO clocks */
   RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE , ENABLE);

   /*-- GPIO Configuration ------------------------------------------------------*/

   /* Control pins (CS, RS, NWR, NRD, RST(if needed)) */
#if defined(USE_DISPLAY_CONTROLLER_SSD1289)
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3;
#elif defined(USE_DISPLAY_CONTROLLER_SSD1963) || defined(USE_DISPLAY_CONTROLLER_ILI9481)
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_7;
#endif
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

   GPIO_Init(GPIOD, &GPIO_InitStructure);

   /* Data pins (D0-D15) */
   GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
   GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
   GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
   GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
   GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;

   GPIO_Init(GPIOE, &GPIO_InitStructure);
   
   // hard delay inserted here for devices that needs delays after reset.
   // Value will vary from device to device, please refer to the specific
   // device data sheet for details.
   Delay10us(20);
}

#endif /** USE_DISPLAY_CONTROLLER_SSD1289 || \
		 * USE_DISPLAY_CONTROLLER_SSD2119 || \
		 * USE_DISPLAY_CONTROLLER_ILI9481
		 */
