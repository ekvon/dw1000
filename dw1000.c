#include "dw1000.h"
#include "stm32.h"

#include <string.h>

static int _dw_spi_read_tr(dw_spi_data_t * data);
static int _dw_spi_write_tr(dw_spi_data_t * data);
/*	copy data to send to transmit buffer in inverse order	*/
static int _copy_data_to_buffer(dw_spi_data_t * data);

int dw_spi_tr(dw_spi_data_t * data){
	/*	clear buffers	*/
	memset(data->txBuf, 0, DW1000_SPI_BUF_SIZE);
	memset(data->rxBuf, 0, DW1000_SPI_BUF_SIZE);
	if(data->rw){
		return _dw_spi_write_tr(data);
	}
	else{
		return _dw_spi_read_tr(data);
	}
}

int _dw_spi_read_tr(dw_spi_data_t * data){
	uint16_t spi_tr_size;
	
	if(data->rw!=0)
		return -1;
	/*	return operation	*/
	switch(data->octets){
		case(0x1):{
			/*	only six bits of Register file ID are used	*/
			data->txBuf[0]=(data->fileID&0x3f);
			/*	receive two more byte just in case	*/
			spi_tr_size=data->len+3;
		}
		case(0x2):{
			data->txBuf[0]=0xc0|(data->fileID&0x3f);
			/*	copy 7-bit subaddress to tx-buffer	*/
			data->txBuf[1]=(uint8_t)(data->subAddress&0x7f);
			/*	receive two more byte just in case	*/
			spi_tr_size=data->len+4;
		}
		case(0x3):{
			data->txBuf[0]=0xc0|(data->fileID&0x3f);
			/*	low order 7-bits of 15-bit file subaddress	*/
			data->txBuf[1]=(uint8_t)(data->subAddress&0x7f);
			/*	high order 8-bits of 15-bit file subaddress	*/
			data->txBuf[2]=(uint8_t)((data->subAddress>>7)&0xff);
			/*	send one more byte just in case	*/
			spi_tr_size=data->len+5;
		}
		default:
			/*	invalid number of octets	*/
			return -1;
	}
	stm32_spi_tr(data->txBuf, data->rxBuf, spi_tr_size, 0);
	return spi_tr_size;
}

int _dw_spi_write_tr(dw_spi_data_t * data){
	uint16_t spi_tr_size;
	
	if(!data->rw)
		return -1;
	/*	write operation	*/
	switch(data->octets){
		case(0x1):{
			/*	only six bits of Register file ID are used	*/
			data->txBuf[0]=0x80|(data->fileID&0x3f);
			/*	send one more byte just in case	*/
			spi_tr_size=data->len+2;
		}
		case(0x2):{
			data->txBuf[0]=0xc0|(data->fileID&0x3f);
			/*	copy 7-bit subaddress to tx-buffer	*/
			data->txBuf[1]=(uint8_t)(data->subAddress&0x7f);
			/*	send one more byte just in case	*/
			spi_tr_size=data->len+3;
		}
		case(0x3):{
			data->txBuf[0]=0xc0|(data->fileID&0x3f);
			/*	low order 7-bits of 15-bit file subaddress	*/
			data->txBuf[1]=(uint8_t)(data->subAddress&0x7f);
			/*	high order 8-bits of 15-bit file subaddress	*/
			data->txBuf[2]=(uint8_t)((data->subAddress>>7)&0xff);
			/*	send one more byte just in case	*/
			spi_tr_size=data->len+4;
		}
		default:
			/*	invalid number of octets	*/
			return -1;
	}
	/*	copy data to buffer in inverse order	*/
	_copy_data_to_buffer(data);
	stm32_spi_tr(data->txBuf, data->rxBuf, spi_tr_size, 0);
	return spi_tr_size;
}

int _copy_data_to_buffer(dw_spi_data_t * data){
	if(!data->len)
		return -1;
	if(DW_SPI_BUF_SIZE-data->octets<data->len)
		return -1;
	uint8_t off=data->octets;
	for(uint8_t i=data->len;0<i--;){
		data->txBuf[off++]=*(data->data+i);
	}
	return (int)data->len;
}
