#ifndef FMS_MRAM_
#define FMS_MRAM_

#include "xparameters.h"
#include "xspi.h"

typedef struct{
	XSpi Spi;
	u8 sendbuf[256];
	u8 recvbuf[256];
} FMSMram;

int FMSMramInitSpi(FMSMram *InstancePtr, u16 SpiDeviceId);
int FMSMramGetId(FMSMram *InstancePtr);
int MRAMRead(FMSMram *mram_ptr, uint32_t addr, uint8_t* buf, uint32_t bytes);
int MRAMWriteEnable(FMSMram *mram_ptr, bool en);
int MRAMWrite(FMSMram *mram_ptr, uint32_t addr, uint8_t* buf, uint32_t bytes);
int MRAMClear(FMSMram *mram_ptr);
int MRAMClear(FMSMram *mram_ptr, u16 startaddr, u16 endaddr);

#endif
