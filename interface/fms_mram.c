#include "fms_mram.h"

#define MRAM_ID_AVALANCHE		0xE6
#define MRAM_SIZE_ID_16M		0x4
#define PAGE_SIZE  		        256

int FMSMramInitSpi(FMSMram *InstancePtr, u16 SpiDeviceId){

	XSpi *spi_ptr = &(InstancePtr->Spi);

	XSpi_Config *xspi_cfg = XSpi_LookupConfig(SpiDeviceId);
	if (xspi_cfg == NULL) {
		xil_printf("XSpi_LookupConfig Error\r\n");
		return XST_FAILURE;
	}

	int Status = XSpi_CfgInitialize(spi_ptr, xspi_cfg, xspi_cfg->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("XSpi_CfgInitialize Error\r\n");
		return XST_FAILURE;
	}

	xil_printf("SPI Selftest...   ");
	Status = XSpi_SelfTest(spi_ptr);
	if (Status != XST_SUCCESS) {
		xil_printf("XSpi_SelfTest Error\r\n");
		return XST_FAILURE;
	}
	xil_printf("Success\r\n");

	Status = XSpi_SetOptions(spi_ptr, XSP_MASTER_OPTION | XSP_MANUAL_SSELECT_OPTION);
	if (Status != XST_SUCCESS) {
		xil_printf("XSpi_SetOptions  Error\r\n");
		return XST_FAILURE;
	}

	Status = XSpi_SetSlaveSelect(spi_ptr, 1);
	if (Status != XST_SUCCESS) {
		xil_printf("XSpi_SetSlaveSelect \r\n");
			return XST_FAILURE;
	}

	Status = XSpi_Start(spi_ptr);
	if (Status != XST_SUCCESS) {
		xil_printf("XSpi_Start Error\r\n");
			return XST_FAILURE;
	}
	XSpi_IntrGlobalDisable(spi_ptr);

	return XST_SUCCESS;
}

int FMSMramGetId(FMSMram *InstancePtr){
	InstancePtr->sendbuf[0] = 0x9f;
	InstancePtr->sendbuf[1] = 0x00;
	InstancePtr->sendbuf[2] = 0x00;
	InstancePtr->sendbuf[3] = 0x00;
	InstancePtr->sendbuf[4] = 0x00;
	if (XSpi_Transfer(&(InstancePtr->Spi), InstancePtr->sendbuf, InstancePtr->recvbuf, 5) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	xil_printf("MRAM MANUFACTURER ID: %02X \r\n", InstancePtr->recvbuf[1]);
	xil_printf("MRAM DENSITY: %02X \r\n", InstancePtr->recvbuf[3] & 0xF);

	return XST_SUCCESS;
}

int MRAMRead(FMSMram *mram_ptr, uint32_t addr, uint8_t* buf, uint32_t bytes){

	//xil_printf("Read %d bytes from 0x%02X", bytes, addr);

	while(bytes){
		int read_size = (bytes > PAGE_SIZE) ? PAGE_SIZE : bytes;
		mram_ptr->sendbuf[0] = 0x03;
		mram_ptr->sendbuf[1] = (uint8_t)((addr >> 16) & 0xFF);
		mram_ptr->sendbuf[2] = (uint8_t)((addr >> 8) & 0xFF);
		mram_ptr->sendbuf[3] = (uint8_t)(addr& 0xFF);

		if (XSpi_Transfer(&(mram_ptr->Spi), mram_ptr->sendbuf, mram_ptr->recvbuf, 4 + read_size) != XST_SUCCESS) {
			return XST_FAILURE;
		}
		memcpy(buf, mram_ptr->recvbuf + 4, read_size);

		bytes -= read_size;
		addr += read_size;
		buf += read_size;
	}

	//xil_printf("  done\r\n");
	return XST_SUCCESS;
}

int MRAMWriteEnable(FMSMram *mram_ptr, bool en){

	mram_ptr->sendbuf[0] = en ? 0x06 : 0x04;
	if (XSpi_Transfer(&(mram_ptr->Spi), mram_ptr->sendbuf, NULL, 1) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	return XST_SUCCESS;
}


int MRAMWrite(FMSMram *mram_ptr, uint32_t addr, uint8_t* buf, uint32_t bytes){

	MRAMWriteEnable(mram_ptr, true);

	mram_ptr->sendbuf[0] = 0x05;
	mram_ptr->sendbuf[1] = 0x00;
	if (XSpi_Transfer(&(mram_ptr->Spi), mram_ptr->sendbuf, mram_ptr->recvbuf, 2) != XST_SUCCESS) {
		return XST_FAILURE;
	}

	if((mram_ptr->recvbuf[1] & 0x2) >> 1 != true){
		xil_printf("MRAM not write-enabled.\r\n");
		return XST_FAILURE;
	}

	xil_printf("Writing %d bytes to 0x%02X... \r\n", bytes, addr);

	while(bytes){
		int write_size = (bytes > PAGE_SIZE) ? PAGE_SIZE : bytes;
		mram_ptr->sendbuf[0] = 0x02;
		mram_ptr->sendbuf[1] = (uint8_t)((addr >> 16) & 0xFF);
		mram_ptr->sendbuf[2] = (uint8_t)((addr >> 8) & 0xFF);
		mram_ptr->sendbuf[3] = (uint8_t)(addr& 0xFF);

		memcpy(mram_ptr->sendbuf + 4, buf, write_size);
		if (XSpi_Transfer(&(mram_ptr->Spi), mram_ptr->sendbuf, NULL, 4 + write_size) != XST_SUCCESS) {
			return XST_FAILURE;
		}

		bytes -= write_size;
		addr += write_size;
		buf += write_size;
	}


	xil_printf("Done \r\n");

	return XST_SUCCESS;
}

int MRAMClear(FMSMram *mram_ptr){

	uint8_t zero[256] = {0};
	for(int i = 0; i < 86; i++){
		MRAMWrite(mram_ptr, i*256, zero, 22020);
	}

	return XST_SUCCESS;
}

int MRAMClear(FMSMram *mram_ptr, u16 startaddr, u16 endaddr){
	int bytes = endaddr - startaddr;
	uint8_t zero[bytes] = {0};

	MRAMWrite(mram_ptr, startaddr, zero, bytes);

	return XST_SUCCESS;
}
