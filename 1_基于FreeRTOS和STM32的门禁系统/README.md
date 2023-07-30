# 基于FreeRTOS和STM32的门禁系统

## 所用器件

STM32F4探索者、RFID-RC522、SG90舵机、AS608指纹模块、4.3寸TFTLCD屏。

## 主要功能

模拟一个智能化的门禁系统。

1. 可在LCD屏上输入三位正确密码开门。
2. 可通过刷绑定的卡进行开门。
3. 可通过指纹进行开门。
4. 以上三种方式任意一种验证通过，蜂鸣器短暂响起，同时模拟大门的舵机会旋转180°。
5. 三种方式中任意一种验证错误时，会发出短暂的蜂鸣声。
6. 当三种方式错误累计三次，舵机任务被挂起，之后无法开门。

## FreeRTOS移植

1. 拷贝正点原子探索者的库函数工程模板。

![image-20230705101154416](../assets/5_基于FreeRTOS和STM32的智能门禁系统/拷贝工程.png)

2. 在拷贝的工程里新建一个FreeRTOS文件夹。

![image-20230705101723106](../assets/5_基于FreeRTOS和STM32的智能门禁系统/新建FreeRTOS文件夹.png)

3. 将所有文件复制到新建的FreeRTOS文件夹中。

![image-20230705104217943](../assets/5_基于FreeRTOS和STM32的智能门禁系统/复制Source中文件.png)

4. 上一张图片中portable只保留下列文件。

![image-20230705104517692](../assets/5_基于FreeRTOS和STM32的智能门禁系统/修减portable文件.png)

5. 打开keil，新建两个分组FreeRTOS_CORD和FreeRTOS_PORTABLE，分组中添加的文件如下。

![image-20230705104926402](../assets/5_基于FreeRTOS和STM32的智能门禁系统/FreeRTOS_CORE文件.png)

![image-20230705105922273](../assets/5_基于FreeRTOS和STM32的智能门禁系统/FreeRTOS_PORTABLE文件.png)

![image-20230705110143023](../assets/5_基于FreeRTOS和STM32的智能门禁系统/RVDS.png)

（注意：这里的port.c根据具体型号选择，因为STM32F407ZGT6是Cortex-M4的，所以这里选择ARM_CM4F）

6. 添加头文件。

![image-20230705110446573](../assets/5_基于FreeRTOS和STM32的智能门禁系统/添加头文件.png)

7. 编译后遇到很多问题。

首先勾选C99，避免一些编译错误，之后参考下面的链接。

解决问题参考链接：https://www.freesion.com/article/41971435876/#03_FreeRTOS_19

## SG90舵机

1. 接线

|      SG90舵机       | STM32F4探索者 |
| :-----------------: | :-----------: |
|     GND(棕色线)     |      GND      |
|     VCC(红色线)     |      5V       |
| SIG(黄色线)——信号线 |      PF9      |

2. 舵机控制原理

![image-20230706141900211](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230706141900211.png)

3. 舵机初始化

​	使用PF9作为舵机的信号输出引脚，同时使用TIM14产生PWM波。

```c
void SG90_Init(u32 arr, u32 psc)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    
    RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOF, ENABLE );
    RCC_APB1PeriphClockCmd( RCC_APB1Periph_TIM14, ENABLE );
    
    GPIO_PinAFConfig( GPIOF, GPIO_PinSource9, GPIO_AF_TIM14 );  //GPIOF9复用为定时器14
    
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init( GPIOF, &GPIO_InitStruct );
    
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = arr;
    TIM_TimeBaseInitStruct.TIM_Prescaler = psc;
    TIM_TimeBaseInit( TIM14, &TIM_TimeBaseInitStruct );
    
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1; //选择定时器模式:TIM脉冲宽度调制模式2
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable; //比较输出使能
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_High; //输出极性:TIM输出比较极性低
    TIM_OC1Init( TIM14, &TIM_OCInitStruct );
    
    TIM_OC1PreloadConfig( TIM14, TIM_OCPreload_Enable );  //使能TIM14在CCR1上的预装载寄存器
    TIM_ARRPreloadConfig( TIM14,ENABLE );//ARPE使能
    TIM_Cmd( TIM14, ENABLE );  //使能TIM14
    
    SG90_SetAngle( 0 );
}
```

4. 舵机控制角度

```c
void SG90_SetAngle( float angle )
{
    int servo_temp;
    
    if ( angle > 180 )
        angle = 180;
    else if ( angle < 0 )
        angle = 0;
    
    servo_temp = angle * 4000 / 180 + 1000;
    TIM_SetCompare1(TIM14,servo_temp);
}
```

## RFID-RC522

1. 接线

| RFID-RC522 | STM32F4探索者 |
| :--------: | :-----------: |
|  SDA / CS  |      PF0      |
|    SCK     |      PF1      |
|    MOSI    |      PF2      |
|    MISO    |      PF3      |
|    IRQ     |     悬空      |
|    GND     |      GND      |
|    RST     |      PF4      |
|    3.3V    |     3.3V      |

2. 工作原理

![image-20230709160755209](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230709160755209.png)

![image-20230709161519881](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230709161519881.png)

- 其中第0扇区的块0是用于存放厂商代码的，已经固化，不可更改，为32位（4Bytes）；
- 每个扇区的块0、块1和块2位数据块，可用于存储数据，每块16个字节（只有S50卡是这样）；
- 每个扇区的块3位控制块，包含了密码A、存取控制、密码B。

具体原理可以参考网上：http://t.csdn.cn/2FyIV

3. 具体使用

与RFID-RC522通信是使用<font color=red>SPI协议</font>，硬件和软件都行，这里使用软件模拟SPI通信。

```C
void SPI1_Init( void )
{
	GPIO_InitTypeDef  GPIO_InitStructure;	

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF,ENABLE); //使能端口F时钟
	
	GPIO_InitStructure.GPIO_Mode=GPIO_Mode_OUT; //输出模式
	GPIO_InitStructure.GPIO_Pin=GPIO_Pin_1|GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_4;//管脚设置
	GPIO_InitStructure.GPIO_Speed=GPIO_Speed_100MHz;//速度为100M
	GPIO_InitStructure.GPIO_OType=GPIO_OType_PP;//推挽输出
	GPIO_InitStructure.GPIO_PuPd=GPIO_PuPd_UP;//上拉
	GPIO_Init(GPIOF,&GPIO_InitStructure); //初始化结构体		     
	GPIO_SetBits(GPIOF,GPIO_Pin_1|GPIO_Pin_0|GPIO_Pin_2|GPIO_Pin_4);  //拉高

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; 	 //上拉输入			   
	GPIO_Init(GPIOF, &GPIO_InitStructure);
}
```

读卡的ID大致分为这几步：

（1）寻卡：当有卡进入读写器范围时，读写器与卡进行通信。

（2）防冲撞：当多个卡进入读写器范围时，选定其中一张卡进行读写。

（3）读ID：读取卡的ID。

（4）验证：将读取到的ID与预先存入的ID进行比较。

```c
unsigned char kahao1[4] = {0x8E, 0x49, 0xC3, 0x20};     //8E49C320  ————绑定的卡ID

u8 Identify_CardID(void)
{
    while(1)
    {
        status = PcdRequest( PICC_REQALL, CT );                /* 寻卡 */
        if ( status != MI_OK )                                  /* 寻卡成功 */
        {
            continue;
        }
        status = PcdAnticoll( ID );                             /* 防冲撞 */

        status = PcdSelect( ID );                               /*选定卡片 */
        if ( status != MI_OK )                                  /*选定卡片成功 */
        {
            if(ID[0]==kahao1[ 0 ]&&ID[ 1 ]==kahao1[ 1 ]&&ID[ 2 ]==kahao1[ 2 ]&&ID[ 3 ]==kahao1[ 3 ])
            {
                printf("ok\r\n");

                delay_ms(1000);
                return 1;
            }
            else
                printf("no\r\n");

            delay_ms(1000);
            return 0;
        }
    }
}
```

## AS608指纹模块

1. 引脚说明

![image-20230707112845263](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230707112845263.png)

2. 接线

先与USB-TO-TTL接线，准备录入指纹。

| AS608指纹模块 |           STM32F4探索者           |
| :-----------: | :-------------------------------: |
|    V+ / Vi    |               3.3V                |
|      TX       |             UART2-RX              |
|      RX       |             UART2-TX              |
|      GND      |                GND                |
|   TCH / WAK   | PA6(设置为下拉，触摸后输出高电平) |
|    VA / Vt    |               3.3V                |
|    D+ / U+    |               不接                |
|    D- / U-    |               不接                |

3. 工作原理

​	AS608通信方式有两种，串口通讯和USB通讯，这里使用串口通讯。而使用串口通信是通过发送一个个数据包/指令包进行通讯。

![image-20230709214208984](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230709214208984.png)

​	使用指纹模块，首先需要先录入指纹，之后才能与已有指纹库进行配对。而录入指纹可以用程序实现，也可以用指纹模块测试上位机实现，这里展示用测试上位机实现录入指纹。

（1）接线

| AS608 | USB-TO-TTL |
| :---: | :--------: |
|  Vi   |    3.3V    |
|  Tx   |     Rx     |
|  Rx   |     Tx     |
|  GND  |    GND     |

（2）打开指纹测试

![image-20230709215743536](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230709215743536.png)

（3）打开设备

其它一般不变，先打开“打开设备”，选择对应的端口。

![image-20230709220345594](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230709220345594.png)

![image-20230709220323950](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230709220323950.png)

## 触摸密码键盘

1. 移植代码

​	将正点原子例程 实验28 触摸屏实验 中的文件复制到该工程中，复制文件如下：

![image-20230710110925544](../assets/5_基于FreeRTOS和STM32的智能门禁系统/image-20230710110925544.png)

2. 思路介绍

我所使用的屏为<font color=blue size=5>4.3寸TFTLCD屏</font>，其它屏幕得自己调整像素点的位置。

大体样式如下：

![image-20230730104706109](https://gitee.com/iot-lzw/notes/raw/master/picture/202307301047249.png)

- coded：文本显示，无用。
- :***    ：密码框，每次点击数字键，生成一个⭐，最多三位。
- lock：文本显示，无用。
- 0~9：密码输入键位。
- DEL：删除上一次输入的数字。
- Enter：确认密码。

按下键位时，对应的按键背景色变为绿色，抬起变回黄色。

确认密码，正确或错误都会在屏幕上提示。



打开lcd.c文件，在最后添加一些函数。

```C
//细节到工程看代码
void Show_Str(u16 x,u16 y,u16 width,u16 height,u8*str,u8 size,u8 mode);
void Show_Str_Mid(u16 x,u16 y,u8*str,u8 size,u8 len);

/* 在LCD屏上显示数字键盘 */
void AS608_load_keyboard(u16 x,u16 y,u8 **kbtbl)
{
    u8 i = 0;
    POINT_COLOR = RED;
    kbd_tbl = kbtbl;
    LCD_Fill( x, y, x + 450, y + 400, YELLOW );
    LCD_Fill( x+150, y, x + 300, y + 80, GREEN );
    LCD_DrawRectangle( x, y, x + 450, y + 400 );
    LCD_DrawRectangle( x + 150, y, x + 300, y + 400 );
    LCD_DrawRectangle( x, y + 80, x + 450, y + 160 );
    LCD_DrawRectangle( x, y + 240, x + 450, y + 320 );
    POINT_COLOR = BLUE;
    
    for(i = 0; i < 15; i++)
    {
        if(i == 1)
            Show_Str(x+(i%3)*150+20,y+28+80*(i/3),150,80,(u8*)kbd_tbl[i],16,0);
        else
			Show_Str_Mid(x+8+(i%3)*150,y+28+80*(i/3),(u8*)kbd_tbl[i],16,150);
    }
}

/* 这里实现点击效果，按下改变背景色 */
void AS608_key_staset(u16 x,u16 y,u8 keyx,u8 sta)
{
    u16 i=keyx/3,j=keyx%3;
    if(keyx>16)return;
    if(sta &&keyx!=1)   //如果被按下且不是“:”，则背景填充绿色
        LCD_Fill(x+j*150+1,y+i*80+1,x+j*150+149,y+i*80+78,GREEN);
    else if(keyx!=1)    //如果没有被按下且不是“:”，则背景填充黄色
        LCD_Fill(x+j*150+1,y+i*80+1,x+j*150+149,y+i*80+78,YELLOW);
    if(keyx!=1)     //如果不是“:”，则将字符显示方格中间
        Show_Str_Mid(x+j*150+8,y+28+80*i,(u8*)kbd_tbl[keyx],16,150);
}

/* 获取被按下键的编号 */
u8 AS608_get_keynum(u16 x,u16 y)
{
    u16 i,j;

    static u8 key_x=0;//0,没有任何按键按下
    u8 key=0;
    tp_dev.scan(0);
    if(tp_dev.sta&TP_PRES_DOWN)//触摸屏被按下
    {
        for(i=0; i<5; i++)
        {
            for(j=0; j<3; j++)
            {
                if(tp_dev.x[0]<(x+j*150+150)&&tp_dev.x[0]>(x+j*150)&&tp_dev.y[0]<(y+i*80+80)&&tp_dev.y[0]>(y+i*80))
                {
                    key=i*3+j+1;
                    break;
                }
            }
            if(key)
            {
                if(key_x==key)key=0;
                else
                {
                    AS608_key_staset(x,y,key_x-1,0);
                    key_x=key;
                    AS608_key_staset(x,y,key_x-1,1);
                    printf( "按键按下\r\n" );
                }
                break;
            }
        }
    } else if(key_x)
    {
        AS608_key_staset(x,y,key_x-1,0);
        key_x=0;
    }
    return key;
}

/* 对输入密码的一些限制 */
u16 GET_NUM(void)
{
    int n=0;
    volatile u8 key[4];
    u8 mima[3]= {'1','2','3'};	//正确密码
    u8  key_num=0;
    u8 key_flag=0;
    u16 num=0;
    u8 a=0;
    while(1)
    {
        key_num=AS608_get_keynum( 15, 350 );
        if(key_num)
        {
            if(key_num>3&&key_num<13&&num<99)//‘1-9’键(限制输入3位数)
            {
                num =num*10+key_num-3;
            }
            if(key_num==4)
            {
                key[n]='1';
                n++;
            }
            if(key_num==5)
            {
                key[n]='2';
                n++;
            }
            if(key_num==6)
            {
                key[n]='3';
                n++;
            }
            if(key_num==7)
            {
                key[n]='4';
                n++;
            }
            if(key_num==8)
            {
                key[n]='5';
                n++;
            }
            if(key_num==9)
            {
                key[n]='6';
                n++;
            }
            if(key_num==10)
            {
                key[n]='7';
                n++;
            }
            if(key_num==11)
            {
                key[n]='8';
                n++;
            }
            if(key_num==12)
            {
                key[n]='9';
                n++;
            }

            if(key_num==13)
            {
                num =num/10;//‘Del’键

                key[n]=' ';
                n--;
                if(n<=0)
                {
                    n=0;
                }
                printf("%d",n);
                key_flag=0;
            }

            if(key_num==14&&num<99)
            {
                num =num*10;//‘0’键
                key[n]='0';
                n++;
            }

            if(key_num==15)//‘Enter’键
            {
                for(a=0; a<3; a++)
                {
                    if(key[a]==mima[a])
                    {
                        key_flag=key_flag+1;
                    }
                }
                if(key_flag==3)
                {
                    printf("密码正确");
                    key_flag=0;
                    return 1;
                }
                else
                {
                    printf("密码错误");
                    key[0]=' ';
                    key[1]=' ';
                    key[2]=' ';
                    key_flag=0;
                    return 0;
                }
            }
        }
        if(n==0)
        {
            LCD_ShowString(165+37,350+28,260,16,16,"   ");
        }
        if(n==1)
        {
            LCD_ShowString(165+37,350+28,260,16,16,"*  ");
        }
        if(n==2)
        {
            LCD_ShowString(165+37,350+28,260,16,16,"** ");
        }
        if(n==3)
        {
            LCD_ShowString(165+37,350+28,260,16,16,"***");
        }
    }
}
```



## 开发架构

首先，在main函数中初始化所有外设。

之后，创建一个开始任务start_task，在开始任务函数中进入<font color=blue size=6>临界区</font>，进入临界区可以保证不会被其它任务打断执行过程。

在临界区里，使用了一个事件组，密码输入、刷卡和指纹三种解锁方式可以对事件组的位[2:0]进行置位，当某种方式解锁成功，会把对应位置1，舵机任务监听到后，会开启大门。

```c
#define EVENTBIT_0	    (1<<0)				//触摸密码键盘事件位
#define EVENTBIT_1	    (1<<1)              //RFID事件位
#define EVENTBIT_2	    (1<<2)              //指纹事件位
#define EVENTBIT_ALL	(EVENTBIT_0|EVENTBIT_1|EVENTBIT_2)
```

再然后，就是相关任务的创建了。

下面是相关任务优先级和栈大小的宏定义：

```c
#define START_TASK_PRIO         1
#define START_STK_SIZE          512
#define SG90_TASK_PRIO		    4
#define SG90_STK_SIZE 		    512  
#define LCD_TASK_PRIO		    3
#define LCD_STK_SIZE 		    512  
#define RFID_TASK_PRIO		    3
#define RFID_STK_SIZE 		    512  
#define AS608_TASK_PRIO		    3
#define AS608_STK_SIZE 		    512  
#define ESP8266_TASK_PRIO		3
#define ESP8266_STK_SIZE 		512 
```

### 开始任务start_task函数

```c
void start_task(void *param)
{
    BaseType_t xReturn;
    taskENTER_CRITICAL();           //进入临界区
    
    EventGroupHandler = xEventGroupCreate();
    if ( EventGroupHandler != NULL )
        printf ( "xEventGroupCreate事件组创建成功\r\n" );
    else 
        printf ( "xEventGroupCreate事件组创建失败\r\n" );
    
    xReturn = xTaskCreate ( SG90_task, "SG90_task", SG90_STK_SIZE, NULL, SG90_TASK_PRIO, &SG90Task_Handler );
    if ( xReturn == pdPASS )
        printf ( "SG90_task任务创建成功\r\n" );
    
    xReturn = xTaskCreate ( LCD_task, "LCD_task", LCD_STK_SIZE, NULL, LCD_TASK_PRIO, &LCDTask_Handler );
    if ( xReturn == pdPASS )
        printf ( "LCD_task任务创建成功\r\n" );
    
    xReturn = xTaskCreate( RFID_task, "RFID_task", RFID_STK_SIZE, NULL, RFID_TASK_PRIO, &RFIDTask_Handler ); 
    if(xReturn==pdPASS)
		printf("RFID_task任务创建成功\r\n");
    
    xReturn = xTaskCreate( AS608_task, "AS608_task", AS608_STK_SIZE, NULL, AS608_TASK_PRIO, &AS608Task_Handler ); 
    if(xReturn==pdPASS)
        printf("AS608_task任务创建成功\r\n");
    
    xReturn = xTaskCreate( ESP8266_task, "ESP8266_task", ESP8266_STK_SIZE, NULL, ESP8266_TASK_PRIO, &ESP8266Task_Handler ); 
    if(xReturn==pdPASS)
		printf("ESP8266_task任务创建成功\r\n");
    
    vTaskDelete(StartTask_Handler); //删除开始任务
    taskEXIT_CRITICAL();            //退出临界区
}
```

### 舵机任务SG90_task函数

```c
void SG90_task(void *pvParameters)
{
    volatile EventBits_t EventValue;
    
    while ( 1 )
    {
        EventValue = xEventGroupWaitBits ( EventGroupHandler, EVENTBIT_ALL, pdTRUE, pdFALSE, portMAX_DELAY );
        printf ( "接收事件成功\r\n" );
        SG90_SetAngle ( 180 );
        delay_xms ( 1000 );
        delay_xms ( 1000 );
        SG90_SetAngle ( 0 );
        LCD_ShowString ( 80,150,260,16,16, (u8*)"              " );
        
        vTaskDelay(100);    //延时10ms，也就是10个时钟节拍
    }
}
```

### 密码输入任务LCD_task函数

```c
void LCD_task(void *pvParameters)
{
    while ( 1 )
    {
        if ( sg90flag == 1 || GET_NUM() )
        {
            BEEP=1;
            delay_xms(100);
            BEEP=0;
            printf("密码输入正确\r\n");
            LCD_ShowString(80,150,260,16,16, (u8*)"password match");
            xEventGroupSetBits(EventGroupHandler,EVENTBIT_0);
        }
        else
        {
            BEEP=1;
            delay_xms(50);
            BEEP=0;
            delay_xms(50);
            BEEP=1;
            delay_xms(50);
            BEEP=0;
            delay_xms(50);
            BEEP=1;
            delay_xms(50);
            BEEP=0;
            printf("密码输入错误\r\n");
            LCD_ShowString(80,150,260,16,16,"password error");
            err++;
            if(err==3)
            {
                vTaskSuspend(SG90Task_Handler);
                printf("舵机任务挂起\r\n");
                LCD_ShowString(0,100,260,16,16,"Task has been suspended");
            }
        }
        vTaskDelay(100); //延时10ms，也就是10个时钟节拍
    }
}    
```

### 刷卡任务RFID_task函数

```C
void RFID_task(void *pvParameters)
{
    while ( 1 )
    {
        if ( rfidflag == 1 || Identify_CardID() )
        {
            BEEP=1;
            delay_xms(100);
            BEEP=0;
            Chinese_Show_two(30,50,16,16,0);
            Chinese_Show_two(50,50,18,16,0);
            Chinese_Show_two(70,50,20,16,0);
            Chinese_Show_two(90,50,8,16,0);
            Chinese_Show_two(110,50,10,16,0);

            xEventGroupSetBits(EventGroupHandler,EVENTBIT_1);
            printf("识别卡号成功\r\n");
        }
        else if(Identify_CardID()==0)
        {
            BEEP=1;
            delay_xms(50);
            BEEP=0;
            delay_xms(50);
            BEEP=1;
            delay_xms(50);
            BEEP=0;
            delay_xms(50);
            BEEP=1;
            delay_xms(50);
            BEEP=0;
            Chinese_Show_two(90,50,12,16,0);
            Chinese_Show_two(110,50,14,16,0);
            printf("识别卡号失败\r\n");
            err++;
            if(err==3)
            {
                vTaskSuspend(SG90Task_Handler);
                printf("舵机任务挂起\r\n");
                LCD_ShowString(0,100,260,16,16,"Task has been suspended");
            }
        }
        vTaskDelay(100); //延时10ms，也就是10个时钟节拍
    }
}    
```

### 指纹任务AS608_task函数

```C
void AS608_task(void *pvParameters)   
{
    while ( 1 )
    {
        if ( PS_Sta )   //如果有手指按下
        {
            if ( press_FR() == 1 )
            {
                BEEP=1;
                delay_xms(100);
                BEEP=0;
                Chinese_Show_two(30,25,0,16,0);
                Chinese_Show_two(50,25,2,16,0);
                Chinese_Show_two(70,25,4,16,0);
                Chinese_Show_two(90,25,6,16,0);
                Chinese_Show_two(110,25,8,16,0);
                Chinese_Show_two(130,25,10,16,0);
                xEventGroupSetBits(EventGroupHandler,EVENTBIT_2);
                printf("指纹识别成功");
            }
            else if(press_FR()==0)
            {
                BEEP=1;
                delay_xms(50);
                BEEP=0;
                delay_xms(50);
                BEEP=1;
                delay_xms(50);
                BEEP=0;
                delay_xms(50);
                BEEP=1;
                delay_xms(50);
                BEEP=0;
                Chinese_Show_two(110,25,12,16,0);
                Chinese_Show_two(130,25,14,16,0);
                printf("指纹识别失败");
                err++;
                if(err==3)
                {
                    vTaskSuspend(SG90Task_Handler);
                    printf("舵机任务挂起\r\n");
                    LCD_ShowString(0,100,260,16,16,"Task has been suspended");
                }
            }
        }
        vTaskDelay(100);
    }
}void AS608_task(void *pvParameters)   
{
    while ( 1 )
    {
        if ( PS_Sta )   //如果有手指按下
        {
            if ( press_FR() == 1 )
            {
                BEEP=1;
                delay_xms(100);
                BEEP=0;
                Chinese_Show_two(30,25,0,16,0);
                Chinese_Show_two(50,25,2,16,0);
                Chinese_Show_two(70,25,4,16,0);
                Chinese_Show_two(90,25,6,16,0);
                Chinese_Show_two(110,25,8,16,0);
                Chinese_Show_two(130,25,10,16,0);
                xEventGroupSetBits(EventGroupHandler,EVENTBIT_2);
                printf("指纹识别成功");
            }
            else if(press_FR()==0)
            {
                BEEP=1;
                delay_xms(50);
                BEEP=0;
                delay_xms(50);
                BEEP=1;
                delay_xms(50);
                BEEP=0;
                delay_xms(50);
                BEEP=1;
                delay_xms(50);
                BEEP=0;
                Chinese_Show_two(110,25,12,16,0);
                Chinese_Show_two(130,25,14,16,0);
                printf("指纹识别失败");
                err++;
                if(err==3)
                {
                    vTaskSuspend(SG90Task_Handler);
                    printf("舵机任务挂起\r\n");
                    LCD_ShowString(0,100,260,16,16,"Task has been suspended");
                }
            }
        }
        vTaskDelay(100);
    }
}
```



## 拓展

可以增加一个WIFI任务，将大门的状态那些通过wifi发送给APP。

还可以添加摄像头模块，进行人脸识别开门，同时也可把非法开门的人的面貌记录下来。
