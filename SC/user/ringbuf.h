#ifndef __RINGBUF_H
#define __RINGBUF_H
/* 引用头文件 *****************************************************************/
#include "stm32f0xx.h"

#define false   0       
#define true    1       
#define NULL    0
/* 外部数据类型 ***************************************************************/
struct Stdringbuf_n{
    uint8_t  buf[49];             //指向队列数组的指针
    uint16_t length;    //长度
    uint16_t head;      //队头
    uint16_t tail;      //队尾
    int fill_cnt;           //队列计数
};

struct Stdringbuf{
    struct Stdringbuf_n ringbuf_n;
    int (*init)(struct Stdringbuf_n* r, uint8_t len);
    int (*put)(struct Stdringbuf_n* r, uint8_t data);
    int (*get)(struct Stdringbuf_n* r, uint8_t *c, uint16_t length);
};
/* 外部常数宏 *****************************************************************/
/* 外部动作宏 *****************************************************************/
/* 外部变量 *******************************************************************/
/* 函数声明 *******************************************************************/
int RingbufInit(struct Stdringbuf_n* r, uint8_t len);
int RingbufPut(struct Stdringbuf_n* r, uint8_t data);
int RingbufGut(struct Stdringbuf_n* r, uint8_t *c, uint16_t length);

#define STDECLARE_RB(name) \
	volatile struct Stdringbuf name = { \
		{{0,},50,0,0,0}, \
		&RingbufInit,&RingbufPut,&RingbufGut};

#endif
/***************************************************************END OF FILE****/
