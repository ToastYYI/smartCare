/** -- 主函数 -- by lhb_steven -- 2017/6/15**/
#include "sys.h"
#include <CoOS.h>/*!< CooCox RTOS header file.*/
#include "platform.h"/*lua head*/
#include "ringbuf.h"
/* 私有类型定义---------------------------------------------------------------*/
/* 私有宏定义 ----------------------------------------------------------------*/
#define TASK_STK_SIZE		128	 		/*!< Define stack size.*/
/* 私有变量 ------------------------------------------------------------------*/
OS_STK   LED_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDA_Task' task. */
OS_STK   MOTO_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDB_Task' task. */
OS_STK   Bat_Task_Stk[TASK_STK_SIZE];	 	  /*!< Stack of 'LEDB_Task' task. */

OS_MutexID	mut_uart;//互斥	 	
OS_FlagID Flag_Massage_ID;//信号 ID
OS_TCID Timer1_ID;//定时器 ID

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
	uint8_t id;//设备ID 现在考虑是256 增加的话可以改成16的
	uint8_t mt;//标志位，可以用来区分命令
	uint8_t data[4];//数据位 用来放置数据
	uint8_t check;//校验位 这边我先试用一下RC校验 累加 
};

/***************************************************************************//**
 * @brief   电池任务
 * @param   无
 * @retval  无
 * @par     详细描述：
 * @details 创建
*******************************************************************************/
void Bat_Task(void *pdata) {
	usart_mode_init();
	adc_init();
	for(;;) {
		uint16_t bat = adc_read()*6600/4096;/* 获取值 * (电压范围 1/2分压 扩大 1000倍数 3.3x2x1000) / 2^16  */
        //to Ble send battery
        char val[20];
        val[0] = 0x30+(bat/1000);
        val[1] = '.';
        val[2] = 0x30+(bat%1000/100);
        val[3] = 0x30+(bat%100/10);
        val[4] = 0x30+(bat%10);
        val[5] = 'V';
		CoEnterMutexSection(mut_uart);//进入临界段
        __io_puts("电池电压:");
        __io_puts(val);
		CoLeaveMutexSection(mut_uart);//退出临界段
		CoTickDelay (1000);
	}
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
    gpio_mode(4,1,0);
    for (;;) {
        gpio_write(4,0);
        CoTickDelay (300);
        gpio_write(4,1);
        CoTickDelay (350);
    }
}
/***************************************************************************//**
 * @brief   闪灯任务B
 * @param   无
 * @retval  无
 * @par     详细描述：
 * @details 创建 任务B，该任务以400ms为周期点亮LED4，实现LED4闪烁。
*******************************************************************************/
void timer_back(void) {
//	struct mt_data rec_dat;//接收数据
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
	//没次包都会在200ms内发送完成，超时就删除
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
		struct mt_data *rec_dat_t;//接收数据
		if( buf.get(&buf.ringbuf_n,h_data,7) ) {
			CoSetTmrCnt(Timer1_ID,800,800);
			uint8_t crc = Add16(h_data,6);//crc 校验
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
				//校验失败
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
	mut_uart = CoCreateMutex();//创建互斥
    Flag_Massage_ID = CoCreateFlag(Co_FALSE,0);//创建串口互斥
    CoCreateTask(LED_Task, (void *)0, 10,&LED_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    CoCreateTask(MOTO_Task, (void *)0, 11,&MOTO_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
	CoCreateTask(Bat_Task, (void *)0, 11,&Bat_Task_Stk[TASK_STK_SIZE-1], TASK_STK_SIZE);
    CoStartOS();
    while(1);
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
