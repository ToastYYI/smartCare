/** -- ������ -- by lhb_steven -- 2017/6/15**/
#include "sys.h"
#include <CoOS.h>/*!< CooCox RTOS header file.*/


/* ˽�����Ͷ���---------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
#define TASK_STK_SIZE		128	 		/*!< Define stack size.*/
#define LED3            GPIO_Pin_4
#define LED4            GPIO_Pin_5

#define LED_PORT        GPIOA
#define LED_GPIO_CLK    RCC_AHBPeriph_GPIOA

/* ˽�б��� ------------------------------------------------------------------*/
OS_STK   LEDA_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDA_Task' task. */
OS_STK   LEDB_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDB_Task' task. */

/* ˽�к��� ------------------------------------------------------------------*/
void LED_Configuration(void);
void LEDA_Task (void* pdata);
void LEDB_Task (void* pdata);

/***************************************************************************//**
  * @brief  ��������Ӳ����ʼ����ʵ��LED1-LED4��˸
  * @note   ��
  * @param  ��
  * @retval ��
*******************************************************************************/
/** -- ������� -- by lhb_steven -- 2017/6/30**/
int main(void) {
    StdSys SYS = {
        &SysInitRcc,
    };
    /** -- ϵͳ��ʼ�� -- by lhb_steven -- 2017/6/17**/
    SYS.SysInitRcc();
    CoInitOS();/*!< Initial CooCox RTOS*/

    CoCreateTask(LEDA_Task, (void *)0, 10,&LEDA_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    CoCreateTask(LEDB_Task, (void *)0, 11,&LEDB_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    
    CoStartOS();
    while(1);
}


/***************************************************************************//**
 * @brief   ��������A
 * @param   ��
 * @retval  ��
 * @par     ��ϸ������
 * @details ���� ����A����������200msΪ���ڵ���LED3��ʵ��LED3��˸��
*******************************************************************************/
void LEDA_Task (void* pdata) 
{
    //��ʼ��LED
    LED_Configuration();

    for (;;) 
    {
        GPIO_SetBits(LED_PORT,LED3);  //����LED1
        //��ʱ100 ms
        CoTickDelay (500);
        GPIO_ResetBits(LED_PORT,LED3);//Ϩ��LED1
        //��ʱ100 ms
        CoTickDelay (500);
    }
}

/***************************************************************************//**
 * @brief   ��������B
 * @param   ��
 * @retval  ��
 * @par     ��ϸ������
 * @details ���� ����B����������400msΪ���ڵ���LED4��ʵ��LED4��˸��
*******************************************************************************/
void LEDB_Task (void* pdata) 
{
    //��ʼ��LED
    GPIO_InitTypeDef GPIO_InitStructure;
    
    //ʹ��LED����GPIO��ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    
    //��ʼ��LED��GPIO
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);
    for (;;) 
    {
        
    }
}

/***************************************************************************//**
  * @brief  ����ʹ��LED
  * @note   LED��صĶ�����Ҫ���������ú궨�����޸�
  * @param  ��
  * @retval ��
*******************************************************************************/
void LED_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    //ʹ��LED����GPIO��ʱ��
    RCC_AHBPeriphClockCmd(LED_GPIO_CLK, ENABLE);

    //��ʼ��LED��GPIO
    GPIO_InitStructure.GPIO_Pin = LED3 | LED4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(LED_PORT, &GPIO_InitStructure);

    GPIO_ResetBits(LED_PORT,LED3 | LED4);  //Ϩ��LED3-4
}


#ifdef  USE_FULL_ASSERT
/***************************************************************************//**
  * @brief  �����ڼ�������������ʱ��Դ�ļ����ʹ�������
  * @param  file: ָ������ļ���Դ�ļ���
  * @param  line: �����Դ������������
  * @retval ��
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
    /* �û����������Լ��Ĵ������ڱ��������ļ�������������,
       ���磺printf("�������ֵ: �ļ��� %s �� %d��\r\n", file, line) */

    /* ��ѭ�� */
    while (1)
    {
    }
}
#endif

/******************* (C) COPYRIGHT wuguoyana ***************�ļ�����***********/


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
* @brief  �����ڼ�������������ʱ��Դ�ļ����ʹ�������
* @param  file: ָ������ļ���Դ�ļ���
* @param  line: �����Դ������������
* @retval ��
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line)
{
    /* �û����������Լ��Ĵ������ڱ��������ļ�������������,
    ���磺printf("�������ֵ: �ļ��� %s �� %d��\r\n", file, line) */
    
    /* ��ѭ�� */
    while (1)
    {
    }
}
#endif

