//#define USE_STDPERIPH_DRIVER
//#include "stm32f4xx.h"
#include <stdint.h>
#include "nvic.h"
#include "usb.h"
#include "stm32f4xx.h"
#include "misc.h"
#include "ramblocks.h"

#include "delay.h"

#include "sump.h"
#include "la_sampling.h"
//#include "stm32f2xx_dma.h"

USART_TypeDef *uart;

void UartCommandInterruptHandler(void);

static void Init();

void Init()
{
	GPIO_InitTypeDef gpioInit;
	InterruptController::RemapToRam();
	InterruptController::PriorityGroupConfig(NVIC_PriorityGroup_4);

	SysTick_Config(120000000 / 1000);

	RCC_AHB1PeriphClockCmd( RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB |
							 RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD |
							 RCC_AHB1Periph_GPIOE | RCC_AHB1Periph_GPIOF, ENABLE);
	RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

	// FSMC GPIO configuration
#ifdef SAMPLING_FSMC
	/* Set
	 * PD.14(D0),  0 - 14 *
	 * PD.15(D1),  1 - 15 *
	 * PD.00(D2),  2 - 00
	 * PD.01(D3),  3 - 01
	 * PE.07(D4),  4 - 07
	 * PE.08(D5),  5 - 08 *
	 * PE.09(D6),  6 - 09 *
	 * PE.10(D7),  7 - 10 *
	 * PE.11(D8),  8 - 11
	 * PE.12(D9),  9 - 12
	 * PE.13(D10),10 - 13
	 * PE.14(D11),11 - 14 *
	 * PE.15(D12),12 - 15 *
	 * PD.08(D13),13 - 08 *
	 * PD.09(D14),14 - 09 *
	 * PD.10(D15),15 - 10 *
	 * as alternate function push pull */
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
	//GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

	gpioInit.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 |
								 GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
								 GPIO_Pin_14 | GPIO_Pin_15;
	gpioInit.GPIO_Mode = GPIO_Mode_AF;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd  = GPIO_PuPd_DOWN;

	GPIO_Init(GPIOD, &gpioInit);

	/* Set PE.07(D4), PE.08(D5), PE.09(D6), PE.10(D7), PE.11(D8), PE.12(D9), PE.13(D10),
	 PE.14(D11), PE.15(D12) as alternate function push pull */
	/* GPIOE configuration */
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource2 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
	GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);

	gpioInit.GPIO_Pin =GPIO_Pin_2 | GPIO_Pin_7 | GPIO_Pin_8  | GPIO_Pin_9  | GPIO_Pin_10 |
								 GPIO_Pin_11| GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;

	GPIO_Init(GPIOE, &gpioInit);

	FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
	FSMC_NORSRAMTimingInitTypeDef FSMC_NORSRAMTimingInitStructure;

	FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
	FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
	FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
	FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
	FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
	FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
	FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
	FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
	FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
	FSMC_NORSRAMInitStructure.FSMC_WriteBurst = FSMC_WriteBurst_Disable;
	FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;

	/* FSMC写速度设置 */
	FSMC_NORSRAMTimingInitStructure.FSMC_AddressSetupTime = 1;//1;   /* 地址建立时间  */
	FSMC_NORSRAMTimingInitStructure.FSMC_AddressHoldTime = 0;
	FSMC_NORSRAMTimingInitStructure.FSMC_DataSetupTime = 1;//1;	   /* 数据建立时间  */
	FSMC_NORSRAMTimingInitStructure.FSMC_BusTurnAroundDuration = 0;
	FSMC_NORSRAMTimingInitStructure.FSMC_CLKDivision = 0;
	FSMC_NORSRAMTimingInitStructure.FSMC_DataLatency = 0;
	FSMC_NORSRAMTimingInitStructure.FSMC_AccessMode = FSMC_AccessMode_A;	/* FSMC 访问模式 */
	FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct = &FSMC_NORSRAMTimingInitStructure;

	FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

	/* Enable FSMC Bank4_SRAM Bank */
	FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
#else
	gpioInit.GPIO_Pin = 0xffff;
	gpioInit.GPIO_Mode = GPIO_Mode_IN;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd = GPIO_PuPd_DOWN;
	gpioInit.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(SAMPLING_PORT, &gpioInit);
#endif

#ifdef SAMPLING_MANUAL
	gpioInit.GPIO_Pin = 0x0001;
	gpioInit.GPIO_Mode = GPIO_Mode_AF;
	gpioInit.GPIO_OType = GPIO_OType_PP;
	gpioInit.GPIO_PuPd = GPIO_PuPd_DOWN;
	gpioInit.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &gpioInit);

	GPIO_PinAFConfig(GPIOA, GPIO_PinSource0 , GPIO_AF_TIM8);
#endif


	USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP ;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2ENR_USART6EN, ENABLE);
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART6, &USART_InitStructure);

	uart = USART6;

	//USART_Cmd(USART6, ENABLE);
	//InterruptController::EnableChannel(USART6_IRQn, 2, 0, UartCommandInterruptHandler);
}

extern "C" int uart_getc(void){return 0;}
extern "C" int uart_putc(int c){return 0;}

extern "C" void HardFault_Handler()
{
	//GUI_Text(0, 70, (uint8_t*)" HardFault", White, Red);
}

extern "C" void MemManage_Handler()
{
	//GUI_Text(0, 70, (uint8_t*)" MemManage", White, Red);
}

extern "C" void BusFault_Handler()
{
	//GUI_Text(0, 70, (uint8_t*)" BusFault", White, Red);
}

extern "C" void NMI_Handler()
{
	//GUI_Text(0, 70, (uint8_t*)" NMI", White, Red);
}

extern "C" void UsageFault_Handler()
{
	//GUI_Text(0, 70, (uint8_t*)" UsageFault", White, Red);
}

//USB transfer function wrappers
extern "C" uint16_t VCP_ByteTx (uint8_t dataByte);
void USBTXByte(uint8_t data)
{
	VCP_ByteTx(data);
}

void USBTXBuffer(uint8_t *data, int count)
{
	for(int i = 0; i < count; i++)
	{
		VCP_ByteTx(data[i]);
	}
}

//UART transfer functions
void UARTTXByte(uint8_t data)
{
	while(0 == (uart->SR & USART_SR_TXE));
	uart->DR = data;
}

void UARTTXBuffer(uint8_t *data, int count)
{
	for(int i = 0; i < count; i++)
	{
		UARTTXByte(data[i]);
	}
}

#define UART_RX_BUFFER_SIZE 100
static uint8_t uartRXBuffer[UART_RX_BUFFER_SIZE];
static int uartRXCount = 0;

void UartCommandInterruptHandler(void)
{
	uint16_t sr = USART6->SR;
	if((sr & USART_SR_RXNE) && (USART6->CR1 & USART_CR1_RXNEIE))
	{
		uartRXBuffer[uartRXCount++] = USART_ReceiveData(USART6);
		if(uartRXCount == UART_RX_BUFFER_SIZE)uartRXCount = 0;
	}
	if((sr & USART_SR_IDLE) && (USART6->CR1 & USART_CR1_IDLEIE))
	{
		//IDLE status is cleared by a software sequence
		//(an read to the USART_SR register followed by a read to the USART_DR register)
		USART6->SR;
		USART6->DR;
		SumpProcessRequest(uartRXBuffer, uartRXCount);
		uartRXCount = 0;
	}
}

int main(void)
{
	const int max = 1000000;
	Init();
//	DCMI_Test();
	GPIOD->ODR = 0;

	//LCD_Initializtion();
	//LCD_Clear(Black);

	Delay(100);

	//SumpSetTXFunctions(UARTTXByte, UARTTXBuffer);//operating over UART

	SumpSetTXFunctions(USBTXByte, USBTXBuffer);//operating over USB
	usbInit();


	for(;;)
	{
		GPIOD->ODR |= GPIO_ODR_ODR_12;
		Delay(100);
		GPIOD->ODR |= GPIO_ODR_ODR_13;
		Delay(100);
		GPIOD->ODR |= GPIO_ODR_ODR_14;
		Delay(100);
		GPIOD->ODR |= GPIO_ODR_ODR_15;
		Delay(100);
		GPIOD->ODR &= ~GPIO_ODR_ODR_12;
		Delay(100);
		GPIOD->ODR &= ~GPIO_ODR_ODR_13;
		Delay(100);
		GPIOD->ODR &= ~GPIO_ODR_ODR_14;
		Delay(100);
		GPIOD->ODR &= ~GPIO_ODR_ODR_15;
		Delay(100);
	}

	return 0;
}
