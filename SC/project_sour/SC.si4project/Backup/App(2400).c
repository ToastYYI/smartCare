/** -- 主函数 -- by lhb_steven -- 2017/6/15**/
#include "sys.h"
#include <CoOS.h>/*!< CooCox RTOS header file.*/
#include "platform.h"/*lua head*/


/* 私有类型定义---------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
#define TASK_STK_SIZE		128	 		/*!< Define stack size.*/
/* 私有变量 ------------------------------------------------------------------*/
OS_STK   LEDA_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDA_Task' task. */
OS_STK   LEDB_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDB_Task' task. */

/* 私有函数 ------------------------------------------------------------------*/
void LEDA_Task (void* pdata);
void LEDB_Task (void* pdata);

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

    CoCreateTask(LEDA_Task, (void *)0, 10,&LEDA_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    CoCreateTask(LEDB_Task, (void *)0, 11,&LEDB_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    
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
void LEDA_Task (void* pdata) {
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
    }
}

extern __IO uint16_t  RegularConvData_Tab;	 
__IO uint16_t ADC_ConvertedValueLocal;    
/***************************************************************************//**
 * @brief   闪灯任务B
 * @param   无
 * @retval  无
 * @par     详细描述：
 * @details 创建 任务B，该任务以400ms为周期点亮LED4，实现LED4闪烁。
*******************************************************************************/
void LEDB_Task (void* pdata) 
{
    //初始化LED
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //使能LED所在GPIO的时钟
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    
    //初始化LED的GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA,GPIO_Pin_5);//sleep
	GPIO_SetBits(GPIOA, GPIO_Pin_7);
	GPIO_SetBits(GPIOA, GPIO_Pin_6);

	//RS232_Config();
	usart_mode_init();

	adc_init();
//	adc_init2();
    for (;;) {
		CoTickDelay(3000);
		while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET );      
		/* Clear DMA TC flag */
		DMA_ClearFlag(DMA1_FLAG_TC1);
		
		ADC_ConvertedValueLocal= RegularConvData_Tab;

		uint8_t val[20];
		val[0] = 0x30+(ADC_ConvertedValueLocal/1000);
		val[1] = 0x30+(ADC_ConvertedValueLocal%1000/100);
		val[2] = 0x30+(ADC_ConvertedValueLocal%100/10);
		val[3] = 0x30+(ADC_ConvertedValueLocal%10);
		__io_puts("bat-val:");
		__io_puts(val);
//		adc_read();
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


#define PWM_MOTOR_MIN 100
#define PWM_MOTOR_MED 1250
#define PWM_MOTOR_MAX 2400

#define PWM1  TIM3->CCR1
#define PWM2  TIM3->CCR2
#define PWM3  TIM3->CCR4
#define PWM4  TIM1->CCR2
#define PWM5  TIM1->CCR3

void PWM_Config( void )
{
    GPIO_InitTypeDef GPIO_InitStruct;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStruct;
    TIM_OCInitTypeDef TIM_OCInitStruct;
    
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA | RCC_AHBPeriph_GPIOB, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource9,  GPIO_AF_2); // TIM1 CH2
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource10, GPIO_AF_2); // TIM1 CH3
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource6,  GPIO_AF_1); // TIM3 CH1
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource7,  GPIO_AF_1); // TIM3 CH2
    GPIO_PinAFConfig(GPIOB, GPIO_PinSource1,  GPIO_AF_1); // TIM3 CH4
    
    /* TIM1 CH2 PA9  */ /* TIM1 CH3 PA10 */ /* TIM3 CH1 PA6  */ /* TIM3 CH2 PA7  */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_6  | GPIO_Pin_7  | GPIO_Pin_9  | GPIO_Pin_10;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    /* TIM3 CH4 PB1  */
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP ;
    GPIO_Init(GPIOB, &GPIO_InitStruct);
    
    /************************** PWM Output **************************************/
    /* TIM1 Time Base */
    TIM_TimeBaseStruct.TIM_Period = (uint16_t)(2500-1);         // Period = 2.5ms => Freq = 400Hz
    TIM_TimeBaseStruct.TIM_Prescaler = (uint16_t)(48-1);        // fCK_PSC = APB1*2 = 42*2 = 84, fCK_PSC /84 = 1M ( 1us )
    TIM_TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStruct.TIM_CounterMode = TIM_CounterMode_Up;    // Count Up
    TIM_TimeBaseStruct.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStruct);
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStruct);
    
    /* Channel 1, 2, 3 and 4 Configuration in PWM mode */
    TIM_OCInitStruct.TIM_OCMode = TIM_OCMode_PWM1;
    TIM_OCInitStruct.TIM_OutputState = TIM_OutputState_Enable;
    TIM_OCInitStruct.TIM_OutputNState = TIM_OutputNState_Enable;
    TIM_OCInitStruct.TIM_OCPolarity = TIM_OCPolarity_Low;
    TIM_OCInitStruct.TIM_OCNPolarity = TIM_OCNPolarity_High;
    TIM_OCInitStruct.TIM_OCIdleState = TIM_OCIdleState_Set;
    TIM_OCInitStruct.TIM_OCNIdleState = TIM_OCIdleState_Reset;
    TIM_OCInitStruct.TIM_Pulse = PWM_MOTOR_MIN;
    TIM_OC2Init(TIM1, &TIM_OCInitStruct);
    TIM_OC3Init(TIM1, &TIM_OCInitStruct);
    TIM_OC1Init(TIM3, &TIM_OCInitStruct);
    TIM_OC2Init(TIM3, &TIM_OCInitStruct);
    TIM_OC4Init(TIM3, &TIM_OCInitStruct);
    
    TIM_OC2PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC3PreloadConfig(TIM1, TIM_OCPreload_Enable);
    TIM_OC1PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC2PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_OC4PreloadConfig(TIM3, TIM_OCPreload_Enable);
    TIM_ARRPreloadConfig(TIM1, ENABLE);
    TIM_ARRPreloadConfig(TIM3, ENABLE);
    
    /* Enable */
    TIM_Cmd(TIM1, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
    
    TIM_CtrlPWMOutputs(TIM1, ENABLE);
    TIM_CtrlPWMOutputs(TIM3, ENABLE);
    
    TIM1->CCR2 = PWM_MOTOR_MIN;
    TIM1->CCR3 = PWM_MOTOR_MIN;
    TIM3->CCR1 = PWM_MOTOR_MIN;
    TIM3->CCR2 = PWM_MOTOR_MIN;
    TIM3->CCR4 = PWM_MOTOR_MIN;
}


#ifdef  USE_FULL_ASSERT
/***************************************************************************//**
* @brief  报告在检查参数发生错误时的源文件名和错误行数
* @param  file: 指向错误文件的源文件名
* @param  line: 错误的源代码所在行数
* @retval 无
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
    /* 用户可以增加自己的代码用于报告错误的文件名和所在行数,
    例如：printf("错误参数值: 文件名 %s 在 %d行\r\n", file, line) */
    
    /* 死循环 */
    while (1)
    {
    }
}
#endif

