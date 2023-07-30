#include "stm32f4xx.h"
#include "usart.h"
#include "delay.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"

void start_task(void *param);
void led0_task(void *param);
void led1_task(void *param);
void float_task(void *param);

#define START_TASK_PRIO		1           //任务优先级
#define START_STK_SIZE 		128         //任务堆栈大小
#define LED0_TASK_PRIO		2
#define LED0_STK_SIZE 		50 
#define LED1_TASK_PRIO		3
#define LED1_STK_SIZE 		50
#define FLOAT_TASK_PRIO		4
#define FLOAT_STK_SIZE 		128

TaskHandle_t StartTask_Handler;
TaskHandle_t LED0Task_Handler;
TaskHandle_t LED1Task_Handler;
TaskHandle_t FLOATTask_Handler;



int main(void)
{
    uart_init(115200);
    delay_init(168);
    LED_Init();

    xTaskCreate(start_task, "start_task", START_STK_SIZE, NULL, START_TASK_PRIO, &StartTask_Handler);
    
    /* 开启任务调度 */
    vTaskStartScheduler();
}

void start_task(void *param)
{
    taskENTER_CRITICAL();       //进入临界区
    
    //创建LED0任务
    xTaskCreate(led0_task, "led0_task", LED0_STK_SIZE, NULL, LED0_TASK_PRIO, &LED0Task_Handler);
    //创建LED0任务
    xTaskCreate(led1_task, "led1_task", LED1_STK_SIZE, NULL, LED1_TASK_PRIO, &LED1Task_Handler);
    //浮点测试任务
    xTaskCreate(float_task, "float_task", FLOAT_STK_SIZE, NULL, FLOAT_TASK_PRIO, &FLOATTask_Handler);
    
    vTaskDelete(StartTask_Handler);
    taskEXIT_CRITICAL();        //退出临界区
}

void led0_task(void *param)
{
    while(1)
    {
        LED0 = ~LED0;
        vTaskDelay(500);
    }
}

void led1_task(void *param)
{
    while(1)
    {
        LED1 = 0;
        vTaskDelay(200);
        LED1 = 1;
        vTaskDelay(800);
    }
}

void float_task(void *param)
{
    static float float_num = 0.00;
    while(1)
    {
        float_num += 0.01f;
        printf("float_num的值为：%.4f\r\n", float_num);
        vTaskDelay(1000);
    }
}


