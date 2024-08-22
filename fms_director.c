#include <stdio.h>
#include "fms_director.h"
#include "fms_def.h"
#include "fms_algorithm.h"
#include "interface/fms_mram.h"
#include "interface/fms_uart.h"


FMSUartHandler Uart_MB0;
FMSUartHandler Uart_MB1;
FMSUartHandler Uart_MB2;
FMSMram Mram_CB;

FMSUartHandler* getChannelUH(u8 channel){
	switch (channel){
		case 0:
			return &Uart_MB0;
		case 1:
			return &Uart_MB1;
		case 2:
			return &Uart_MB2;
	}
	return NULL;
}

void FMSInitialize(){
	xil_printf("SPI Initialization for MRAM \r\n");
	FMSMramInitSpi(&Mram_CB, XPAR_SPI_0_DEVICE_ID);
	xil_printf("SPI init done.\r\n");
	FMSMramGetId(&Mram_CB);
	UHInit(getChannelUH(CHANNEL0), XPAR_UARTLITE_0_DEVICE_ID);
	//UHInit(getChannelUH(CHANNEL1), XPAR_UARTLITE_1_DEVICE_ID);
	//UHInit(getChannelUH(CHANNEL2), XPAR_UARTLITE_2_DEVICE_ID);
}

void FMSUpdateFAT(u8 channel){
	FMSAlgorithmUpdateFreeSector(&Mram_CB, channel);
}

void FMSReadReadyFromTop(u8 channel, u16 fileno){
	u16 filesize = FMSAlgorithmGetFileSize(&Mram_CB, channel, fileno);
	u16* sectorlist = FMSAlgorithmGetFileSectors(&Mram_CB, channel, fileno);

	for(u16 i = 0; i < filesize; i++){
		xil_printf("0x%04X ", sectorlist[i]);
	}
	xil_printf("\r\n");

	u8 sectorlist_8b[filesize*2];
	for(u16 i = 0; i < filesize; i++){
		sectorlist_8b[i*2] = (u8)(sectorlist[i] >> 8);
		sectorlist_8b[(i*2)+1] = (u8)(sectorlist[i]);
	}
	delete(sectorlist);

	UHCmdGen(getChannelUH(CHANNEL0), UH_CMD_READ_READY, (filesize*2), channel, sectorlist_8b);\
	UHSendTxBuffer(getChannelUH(CHANNEL0));
}


void FMSWriteReady(u8 channel, u16 fileno){
	u16 filesize = FMSAlgorithmGetFileSize(&Mram_CB, channel, fileno);
	u16* sectorlist = FMSAlgorithmGetFileSectors(&Mram_CB, channel, fileno);

	for(u16 i = 0; i < filesize; i++){
		xil_printf("0x%04X ", sectorlist[i]);
	}
	xil_printf("\r\n");

	u8 sectorlist_8b[filesize*2];
	for(u16 i = 0; i < filesize; i++){
		sectorlist_8b[i*2] = (u8)(sectorlist[i] >> 8);
		sectorlist_8b[(i*2)+1] = (u8)(sectorlist[i]);
	}
	delete(sectorlist);

	UHCmdGen(getChannelUH(CHANNEL0), UH_CMD_WRITE_READY, (filesize*2), channel, sectorlist_8b);
	UHSendTxBuffer(getChannelUH(CHANNEL0));
}

void FMSReadStart(u8 channel, u16 fileno){
	UHCmdGen(getChannelUH(CHANNEL0), UH_CMD_READ_START, 0, channel, NULL);
	UHSendTxBuffer(getChannelUH(CHANNEL0));
}

void FMSReadStop(u8 channel, u16 fileno){
	UHCmdGen(getChannelUH(CHANNEL0), UH_CMD_READ_STOP, 0, channel, NULL);
	UHSendTxBuffer(getChannelUH(CHANNEL0));
}


void FMSWriteStart(u8 channel, u16 fileno){
	UHCmdGen(getChannelUH(CHANNEL0), UH_CMD_WRITE_START, 0, channel, NULL);
	UHSendTxBuffer(getChannelUH(CHANNEL0));
}

void FMSWriteStop(u8 channel, u16 fileno){
	UHCmdGen(getChannelUH(CHANNEL0), UH_CMD_WRITE_STOP, 0, channel, NULL);
	UHSendTxBuffer(getChannelUH(CHANNEL0));
}

void FMSCreate(u8 channel, u16 fileno, u16 filesize){
	FMSAlgorithmCreateFile(&Mram_CB, channel, fileno, filesize);
	FMSAlgorithmUpdateFreeSector(&Mram_CB, channel);
}

void FMSDelete(u8 channel, u16 fileno){
	u16 filesize = FMSAlgorithmGetFileSize(&Mram_CB, channel, fileno);
	u16* sectorlist = FMSAlgorithmGetFileSectors(&Mram_CB, channel, fileno);
	u8 sectorlist_8b[filesize*2];
	for(u16 i = 0; i < filesize; i++){
		sectorlist_8b[i*2] = (u8)(sectorlist[i] >> 8);
		sectorlist_8b[(i*2)+1] = (u8)(sectorlist[i]);
	}
	delete(sectorlist);

	FMSAlgorithmDeleteFile(&Mram_CB, channel, fileno);
	FMSAlgorithmUpdateFreeSector(&Mram_CB, channel);

	UHCmdGen(getChannelUH(CHANNEL0), UH_CMD_ERASE, (filesize*2), channel, sectorlist_8b);
	UHSendTxBuffer(getChannelUH(CHANNEL0));
}

void FMSFormat(u8 channel){
	FMSAlgorithmFormatFAT(&Mram_CB, channel);
	FMSAlgorithmUpdateFreeSector(&Mram_CB, channel);

	UHCmdGen(getChannelUH(CHANNEL0), UH_CMD_ERASE_ALL, 0, channel, NULL);
	UHSendTxBuffer(getChannelUH(CHANNEL0));
}

