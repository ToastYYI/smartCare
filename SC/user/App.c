/** -- 主函数 -- by lhb_steven -- 2017/6/15**/
#include "sys.h"
#include <CoOS.h>/*!< CooCox RTOS header file.*/
#include "platform.h"/*lua head*/

/* 私有类型定义---------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
#define TASK_STK_SIZE		128	 		/*!< Define stack size.*/
/* 私有变量 ------------------------------------------------------------------*/
OS_STK   LED_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDA_Task' task. */
OS_STK   MOTO_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDB_Task' task. */

OS_MutexID	mut_uart;	 	
OS_FlagID Flag_Massage_ID;
/* 私有函数 ------------------------------------------------------------------*/
void LED_Task (void* pdata);
void MOTO_Task (void* pdata);

/* USART interrupt handler */
USART_Handler() {
	#if USECOOS
	CoEnterISR(); // Enter the interrupt
	//CoEnterMutexSection(mut_uart);
	#endif
	if(USART_GetITStatus(USART,USART_IT_RXNE)==SET){
		RingbufPut(USART_ReceiveData(USART));
		CoSetFlag(Flag_Massage_ID);
	}
	#if USECOOS
	//CoLeaveMutexSection(mut_uart); 
	CoExitISR(); // Enter the interrupt
	#endif
}

/***************************************************************************//**
  * @brief  主函数，硬件初始化，实现LED1-LED4闪烁
  * @note   无
  * @param  无
  * @retval 无
*******************************************************************************/
/** -- 程序入口 -- by lhb_steven -- 2017/6/30**/
int main(void) {
    StdSys SYS = {
        &SysInitRcc,
    };
    /** -- 系统初始化 -- by lhb_steven -- 2017/6/17**/
    SYS.SysInitRcc();
	
    CoInitOS();/*!< Initial CooCox RTOS*/
	
	Flag_Massage_ID = CoCreateFlag(Co_FALSE,0);	
	
    CoCreateTask(LED_Task, (void *)0, 10,&LED_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    CoCreateTask(MOTO_Task, (void *)0, 11,&MOTO_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    CoStartOS();
    while(1);
}


/***************************************************************************//**
 * @brief   闪灯任务A
 * @param   无
 * @retval  无
 * @par     详细描述：
 * @details 创建 任务A，该任务以200ms为周期点亮LED3，实现LED3闪烁。
*******************************************************************************/
void LED_Task (void* pdata) {
    //使能LED所在GPIO的时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	gpio_mode(0,1,0);
    for (;;) {
        gpio_write(0,0);
        //__io_puts("LED On");
        //延时100 ms
        CoTickDelay (300);
        gpio_write(0,1);
        //__io_puts("LED Off");
        //延时100 ms
        CoTickDelay (350);
		//mini_snprintf("lhb",3,"3123");
    }
}

/***************************************************************************//**
 * @brief   闪灯任务B
 * @param   无
 * @retval  无
 * @par     详细描述：
 * @details 创建 任务B，该任务以400ms为周期点亮LED4，实现LED4闪烁。
*******************************************************************************/
void MOTO_Task (void* pdata) {
    //初始化LED
    GPIO_InitTypeDef GPIO_InitStructure;
    //使能LED所在GPIO的时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    //初始化LED的GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_SetBits(GPIOA,GPIO_Pin_5);//sleep

	usart_mode_init();

	adc_init();
	

	pwm_init();//pwm init
    for (;;) {
		//CoTickDelay(3000);//wait solw send
		uint8_t data[8];
		CoWaitForSingleFlag(Flag_Massage_ID,0);
		CoClearFlag(Flag_Massage_ID);
		
		//RingbufGut(data,1);
		
		uint16_t bat = adc_read()*6600/4096;/* 获取值 * (电压范围 1/2分压 扩大 1000倍数 3.3x2x1000) / 2^16  */
		//to Ble send battery
		char val[20];
		val[0] = 0x30+(bat/1000);
		val[1] = '.';
		val[2] = 0x30+(bat%1000/100);
		val[3] = 0x30+(bat%100/10);
		val[4] = 0x30+(bat%10);
		val[5] = 'V';
		__io_puts("bat-val:");
		__io_puts(val);
    }
}


#ifdef  USE_FULL_ASSERT
/***************************************************************************//**
  * @brief  报告在检查参数发生错误时的源文件名和错误行数
  * @param  file: 指向错误文件的源文件名
  * @param  line: 错误的源代码所在行数
  * @retval 无
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line) {
    /* 用户可以增加自己的代码用于报告错误的文件名和所在行数,
       例如：printf("错误参数值: 文件名 %s 在 %d行\r\n", file, line) */
	
    /* 死循环 */
    while (1) {
    }
}
#endif

/******************* (C) COPYRIGHT wuguoyana ***************文件结束***********/
