/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   STM32 SD卡 usb mass storage （模拟U盘）
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 F103-霸道 STM32 开发板 
  * 论坛    :http://www.firebbs.cn
  * 淘宝    :https://fire-stm32.taobao.com
  *
  ******************************************************************************
  */
  
#include "stm32f10x.h"
#include "./sdio/bsp_sdio_sdcard.h"
#include "./sdio/sdio_test.h"
#include "./usart/bsp_usart.h"
#include "./led/bsp_led.h"  
#include "./usart/bsp_usart.h"	
#include "./lcd/bsp_ili9341_lcd.h"
#include "./lcd/bsp_xpt2046_lcd.h"
#include "./flash/fatfs_flash_spi.h"
#include "./SysTick/bsp_SysTick.h"
#include "./led/bsp_led.h"
#include "./key/bsp_exti.h"
#include "./Timbase/bsp_TiMbase.h" 
#include "./palette/palette.h"
#include "./i2c/bsp_i2c_ee.h"
#include "./i2c/bsp_i2c_gpio.h"

#include "hw_config.h" 
#include "usb_lib.h"
#include "usb_pwr.h"

uint8_t i = 0;
uint8_t textbuf[14] = {0};
uint32_t ILI9341_ID = 0;

uint8_t lcd_flag = 0;
uint8_t touch_flag = 0;
uint8_t flash_flag = 0;
uint8_t eeprom_flag = 0;
uint8_t sdio_flag = 0;
uint8_t usb_flag = 0;

extern volatile uint8_t KEY_DOWN;

static void SPI_Flash_Text(void);
static void LCD_Text(void);
static void EEPROM_Text(void);
static void KEY_Text(void);
//static void USB_Text(void);
static void Periph_Display(void);


void USB_Delay(__IO uint32_t nCount)
{
	for(; nCount != 0; nCount--);
}

int main(void)
{
	uint32_t i = 0;
	
	/* LCD 初始化 */
	ILI9341_Init();  
	/* USART config */
	USART_Config();
	/* LED 初始化 */
  LED_GPIO_Config();
	/* SPI FLASH 初始化 */
	SPI_FLASH_Init();
	
	printf("\r\n *********** F103核心板综合测试程序 *********** \r\n");
	
	/* SD卡 测试 */
	SD_Test();
	/* EEPROM 测试 */
	EEPROM_Text();
	/* SPI FLASH 测试 */
	SPI_Flash_Text();
	/* LCD 测试 */
	LCD_Text();
	
	/*初始化SD卡*/
	Set_System(); 	
	/*设置USB时钟为48M*/
	Set_USBClock();	
	/*配置USB中断(包括SDIO中断)*/
	USB_Interrupts_Config();
	/*USB初始化*/
 	USB_Init();	
 	while (bDeviceState != CONFIGURED)
	{//等待配置完成
		i++;
		if(i >= 0x1FFFFFF)
			break;
	}
	/*打印USB状态*/
	if(i >= 0x1FFFFFF)
	{
		printf("USB:FAILED\r\n");
	}
	else
	{
		usb_flag = 1;
		printf("USB:OK\r\n");
	}
	
	SysTick_Init();
	/* 按键 初始化 */
	EXTI_Key_Config();
	/* TIM6 初始化 */
	BASIC_TIM_Init();
	
	/* 显示测试结果 */
	Periph_Display();
	
  while (1)
  {
    //按键测试
		KEY_Text();
		//触摸检测
    XPT2046_TouchEvenHandler();
  }
}

/**
  * @brief  SPI FLASH测试
  * @param  无
  * @param  无
  * @retval 无
  */
static void SPI_Flash_Text(void)
{
	if(SPI_FLASH_ReadID() == sFLASH_ID)
	{
		flash_flag = 1;
		printf("SPI FLASH: OK\r\n");
	}
	else
	{
		printf("SPI FLASH: FAILED\r\n");
	}
}

/**
  * @brief  LCD和触摸板测试
  * @param  无
  * @param  无
  * @retval 无
  */
static void LCD_Text(void)
{
	if(ILI9341_ReadID() == 0x9341)
	{
		//触摸屏初始化
		XPT2046_Init();
		//从FLASH里获取校正参数，若FLASH无参数，则使用模式3进行校正
		Calibrate_or_Get_TouchParaWithFlash(3,0);
		lcd_flag = 1;
		touch_flag = 1;
		printf("LCD: OK\r\n");
		printf("TOUCH PAD: OK\r\n");
		
		ILI9341_GramScan ( 3 );	
		LCD_SetFont(&Font16x24);
		//绘制触摸画板界面
		Palette_Init(LCD_SCAN_MODE);
	}
	else
	{
		 __set_PRIMASK(1);//关总中断
		printf("LCD: FAILED\r\n");
		printf("TOUCH PAD: FAILED\r\n");
		 __set_PRIMASK(0);//开总中断
	}
}

/**
  * @brief  EEPROM测试
  * @param  无
  * @param  无
  * @retval 无
  */
static void EEPROM_Text(void)
{
	if(ee_CheckOk() ==1)
	{
		eeprom_flag = 1;
		printf("EEPROM：OK\r\n");
	}
	else
	{
		printf("EEPROM：FAILED\r\n");
	}
}

/**
  * @brief  按键测试
  * @param  无
  * @param  无
  * @retval 无
  */
static void KEY_Text(void)
{
	switch(KEY_DOWN)
	{
		case 1:
			switch(i)
			{
				case 0:
					LED1_TOGGLE;
					ILI9341_BackLed_Control(DISABLE);
					printf("KEY 1 is pressed\r\n");
					KEY_DOWN = 0;
					i++;
				break;
				case 1:
					LED1_TOGGLE;
					ILI9341_BackLed_Control(ENABLE);
					printf("KEY 1 is pressed\r\n");
					KEY_DOWN = 0;
					i = 0;
				break;
			}break;
		case 2: 
			switch(i)
			{
				case 0:
					LED2_TOGGLE;
					ILI9341_BackLed_Control(DISABLE);
					printf("KEY 2 is pressed\r\n");
					KEY_DOWN = 0;
					i++;
				break;
				case 1:
					LED2_TOGGLE;
					ILI9341_BackLed_Control(ENABLE);
					printf("KEY 2 is pressed\r\n");
					KEY_DOWN = 0;
					i = 0;
				break;
			}break;
		default: break;
	}
}

/**
  * @brief  测试结果显示
  * @param  无
  * @param  无
  * @retval 无
  */
static void Periph_Display(void)
{
	LCD_SetFont(&Font8x16);
	if(lcd_flag)
	{
		LCD_SetColors(BLACK,WHITE);
		ILI9341_DispString_EN(80, 0,"LCD:OK");
	}
	if(touch_flag)
	{
		LCD_SetColors(BLACK,WHITE);
		ILI9341_DispString_EN(80, 24,"TOUCH PAD:OK");
	}
	else
	{
		LCD_SetColors(RED,WHITE);
		ILI9341_DispString_EN(80, 24,"TOUCH PAD:FAILED");
	}
	if(flash_flag)
	{
		LCD_SetColors(BLACK,WHITE);
		ILI9341_DispString_EN(80, 48,"SPI FLASH:OK");
	}
	else
	{
		LCD_SetColors(RED,WHITE);
		ILI9341_DispString_EN(80, 48,"SPI FLASH:FAILED");
	}
	if(eeprom_flag)
	{
		LCD_SetColors(BLACK,WHITE);
		ILI9341_DispString_EN(80, 72,"EEPROM: OK");
	}
	else
	{
		LCD_SetColors(RED,WHITE);
		ILI9341_DispString_EN(80, 72,"EEPROM: FAILED");
	}
	if(sdio_flag)
	{
		LCD_SetColors(BLACK,WHITE);
		ILI9341_DispString_EN(80, 94,"SDIO: OK");
	}
	else
	{
		LCD_SetColors(RED,WHITE);
		ILI9341_DispString_EN(80, 94,"SDIO: FAILED");
	}
	if(usb_flag)
	{
		LCD_SetColors(BLACK,WHITE);
		ILI9341_DispString_EN(80, 118,"USB: OK");
	}
	else
	{
		LCD_SetColors(RED,WHITE);
		ILI9341_DispString_EN(80, 118,"USB: FAILED");
	}
}

/* -------------------------------------end of file -------------------------------------------- */
