#include <stdio.h> /* for NULL */
#include <platform.h>

/*
*			   GPIO
*         ------------>   
* PA1(Tx)          |------>        	PA7
* PA2(Rx)     <----|------>			PA9
* PA3	  <----|------>			PA10
* PA4	  <----|------>			PA13
* PA5     <----|------>			PA14
* PA6     <----|------>			PB1
* PF0     <----|------>			PF1
*/

struct pininfo {
	GPIO_TypeDef *GPIO;
	uint16_t GPIO_Pin;
};

const struct pininfo pininfo[]= {
	{GPIOA,GPIO_Pin_4},
	{NULL,0},
	{GPIOA,GPIO_Pin_10},
	{GPIOB,GPIO_Pin_3},
	{GPIOB,GPIO_Pin_5},
	{GPIOB,GPIO_Pin_4},
	{GPIOB,GPIO_Pin_10},
	{GPIOA,GPIO_Pin_8},
/*------------------*/
	{GPIOA,GPIO_Pin_9},
	{GPIOC,GPIO_Pin_7},
	{GPIOB,GPIO_Pin_6},
	{NULL,0},
	{NULL,0},
	{NULL,0},
	{NULL,0},
	{NULL,0},
/*------------------*/
	{GPIOA,GPIO_Pin_0},
	{GPIOA,GPIO_Pin_1},
	{GPIOA,GPIO_Pin_4},
	{GPIOB,GPIO_Pin_0},
	{GPIOC,GPIO_Pin_1},
	{GPIOC,GPIO_Pin_0},
};

#define pin2GPIO(pin) ( pininfo[pin].GPIO )
#define pin2pin(pin)	( pininfo[pin].GPIO_Pin)

/* enum in platform.h 
 *
 * enum {GPIO_INPUT,GPIO_OUTPUT,GPIO_ANALOG};
 * enum {GPIO_PULLNO,GPIO_PULLUP,GPIO_PULLDOWN};
 */
 
static const GPIOMode_TypeDef gpio_mode_buff[3] =
 {GPIO_Mode_IN,GPIO_Mode_OUT,GPIO_Mode_AN};
static const GPIOPuPd_TypeDef gpio_pupd_buff[3] =
 {GPIO_PuPd_NOPULL,GPIO_PuPd_UP,GPIO_PuPd_DOWN};
 
int gpio_mode(int pin,int mode,int pull) {
	DECLARE_GPIO_INIT(ioIt,IN,PP,UP,3);
	
	if(!is_gpio_pin(pin) || !is_gpio_mode(mode) || !is_gpio_pull(pull) )
		return -1;
	ioIt.GPIO_Pin=pin2pin(pin);
	ioIt.GPIO_Mode=gpio_mode_buff[mode];
	ioIt.GPIO_PuPd=gpio_pupd_buff[pull];
	
	GPIO_Init(pin2GPIO(pin),&ioIt);
	return 0;
}

int gpio_write(int pin,int value) {
	if(!is_gpio_pin(pin))
		return -1;
	if(value) pin2GPIO(pin)->BSRR = pin2pin(pin);
	else
	pin2GPIO(pin)->BRR  = pin2pin(pin);
	return 0;
}

int gpio_read(int pin) {
	if(!is_gpio_pin(pin)) 
		return -1;
	return GPIO_ReadInputDataBit(pin2GPIO(pin),pin2pin(pin));
}

int gpio_toggle(int pin) {
	if(!is_gpio_pin(pin))
		return -1;
	pin2GPIO(pin)->ODR ^= pin2pin(pin);
	return 0;
}









