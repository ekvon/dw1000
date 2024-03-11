#ifndef __DW1000__DW1000__H
#define __DW1000__DW1000__H

/*	this buffer size is enough to write maximum (standard) number of bytes to TX_BUFFER	*/
#define DW1000_SPI_BUF_SIZE 0x80
#define DW1000_TFLEN 0x40

#define DW1000_RST_High GPIOB->BSRR&=~GPIO_BSRR_BR1;\
	GPIOB->BSRR|=GPIO_BSRR_BS1
#define DW1000_RST_Low GPIOB->BSRR&=~GPIO_BSRR_BS1;\
	GPIOB->BSRR|=GPIO_BSRR_BR1
	
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
	uint8_t chan ;           //!< channel number {1, 2, 3, 4, 5, 7 }
	uint8_t prf ;            //!< Pulse Repetition Frequency {DWT_PRF_16M or DWT_PRF_64M}
	uint8_t txPreambLength ; //!< DWT_PLEN_64..DWT_PLEN_4096
	uint8_t rxPAC ;          //!< Acquisition Chunk Size (Relates to RX preamble length)
	uint8_t txCode ;         //!< TX preamble code
	uint8_t rxCode ;         //!< RX preamble code
	uint8_t nsSFD ;          //!< Boolean should we use non-standard SFD for better performance
	uint8_t dataRate ;       //!< Data Rate {DWT_BR_110K, DWT_BR_850K or DWT_BR_6M8}
	uint8_t phrMode ;        //!< PHR mode {0x0 - standard DWT_PHRMODE_STD, 0x3 - extended frames DWT_PHRMODE_EXT}
	uint16_t sfdTO ;         //!< SFD timeout value (in symbols)
} dw_config_t ;
	
	
#endif
