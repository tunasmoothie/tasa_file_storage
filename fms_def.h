#ifndef FMS_DEFINITIONS_
#define FMS_DEFINITIONS_

#include "xparameters.h"
#include "xbasic_types.h"
#include "interface/fms_mram.h"
#include "interface/fms_uart.h"


#define CHANNEL0   0
#define CHANNEL1   1
#define CHANNEL2   2

#define FET_ADDR_OFFSET 0x4      //4 bytes
#define SCT_ADDR_OFFSET 0x1604   //5636 bytes

#define FAT0_BASEADDR   0x000000
#define FAT1_BASEADDR   0x005604
#define FAT2_BASEADDR   0x00AC08
//#define FAT3_BASEADDR   0x01020C
//#define FAT4_BASEADDR   0x015810
//#define FAT5_BASEADDR   0x01AE14

#define FAT0_HIGHADDR   0x005603
#define FAT1_HIGHADDR   0x00AC07
#define FAT2_HIGHADDR   0x01020B


#define FAT_SIZE_BYTES  22020
#define FET_SIZE_BYTES  11
#define SCT_SIZE_BYTES  16384
#define TOTAL_SECTORS   8192

inline u32 getFATBaseAddr(u8 channel){
	switch(channel){
		case 0:
			return FAT0_BASEADDR;
		case 1:
			return FAT1_BASEADDR;
		case 2:
			return FAT2_BASEADDR;
	}
	return 0;
}

#endif
