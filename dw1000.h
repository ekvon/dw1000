#ifndef __DW1000__DW1000__H
#define __DW1000__DW1000__H

/*	this buffer size is enough to write maximum (standard) number of bytes to TX_BUFFER	*/
#define DW1000_SPI_BUF_SIZE 0x80
/*	default size of SPI transaction	*/
#define DW1000_SPI_TR_SIZE 0x8
#define DW1000_TFLEN 0x40

#define DW1000_RST_High GPIOB->BSRR&=~GPIO_BSRR_BR1;\
	GPIOB->BSRR|=GPIO_BSRR_BS1
#define DW1000_RST_Low GPIOB->BSRR&=~GPIO_BSRR_BS1;\
	GPIOB->BSRR|=GPIO_BSRR_BR1
	
#define DW_SPI_READ 0
#define DW_SPI_WRITE 1

/*	register file ID	*/
#define DW_REG_DEV_ID 0x00
#define DW_REG_PANADR 0x03

#define DW_USE_PANADR 0
#define DW_PAN 0x0001
#define DW_SHORT_ADDRESS 0x0203

#include <stdint.h>
	
/*	TX/RX call-back data (loaded from deca_device_api.h)	*/
typedef struct
{
	uint32_t status;      //initial value of register as ISR is entered
	uint16_t datalength;  //length of frame
	uint8_t  fctrl[2];    //frame control bytes
	uint8_t  rx_flags;    //RX frame flags, see above
} dw_cb_data_t;

/*	Call-back type for all events	(loaded from deca_device_api.h)	*/
typedef void (*dw_cb_t)(const dw_cb_data_t *);

/* 
 * Structure to store device configuration.
 * Loaded from deca_device_api.h
 *
 */
typedef struct
{
	/*	channel number {1, 2, 3, 4, 5, 7 }	*/
	uint8_t chan;
	/*	Pulse Repetition Frequency {DWT_PRF_16M or DWT_PRF_64M}	*/
	uint8_t prf;
	/*	DWT_PLEN_64..DWT_PLEN_4096	*/
	uint8_t txPreambLength ;
	/*	Acquisition Chunk Size (Relates to RX preamble length)	*/
	uint8_t rxPAC;
	/*	TX preamble code	*/
	uint8_t txCode;
	/*	RX preamble code	*/
	uint8_t rxCode;
	/*	Boolean should we use non-standard SFD for better performance	*/
	uint8_t nsSFD;
	/*	Data Rate {DWT_BR_110K, DWT_BR_850K or DWT_BR_6M8}	*/
	uint8_t dataRate;
	/*	PHR mode {0x0 - standard DWT_PHRMODE_STD, 0x3 - extended frames DWT_PHRMODE_EXT}	*/
	uint8_t phrMode;
	/*	SFD timeout value (in symbols)	*/
	uint16_t sfdTO;
} dw_config_t;

/*
 * Structure used for the single SPI transaction.
 */
typedef struct
{
	/*	0: read, 1: write (mandatory)	*/
	uint8_t rw;
	/*	Register file ID (mandatory)	*/
	uint8_t fileID;
	/*	number of octets in transaction header (mandatory)	*/
	uint8_t octets;
	/*	Register sub-address (optional)	*/
	uint16_t subAddress;
	/*	send buffer	*/
	uint8_t txBuf[DW1000_SPI_BUF_SIZE];
	/*	recv buffer	*/
	uint8_t rxBuf[DW1000_SPI_BUF_SIZE];
	/*	the length of data to read or write	*/
	uint8_t len;
	/*	data are used for write transaction	*/
	void * data;
} dw_spi_data_t;
	
int dw_spi_tr(dw_spi_data_t * data);
#endif
