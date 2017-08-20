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


typedef char *va_list;
#define _AUPBND (sizeof (acpi_native_int) - 1)
#define _ADNBND (sizeof (acpi_native_int) - 1)
#define _bnd(X, bnd) (((sizeof (X)) + (bnd)) & (~(bnd)))
#define va_arg(ap, T) (*(T (((ap) += (_bnd (T, _AUPBND))) - (_bnd (T,_ADNBND))))
#define va_end(ap) (void) 0
#define va_start(ap, A) (void) ((ap) = (((char *) &(A)) + (_bnd (A,_AUPBND))))


int m_printf(const char *str,...)
{
​     va_list ap;//定义一个可变 参数的（字符指针） 
     ​int val,r_val;
     char count,ch;​
     char *s = NULL;​
     int res = 0;//返回值

     va_start(ap,str);//初始化ap
     while('\0' != *str)//str为字符串,它的最后一个字符肯定是'\0'（字符串的结束符）
     { 
          switch(*str)
          {
              case '%':<span style="white-space:pre">	</span>//发送参数
              str++;
              ​switch(*str)
              {
                   case 'd': //10进制输出
                        val = va_arg(ap, int); 
 <span style="white-space:pre">			</span>r_val = val; 
                        count = 0; 
                        while(r_val)​
                        { 
                             count++; //整数的长度
                             r_val /= 10;
                        }​
                        res += count;​//返回值长度增加​ 
                        r_val = val; 
                        while(count)
                        { 
                              ch = r_val / m_pow(10,count - 1);
                              r_val %= m_pw(10,count - 1);​
                              m_putchar(ch + '0');​ //数字到字符的转换 
                              count--;​ 
                        }​ 
                        break;
                  case 'x': //16进制输出 
                        val = va_arg(ap, int); 
                        r_val = val; 
                        count = 0;
                        while(r_val)​ 
                        { 
                             count++; //整数的长度 
                             r_val /= 16; 
                        }​ 
                        res += count;​ //返回值长度增加​ 
                        r_val = val; 
                        while(count) 
                        { 
                              ch = r_val / m_pow(16, count - 1); 
                              r_val %= m_pw(16, count - 1);​ 
                              if(ch <= 9)​
                                  m_putchar(ch + '0');​ <span style="white-space:pre">	</span>//数字到字符的转换 
                              else 
                                  m_putchar(ch - 10 + 'a')​; 
                              count--;​ 
                        }​ 
                 break;
                 case: 's': //发送字符串 
                      s = va_arg(a, char *); ​<span style="white-space:pre">	</span>
                      m_putstr(s);​ //字符串,返回值为字符指针 
                      res += strlen(s);​ //返回值长度增加 ​ 
                 break;​​ 
                 case 'c' 
                      m_putchar( (char)va_arg(ap, int )); //大家猜为什么不写char，而要写int 
                      res += 1;​ 
                 ​break;
                default :;
             }​
             break;
          case '\n':
               ​m_putchar('\n'); 
               res += 1;​
               break;​
          case '\r':
               m_putchar('\r'); 
               res += 1;​
               break;​
​          defaut ：  //显示原来的参数的字符串(不是..里的参数o)
               m_putchar(*str)​;
               res += 1;​
          }​
         str++;​
     }
     va_end(ap);
     return res;​
}

/* USART interrupt handler */
USART_Handler()
{
	if(USART_GetITStatus(USART,USART_IT_RXNE)==SET){
		rbput(&rb_usart,USART_ReceiveData(USART));
	}
}


