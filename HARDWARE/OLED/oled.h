#ifndef __OLED_H__
#define __OLED_H__	
#include "includes.h"

#define OLED_MODE 0
#define SIZE 8
#define XLevelL		0x00
#define XLevelH		0x10
#define Max_Column	128
#define Max_Row		64
#define	Brightness	0xFF 
#define X_WIDTH 	128
#define Y_WIDTH 	64	    						  
//-----------------OLED IIC端口定义----------------  					   

#define I2C_PORT_1      0
#define I2C_PORT_2      1
#define OLED_I2C_PORT   I2C_PORT_2

#if OLED_I2C_PORT == I2C_PORT_1
#define SCL				PBout(2)

#define SCL_PORT        GPIOB
#define SDA_PORT        GPIOB

#define SCL_PORT_RCC    RCC_AHB1Periph_GPIOB
#define SDA_PORT_RCC    RCC_AHB1Periph_GPIOB

#define SCL_PIN         GPIO_Pin_2
#define SDA_PIN         GPIO_Pin_1

#define SDA_W			PBout(1)
#define SDA_R			PBin(1)

#endif

#if OLED_I2C_PORT == I2C_PORT_2
#define SCL				PBout(15)

#define SCL_PORT        GPIOB
#define SDA_PORT        GPIOD

#define SCL_PORT_RCC    RCC_AHB1Periph_GPIOB
#define SDA_PORT_RCC    RCC_AHB1Periph_GPIOD

#define SCL_PIN         GPIO_Pin_15
#define SDA_PIN         GPIO_Pin_10

#define SDA_W			PDout(10)
#define SDA_R			PDin(10)

#endif

#define OLED_CMD  0		//写命令
#define OLED_DATA 1		//写数据


//OLED控制用函数
extern void OLED_WR_Byte(unsigned dat,unsigned cmd);  
extern void OLED_Display_On(void);
extern void OLED_Display_Off(void);	   							   		    
extern void OLED_Init(void);
extern void OLED_Clear(void);
extern void OLED_Clear_Line(uint8_t line);  
extern void OLED_DrawPoint(uint8_t x,uint8_t y,uint8_t t);
extern void OLED_Fill(uint8_t x1,uint8_t y1,uint8_t x2,uint8_t y2,uint8_t dot);
extern void OLED_ShowChar(uint8_t x,uint8_t y,uint8_t chr,uint8_t Char_Size);
extern void OLED_ShowNum(uint8_t x,uint8_t y,uint32_t num,uint8_t len,uint8_t size);
extern void OLED_ShowString(uint8_t x,uint8_t y, uint8_t *p,uint8_t Char_Size);	 
extern void OLED_Set_Pos(unsigned char x, unsigned char y);
extern void OLED_ShowCHinese(uint8_t x,uint8_t y,uint8_t no);
extern void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char BMP[]);
extern void OLED_ClearRegion(unsigned char x0,unsigned y0,unsigned char x1, unsigned char y1);
extern void OLED_FillRegion(unsigned char x0,unsigned y0,unsigned char x1, unsigned char y1);
extern void OLED_Set_Direction(uint32_t direction);
extern uint32_t OLED_Get_Direction(void);

extern void OLED_GDDRAM_Backup(void);
extern void OLED_GDDRAM_Restore(void);
#endif  
	 



