#include "platform.h"
#include "ringbuffer.h"

DECLARE_RB(rb_usart,USART_RD_BUF_LEN);


#define xUSARTx                USART1
#define xUSARTx_CLK            RCC_APB2Periph_USART1

#define xUSARTx_TX_PIN         GPIO_Pin_9
#define xUSARTx_TX_GPIO_PORT   GPIOA
#define xUSARTx_TX_GPIO_CLK    RCC_AHBPeriph_GPIOA
#define xUSARTx_TX_SOURCE      GPIO_PinSource9
#define xUSARTx_TX_AF          GPIO_AF_1

#define xUSARTx_RX_PIN         GPIO_Pin_10
#define xUSARTx_RX_GPIO_PORT   GPIOA
#define xUSARTx_RX_GPIO_CLK    RCC_AHBPeriph_GPIOA
#define xUSARTx_RX_SOURCE      GPIO_PinSource10
#define xUSARTx_RX_AF          GPIO_AF_1

#define xUSARTx_BAUDRATE       115200
#define xUSARTx_BYTESIZE       USART_WordLength_8b
#define xUSARTx_STOPBITS       USART_StopBits_1
#define xUSARTx_PARITY         USART_Parity_No
#define xUSARTx_HARDWARECTRL   USART_HardwareFlowControl_None
/*====================================================================================================*/
/*====================================================================================================*
**函數 : RS232_Config
**功能 : RS232 配置
**輸入 : None
**輸出 : None
**使用 : RS232_Config();
**====================================================================================================*/
/*====================================================================================================*/
void RS232_Config( void )
{
  GPIO_InitTypeDef GPIO_InitStruct;
  USART_InitTypeDef USART_InitStruct;

  /* UART Clk Init *************************************************************/
  RCC_APB2PeriphClockCmd(xUSARTx_CLK, ENABLE);
  RCC_AHBPeriphClockCmd(xUSARTx_TX_GPIO_CLK | xUSARTx_RX_GPIO_CLK, ENABLE);

  GPIO_PinAFConfig(xUSARTx_TX_GPIO_PORT, xUSARTx_TX_SOURCE, xUSARTx_TX_AF);
  GPIO_PinAFConfig(xUSARTx_RX_GPIO_PORT, xUSARTx_RX_SOURCE, xUSARTx_RX_AF);

  /* USARTx Tx PA9 */
  GPIO_InitStruct.GPIO_Pin = xUSARTx_TX_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(xUSARTx_TX_GPIO_PORT, &GPIO_InitStruct);
  /* USARTx Rx PA10 */
  GPIO_InitStruct.GPIO_Pin = xUSARTx_RX_PIN;
  GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(xUSARTx_RX_GPIO_PORT, &GPIO_InitStruct);

  /* UART Init *****************************************************************/
  USART_InitStruct.USART_BaudRate = xUSARTx_BAUDRATE;
  USART_InitStruct.USART_WordLength = xUSARTx_BYTESIZE;
  USART_InitStruct.USART_StopBits = xUSARTx_STOPBITS;
  USART_InitStruct.USART_Parity = xUSARTx_PARITY;
  USART_InitStruct.USART_HardwareFlowControl = xUSARTx_HARDWARECTRL;
  USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(xUSARTx, &USART_InitStruct);
  USART_Cmd(xUSARTx, ENABLE);

  USART_ClearFlag(xUSARTx, USART_FLAG_TC);
}


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


