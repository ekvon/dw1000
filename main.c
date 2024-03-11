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

void goodbye_world(){
	/*	turn on leds	*/
	GPIOB->BSRR&=~GPIO_BSRR_BR8;
	GPIOB->BSRR|=GPIO_BSRR_BS8;
	GPIOB->BSRR&=~GPIO_BSRR_BR9;
  GPIOB->BSRR|=GPIO_BSRR_BS9;
  /*	*/
	stm32_usart_tx("Goodbye, world!\n",0);
}

void main(void)
{
	int rv,i;
	/*	register address	*/
	uint16_t addr;
	/*	short buffer for read/write operations	*/
	uint16_t value;
	/*	SPI transaction size	*/
	uint16_t spi_tr_size;
	/*	sended message	*/
	const char * msg="hello, world\n";
	/*	buffer for read/write operations	*/
	uint32_t value32;
	
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
	sprintf(usart_tx_buf,"Device Identifier Register (reg:00:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	/*	set value to PAN Identifier and Short Address register (0x03) and check ii value	*/
	spi_tx_buf[0]=0x80|0x03;
	spi_tx_buf[1]=0x3;
	spi_tx_buf[2]=0x2;
	spi_tx_buf[3]=0x1;
	spi_tx_buf[4]=0x0;
	spi_tr_size=0x8;
	/*	write 0x00010001 value to 0x03 register	*/
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	
	/*	clear buffers and check written value	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
	spi_tx_buf[0]=0x3;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"PAN and Short Address Register (reg:03:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	goodbye_world();
	return;
	
	/*	System Configuration Register (0x04)	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
	/* Allowed bits: 0:FFEN, 1:FFBC(?), 3:FFAD, 4:FFAA, 5:FFAM, 9:HIRQ_POL, 22:RXM110K, 30:AUTOACK
	 * Summary : 0x40400272
	 * Note: All of these fields are used in receiver mode.
	 * To write register value LSB mode is used.
	 */
	spi_tx_buf[0]=0x80|0x04;
	spi_tx_buf[1]=0x72;
	spi_tx_buf[2]=0x02;
	spi_tx_buf[3]=0x40;
	spi_tx_buf[4]=0x40;
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	/*	clear SPI buffers	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
	/*	check written value	*/
	spi_tx_buf[0]=0x4;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"System Configuration Register (reg:04:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	/*	Transmit Frame Control Register (0x08)	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
	/*
	 * Assigned values:
	 *	TFLEN: 64; The length of data portion in bytes includin two-octet CRC appended at the end of the frame. Copied to the PHY header.
	 *	TXBR: 0; User bit rate for the data portion of the frame (see receiver configurtation).
	 *	TXPRF: 2'b10; 64MHz (see receiver configuration).
	 *	TXPSR PE:4'b1100; 4096 preambule length in symbols (Recomended for 11pkBps bit rate).
	 *	TXBOFFS: 0; Transmit buffer index offset.
	 *	IFSDELAY: 0; Later (see receiver configuration)
	 * Summary: reg:08:00 value is 0x00320020
	 * To configure TX_FCTRL register sub-index (2-octet header SPI transaction) is used. 
	*/
	spi_tx_buf[0]=0xc8;
	spi_tx_buf[1]=0x00;
	spi_tx_buf[2]=0x20;
	spi_tx_buf[3]=0x00;
	spi_tx_buf[4]=0x32;
	spi_tx_buf[5]=0x00;
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	/* IFSDELAY field configuration (sub-index 0x04):
	 * The value of this field will be defined later.
	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
	spi_tx_buf[0]=0xc8;
	spi_tx_buf[1]=0x04;
	spi_tx_buf[2]=0x10;
	stm32_spi_tr(spi_tx_buf, spi_rx_buf, spi_tr_size, 0);
	/*	check written values (at sub-index 0x00)	*/
	spi_tx_buf[0]=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"Transmit Frame Control Register (reg:08:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	/* Transmit Data Buffer Register (0x09)	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
	/*	
	* Copy sended mesage to the application transmit buffer.
	* All application transmit buffer will be written to TX_BUFFER register using the only SPI transaction.
	* Algorithm of SPI transaction may be changed in the future.
	*/ 
	spi_tx_buf[0]=0xc9;
	spi_tx_buf[1]=0x80;
	spi_tx_buf[2]=0x00;
	strcpy(spi_tx_buf+3, msg);
	spi_tr_size=DW1000_TFLEN-0x2;
	stm32_spi_tr(spi_tx_buf, spi_rx_buf, spi_tr_size, 0);
	/*	check the first written byte	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
	spi_tx_buf[0]=0x49;
	spi_tx_buf[1]=0x80;
	spi_tx_buf[2]=0x00;
	spi_tr_size=0x10;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"Transmit Buffer Register (reg:09:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	/*	start transmission	*/
	return;
}
