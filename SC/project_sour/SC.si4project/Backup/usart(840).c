#include "platform.h"
#include "ringbuffer.h"

DECLARE_RB(rb_usart,USART_RD_BUF_LEN);

void usart_mode_init(void)
{
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


//void spi_mode_init(void)
//{
//	DECLARE_GPIO_INIT(ioIt,AF,PP,UP,3);
//	ioIt.GPIO_Pin=GPIO_Pin_5|GPIO_Pin_6|GPIO_Pin_7;
//	GPIO_Init(GPIOA,&ioIt);/* CLK,MISO,MOSI */
//	
//	ioIt.GPIO_Mode=GPIO_Mode_OUT;
//	ioIt.GPIO_Pin=GPIO_Pin_6;
//	GPIO_Init(GPIOB,&ioIt);/* CS */
//	spi_cshigh();
//	
//	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5,GPIO_AF_5);
//	GPIO_PinAFConfig(GPIOA,GPIO_PinSource6,GPIO_AF_5);
//	GPIO_PinAFConfig(GPIOA,GPIO_PinSource7,GPIO_AF_5);
//	
//	spi_init(SPI_MODE0,SPI_MSBFIRST,7);/* spi baud div 256 */
//}


int __io_putchar(int ch)
{
	uint16_t timeout = 0;
	while((USART_GetFlagStatus(USART,USART_FLAG_TXE)!=SET) && (timeout < 40000)){
		timeout++;
	}
	USART_SendData(USART,ch);
	return ch;
}

int __io_getchar(void)
{
	signed char ch;
	while(rbempty(&rb_usart)) continue;
	ch=rbget(&rb_usart);
	switch(ch)
	{
		case '\r':ch='\n';return __io_putchar(ch);
		case 0x1b:ch='^'; return __io_putchar(ch);
		case 0x08:/* backspace in minicom or putty */
		case 0x7F: return ch; /* send backspace or not depend on user app */
		default: return __io_putchar(ch);
	}
	/* echo it */
	return __io_putchar(ch);
}

int __io_puts(const char *str)
{
	const char *tmp=str;
	while(*tmp!='\0'){
		__io_putchar(*tmp);
		tmp++;
	}
	return tmp-str;
}

/** gets one line from serial.Save all chars into buf,
 ** the length of buf is 'len'.
 ** return read string
 */
char * __io_gets(char *buf,int len)
{
	int idx=0;
	signed char ch;
	while(idx<len-1){ /* 1 is for '\0' */
		switch(ch=__io_getchar()){
		case '\n':buf[idx]='\0';return buf;
		case 0x08:
		case 0x7F: /* backspace in minicom or putty */
			if(idx>0) { /* need earse a char in computer */
				idx--;
				__io_putchar(ch); __io_putchar(' ');__io_putchar(ch);
			}
			break;
		default:
			buf[idx]=ch;idx++;break;
		}
	}
	buf[len-1]='\0';/* data too long,buf is not enough */
	return buf;
}


/* USART interrupt handler */
USART_Handler()
{
	if(USART_GetITStatus(USART,USART_IT_RXNE)==SET){
		rbput(&rb_usart,USART_ReceiveData(USART));
	}
}


