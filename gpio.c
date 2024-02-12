#include "stm32.h"

int stm32_gpio_init(){	
	/*	general output for PB0 (DW_WAKEUP)	*/
	GPIOB->MODER|=GPIO_MODER_MODER0_0;
	/*	general output for PB1 (DW_RST)	*/
	GPIOB->MODER|=GPIO_MODER_MODER1_0;
	/*	general output for PB8 (led)	*/
	GPIOB->MODER|=GPIO_MODER_MODER8_0;
	/*	general output for PB9 (led)	*/
	GPIOB->MODER|=GPIO_MODER_MODER9_0;
	/*	pull-down for all gpio pins	*/
	GPIOB->OTYPER|=0x1|(0x1<<2)|(0x1<<16)|(0x1<<18);
	/*	low level at PB0	*/
	GPIOB->BSRR&=~GPIO_BSRR_BS0;
	GPIOB->BSRR|=GPIO_BSRR_BR0;
	/*	low level at PB1	*/
	GPIOB->BSRR&=~GPIO_BSRR_BS1;
	GPIOB->BSRR|=GPIO_BSRR_BR1;
	/*	turn off leds	*/
	GPIOB->BSRR&=~GPIO_BSRR_BS8;
	GPIOB->BSRR|=GPIO_BSRR_BR8;
	GPIOB->BSRR&=~GPIO_BSRR_BS9;
  GPIOB->BSRR|=GPIO_BSRR_BR9;
  return STM32_SUCCESS;
}

