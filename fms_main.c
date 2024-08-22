#include <stdio.h>
#include "xparameters.h"
#include "xstatus.h"
#include "xil_printf.h"
#include <sleep.h>

#include "interface/fms_mram.h"
#include "interface/fms_uart.h"
#include "fms_algorithm.h"
#include "fms_director.h"
#include "fms_def.h"



int main()
{
	int Status = XST_SUCCESS;
	xil_printf("Init FMS Algorithm... \r\n");
	FMSInitialize();

	xil_printf("Algorithm init done. \r\n");

//	FMSCreate(CHANNEL0, 2, 10);
//	FMSCreate(CHANNEL0, 0, 4);
//	FMSCreate(CHANNEL0, 34, 50);
//	FMSReadReadyFromTop(CHANNEL0, 2);
//	FMSDelete(CHANNEL0, 34);
//	FMSReadReadyFromTop(CHANNEL0, 34);
//	FMSCreate(CHANNEL0, 34, 5);
	//FMSUpdateFAT(0);

	FMSFormat(CHANNEL0);
	FMSCreate(CHANNEL0, 5, 4000);
	FMSCreate(CHANNEL0, 6, 4000);
	FMSDelete(CHANNEL0, 5);
	FMSCreate(CHANNEL0, 7, 200);
	//FMSReadReadyFromTop(CHANNEL0, 34);
	//FMSReadStart(CHANNEL0, 34);
	//FMSReadStop(CHANNEL0, 34);


    return Status;
}

