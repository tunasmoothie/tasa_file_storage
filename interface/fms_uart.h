#ifndef FMS_UART_
#define FMS_UART_

#include "fms_uart.h"
#include "xparameters.h"
#include "xuartlite.h"
#include "xintc.h"

#define UH_CMD_READ_READY   0x00
#define UH_CMD_WRITE_READY  0x01
#define UH_CMD_ERASE        0x02
#define UH_CMD_READ_START   0x03
#define UH_CMD_WRITE_START  0x04
#define UH_CMD_READ_STOP    0x05
#define UH_CMD_WRITE_STOP   0x06
#define UH_CMD_ERASE_ALL    0x07

#define UART_BUFFER_SIZE 8192

typedef struct{
	XUartLite xuart;
	XIntc xint;
	u8 txbuf[UART_BUFFER_SIZE];
	u8 rxbuf[UART_BUFFER_SIZE];
	int txbufused;
	int rxbufused;
} FMSUartHandler;

int UHInit(FMSUartHandler *uh_ptr, u16 dev_id);
int UHCmdGen(FMSUartHandler *uh_ptr, u8 cmd_type, u16 data_len, u8 channel, u8 data[]);
void UHSendTxBuffer(FMSUartHandler *uh_ptr);

#endif
