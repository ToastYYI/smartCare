/* ����ͷ�ļ� *****************************************************************/
#include "ringbuf.h"
/* ˽���������� ***************************************************************/
/* ˽�г����� *****************************************************************/
/* ˽�ж����� *****************************************************************/
/* ˽�б��� *******************************************************************/
/* ȫ�ֱ��� */
/* ˽�к������� ***************************************************************/
/* ˽�к���  ******************************************************************/
//int RingbufInit(struct Stdringbuf_n* r, uint8_t array[], uint8_t len) {
//    if (len < 2 || array==NULL){
//        return false;
//    }
//   // r->buf = array;
//    r->length = len;
//    r->fill_cnt = 0;
//    r->head = r->tail = 0;
//    return true;
//}

//int RingbufPut(struct Stdringbuf_n* r, uint8_t data) {
//    //��tail+1����headʱ��˵����������
//    if (r->fill_cnt >= r->length) {
//        return false;                  // ������������ˣ��򷵻ش���
//    }
//    r->buf[r->tail] = data;
//    r->tail++;
//    r->fill_cnt++;
//    //����tail�Ƿ񳬳����鷶Χ������������Զ��л���0
//    r->tail = r->tail % r->length;
//    return true;
//}

//int RingbufGut(struct Stdringbuf_n* r, uint8_t *c, uint16_t length) {
//    //��tail����headʱ��˵�����п�
//    if (r->fill_cnt<=0) {
//        return false;
//    }
//    //���ֻ�ܶ�ȡr->length��������
//    if (length > r->length) {
//        length = r->length;
//    }
//    
//    if(r->tail > r->head) {
//        if(r->tail < (length + r->head)) {
//            return false;
//        }
//    } else {
//        if( ((r->length-r->head)+r->tail) < length ) {
//            return false;
//        }
//    }
//    
//    int i;
//    for (i = 0; i<length; i++)
//    {
//        r->fill_cnt--;
//        *c = r->buf[r->head++];                 // �������ݸ�*c
//        *c++;
//        //����head�ԼӺ���±��Ƿ񳬳����鷶Χ������������Զ��л���0
//        r->head = r->head % r->length;
//    }
//    return true;
//}

uint8_t  buf[50] = {0,};	//ָ����������ָ��
uint16_t length = 50;    //����
uint16_t head = 0;      //��ͷ
uint16_t tail = 0;      //��β
int fill_cnt = 0;	//���м���

int RingbufPut(uint8_t data) {
	if (fill_cnt >= length) {
		return false;   
	} else {
		buf[tail] = data;
		tail++;
		fill_cnt++;
		//����tail�Ƿ񳬳����鷶Χ������������Զ��л���0
		tail = tail % length;
	}
	return true;
}

int RingbufGut(uint8_t *c, uint16_t leng) {
    //��tail����headʱ��˵�����п�
    if (fill_cnt<=0) {
        return false;
    }
    //���ֻ�ܶ�ȡr->length��������
    if (leng > length) {
        leng = length;
    }
    
    if(tail > head) {
        if(tail < (leng + head)) {
            return false;
        }
    } else {
        if( ((length-head)+tail) < leng ) {
            return false;
        }
    }
    
    int i;
    for (i = 0; i<leng; i++)
    {
        fill_cnt--;
        *c = buf[head++];                 // �������ݸ�*c
        *c++;
        //����head�ԼӺ���±��Ƿ񳬳����鷶Χ������������Զ��л���0
        head = head % length;
    }
    return true;
}

/***************************************************************END OF FILE****/
