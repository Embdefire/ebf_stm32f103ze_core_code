/**
  ******************************************************************************
  * @file    bsp_led.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   STM32 SD�� usb mass storage ��ģ��U�̣�
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� F103-�Ե� STM32 ������ 
  * ��̳    :http://www.firebbs.cn
  * �Ա�    :https://fire-stm32.taobao.com
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
	
	/* LCD ��ʼ�� */
	ILI9341_Init();  
	/* USART config */
	USART_Config();
	/* LED ��ʼ�� */
  LED_GPIO_Config();
	/* SPI FLASH ��ʼ�� */
	SPI_FLASH_Init();
	
	printf("\r\n *********** F103���İ��ۺϲ��Գ��� *********** \r\n");
	
	/* SD�� ���� */
	SD_Test();
	/* EEPROM ���� */
	EEPROM_Text();
	/* SPI FLASH ���� */
	SPI_Flash_Text();
	/* LCD ���� */
	LCD_Text();
	
	/*��ʼ��SD��*/
	Set_System(); 	
	/*����USBʱ��Ϊ48M*/
	Set_USBClock();	
	/*����USB�ж�(����SDIO�ж�)*/
	USB_Interrupts_Config();
	/*USB��ʼ��*/
 	USB_Init();	
 	while (bDeviceState != CONFIGURED)
	{//�ȴ��������
		i++;
		if(i >= 0x1FFFFFF)
			break;
	}
	/*��ӡUSB״̬*/
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
	/* ���� ��ʼ�� */
	EXTI_Key_Config();
	/* TIM6 ��ʼ�� */
	BASIC_TIM_Init();
	
	/* ��ʾ���Խ�� */
	Periph_Display();
	
  while (1)
  {
    //��������
		KEY_Text();
		//�������
    XPT2046_TouchEvenHandler();
  }
}

/**
  * @brief  SPI FLASH����
  * @param  ��
  * @param  ��
  * @retval ��
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
  * @brief  LCD�ʹ��������
  * @param  ��
  * @param  ��
  * @retval ��
  */
static void LCD_Text(void)
{
	if(ILI9341_ReadID() == 0x9341)
	{
		//��������ʼ��
		XPT2046_Init();
		//��FLASH���ȡУ����������FLASH�޲�������ʹ��ģʽ3����У��
		Calibrate_or_Get_TouchParaWithFlash(3,0);
		lcd_flag = 1;
		touch_flag = 1;
		printf("LCD: OK\r\n");
		printf("TOUCH PAD: OK\r\n");
		
		ILI9341_GramScan ( 3 );	
		LCD_SetFont(&Font16x24);
		//���ƴ����������
		Palette_Init(LCD_SCAN_MODE);
	}
	else
	{
		 __set_PRIMASK(1);//�����ж�
		printf("LCD: FAILED\r\n");
		printf("TOUCH PAD: FAILED\r\n");
		 __set_PRIMASK(0);//�����ж�
	}
}

/**
  * @brief  EEPROM����
  * @param  ��
  * @param  ��
  * @retval ��
  */
static void EEPROM_Text(void)
{
	if(ee_CheckOk() ==1)
	{
		eeprom_flag = 1;
		printf("EEPROM��OK\r\n");
	}
	else
	{
		printf("EEPROM��FAILED\r\n");
	}
}

/**
  * @brief  ��������
  * @param  ��
  * @param  ��
  * @retval ��
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
  * @brief  ���Խ����ʾ
  * @param  ��
  * @param  ��
  * @retval ��
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
