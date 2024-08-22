#include <stdlib.h>
#include "xil_printf.h"
#include "xbasic_types.h"
#include "interface/fms_mram.h"
#include "interface/fms_uart.h"
#include "fms_algorithm.h"
#include "fms_def.h"


/********************************************************************
 * Cache space
 ********************************************************************/
u8 info_cache[4];
u8 fet_cache[11];
u8 sct_cache[16384];


/********************************************************************
 * Cache fetching & transfer function macros
 ********************************************************************/
#define fetchCacheInfo(mram_ptr, channel) \
	MRAMRead(mram_ptr, getFATBaseAddr(channel), info_cache, 4);

#define transferCacheInfo(mram_ptr, channel) \
	MRAMWrite(mram_ptr, getFATBaseAddr(channel), info_cache, 4);

#define fetchCacheFET(mram_ptr, channel, fileno) \
	MRAMRead(mram_ptr, getFATBaseAddr(channel)+FET_ADDR_OFFSET+(fileno*11), fet_cache, FET_SIZE_BYTES);

#define transferCacheFET(mram_ptr, channel, fileno) \
	MRAMWrite(mram_ptr, getFATBaseAddr(channel)+FET_ADDR_OFFSET+(fileno*11), fet_cache, FET_SIZE_BYTES);

#define fetchCacheSCT(mram_ptr, channel) \
	MRAMRead(mram_ptr, getFATBaseAddr(channel)+SCT_ADDR_OFFSET, sct_cache, SCT_SIZE_BYTES);

#define transferCacheSCT(mram_ptr, channel) \
	MRAMWrite(mram_ptr, getFATBaseAddr(channel)+SCT_ADDR_OFFSET, sct_cache, SCT_SIZE_BYTES);


/********************************************************************
 * Bit manipulation macros
 ********************************************************************/
#define FREE_SECTOR_CNT \
	(((u16)(info_cache[1] & 0x3F) << 8) | (u16)info_cache[0])

#define FREE_SECTOR_NEXT \
	(((u16)(info_cache[3] & 0x3F) << 8) | (u16)info_cache[2])

#define SECTOR_END(no) \
	((sct_cache[no*2+1] & 0x20) > 0)

#define SECTOR_USED(no) \
	((sct_cache[no*2+1] & 0x40) > 0)

#define SECTOR_BAD(no) \
	((sct_cache[no*2+1] & 0x80) > 0)

#define SECTOR_NEXT(no) \
	(((u16)(sct_cache[no*2+1] & 0x1F) << 8) | sct_cache[no*2])

#define FILE_EXIST \
	((fet_cache[0] & 0x08) > 0)

#define FILE_FULL \
	(fet_cache[0] & 0x04 > 0)

#define FILE_SIZE \
	(((u16)(fet_cache[2] & 0x1F) << 8) | fet_cache[1])

#define FILE_START \
	(((u16)(fet_cache[4] & 0x1F) << 8) | fet_cache[3])



/********************************************************************
 * Function Bodies
 ********************************************************************/
void FMSAlgorithmUpdateFreeSector(FMSMram *mram_ptr, u8 channel){
	fetchCacheInfo(mram_ptr, channel);
	fetchCacheSCT(mram_ptr, channel);
	u16 freecnt = 0;
	u16 nextfree = FREE_SECTOR_NEXT;

	for(u16 i = 0; i < TOTAL_SECTORS; i++){
		freecnt += !SECTOR_USED(i);
	}

	for(u16 i = 0; i < TOTAL_SECTORS; i++){
		if (!SECTOR_USED((i+nextfree)%8192)){
			nextfree = (i+nextfree)%8192;
			break;
		}
	}

	info_cache[0] = (u8)freecnt;
	info_cache[1] = freecnt >> 8;
	info_cache[2] = (u8)nextfree;
	info_cache[3] = nextfree >> 8;

	transferCacheInfo(mram_ptr, channel)

	xil_printf("Free sector info updated: \r\n\n");
	xil_printf("  Next Free Sector at: 0x%02X\r\n", FREE_SECTOR_NEXT);
	xil_printf("  Available free sectors: %d\r\n\n", FREE_SECTOR_CNT);
}


// TBD::: BAD SECTORS ARE OVERIDDEN
void FMSAlgorithmFormatFAT(FMSMram *mram_ptr, u8 channel){
	u8 tmp[22020] = {0};
	tmp[0] = 0x00;
	tmp[1] = 0x20;
	MRAMWrite(mram_ptr, getFATBaseAddr(channel), tmp, 22020);
}

u16 FMSAlgorithmCreateFile(FMSMram *mram_ptr, u8 channel, u16 fileno, u16 filesize){
	// Error checking
	if(fileno > 5631){
		xil_printf("ERROR: Cannot create file (%d), exceeds max fileno. \r\n", fileno);
		return 0;
	}
	fetchCacheInfo(mram_ptr, channel);
	if(FREE_SECTOR_CNT < filesize){
		xil_printf("ERROR: Cannot create file (%d), not enough space. \r\n", fileno);
		return 0;
	}
	fetchCacheFET(mram_ptr, channel, fileno);
	if(FILE_EXIST){
		xil_printf("ERROR: Cannot create file (%d), already exists. \r\n", fileno);
		return 0;
	}
	fetchCacheSCT(mram_ptr, channel);


	// Find sectors to use for new file, and store into sectorlist
	u16 sectorlist[filesize];
	sectorlist[0] = FREE_SECTOR_NEXT; //start sector
	u16 cnt = 1;
	for(u16 i = sectorlist[0]+1; i < 8192; i++){
		if(!SECTOR_USED(i) && !SECTOR_BAD(i)) {
			sectorlist[cnt++] = i;
			if(cnt == filesize) break;
		}
	}
	if(cnt < filesize)
	for(u16 i = 0; i < sectorlist[0]; i++){
		if(!SECTOR_USED(i) && !SECTOR_BAD(i)) {
			sectorlist[cnt++] = i;
			if(cnt == filesize) break;
		}
	}


	// Link sector next ptrs and mark as used
	for(u16 i = 0; i < filesize-1; i++){
		u16 tmp = 0x4000 | sectorlist[i+1];
		sct_cache[sectorlist[i]*2] = (u8)tmp;
		sct_cache[sectorlist[i]*2+1] = (u8)(tmp >> 8);
	}
	u16 tmp = 0x6000 | sectorlist[filesize-1]; // Final sector mark as end and link to itself
	sct_cache[sectorlist[filesize-1]*2] = (u8)tmp;
	sct_cache[sectorlist[filesize-1]*2+1] = (u8)(tmp >> 8);


	// Update FET entry
	fet_cache[0] = 0x08;
	fet_cache[1] = (u8)filesize;
	fet_cache[2] = (u8)(filesize >> 8);
	fet_cache[3] = (u8)sectorlist[0];
	fet_cache[4] = (u8)(sectorlist[0] >> 8);

	// Sync cache into MRAM
	transferCacheSCT(mram_ptr, channel);
	transferCacheFET(mram_ptr, channel, fileno);

	xil_printf("Created file [%d] starting at sector 0x%04X, size %d sector(s).\r\n", fileno, sectorlist[0], filesize);

	return filesize;
}

u16 FMSAlgorithmGetFileSize(FMSMram *mram_ptr, u8 channel, u16 fileno){
	// Error checking
	fetchCacheFET(mram_ptr, channel, fileno);
	if(!FILE_EXIST || fileno > 5631){
		xil_printf("ERROR: Cannot find file [%d], does not exist. \r\n", fileno);
		return 0;
	}

	return FILE_SIZE;
}

u16* FMSAlgorithmGetFileSectors(FMSMram *mram_ptr, u8 channel, u16 fileno){
	// Error checking
	fetchCacheFET(mram_ptr, channel, fileno);
	if(!FILE_EXIST || fileno > 5631){
		xil_printf("ERROR: Cannot find file [%d], does not exist. \r\n", fileno);
		return NULL;
	}
	fetchCacheSCT(mram_ptr, channel);

	u16 filesize = FILE_SIZE;
	u16 *sectorlist = (u16*)malloc(filesize*sizeof(u16));
	sectorlist[0] = FILE_START;
	for(u16 i = 1; i < filesize; i++){
		sectorlist[i] = SECTOR_NEXT(sectorlist[i-1]);
	}

	xil_printf("Found [%d] sector(s) for file [%d].\r\n", filesize, fileno);

	// !! IMPORTANT !!
	// Function caller must free allocated memory for list manually
	return sectorlist;
}

u16 FMSAlgorithmDeleteFile(FMSMram *mram_ptr, u8 channel, u16 fileno){
	// Error checking
	fetchCacheInfo(mram_ptr, channel);
	fetchCacheFET(mram_ptr, channel, fileno);
	if(!FILE_EXIST || fileno > 5631){
		xil_printf("ERROR: Cannot find file (%d), does not exist. \r\n", fileno);
		return 0;
	}
	fetchCacheSCT(mram_ptr, channel);

	u16 filesize = FILE_SIZE;
	u16 *sectorlist = FMSAlgorithmGetFileSectors(mram_ptr, channel, fileno);
	for(int i = 0; i < filesize; i++){
		sct_cache[sectorlist[i]*2] = 0x00;
		sct_cache[sectorlist[i]*2+1] = sct_cache[sectorlist[i]*2+1] & 0x80;
	}

	memset(fet_cache, 0x00, sizeof(fet_cache));

	transferCacheSCT(mram_ptr, channel);
	transferCacheFET(mram_ptr, channel, fileno);

	return filesize;
}
