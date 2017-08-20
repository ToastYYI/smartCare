/** -- ������ -- by lhb_steven -- 2017/6/15**/
#include "sys.h"
#include <CoOS.h>/*!< CooCox RTOS header file.*/
#include "platform.h"/*lua head*/


/* ˽�����Ͷ���---------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
#define TASK_STK_SIZE		128	 		/*!< Define stack size.*/
/* ˽�б��� ------------------------------------------------------------------*/
OS_STK   LEDA_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDA_Task' task. */
OS_STK   LEDB_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDB_Task' task. */

/* ˽�к��� ------------------------------------------------------------------*/
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
void LEDA_Task (void* pdata) {
    //ʹ��LED����GPIO��ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
	gpio_mode(0,1,0);
    for (;;) {
        gpio_write(0,0);
        //__io_puts("LED On");
        //��ʱ100 ms
        CoTickDelay (300);
        gpio_write(0,1);
        //__io_puts("LED Off");
        //��ʱ100 ms
        CoTickDelay (350);
    }
}

extern __IO uint16_t  RegularConvData_Tab;	 
__IO uint16_t ADC_ConvertedValueLocal;    
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
    GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_SetBits(GPIOA,GPIO_Pin_5);//sleep
	GPIO_SetBits(GPIOA, GPIO_Pin_7);
	GPIO_SetBits(GPIOA, GPIO_Pin_6);

	//RS232_Config();
	usart_mode_init();

	adc_init();

	pwm_init();//pwm init
    for (;;) {
		CoTickDelay(3000);
		while((DMA_GetFlagStatus(DMA1_FLAG_TC1)) == RESET );      
		/* Clear DMA TC flag */
		DMA_ClearFlag(DMA1_FLAG_TC1);
		
		ADC_ConvertedValueLocal= RegularConvData_Tab;
		uint16_t bat = ADC_ConvertedValueLocal*6600/4096;/* ��ȡֵ * (��ѹ��Χ 1/2��ѹ ���� 1000���� 3.3x2x1000) / 2^16  */
		
		char val[20];
		val[0] = 0x30+(bat/1000);
		val[1] = '.';
		val[2] = 0x30+(bat%1000/100);
		val[3] = 0x30+(bat%100/10);
		val[4] = 0x30+(bat%10);
		val[5] = 'V';
		__io_puts("bat-val:");
		__io_puts(val);
//		adc_read();
    }
}


#ifdef  USE_FULL_ASSERT
/***************************************************************************//**
  * @brief  �����ڼ�������������ʱ��Դ�ļ����ʹ�������
  * @param  file: ָ������ļ���Դ�ļ���
  * @param  line: �����Դ������������
  * @retval ��
*******************************************************************************/
void assert_failed(uint8_t* file, uint32_t line) {
    /* �û����������Լ��Ĵ������ڱ��������ļ�������������,
       ���磺printf("�������ֵ: �ļ��� %s �� %d��\r\n", file, line) */
	
    /* ��ѭ�� */
    while (1) {
    }
}
#endif

/******************* (C) COPYRIGHT wuguoyana ***************�ļ�����***********/
