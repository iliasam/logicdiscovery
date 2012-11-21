/*
 * usb.cpp
 *
 *  Created on: 01.07.2011
 *      Author: Flexz
 */


//#include "bsp/bsp.h"
#include "usb.h"
#include "misc.h"
#include "nvic.h"
#include "usbd_core.h"
#include "usbd_desc.h"
//#include "usb_core.h"
//#include "usb_bsp.h"
//#include "usbd_ioreq.h"
//#include "usbd_conf.h"
//#include "usb_conf.h"
#include "usb_dcd_int.h"
#include "usbd_usr.h"

extern "C" void	OTG_FS_IRQHandler(void);
extern "C" void OTG_HS_IRQHandler(void);

__ALIGN_BEGIN USB_OTG_CORE_HANDLE     USB_OTG_dev __ALIGN_END ;

static USB_OTG_CORE_HANDLE  *usbDev = NULL;
extern USBD_Class_cb_TypeDef  USBD_CDC_cb;

void usbInit()
{
#ifdef USB_OTG_FS_CORE
	USBD_Init(&USB_OTG_dev, USB_OTG_FS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);
#elif defined(USB_OTG_HS_CORE)
	USBD_Init(&USB_OTG_dev, USB_OTG_HS_CORE_ID, &USR_desc, &USBD_CDC_cb, &USR_cb);
#endif
	usbDev = &USB_OTG_dev;
}


extern "C" void USB_OTG_BSP_Init(USB_OTG_CORE_HANDLE *pdev)
{
	GPIO_InitTypeDef GPIO_InitStructure;

#ifdef USB_OTG_FS_CORE
	//RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA , ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	/* Configure SOF ID DM DP Pins */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8  |
								GPIO_Pin_11 |
								GPIO_Pin_12;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource8,GPIO_AF_OTG1_FS) ;
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource11,GPIO_AF_OTG1_FS) ;
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource12,GPIO_AF_OTG1_FS) ;

	/* Configure  VBUS Pin */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* Configure ID pin */
	GPIO_InitStructure.GPIO_Pin =  GPIO_Pin_10;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_OTG1_FS) ;


	RCC_AHB2PeriphClockCmd(RCC_AHB2Periph_OTG_FS, ENABLE) ;
#elif defined(USB_OTG_HS_CORE)

	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3, GPIO_AF_OTG2_HS) ; // D0
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource5, GPIO_AF_OTG2_HS) ; // CLK
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource0, GPIO_AF_OTG2_HS) ; // D1
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource1, GPIO_AF_OTG2_HS) ; // D2
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource5, GPIO_AF_OTG2_HS) ; // D7
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource10,GPIO_AF_OTG2_HS) ; // D3
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource11,GPIO_AF_OTG2_HS) ; // D4
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource12,GPIO_AF_OTG2_HS) ; // D5
	GPIO_PinAFConfig(GPIOB,GPIO_PinSource13,GPIO_AF_OTG2_HS) ; // D6
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource3, GPIO_AF_OTG2_HS) ; // NXT
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource2, GPIO_AF_OTG2_HS) ; // DIR
	GPIO_PinAFConfig(GPIOC,GPIO_PinSource0, GPIO_AF_OTG2_HS) ; // STP

	// CLK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	// D0
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3  ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);



	// D1 D2 D3 D4 D5 D6 D7
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1  |
	GPIO_Pin_5 | GPIO_Pin_10 |
	  GPIO_Pin_11| GPIO_Pin_12 |
		GPIO_Pin_13 ;

	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL ;
	GPIO_Init(GPIOB, &GPIO_InitStructure);


	// STP
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0  ;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//NXT
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	//DIR
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_Init(GPIOC, &GPIO_InitStructure);


	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_OTG_HS |
						 RCC_AHB1Periph_OTG_HS_ULPI, ENABLE) ;
#endif

  //Peripheral<STM32_OTG_FS>::Clock(true);
}

#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
extern "C" uint32_t USBD_OTG_EP1IN_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);
extern "C" uint32_t USBD_OTG_EP1OUT_ISR_Handler (USB_OTG_CORE_HANDLE *pdev);

extern "C" void OTG_HS_EP1_IN_IRQHandler(void)
{
  USBD_OTG_EP1IN_ISR_Handler (&USB_OTG_dev);
}

/**
* @brief  This function handles EP1_OUT Handler.
* @param  None
* @retval None
*/
extern "C" void OTG_HS_EP1_OUT_IRQHandler(void)
{
  USBD_OTG_EP1OUT_ISR_Handler (&USB_OTG_dev);
}

#endif

void USB_OTG_BSP_EnableInterrupt(USB_OTG_CORE_HANDLE *pdev)
{

  NVIC_InitTypeDef NVIC_InitStructure;
#ifdef USB_OTG_FS_CORE
  InterruptController::EnableChannel(OTG_FS_IRQn, 1, 0, OTG_HS_IRQHandler);
#elif defined(USB_OTG_HS_CORE)
  InterruptController::EnableChannel(OTG_HS_IRQn, 1, 0, OTG_HS_IRQHandler);
#endif

#ifdef USB_OTG_HS_DEDICATED_EP1_ENABLED
  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_OUT_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
  NVIC_InitStructure.NVIC_IRQChannel = OTG_HS_EP1_IN_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);
#endif
}

extern "C" void OTG_FS_IRQHandler(void)
{
	//GPIOD->ODR = GPIOD->ODR ^ 0xC000;
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

extern "C" void OTG_HS_IRQHandler(void)
{
	//GPIOD->ODR = GPIOD->ODR ^ 0xC000;
  USBD_OTG_ISR_Handler (&USB_OTG_dev);
}

extern "C" volatile int usb_connected;
void	usbWrite(void *buffer_in, int count_in)
{
	//if(usb_connected)
		//DCD_EP_Tx (usbDev, RAW_IN_EP, (uint8_t *)buffer_in, count_in);
	return;
}

/**
* @brief  USB_OTG_BSP_uDelay
*         This function provides delay time in micro sec
* @param  usec : Value of delay required in micro sec
* @retval None
*/
void USB_OTG_BSP_uDelay (const uint32_t usec)
{
  uint32_t count = 0;
  const uint32_t utime = (120 * usec / 7);
  do
  {
    if ( ++count > utime )
    {
      return ;
    }
  }
  while (1);
}


/**
* @brief  USB_OTG_BSP_mDelay
*          This function provides delay time in milli sec
* @param  msec : Value of delay required in milli sec
* @retval None
*/
void USB_OTG_BSP_mDelay (const uint32_t msec)
{
  USB_OTG_BSP_uDelay(msec * 1000);
}
