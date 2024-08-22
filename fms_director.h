#ifndef FMS_DIRECTOR_
#define FMS_DIRECTOR_

#include "xbasic_types.h"

void FMSInitialize();
void FMSUpdateFAT(u8 channel);

void FMSReadReadyFromTop(u8 channel, u16 fileno);
void FMSReadReadyFromSector(u8 channel, u16 fileno, u16 sector);
void FMSReadReadyFromPause(u8 channel, u16 fileno);
void FMSReadStart(u8 channel, u16 fileno);
void FMSReadStop(u8 channel, u16 fileno);

void FMSWriteReady(u8 channel, u16 fileno);
void FMSWriteStart(u8 channel, u16 fileno);
void FMSWriteStop(u8 channel, u16 fileno);

void FMSCreate(u8 channel, u16 fileno, u16 size);
void FMSDelete(u8 channel, u16 fileno);
void FMSFormat(u8 channel);


#endif
