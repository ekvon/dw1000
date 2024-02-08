#include "stm32.h"

#include <math.h>
#include <string.h>

int stm32_usart_init(stm32_usart_br_param_t * usart_br){
	/*	AF for PA9 (AF7)	*/
	GPIOA->MODER|=GPIO_MODER_MODER9_1;
	/*	AF for PA10 (AF7)	*/
	GPIOA->MODER|=GPIO_MODER_MODER10_1;
	
	/*	AF7 for PA9 (USART1_Tx)	*/
	GPIOA->AFR[1]|=GPIO_AFRH_AFSEL9_0|GPIO_AFRH_AFSEL9_1|GPIO_AFRH_AFSEL9_2;
	/*	AF7 for PA10 (USART_Rx)	*/
	GPIOA->AFR[1]|=GPIO_AFRH_AFSEL10_0|GPIO_AFRH_AFSEL10_1|GPIO_AFRH_AFSEL10_2;
	/*	set high speed for PB6 (USART1_Tx) and PB7 (USART1_Rx)	*/
	GPIOA->OSPEEDR|=0xa000;
	
	/*	USART1 (8N1)	*/
	USART1->BRR=usart_br->reg;
	/*	USART enable	*/
	USART1->CR1|=USART_CR1_UE;
	return STM32_SUCCESS;
}

int stm32_usart_br_init(stm32_usart_br_param_t * usart_br){
	float USARTDIV;
	float mantissa,fraction;
	/*
	* The following formula is used to define the value of USART baud rate register:
	*		br=f_clk/(8*(2-over8)*USARTDIV)
	* where f_clk is frequency of the bus on which USART is placed. The fixed point format is used.
	* When over8=0, the fractional part is coded on 4 bits. To get correct value of fractional part of BRR
	* fractional part of USARTDIV must be multiplied on 16 and round up.
	*/	
	if(usart_br->over8)
		/*	not supported	*/
		return STM32_ERROR;
	USARTDIV=(1.0*usart_br->f_ck)/(8*(2-usart_br->over8)*usart_br->br);
	mantissa=floorf(USARTDIV);
	fraction=ceilf(0x10*(USARTDIV-mantissa));
	
	usart_br->mantissa=(uint16_t)mantissa;
	usart_br->fraction=(uint8_t)fraction;
	usart_br->reg=(usart_br->mantissa<<4)|usart_br->fraction;
	return STM32_SUCCESS;
}

int stm32_usart_tx(int8_t * data,size_t len){
	size_t i;
	
	if((USART1->CR1&USART_CR1_TE))
		/*	transmitter MUST be disabled	*/
		return STM32_ERROR;
	if(!(USART1->SR&USART_SR_TXE)||!(USART1->SR&USART_SR_TC))
		/*	data register MUST be empty and all transmissions MUST be completed	*/
		return STM32_ERROR;
		
	if(len==0)
		len=strlen(data);
	/*	start transmission	*/
	USART1->CR1|=USART_CR1_TE;
	for(i=0;i<len;i++){
		USART1->DR=(uint32_t)*(data+i);
		while(!(USART1->SR&USART_SR_TXE)){
		}
	}
	while(!(USART1->SR&USART_SR_TC)){
	}
	/*	disable transmitter	*/
	USART1->CR1&=~USART_CR1_TE;
	return (int)i;
}

int stm32_usart_rx(int8_t * data,size_t len){
	size_t i;

	/*	nulling buffer	*/
	memset(data,0,len);
	/*	disable transmitter	*/
	USART1->CR1&=~USART_CR1_TE;
	/*	enable receiver	*/
	USART1->CR1|=USART_CR1_RE;
	for(i=0;i<len;i++){
		if(USART1->SR&USART_SR_RXNE){
			/*	receive character	*/
			data[i]=(uint8_t)USART1->DR;
			if(data[i]=='\n')
				break;
		}
	}
	/*	disable	receiver	*/
	USART1->CR1&=~USART_CR1_RE;
	/*	enable transmitter	*/
	USART1->CR1|=USART_CR1_TE;
	return (int)i;
}

