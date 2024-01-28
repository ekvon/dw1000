#include "stm32.h"

int stm32_gpio_init(){	
	/*	general output for PB8 (led)	*/
	GPIOB->MODER|=GPIO_MODER_MODER8_0;
	/*	general output for PB9 (led)	*/
	GPIOB->MODER|=GPIO_MODER_MODER9_0;
	/*	pull-down for gpio pins	*/
	GPIOB->OTYPER|=(0x1<<16)|(0x1<<18);
	/*	turn off leds	*/
	GPIOB->BSRR&=~GPIO_BSRR_BS8;
	GPIOB->BSRR|=GPIO_BSRR_BR8;
	GPIOB->BSRR&=~GPIO_BSRR_BS9;
  GPIOB->BSRR|=GPIO_BSRR_BR9;
  return STM32_SUCCESS;
}

