#ifndef __AMUNGO__DW1000__STM32__H
#define __AMUNGO__DW1000__STM32__H

/*
 * Functions, structures, constants specific for Stm32 board are declared here.
 */

#include <stm32f4xx.h>

#include <stddef.h>
#include <stdint.h>

#define STM32_HSE_FREQ 8000000
#define STM32_SUCCESS 0
#define STM32_ERROR -1

/*	supported baudrate values	*/
enum usart_br_t
{
	STM32_USART_B9600=9600,
	STM32_USART_B19200=19200,
	STM32_USART_B115200=115200
};

typedef struct stm32_usart_br_param
{
	/*	input settings	*/
	uint32_t f_ck;
	uint32_t br;
	uint8_t over8;
	/*	calculated parameters	*/
	uint16_t mantissa;
	uint8_t fraction;
	uint32_t reg;
} stm32_usart_br_param_t;

/*	setting for PLL clock	*/
typedef struct stm32_pll
{
	/*	input settings (should be filled by the user)	*/
	uint32_t f_in;
	uint8_t P;
	uint8_t M;
	uint16_t N;
	/*	output frequency at specified settings	*/
	uint32_t f_out;
	/*	RCC_PLLCFGR value	*/
	uint32_t reg;
	/*	flag to switch to PLL clock	*/
	uint8_t sw;
} stm32_pll_t;

/*	RCC initialization	*/
int stm32_rcc_init();
/*	define value of RCC_PLLCFGR at specified settings	*/
int stm32_rcc_pll_init(stm32_pll_t * stm32_pll);
/*	switch to PLLCLK; before calling this function struct stm32_pll should be initialized with the help of 'stm32_rcc_pll_init'	*/
void stm32_rcc_pll(stm32_pll_t * stm32_pll);

/*	GPIO initialization	*/
int stm32_gpio_init();

/*	USART initialization	*/
int stm32_usart_init(stm32_usart_br_param_t * usart_br);
/*	define value of USART->BRR at specified settings	*/
int stm32_usart_br_init(stm32_usart_br_param_t * usart_br);
/*	send data using USART1	*/
int stm32_usart_tx(int8_t * data,size_t len);
/*	receive data using USART1	(without interrupt)	*/
int stm32_usart_rx(int8_t * data,size_t len);

/*	stm32 SPI module interface	*/
int stm32_spi_init(uint8_t ssm_enable);
/*	transmit only procedure for BIDIMODE=0 RXONLY=0 (rm0383 p.573)	*/
int stm32_spi_send(int8_t * data,uint16_t size,uint8_t mode);
/*	receive only procedure for BIDIMODE=0 RXONLY=1 (rm0383 p.575)	*/
int stm32_spi_recv(int8_t * data,uint16_t size,uint8_t mode);
/*	full-duplex transmit procedure for BIDIMODE=0 RXONLY=0 (rm0383 p.572)	*/
int stm32_spi_tr(int8_t * tx_buf,int8_t * rx_buf,uint16_t size,uint8_t mode);
/*	chip select management in SSM enable mode	*/
/*	void stm32_spi_cs_high();	*/
/*	void stm32_spi_cs_low();	*/

#endif
