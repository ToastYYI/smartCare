#ifndef __RINGBUF_H
#define __RINGBUF_H
/* ����ͷ�ļ� *****************************************************************/
#include "stm32f0xx.h"

#define false   0       
#define true    1       
#define NULL    0
/* �ⲿ�������� ***************************************************************/
struct Stdringbuf_n{
    uint8_t  buf[49];             //ָ����������ָ��
    uint16_t length;    //����
    uint16_t head;      //��ͷ
    uint16_t tail;      //��β
    int fill_cnt;           //���м���
};

struct Stdringbuf{
    struct Stdringbuf_n ringbuf_n;
    int (*init)(struct Stdringbuf_n* r, uint8_t len);
    int (*put)(struct Stdringbuf_n* r, uint8_t data);
    int (*get)(struct Stdringbuf_n* r, uint8_t *c, uint16_t length);
};
/* �ⲿ������ *****************************************************************/
/* �ⲿ������ *****************************************************************/
/* �ⲿ���� *******************************************************************/
/* �������� *******************************************************************/
int RingbufInit(struct Stdringbuf_n* r, uint8_t len);
int RingbufPut(struct Stdringbuf_n* r, uint8_t data);
int RingbufGut(struct Stdringbuf_n* r, uint8_t *c, uint16_t length);

#define STDECLARE_RB(name) \
	volatile struct Stdringbuf name = { \
		{{0,},50,0,0,0}, \
		&RingbufInit,&RingbufPut,&RingbufGut};

#endif
/***************************************************************END OF FILE****/
