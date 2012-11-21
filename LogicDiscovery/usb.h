/*
 * usb.h
 *
 *  Created on: 01.07.2011
 *      Author: Flexz
 */

#ifndef USB_H_
#define USB_H_

#include "usb_bsp.h"

typedef void	(*usbTrnansferDoneCallback)(void);
struct ExternalTransferUnit
{
	void *ptr;
	int  size;
};

typedef ExternalTransferUnit	(*usbExternalTransferCallback)(void);
void	usbInit(void);
void	usbWrite(void *buffer_in, int count_in);
void	usbWriteFirst(void *buffer_in0, int count_in0, void *buffer_in1, int count_in1);
//Auto-transfer mode (just push teh buffer)
void	usbStartTransfer(void *buffer_ptr, int count, usbTrnansferDoneCallback callback);
void	usbPrepareTransfer(void *buffer_ptr, int count, usbTrnansferDoneCallback callback);
//User-controlled transfer (see .cpp for details)
void	usbStartExternalTransfer(usbExternalTransferCallback callback);

//#include "usb_app_interface.h"

//extern "C" void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev);
//extern "C" void USB_HP_CAN1_TX_IRQHandler(void);
//extern "C" void USB_LP_CAN1_RX0_IRQHandler(void);

#endif /* USB_H_ */
