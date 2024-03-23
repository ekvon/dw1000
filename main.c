#include "main.h"
#include "stm32.h"
#include "dw1000.h"

#include <memory.h>
#include <stdio.h> 

char usart_tx_buf[CHAR_BUF_SIZE];
/*	WARNING: LSB is sended/recived first at write/read SPI operations	*/
char spi_tx_buf[DW1000_SPI_BUF_SIZE];
char spi_rx_buf[DW1000_SPI_BUF_SIZE];

/*
void SysTick_Handler(){
	SysTick->CTRL&=~SysTick_CTRL_ENABLE_Msk;
}
*/

void clear_spi_buffers(){
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
}

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
	/*	number of transmitted frames	*/
	int tx_number=0x100;
	
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
	
	/*	check written values	*/
	clear_spi_buffers();
	spi_tx_buf[0]=0x3;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"PAN and Short Address Register (reg:03:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	/*	System Configuration Register (0x04)	*/
	clear_spi_buffers();
	
	/* These settings are actually for receiver. 
	 * Allowed bits: 0:FFEN, 1:FFBC(?), 3:FFAD, 4:FFAA, 5:FFAM, 9:HIRQ_POL, 22:RXM110K, 30:AUTOACK
	 * Summary : 0x40400272
	 * Note: All of these fields are used in receiver mode.
	 * spi_tx_buf[0]=0x80|0x04;
	 * spi_tx_buf[1]=0x72;
	 * spi_tx_buf[2]=0x02;
	 * spi_tx_buf[3]=0x40;
	 * spi_tx_buf[4]=0x40;
	 */
	 
	/*	The only bit used in transmitter mode is HIRQ_POL	*/
	spi_tx_buf[0]=0x80|0x04;
	spi_tx_buf[1]=0x00;
	spi_tx_buf[2]=0x02;
	spi_tx_buf[3]=0x00;
	spi_tx_buf[4]=0x00;
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	
	/*	check written values	*/
	clear_spi_buffers();
	spi_tx_buf[0]=0x4;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"System Configuration Register (reg:04:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	/*	Transmit Frame Control Register (0x08)	*/
	clear_spi_buffers();
	
	/*
	 * Assigned values:
	 *	TFLEN: 64; The length of data portion in bytes includin two-octet CRC appended at the end of the frame. Copied to the PHY header.
	 *	TXBR: 0; User bit rate for the data portion of the frame (see receiver configurtation).
	 *	TXPRF: 2'b10; 64MHz (see receiver configuration).
	 *	PE TXPSR:4'b0011; 4096 preambule length in symbols (Recomended for 110kBps bit rate).
	 *	TXBOFFS: 0; Transmit buffer index offset.
	 * Summary: reg:08:00 value is 0x00e00040
	 * Two octet SPI header is used. 
	*/
	spi_tx_buf[0]=0xc8;
	/*	sub-index	*/
	spi_tx_buf[1]=0x00;
	spi_tx_buf[2]=0x40;
	spi_tx_buf[3]=0x00;
	spi_tx_buf[4]=0xe0;
	spi_tx_buf[5]=0x00;
	spi_tx_buf[6]=0x00;
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	
	/*	
	* IFSDELAY (reg:08:04): 0xa5; This value is experimental and will be changed in the future.
	* Two octet SPI header is used.
	*/
	clear_spi_buffers();
	spi_tx_buf[0]=0xc8;
	/*	sub-index	*/
	spi_tx_buf[1]=0x04;
	spi_tx_buf[2]=0xff;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	
	/*	check written values (at sub-index 0x00)	*/
	clear_spi_buffers();
	spi_tx_buf[0]=0x48;
	spi_tx_buf[1]=0x00;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"Transmit Frame Control Register (reg:08:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	/* Transmit Data Buffer Register (0x09)	*/
	clear_spi_buffers();
	/*	
	* Copy sended mesage to the SPI transmit buffer after 3-octet header plus one byte.
	* The first byte in TX_BUFFER must be equal to zero (see algorithm of receiver).
	* Algorithm of SPI transaction may be changed in the future.
	*/ 
	spi_tx_buf[0]=0xc9;
	/*	extended sub-index	*/
	spi_tx_buf[1]=0x80;
	spi_tx_buf[2]=0x00;
	strcpy(spi_tx_buf+3+1, msg);
	spi_tr_size=DW1000_TFLEN-0x2;
	stm32_spi_tr(spi_tx_buf, spi_rx_buf, spi_tr_size, 0);
	
	/*	check the first written two bytes	*/
	memset(spi_tx_buf,0,DW1000_SPI_BUF_SIZE);
	memset(spi_rx_buf,0,DW1000_SPI_BUF_SIZE);
	spi_tx_buf[0]=0x49;
	spi_tx_buf[1]=0x80;
	spi_tx_buf[2]=0x00;
	spi_tr_size=0x08;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	sprintf(usart_tx_buf,"Transmit Buffer Register (reg:09:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
		spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
	stm32_usart_tx(usart_tx_buf,0);
	
	/*
	 * Channel Control Register (0x1f)
	 * The following configuration is applied:
	 * TX_CHAN: 0x4
	 * RX_CHAN: 0x4
	 * DWSFD: 0
	 * RXPRF: 2'b10
	 * TNSSFD: 0
	 * RNSSFD: 0
	 * TX_PCODE: 0x11
	 * RX_PCODE: 0x11
	 * Summary:
	 * The register value is 0x8c480044.
	 * SPI transaction with 1-octet header will be used (LSB is sended first).
	 * Notes:
	 * Available codes for 4-th channel and 64 MHz PRF are 0x11, 0x12, 0x13, 0x14.
	 * The values of TX_PCODE and RX_PCODE may be changed.
	 */
	clear_spi_buffers();
	spi_tx_buf[0]=0x80|0x1f;
	spi_tx_buf[1]=0x44;
	spi_tx_buf[2]=0x00;
	spi_tx_buf[3]=0x48;
	spi_tx_buf[4]=0x8c;
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	
	/*
	 * Analog RF Configuration Block (0x28)
	 * The only subregister which will be programmed is 0x28:0c (RF_TXCTRL).
	 * It's value (specified for 4-th channel) is 0x00045c80.
	 * 2-octet header SPI transcation is used.
	 */
	clear_spi_buffers();
	spi_tx_buf[0]=0xc0|0x28;
	spi_tx_buf[1]=0x0c;
	spi_tx_buf[2]=0x80;
	spi_tx_buf[3]=0x5c;
	spi_tx_buf[4]=0x04;
	spi_tx_buf[5]=0x00;
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	
	/*
	* Frequency Synthesiser Control Block (0x2b)
	* Sub-Register 0x2b:07-FS_PLLCFG: 0x08400508 (specified for 4-th channel)
	* Sub-Register 0x2b:0b-FS_PLLTUNE: 0x26 (specified for 4-th channel)
	* 2-octet header SPI transcation is used.
	*/
	clear_spi_buffers();
	spi_tx_buf[0]=0xc0|0x2b;
	spi_tx_buf[1]=0x07;
	spi_tx_buf[2]=0x08;
	spi_tx_buf[3]=0x05;
	spi_tx_buf[4]=0x40;
	spi_tx_buf[5]=0x08;
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	
	spi_tx_buf[0]=0xc0|0x2b;
	spi_tx_buf[1]=0x0b;
	spi_tx_buf[2]=0x26;
	spi_tx_buf[3]=0x00;
	spi_tx_buf[4]=0x00;
	spi_tx_buf[5]=0x00;
	spi_tr_size=0x8;
	stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
	
	/*	send tx_number frames	*/
	clear_spi_buffers();
	
	for(i=0;i<tx_number;i++){
		/*	the only bit is written to SYS_STRL is TXSTRT	*/
		spi_tx_buf[0]=0x8d;
		spi_tx_buf[1]=0x02;
		spi_tr_size=DW1000_SPI_TR_SIZE;
		stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);
		
		clear_spi_buffers();
		value32=0;
		/*	don't check written value; monitor status register (2-octet SPI header is used)	*/
		while(!value32){
			clear_spi_buffers();
			spi_tx_buf[0]=0x4f;
			spi_tx_buf[1]=0x00;
			stm32_spi_tr(spi_tx_buf,spi_rx_buf,spi_tr_size,0);		
			/*	WARNING: the size of transaction should be defined exactly	*/
			value32=((int)spi_rx_buf[4])&0x80;
		}
		sprintf(usart_tx_buf,"Frame %d is sended. System Event Status Register (reg:0f:00):\n\t{0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x,0x%x}\n",
			i,
			spi_rx_buf[0],spi_rx_buf[1],spi_rx_buf[2],spi_rx_buf[3],spi_rx_buf[4],spi_rx_buf[5],spi_rx_buf[6],spi_rx_buf[7]);
			stm32_usart_tx(usart_tx_buf,0);
	}
	/*	end of program	*/
	goodbye_world();
	return;
}
