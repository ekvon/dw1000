#include "main.h"
#include "stm32.h"
#include "dw1000.h"

#include <memory.h>
#include <stdio.h>


char usart_tx_buf[CHAR_BUF_SIZE];
char spi_tx_buf[DW1000_SPI_BUF_SIZE];
char spi_rx_buf[DW1000_SPI_BUF_SIZE];

/*	not in use	*/
/*
void SysTick_Handler(){
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;
}
*/

void main(void)
{
	int rv,i;
	/*	register address	*/
	uint16_t addr;
	/*	use to write data to the only register	*/
	uint16_t value;
	/*	SPI transaction size	*/
	uint16_t spi_tr_size;
	
	/*	PLLCLK settings	*/
	stm32_pll_t stm32_pll;
	/*	USART baudrate settings	*/
	stm32_usart_br_param_t usart_br;

	stm32_rcc_init();
	/*	get current value of SYSCLK (so HSE is used as invalid value may be returned)	*/
	SystemCoreClockUpdate();
	/*
	* PLLCLK settings.
	* HSE is not used as source clock and so SystemCoreClockUpdate() MUST return the true value.
	* At this configuration PLL frequency is 40MHz.
	*/
	stm32_pll.f_in=/*8000000*/SystemCoreClock;
	stm32_pll.P=4;
	stm32_pll.M=8;
	stm32_pll.N=160;
	if(stm32_rcc_pll_init(&stm32_pll)<0)
		/*	PLLCLK configuration error	*/
		return;
	/*	switch to PLLCLK	*/
	stm32_pll.sw=1;
	/*	stm32_rcc_pll(&stm32_pll);	*/
	/*	get current value of SYSCLK (invalid value is returned)	*/
	SystemCoreClockUpdate();
		
	stm32_gpio_init();
	
	/*	use SystemCoreClock variable	*/
	usart_br.f_ck=SystemCoreClock;
	usart_br.br=STM32_USART_B9600;
	usart_br.over8=0;
	if(stm32_usart_br_init(&usart_br)){
		return;
	}
	stm32_usart_init(&usart_br);
	sprintf(usart_tx_buf,"System is configured. SystemCoreClock is %li\n",SystemCoreClock);	
	stm32_usart_tx(usart_tx_buf,0);
	/*	
	* SPI initialization.
	* Software slave management is enable.
	* SPI1_NSS pin will be moved manually.
	*/

	if(stm32_spi_init(1)<0){
		sprintf(usart_tx_buf,"SPI configuration error\n");
		stm32_usart_tx(usart_tx_buf,0);
		return;
	}
	sprintf(usart_tx_buf,"SPI is configured successfully\n");
	stm32_usart_tx(usart_tx_buf,0);
	
	/*	soft reset of the chip	*/
	DW1000_RST_Low;
	DW1000_RST_High;
	/*	read register using 1-bit transaction header	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	/*	rx-register will be cleared	in SPI procedure	*/
	spi_tx_buf[0]=0x0;
	/*	*/
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"dw1000 SPI buffer content: {0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);

	/*	turn on leds	*/
	GPIOB->BSRR&=~GPIO_BSRR_BR8;
	GPIOB->BSRR|=GPIO_BSRR_BS8;
	GPIOB->BSRR&=~GPIO_BSRR_BR9;
  GPIOB->BSRR|=GPIO_BSRR_BS9;
  /*	*/
	stm32_usart_tx("Programm is finished\n",0);
}
