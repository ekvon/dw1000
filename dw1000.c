#include "dw1000.h"

static int _dw_spi_read_tr(dw_spi_data_t * data);
static int _dw_spi_write_tr(dw_spi_data_t * data);

int dw_spi_tr(dw_spi_data_t * data){
	/*	clear buffers	*/
	memset(data->txBuf, 0, DW1000_SPI_BUF_SIZE);
	memset(data->rxBUF, 0, DW1000_SPI_BUF_SIZE);
	if(data->rw){
		_dw_spi_write_tr(data);
	}
	else{
		_dw_spi_read_tr(data);
	}
}

int _dw_spi_read_tr(dw_spi_data_t * data){
}

int _dw_spi_write_tr(dw_spi_data_t * data){
	/*	write operation	*/
	switch(data->octets){
		case(0x1):{
			/*	only six bits of Register file ID are used	*/
			data->txBuf[0]=0x80|(data->fileID&0x3f);
			/*	copy data to send to tx-buffer	*/
			memcpy(data->txBuf+1, data, len);
		}
		case(0x2):{
			data->txBuf[0]=0xc0|(data->fileID&0x3f);
			/*	copy 7-bit subaddress to tx-buffer	*/
			data->txBuf[1]=(uint8_t)(data->subaddress&0x3f);;
			/*	copy data	*/
			memcpy(data->txBuf+2, data, len);
		}
		case(0x3):{
			data->txBuf[0]=0xc0|(data->fileID&0x3f);
			/*	low order 7-bits of 15-bit file subaddress	*/
			data->txBuf[1]=(uint8_t)(data->subaddress&0x3f);
			/*	high order 8-bits of 15-bit file subaddress	*/
			data->txBuf[2]=(uint8_t)((data->subaddress>>7)&0xff);
			/*	copy data	*/
			memcpy(data->txBuf+3, data, len);
		}
		default:
			/*	invalid number of octets	*/
			return -1;
	}
}
