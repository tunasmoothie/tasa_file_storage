#include "fms_uart.h"
#include "xparameters.h"
#include "xuartlite.h"
#include "xintc.h"
#include "xil_exception.h"
#include "xil_printf.h"

static volatile int sentcnt;
static volatile int recvcnt;

void SendHandler(void *CallBackRef, unsigned int EventData)
{
	sentcnt = EventData;
}

void RecvHandler(void *CallBackRef, unsigned int EventData)
{
	recvcnt = EventData;
}

int UHSetupIntr(FMSUartHandler *uh_ptr){

	int Status;

	Status = XIntc_Initialize(&uh_ptr->xint, XPAR_INTC_0_DEVICE_ID);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}


	/*
	 * Connect a device driver handler that will be called when an interrupt
	 * for the device occurs, the device driver handler performs the
	 * specific interrupt processing for the device.
	 */
	Status = XIntc_Connect(&uh_ptr->xint, XPAR_INTC_0_UARTLITE_0_VEC_ID,
			(XInterruptHandler)XUartLite_InterruptHandler,
			(void *)(&uh_ptr->xuart));
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Start the interrupt controller such that interrupts are enabled for
	 * all devices that cause interrupts, specific real mode so that
	 * the UartLite can cause interrupts through the interrupt controller.
	 */
	Status = XIntc_Start(&uh_ptr->xint, XIN_REAL_MODE);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	/*
	 * Enable the interrupt for the UartLite device.
	 */
	XIntc_Enable(&uh_ptr->xint, XPAR_INTC_0_UARTLITE_0_VEC_ID);

	/*
	 * Initialize the exception table.
	 */
	Xil_ExceptionInit();

	/*
	 * Register the interrupt controller handler with the exception table.
	 */
	Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
			 (Xil_ExceptionHandler)XIntc_InterruptHandler,
			 &uh_ptr->xint);

	/*
	 * Enable exceptions.
	 */
	Xil_ExceptionEnable();

	return XST_SUCCESS;
}

int UHInit(FMSUartHandler *uh_ptr, u16 dev_id){

	int Status;

	Status = XUartLite_Initialize(&(uh_ptr->xuart), dev_id);
	if (Status != XST_SUCCESS) {
		xil_printf("UART init failed. \r\n");
		return XST_FAILURE;
	}

	Status = XUartLite_SelfTest(&(uh_ptr->xuart));
	if (Status != XST_SUCCESS) {
		xil_printf("UART init failed. \r\n");
		return XST_FAILURE;
	}



	Status = UHSetupIntr(uh_ptr);
	if (Status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XUartLite_SetSendHandler(&uh_ptr->xuart, SendHandler, &uh_ptr->xuart);
	XUartLite_SetRecvHandler(&uh_ptr->xuart, RecvHandler, &uh_ptr->xuart);
	XUartLite_EnableInterrupt(&uh_ptr->xuart);

	return XST_SUCCESS;
}


int UHCmdGen(FMSUartHandler *uh_ptr, u8 cmd_type, u16 data_len, u8 channel, u8 data[]){
	uh_ptr->txbuf[0] = 0x55;
	uh_ptr->txbuf[1] = 0xAA;
	uh_ptr->txbuf[2] = cmd_type;
	uh_ptr->txbuf[3] = (u8)((data_len+1) >> 8);
	uh_ptr->txbuf[4] = (u8)((data_len+1) & 0x00FF);
	uh_ptr->txbuf[5] = channel;

	u8 checksum = cmd_type + uh_ptr->txbuf[3] + uh_ptr->txbuf[4] + channel;

	for(int i = 0; i < data_len; i++){
		uh_ptr->txbuf[i+6] = data[i];
		checksum += data[i];
	}

	uh_ptr->txbuf[6+data_len] = checksum;
	uh_ptr->txbuf[7+data_len] = 0xFB;
	uh_ptr->txbuf[8+data_len] = 0xF2;

	uh_ptr->txbufused = 9+data_len;

	xil_printf("Generated command in TX buffer: \r\n");
//	for(int i = 0; i < 9+data_len; i++){
//		xil_printf("0x%02X ", uh_ptr->txbuf[i]);
//	}
	xil_printf("\r\n");

	return 9+data_len;
}

void UHSendTxBuffer(FMSUartHandler *uh_ptr){

	XUartLite_Send(&(uh_ptr->xuart), uh_ptr->txbuf, uh_ptr->txbufused);
	while(sentcnt != uh_ptr->txbufused);

	xil_printf("   --> Sent TX buffer (%d bytes). \r\n\n", sentcnt);
}

