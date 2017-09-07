/** -- ������ -- by lhb_steven -- 2017/6/15**/
#include "sys.h"
#include <CoOS.h>/*!< CooCox RTOS header file.*/
#include "platform.h"/*lua head*/
#include "ringbuf.h"
/* ˽�����Ͷ���---------------------------------------------------------------*/
/* ˽�к궨�� ----------------------------------------------------------------*/
#define TASK_STK_SIZE		128	 		/*!< Define stack size.*/
/* ˽�б��� ------------------------------------------------------------------*/
OS_STK   LED_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDA_Task' task. */
OS_STK   MOTO_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDB_Task' task. */
OS_STK   Bat_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDB_Task' task. */

OS_MutexID	mut_uart;//����	 	
OS_FlagID Flag_Massage_ID;//�ź� ID
OS_TCID Timer1_ID;//��ʱ�� ID

struct Stdringbuf buf = {
	{
		{0,},
		50,
		0,
		0,
		0,
	},
	&RingbufInit,
	&RingbufPut,
	&RingbufGut,
};

struct mt_data {
	uint8_t id;//�豸ID ���ڿ�����256 ���ӵĻ����Ըĳ�16��
	uint8_t mt;//��־λ������������������
	uint8_t data[4];//����λ ������������
	uint8_t check;//У��λ �����������һ��RCУ�� �ۼ� 
};

/***************************************************************************//**
 * @brief   �������
 * @param   ��
 * @retval  ��
 * @par     ��ϸ������
 * @details ����
*******************************************************************************/
void Bat_Task(void *pdata) {
	usart_mode_init();
	adc_init();
	for(;;) {
		uint16_t bat = adc_read()*6600/4096;/* ��ȡֵ * (��ѹ��Χ 1/2��ѹ ���� 1000���� 3.3x2x1000) / 2^16  */
        //to Ble send battery
        char val[20];
        val[0] = 0x30+(bat/1000);
        val[1] = '.';
        val[2] = 0x30+(bat%1000/100);
        val[3] = 0x30+(bat%100/10);
        val[4] = 0x30+(bat%10);
        val[5] = 'V';
		CoEnterMutexSection(mut_uart);//�����ٽ��
        __io_puts("��ص�ѹ:");
        __io_puts(val);
		CoLeaveMutexSection(mut_uart);//�˳��ٽ��
		CoTickDelay (1000);
	}
}
/***************************************************************************//**
 * @brief   ��������A
 * @param   ��
 * @retval  ��
 * @par     ��ϸ������
 * @details ���� ����A����������200msΪ���ڵ���LED3��ʵ��LED3��˸��
*******************************************************************************/
void LED_Task (void* pdata) {
    //ʹ��LED����GPIO��ʱ��
    RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
    gpio_mode(4,1,0);
    for (;;) {
        gpio_write(4,0);
        CoTickDelay (300);
        gpio_write(4,1);
        CoTickDelay (350);
    }
}
/***************************************************************************//**
 * @brief   ��������B
 * @param   ��
 * @retval  ��
 * @par     ��ϸ������
 * @details ���� ����B����������400msΪ���ڵ���LED4��ʵ��LED4��˸��
*******************************************************************************/
void timer_back(void) {
//	struct mt_data rec_dat;//��������
//	rec_dat.id = 0x02;
//	for(int k = 0;k < 10;k++) {
//		rec_dat.data[k] = 0x01;
//	}
//	rec_dat.mt = 0xa5;
//	rec_dat.check = 0xee;

//	uint8_t x_buf[20];
//	x_buf[19]= sizeof(rec_dat);

//	int i=0;
//	for(i=0; i<sizeof(rec_dat);i++) {
//		USART_SendData(USART,*((unsigned char*)&rec_dat+i));
//		x_buf[i] = *((unsigned char*)&rec_dat+i);
//		CoTickDelay (1);
//	}
	//û�ΰ�������200ms�ڷ�����ɣ���ʱ��ɾ��
	buf.init(&buf.ringbuf_n,49);
}

void MOTO_Task (void* pdata) {
    MotoInit();//pwm init
	//timer
	Timer1_ID = CoCreateTmr(TMR_TYPE_PERIODIC,800,800,timer_back);
	CoStartTmr(Timer1_ID);
    for (;;) {
        CoWaitForSingleFlag(Flag_Massage_ID,0);
        CoClearFlag(Flag_Massage_ID);
		uint8_t h_data[7];
		struct mt_data *rec_dat_t;//��������
		if( buf.get(&buf.ringbuf_n,h_data,7) ) {
			CoSetTmrCnt(Timer1_ID,800,800);
			uint8_t crc = Add16(h_data,6);//crc У��
			if(crc == h_data[6]) {
				rec_dat_t = (struct mt_data *)h_data;
				if(rec_dat_t->id == 0) {
					switch(rec_dat_t->mt) {
						case 0:{
							int left = rec_dat_t->data[0];
							left |= ((rec_dat_t->data[1] & 0x7f) << 8);
							if( (rec_dat_t->data[1] & 0x80) == 0x00 ) {
								left = 0-left;
							}
							int right = rec_dat_t->data[2];
							right |= ((rec_dat_t->data[3] & 0x7f) << 8);
							if( (rec_dat_t->data[3] & 0x80) == 0x00 ) {
								right = 0-right;
							}
							MotoSleepSet(left,right);
						}	
						break;
						case 1:{
							
						}	
						break;
					}
				}
			} else {
				//У��ʧ��
				//doing something
			}
		}
    }
}


/* USART interrupt handler */
USART_Handler() {
	#if USECOOS
	CoEnterISR(); // Enter the interrupt
	#endif
	if(USART_GetITStatus(USART,USART_IT_RXNE)==SET){
		uint8_t data;
		data = USART_ReceiveData(USART);
		buf.put(&buf.ringbuf_n,data);
		CoSetFlag(Flag_Massage_ID);
	}
	#if USECOOS
	CoExitISR(); // Enter the interrupt
	#endif
}

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
	mut_uart = CoCreateMutex();//��������
    Flag_Massage_ID = CoCreateFlag(Co_FALSE,0);//�������ڻ���
    CoCreateTask(LED_Task, (void *)0, 10,&LED_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    CoCreateTask(MOTO_Task, (void *)0, 11,&MOTO_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
	CoCreateTask(Bat_Task, (void *)0, 11,&Bat_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    CoStartOS();
    while(1);
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
