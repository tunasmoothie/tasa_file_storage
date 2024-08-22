#ifndef FMS_ALGORITHM_
#define FMS_ALGORITHM_

#include "interface/fms_mram.h"

void FMSAlgorithmUpdateFreeSector(FMSMram *mram_ptr, u8 channel);
void FMSAlgorithmFormatFAT(FMSMram *mram_ptr, u8 channel);
u16 FMSAlgorithmCreateFile(FMSMram *mram_ptr, u8 channel, u16 fileno, u16 filesize);
u16 FMSAlgorithmGetFileSize(FMSMram *mram_ptr, u8 channel, u16 fileno);
u16* FMSAlgorithmGetFileSectors(FMSMram *mram_ptr, u8 channel, u16 fileno);
u16 FMSAlgorithmDeleteFile(FMSMram *mram_ptr, u8 channel, u16 fileno);

#endif
