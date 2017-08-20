#ifndef platform_h
#define platform_h

#include "stm32f0xx.h"



 /* Left			   NucleoF303RE				   	 Right			
  *                                   15 PB8(SCL)
  *													          14 PB9(SDA)
  *                                   13 PA5(SCK)
  *                                   12 PA6(MISO)
  *               GPIO                11 PA7(MOSI)
  *           ------------>           10 PB6
  *                |------>            9 PC7
  *                |------>            8 PA9
  *------------------------------------------------
  *   ANALOG       |------>            7 PA8
  *                |------>            6 PB10
  * 16 A0 PA0 <----|------>            5 PB4
  * 17 A1 PA1 <----|------>            4 PB5
  * 18 A2 PA4 <----|------>            3 PB3 
  * 19 A3 PB0 <----|------>            2 PA10
  * 20 A4 PC1 <----|                   1 PA2(Tx)
  * 21 A5 PC0 <----|                   0 PA3(Rx)
  */
  
enum {GPIO_A0=16,GPIO_A1,GPIO_A2,GPIO_A3,GPIO_A4,GPIO_A5};
#define is_gpio_pin(pin)   ( ((pin>=2)&&(pin<=10)) || \
													   ((pin>=GPIO_A0)&&(pin<=GPIO_A5)) )
#define is_analog_pin(pin) ( ((pin>=GPIO_A0)&&(pin<=GPIO_A5)) )
 
#define USART USART2
#define USART_GPIO GPIOA
#define USART_BAUD 115200
#define USART_RD_BUF_LEN 256
#define USART_GPIO_AF GPIO_AF_7
#define USART_RX_PIN GPIO_Pin_2
#define USART_TX_PIN GPIO_Pin_3
#define USART_RX_SRC GPIO_PinSource2
#define USART_TX_SRC GPIO_PinSource3
#define USART_IRQn   USART2_IRQn
#define USART_Handler() void USART2_IRQHandler(void)
#define USART_ClkEn() \
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE)
	
#define SPI SPI1
#define spi_cshigh() (GPIOB->BSRR = (1<<6) ) 
#define spi_cslow()  (GPIOB->BRR  = (1<<6) )

/* serial communication functions */
int __io_putchar(int );
int __io_getchar(void);
int __io_puts(const char *);
char *__io_gets(char *,int);

/* platform gpio functions */
enum {GPIO_INPUT,GPIO_OUTPUT,GPIO_ANALOG};
enum {GPIO_PULLNO,GPIO_PULLUP,GPIO_PULLDOWN};
#define is_gpio_mode(mode) ( ((mode)>=GPIO_INPUT)&&((mode)<=GPIO_ANALOG) )
#define is_gpio_pull(pull) ( ((pull)>=GPIO_PULLNO)&&((pull)<=GPIO_PULLDOWN) )

/* if param 'pin' isn't a really pin,this function will return -1. 
 * else return 0 except gpio_read 
 * (gpio_read will return the pin's actual value).
 */
int gpio_mode(int pin,int mode,int pull);
int gpio_write(int pin,int value);
int gpio_read(int pin);
int gpio_toggle(int pin);

/* platform spi functions */
enum {SPI_MODE0,SPI_MODE1,SPI_MODE2,SPI_MODE3};
enum {SPI_MSBFIRST,SPI_LSBFIRST};
#define is_spi_mode(mode) ( ((mode)>=SPI_MODE0)&&((mode)<=SPI_MODE3) )
#define is_spi_order(order) (((order)==SPI_MSBFIRST)||((order)==SPI_LSBFIRST))
#define is_spi_bauddiv(div) (((div)>=0)&&((div)<=7))

int spi_init(int mode,int order,int bauddiv);
int spi_mode(int mode);
int spi_order(int order);
int spi_div(int div);
int spi_info(void);
uint8_t spi_transfer(uint8_t data);

/* decalre a GPIO_InitTypeDef valible. named as 'name'
 *  mode can be 'IN' ,'OUT', 'AF', 'AN'
 *  ppod can be 'PP' or 'OD'
 *  updown can be 'UP','DOWN'
 *  speed can from 1 to 3
 */
#define DECLARE_GPIO_INIT(name,mode,ppod,updown,speed) \
	GPIO_InitTypeDef name ={ \
			.GPIO_Pin=0, \
			.GPIO_Mode=GPIO_Mode_##mode, \
			.GPIO_OType=GPIO_OType_##ppod,\
			.GPIO_PuPd=GPIO_PuPd_##updown,\
			.GPIO_Speed=GPIO_Speed_Level_##speed,\
	};

#define DECLARE_USART_INIT(name,baud) \
		USART_InitTypeDef name ={ \
				.USART_BaudRate=baud, \
				.USART_HardwareFlowControl=USART_HardwareFlowControl_None,\
				.USART_Mode=USART_Mode_Rx|USART_Mode_Tx,\
				.USART_Parity=USART_Parity_No, \
				.USART_StopBits=USART_StopBits_1,\
				.USART_WordLength=USART_WordLength_8b,\
		};

/* cpol can be 'Low' or 'High'
 * cpha can be '1Edge' or '2Edge'
 * order can be 'LSB' or 'MSB'
 */
#define DECLARE_SPI_INIT(name,cpol,cpha,order) \
	SPI_InitTypeDef name = { \
		.SPI_CRCPolynomial = 7, \
		.SPI_NSS = SPI_NSS_Soft, \
		.SPI_CPOL = SPI_CPOL_##cpol,\
		.SPI_CPHA = SPI_CPHA_##cpha, \
		.SPI_Mode = SPI_Mode_Master, \
		.SPI_DataSize = SPI_DataSize_8b, \
		.SPI_FirstBit = SPI_FirstBit_##order, \
		.SPI_Direction = SPI_Direction_2Lines_FullDuplex, \
		.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16, \
	};


#endif




