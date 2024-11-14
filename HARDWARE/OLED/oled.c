#include "includes.h"

static uint32_t g_oled_direction=0;

static uint8_t g_oled_gddram    [8][128]={0};
static uint8_t g_oled_gddram_bak[8][128]={0};

//OLED的显存
//存放格式如下.
//[0]0 1 2 3 ... 127	
//[1]0 1 2 3 ... 127	
//[2]0 1 2 3 ... 127	
//[3]0 1 2 3 ... 127	
//[4]0 1 2 3 ... 127	
//[5]0 1 2 3 ... 127	
//[6]0 1 2 3 ... 127	
//[7]0 1 2 3 ... 127 			   
static void i2c_sda_mode(GPIOMode_TypeDef mode)
{
	/* 配置指定引脚为输出模式 */
	GPIO_InitStructure.GPIO_Pin =  SDA_PIN;					//指定引脚
	GPIO_InitStructure.GPIO_Mode = mode;					//设置输出/输入模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;			//开漏
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;		//设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;		//不需要上下拉电阻
	GPIO_Init(SDA_PORT, &GPIO_InitStructure);
}

static void i2c_start(void)
{

	//保证SDA引脚为输出模式
	i2c_sda_mode(GPIO_Mode_OUT);
	
	SCL=1;
	SDA_W=1;
	
	//延时1us
	delay_us(1);
	
	SDA_W=0;
	
	//延时1us
	delay_us(1);	
	
	
	//占用时钟线，告诉其他从机这是忙的状态
	SCL=0;
	
	//延时1us
	delay_us(1);
}

/**********************************************
//IIC Stop
**********************************************/
static void i2c_stop(void)
{
	//保证SDA引脚为输出模式
	i2c_sda_mode(GPIO_Mode_OUT);
	
	SCL=0;
	SDA_W=0;

	//延时1us
	delay_us(1);
	
	SCL=1;	
	
	//延时1us
	delay_us(1);	
	
	SDA_W=1;

	//延时1us
	delay_us(1);	
}

static uint8_t i2c_wait_ack(void)
{

	uint8_t ack=0;
	
	//保证SDA引脚为输入模式
	i2c_sda_mode(GPIO_Mode_IN);
	
	SCL=1;
	
	//延时1us
	delay_us(1);
	
	//判断SDA引脚的电平
	if(SDA_R)
	{
		ack=1;
		
		//中止整个i2c通信
		i2c_stop();
	}
	
	SCL=0;
	
	//延时1us
	delay_us(1);
	
	return ack;
}

void Write_IIC_Byte(unsigned char IIC_Byte)
{
	uint32_t i=0;
	
	//保证SDA引脚为输出模式
	i2c_sda_mode(GPIO_Mode_OUT);
	
	SCL=0;
	SDA_W=0;

	//延时1us
	delay_us(1);
	
	for(i=0; i<8; i++)
	{
		//以MSB最高有效位进行数据的发送
		if(IIC_Byte & (1<<(7-i)))
			SDA_W=1;
		else
			SDA_W=0;
		
		//延时1us
		delay_us(1);
		
		//设置时钟线为高电平，告诉从机现在可以读取SDA引脚的电平
		SCL=1;	
		
		//延时1us
		delay_us(1);	

		//设置时钟线为低电平，告诉从机现在不可以读取SDA引脚的电平，因为主机现在要更改SDA引脚的电平
		SCL=0;	
		
		//延时1us
		delay_us(1);			
	}
}

void Write_IIC_Command(unsigned char IIC_Command)
{
	i2c_start();
	Write_IIC_Byte(0x78);         
	i2c_wait_ack();	
	Write_IIC_Byte(0x00);		
	i2c_wait_ack();	
	Write_IIC_Byte(IIC_Command); 
	i2c_wait_ack();	
	i2c_stop();
}

void Write_IIC_Data(unsigned char IIC_Data)
{
	i2c_start();
	Write_IIC_Byte(0x78);			//D/C#=0; R/W#=0
	i2c_wait_ack();	
	Write_IIC_Byte(0x40);			//write data
	i2c_wait_ack();	
	Write_IIC_Byte(IIC_Data);
	i2c_wait_ack();	
	i2c_stop();
}
void OLED_WR_Byte(unsigned dat,unsigned cmd)
{
	if(cmd)
	{

		Write_IIC_Data(dat);

	}
	else 
	{
		Write_IIC_Command(dat);
		
	}
}

void Write_IIC_Datas(unsigned char *IIC_Data,uint32_t len)
{
	unsigned char *p = IIC_Data;
	
	i2c_start();
	Write_IIC_Byte(0x78);			//D/C#=0; R/W#=0
	i2c_wait_ack();	
	

	Write_IIC_Byte(0x40);			
	i2c_wait_ack();	

	while(len--)
	{	
		Write_IIC_Byte(*p++);
		i2c_wait_ack();		
	}

	i2c_stop();
}

void Write_IIC_Datas_Fix(unsigned char IIC_Data,uint32_t len)
{
	
	i2c_start();
	Write_IIC_Byte(0x78);			//D/C#=0; R/W#=0
	i2c_wait_ack();	
	

	Write_IIC_Byte(0x40);			
	i2c_wait_ack();	

	while(len--)
	{	
		Write_IIC_Byte(IIC_Data);
		i2c_wait_ack();		
	}

	i2c_stop();
}


void OLED_Set_Direction(uint32_t direction)
{
	uint32_t i;
	
	g_oled_direction = direction;
	
	if(direction == 0)
	{
		OLED_WR_Byte(0xC8,OLED_CMD);//行扫描顺序：从上到下
		OLED_WR_Byte(0xA1,OLED_CMD);//列扫描顺序：从左到右	
	}

	if(direction == 180)
	{
		OLED_WR_Byte(0xC0,OLED_CMD);//行扫描顺序：从下到上
		OLED_WR_Byte(0xA0,OLED_CMD);//列扫描顺序：从右到左
	}
	
			
    /* 更新显示 */	
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址

		Write_IIC_Datas((unsigned char *)&g_oled_gddram[i],128);
			
	} 
}


void OLED_GDDRAM_Backup(void)
{
	
	memcpy(g_oled_gddram_bak,g_oled_gddram,sizeof(g_oled_gddram));

}

void OLED_GDDRAM_Restore(void)
{
	uint32_t i;
	
	memcpy(g_oled_gddram,g_oled_gddram_bak,sizeof(g_oled_gddram));
	
    /* 更新显示 */	
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址

		Write_IIC_Datas((unsigned char *)&g_oled_gddram[i],128);
			
	} 	

}

uint32_t OLED_Get_Direction(void)
{
	return g_oled_direction;
}

//坐标设置

void OLED_Set_Pos(unsigned char x, unsigned char y) 
{ 	
	OLED_WR_Byte(0xb0+y,OLED_CMD);
	OLED_WR_Byte(((x&0xf0)>>4)|0x10,OLED_CMD);
	OLED_WR_Byte((x&0x0f),OLED_CMD); 
}   	  
//开启OLED显示    
void OLED_Display_On(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X14,OLED_CMD);  //DCDC ON
	OLED_WR_Byte(0XAF,OLED_CMD);  //DISPLAY ON
}
//关闭OLED显示     
void OLED_Display_Off(void)
{
	OLED_WR_Byte(0X8D,OLED_CMD);  //SET DCDC命令
	OLED_WR_Byte(0X10,OLED_CMD);  //DCDC OFF
	OLED_WR_Byte(0XAE,OLED_CMD);  //DISPLAY OFF
}		   			 
//清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!	  
void OLED_Clear(void)  
{  
	u8 i;		
    
	for(i=0;i<8;i++)  
	{  
		OLED_WR_Byte (0xb0+i,OLED_CMD);    //设置页地址（0~7）
		OLED_WR_Byte (0x00,OLED_CMD);      //设置显示位置—列低地址
		OLED_WR_Byte (0x10,OLED_CMD);      //设置显示位置—列高地址

		Write_IIC_Datas_Fix(0,128);
		
		/* 将oled每列像素点数据写入g_oled_gddram */
		memset(&g_oled_gddram[i],0,128);
		
	} //更新显示
}

//在指定位置显示一个字符,包括部分字符
//x:0~127
//y:0~63
//mode:0,反白显示;1,正常显示				 
//size:选择字体 16/12 
void OLED_ShowChar(u8 x,u8 y,u8 chr,u8 Char_Size)
{      	
	unsigned char c=0;	
	c=chr-' ';//得到偏移后的值	
	
	if(x>Max_Column-1){x=0;y=y+2;}
	
	if(Char_Size ==16)
	{
		OLED_Set_Pos(x,y);	
		Write_IIC_Datas((unsigned char *)&F8X16[c*16],8);

		/* 将oled每列像素点数据写入g_oled_gddram */
		memcpy(&g_oled_gddram[y][x],(unsigned char *)&F8X16[c*16],8);		

		OLED_Set_Pos(x,y+1);
		Write_IIC_Datas((unsigned char *)&F8X16[c*16+8],8);
		
		/* 将oled每列像素点数据写入g_oled_gddram */
		memcpy(&g_oled_gddram[y+1][x],(unsigned char *)&F8X16[c*16+8],8);			

	}
	else 
	{	
		OLED_Set_Pos(x,y);
		Write_IIC_Datas((unsigned char *)&F6x8[c],6);
		
		/* 将oled每列像素点数据写入g_oled_gddram */
		memcpy(&g_oled_gddram[y][x],(unsigned char *)&F8X16[c],6);			

	}
}
//m^n函数
u32 oled_pow(u8 m,u8 n)
{
	u32 result=1;	 
	while(n--)result*=m;    
	return result;
}				  
//显示2个数字
//x,y :起点坐标	 
//len :数字的位数
//size:字体大小
//mode:模式	0,填充模式;1,叠加模式
//num:数值(0~4294967295);	 		  
void OLED_ShowNum(u8 x,u8 y,u32 num,u8 len,u8 size2)
{         	
	u8 t,temp;
	u8 enshow=0;						   
	for(t=0;t<len;t++)
	{
		temp=(num/oled_pow(10,len-t-1))%10;
		if(enshow==0&&t<(len-1))
		{
			if(temp==0)
			{
				OLED_ShowChar(x+(size2/2)*t,y,' ',size2);
				continue;
			}else enshow=1; 
		 	 
		}
	 	OLED_ShowChar(x+(size2/2)*t,y,temp+'0',size2); 
	}
} 
//显示一个字符号串
void OLED_ShowString(u8 x,u8 y,u8 *chr,u8 Char_Size)
{
	unsigned char j=0;
	while (chr[j]!='\0')
	{		OLED_ShowChar(x,y,chr[j],Char_Size);
			x+=8;
		if(x>120){x=0;y+=2;}
			j++;
	}
}
//显示汉字
void OLED_ShowCHinese(u8 x,u8 y,u8 no)
{      			    
	OLED_Set_Pos(x,y);	
	Write_IIC_Datas((unsigned char *)&Hzk[2*no],16);

	/* 将oled每列像素点数据写入g_oled_gddram */
	memcpy(&g_oled_gddram[y][x],(unsigned char *)&Hzk[2*no],16);		
	
	OLED_Set_Pos(x,y+1);	
	Write_IIC_Datas((unsigned char *)&Hzk[2*no+1],16);	
	
	/* 将oled每列像素点数据写入g_oled_gddram */
	memcpy(&g_oled_gddram[y+1][x],(unsigned char *)&Hzk[2*no+1],16);		
	

}
/***********功能描述：显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7*****************/
void OLED_DrawBMP(unsigned char x0, unsigned char y0,unsigned char x1, unsigned char y1,const unsigned char *BMP)
{ 	
	uint32_t j=0;
	unsigned char y;

	if(y1%8==0) 
		y=y1/8;      
	else 
		y=y1/8+1;
	
	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
	
		Write_IIC_Datas((unsigned char *)&BMP[j],x1-x0);

		/* 将oled每列像素点数据写入g_oled_gddram */
		memcpy(&g_oled_gddram[y][x0],(unsigned char *)&BMP[j],x1-x0);
		
		j+=(x1-x0);
	
	}
}
void OLED_ClearRegion(unsigned char x0,unsigned y0,unsigned char x1, unsigned char y1)
{
	unsigned char y;
	
	if(y1%8==0) 
		y=y1/8;      
	else 
		y=y1/8+1;

	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
		Write_IIC_Datas_Fix(0,x1-x0);
		
		/* 将oled每列像素点数据写入g_oled_gddram */
		memset(&g_oled_gddram[y][x0],0,x1-x0);		
	}
}

void OLED_FillRegion(unsigned char x0,unsigned y0,unsigned char x1, unsigned char y1)
{
	unsigned char y;
	
	if(y1%8==0) 
		y=y1/8;      
	else 
		y=y1/8+1;

	for(y=y0;y<y1;y++)
	{
		OLED_Set_Pos(x0,y);
		
		Write_IIC_Datas_Fix(1,x1-x0);
		
		/* 将oled每列像素点数据写入g_oled_gddram */
		memset(&g_oled_gddram[y][x0],0,x1-x0);		
	}
}

//初始化SSD1306					    
void OLED_Init(void)
{ 	

	GPIO_InitTypeDef  GPIO_InitStructure;
	
	//使能端口的时钟
	RCC_AHB1PeriphClockCmd(SCL_PORT_RCC|SDA_PORT_RCC, ENABLE);		

	/* 配置SCL引脚为开漏输出模式 */
	GPIO_InitStructure.GPIO_Pin = SCL_PIN;				//指定引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		//设置输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//开漏(若当前接口没有外部上拉电阻，且只连接oled屏，可以使用推挽输出来提供足够的驱动电流)
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//不需要上下拉电阻
	
	GPIO_Init(SCL_PORT, &GPIO_InitStructure);


	/* 配置SDA引脚为开漏输出模式 */
	GPIO_InitStructure.GPIO_Pin = SDA_PIN;				//指定引脚
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;		//设置输出模式
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;		//开漏
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;	//设置IO的速度为100MHz，频率越高性能越好，频率越低，功耗越低
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;	//不需要上下拉电阻


	GPIO_Init(SDA_PORT, &GPIO_InitStructure);


	delay_ms(200);

	OLED_WR_Byte(0xAE,OLED_CMD);//--display off,关显示
	OLED_WR_Byte(0x00,OLED_CMD);//---set low column address
	OLED_WR_Byte(0x10,OLED_CMD);//---set high column address
	OLED_WR_Byte(0x40,OLED_CMD);//--set start line address  
	OLED_WR_Byte(0xB0,OLED_CMD);//--set page address
	OLED_WR_Byte(0x81,OLED_CMD); // contract control
	OLED_WR_Byte(0xFF,OLED_CMD);//--128   
	OLED_WR_Byte(0xA6,OLED_CMD);//--normal / reverse
	OLED_WR_Byte(0xA8,OLED_CMD);//--set multiplex ratio(1 to 64)
	OLED_WR_Byte(0x3F,OLED_CMD);//--1/32 duty


	OLED_WR_Byte(0xC8,OLED_CMD);//行扫描顺序：从上到下
	OLED_WR_Byte(0xA1,OLED_CMD);//列扫描顺序：从左到右

	OLED_WR_Byte(0xD3,OLED_CMD);//-set display offset
	OLED_WR_Byte(0x00,OLED_CMD);//
	
	OLED_WR_Byte(0xD5,OLED_CMD);//set osc division
	OLED_WR_Byte(0x80,OLED_CMD);//
	
	OLED_WR_Byte(0xD8,OLED_CMD);//set area color mode off
	OLED_WR_Byte(0x05,OLED_CMD);//
	
	OLED_WR_Byte(0xD9,OLED_CMD);//Set Pre-Charge Period
	OLED_WR_Byte(0xF1,OLED_CMD);//
	
	OLED_WR_Byte(0xDA,OLED_CMD);//set com pin configuartion
	OLED_WR_Byte(0x12,OLED_CMD);//
	
	OLED_WR_Byte(0xDB,OLED_CMD);//set Vcomh
	OLED_WR_Byte(0x30,OLED_CMD);//
	
	OLED_WR_Byte(0x8D,OLED_CMD);//set charge pump enable，升压允许
	OLED_WR_Byte(0x14,OLED_CMD);//
	
	OLED_WR_Byte(0xAF,OLED_CMD);//--turn on oled panel,开启显示
}  
