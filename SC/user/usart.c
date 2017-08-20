#include "platform.h"
#include "ringbuffer.h"
#include <CoOS.h>/*!< CooCox RTOS header file.*/
#include "ringbuf.h"

//DECLARE_RB(rb_usart,USART_RD_BUF_LEN);
STDECLARE_RB(rb_usart,500);

void usart_mode_init(void) {
	DECLARE_GPIO_INIT(ioIt,AF,PP,UP,3);
	ioIt.GPIO_Pin=USART_RX_PIN | USART_TX_PIN;
	DECLARE_USART_INIT(uIt,USART_BAUD);
	
	GPIO_Init(USART_GPIO,&ioIt);
	GPIO_PinAFConfig(USART_GPIO,USART_RX_SRC,USART_GPIO_AF);
	GPIO_PinAFConfig(USART_GPIO,USART_TX_SRC,USART_GPIO_AF);

	USART_ClkEn();
	USART_Init(USART,&uIt);
	USART_ClearFlag(USART,USART_FLAG_RXNE);
	USART_ClearFlag(USART,USART_FLAG_TXE);
	NVIC_EnableIRQ(USART_IRQn);
	USART_ITConfig(USART,USART_IT_RXNE,ENABLE);
	
	USART_Cmd(USART,ENABLE);
}

int __io_putchar(int ch) {
	uint16_t timeout = 0;
	while((USART_GetFlagStatus(USART,USART_FLAG_TXE)!=SET) && (timeout < 40000)){
		timeout++;
	}
	USART_SendData(USART,ch);
	return ch;
}

int __io_getchar(void) {
	signed char ch;
	//while(rbempty(&rb_usart)) continue;
	//ch=rbget(&rb_usart);
	return ch;
//	switch(ch)
//	{
//		case '\r':ch='\n';return __io_putchar(ch);
//		case 0x1b:ch='^'; return __io_putchar(ch);
//		case 0x08:/* backspace in minicom or putty */
//		case 0x7F: return ch; /* send backspace or not depend on user app */
//		default: return __io_putchar(ch);
//	}
//	/* echo it */
//	return __io_putchar(ch);
}

int __io_puts(const char *str) {
	const char *tmp=str;
	while(*tmp!='\0'){
		__io_putchar(*tmp);
		tmp++;
	}
	return tmp-str;
}

/* USART interrupt handler */
USART_Handler() {
	#if USECOOS
	CoEnterISR(); // Enter the interrupt
	#endif
//	if(USART_GetITStatus(USART,USART_IT_RXNE)==SET){
//		rbput(&rb_usart,USART_ReceiveData(USART));
//	}
	if((USART->ISR & USART_FLAG_RXNE) == USART_FLAG_RXNE) {
		uint8_t data = USART->RDR;
		//rbput(&rb_usart,USART_ReceiveData(USART));
		rb_usart.put(&rb_usart.ringbuf_n,data);
	}
	#if USECOOS
	CoExitISR(); // Enter the interrupt
	#endif
}


