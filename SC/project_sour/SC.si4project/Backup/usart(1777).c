#include "platform.h"
#include "ringbuffer.h"

DECLARE_RB(rb_usart,USART_RD_BUF_LEN);

void UsartInit(void) {
    GPIO_InitTypeDef  GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef nvic_init_struct;
    
    RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE );
    
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_1);        
    /*
    *  USART1_TX -> PA9 , USART1_RX ->        PA3
    */                                
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;                 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; 
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_Init(GPIOA, &GPIO_InitStructure);        
    
    USART_InitStructure.USART_BaudRate = 9600;//���ô��ڲ�����115200
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;//��������λ
    USART_InitStructure.USART_StopBits = USART_StopBits_1;//����ֹͣλ
    USART_InitStructure.USART_Parity = USART_Parity_No;//����Ч��λ
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//����������
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;//���ù���ģʽ
    USART_Init(USART2, &USART_InitStructure); //������ṹ��
    
    /***** NVIC�ж����� *****/
    nvic_init_struct.NVIC_IRQChannel = USART2_IRQn;
    nvic_init_struct.NVIC_IRQChannelPriority = 0;
    nvic_init_struct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&nvic_init_struct);
    
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);
    USART_ITConfig(USART2,USART_IT_TC,DISABLE);
    USART_Cmd(USART2, ENABLE);//ʹ�ܴ���1
}

/** -- ����һ���ֽ� -- by lhb_steven -- 2017/6/14**/
void UsartSendByte(uint8_t byte) {
    while(!((USART1->ISR)&(1<<7)));
    USART1->TDR=byte;	
}

/** -- �����ַ��� -- by lhb_steven -- 2017/6/14**/
void UsartSend(uint8_t *Buffer, uint32_t Length) {
    while(Length != 0) {
        while(!((USART2->ISR)&(1<<7)));//�ȴ�������
        USART2->TDR= *Buffer;
        Buffer++;
        Length--;
    }
}

/** -- ��ȡ���յ������� -- by lhb_steven -- 2017/6/14**/
uint8_t UART_Recive(void) {	
    while(!(USART2->ISR & (1<<5)));//�ȴ����յ�����
    return(USART2->RDR);   //��������
}

/*====================================================================================================*/
/*====================================================================================================*
**���� : RS232_Config
**���� : RS232 ����
**ݔ�� : None
**ݔ�� : None
**ʹ�� : RS232_Config();
**====================================================================================================*/
/*====================================================================================================*/
void RS232_Config( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  NVIC_InitTypeDef  NVIC_InitStructure;

  NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  RCC_AHBPeriphClockCmd( RCC_AHBPeriph_GPIOA, ENABLE);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_1);
  GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_1);
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2|GPIO_Pin_3;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);

  RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE ); 
  USART_InitStructure.USART_BaudRate = 9600;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
  USART_Init(USART2, &USART_InitStructure);
  USART_Cmd(USART2, ENABLE);
}

/*====================================================================================================*/
/*====================================================================================================*
**���� : RS232_SendStr
**���� : �l���ִ�
**ݔ�� : *pWord
**ݔ�� : None
**ʹ�� : RS232_SendStr((int8_t*)"Hello World!");
**====================================================================================================*/
/*====================================================================================================*/
void RS232_SendStr( int8_t* pWord )
{
  USART_SendData(USART2, 0x55);
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


