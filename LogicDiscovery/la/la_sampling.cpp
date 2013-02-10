/*
 * la_sampling.cpp
 *
 *  Created on: 11.11.2012
 *      Author: user
 */

#include "la_sampling.h"
#include <stm32f4xx.h>
#include "nvic.h"
#include <stdlib.h>
#include "sump.h"

Sampler sampler;

static uint32_t transferCount;
static uint32_t delayCount;

static void SamplingFrameCompelte();
static void SamplingExternalEventInterrupt();
static void SamplingRLEExternalEventInterrupt();
static void SamplingManualStart();

template <class samplesType, uint32_t FLAG, uint32_t MAX_COUNT>
static void SamplingRLEFrameInterrupt() __attribute__ ( ( isr ) );//__attribute__ ( ( isr ) ) ;

template <class samplesType, uint32_t FLAG, uint32_t MAX_COUNT>
static void SamplingRLETailFrameInterrupt() __attribute__ ( ( isr ) );// __attribute__ ( ( isr, naked ) );

static InterruptHandler samplingManualToExternalTransit = NULL;
static InterruptHandler samplingRLETailFrameInterrupt = NULL;
static InterruptHandler comletionHandler = NULL;
uint32_t _AHBBSS samplingRam[MAX_SAMPLING_RAM/4];
uint32_t _AHBBSS rleTempSamplingRamA[MAX_RLE_SAMPLE_COUNT];
uint32_t _AHBBSS rleTempSamplingRamB[MAX_RLE_SAMPLE_COUNT];

static uint32_t rlePtr;
static uint32_t rleValue;
static uint32_t rleRepeatCount;
static int rleDelayCount;
static bool rleTailSampling;

#define RLE_16BIT_FLAG 0x8000
#define RLE_8BIT_FLAG 0x80

#define RLE_16BIT_MAX_COUNT 0x7fff
#define RLE_8BIT_MAX_COUNT 0x7f

void Sampler::SetBufferSize(uint32_t value)
{
	transferCount = value;
}
void Sampler::SetDelayCount(uint32_t value)
{
	delayCount = value & 0xfffffffe;
	rleDelayCount = value & 0xfffffffe;
}

void Sampler::SetupSamplingTimer()
{
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM1EN, ENABLE);
	//Main sampling timer
	TIM1->DIER = 0;
	TIM1->SR &= ~TIM_SR_UIF;
	TIM1->CNT = 0;
	TIM1->PSC = 0;
	TIM1->CR1 = TIM_CR1_URS;
	TIM1->ARR = period;//actual period is +1 of this value
	TIM1->CR2 = 0;
	TIM1->DIER = TIM_DIER_UDE;
	TIM1->EGR = TIM_EGR_UG;
}

uint32_t Sampler::CalcDMATransferSize()
{
	uint32_t dmaSize = 0;
	//handle 8/16/32 bit samplings
	switch(flags & SUMP_FLAG1_GROUPS)
	{
	case SUMP_FLAG1_GR_16BIT:
		transferSize = 2;
		dmaSize = DMA_SxCR_MSIZE_0 | DMA_SxCR_PSIZE_0;
		break;
	case SUMP_FLAG1_GR_32BIT:
		transferSize = 4;
		dmaSize = DMA_SxCR_MSIZE_1 | DMA_SxCR_PSIZE_1;
		break;
	case SUMP_FLAG1_GR_8BIT:
	default:
		dmaSize = 0;
		transferSize = 1;
		break;
	}
	return dmaSize;
}

void Sampler::SetupSamplingDMA(void *dataBuffer, uint32_t dataTransferCount)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA2EN, ENABLE);
	uint32_t dmaSize = CalcDMATransferSize();

	//TIM8->DIER = 0;
	//TIM8->SR &= ~TIM_SR_UIF;

	//TIM1_UP -> DMA2, Ch6, Stream5
	//DMA should be stopped before this point
	DMA2_Stream5->CR = (DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2) | dmaSize | DMA_SxCR_MINC | DMA_SxCR_CIRC;
	//DMA2_Stream5->CR = D<>| dmaSize | DMA_SxCR_MINC | DMA_SxCR_CIRC;
	DMA2_Stream5->M0AR = (uint32_t)dataBuffer;//samplingRam;
#ifdef SAMPLING_FSMC
	DMA2_Stream5->PAR  = (uint32_t)FSMC_ADDR;
#else
	DMA2_Stream5->PAR  = (uint32_t)&(SAMPLING_PORT->IDR);
#endif
	DMA2_Stream5->NDTR = dataTransferCount;//transferCount;// / transferSize;
	DMA2_Stream5->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH;
}

void Sampler::SetupRLESamplingDMA(void *dataBufferA, void *dataBufferB, uint32_t dataTransferCount)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA2EN, ENABLE);
	uint32_t dmaSize = CalcDMATransferSize();

	//TIM1_UP -> DMA2, Ch6, Stream5
	//DMA should be stopped before this point
	DMA2_Stream5->CR = (DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2) | dmaSize | DMA_SxCR_MINC | DMA_SxCR_DBM | DMA_SxCR_TCIE;
	//DMA2_Stream5->CR = D<>| dmaSize | DMA_SxCR_MINC | DMA_SxCR_CIRC;
	DMA2_Stream5->M0AR = (uint32_t)dataBufferA;
	DMA2_Stream5->M1AR = (uint32_t)dataBufferB;
#ifdef SAMPLING_FSMC
	DMA2_Stream5->PAR  = (uint32_t)FSMC_ADDR;
#else
	DMA2_Stream5->PAR  = (uint32_t)&(SAMPLING_PORT->IDR);
#endif
	DMA2_Stream5->NDTR = dataTransferCount;//transferCount;// / transferSize;
	DMA2_Stream5->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH;
	DMA2->HIFCR = DMA_HIFCR_CTCIF5;

	switch(transferSize)
	{
	case 1:
		InterruptController::EnableChannel(DMA2_Stream5_IRQn, 0, 0,
			SamplingRLEFrameInterrupt<uint8_t, RLE_8BIT_FLAG, RLE_8BIT_MAX_COUNT>);
		samplingRLETailFrameInterrupt = SamplingRLETailFrameInterrupt<uint8_t, RLE_8BIT_FLAG, RLE_8BIT_MAX_COUNT>;
		break;
	default:
	case 2:
		InterruptController::EnableChannel(DMA2_Stream5_IRQn, 0, 0,
				SamplingRLEFrameInterrupt<uint16_t, RLE_16BIT_FLAG, RLE_16BIT_MAX_COUNT>);
		samplingRLETailFrameInterrupt = SamplingRLETailFrameInterrupt<uint16_t, RLE_16BIT_FLAG, RLE_16BIT_MAX_COUNT>;
		break;
	}

}

void Sampler::SetupRegularEXTITrigger(InterruptHandler interruptHandler)
{
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFGEN, ENABLE);

	//Trigger setup
	uint32_t rising = triggerMask & triggerValue;
	uint32_t falling = triggerMask & ~triggerValue;
	//route exti to triggerMask GPIO port
	uint32_t extiCR = 0;
	switch((uint32_t)SAMPLING_PORT)
	{
	case GPIOA_BASE:extiCR = 0x0000;break;
	case GPIOB_BASE:extiCR = 0x1111;break;
	case GPIOC_BASE:extiCR = 0x2222;break;
	case GPIOD_BASE:extiCR = 0x3333;break;
	case GPIOE_BASE:extiCR = 0x4444;break;
	case GPIOF_BASE:extiCR = 0x5555;break;
	case GPIOG_BASE:extiCR = 0x6666;break;
	case GPIOH_BASE:extiCR = 0x7777;break;
	case GPIOI_BASE:extiCR = 0x8888;break;
	}
	SYSCFG->EXTICR[0] = extiCR;
	SYSCFG->EXTICR[1] = extiCR;
	SYSCFG->EXTICR[2] = extiCR;
	SYSCFG->EXTICR[3] = extiCR;

	EXTI->IMR  = 0;//mask;
	EXTI->PR = 0xffffffff;
	EXTI->RTSR = rising;
	EXTI->FTSR = falling;

	__DSB();

	if(triggerMask & 0x0001)InterruptController::EnableChannel(EXTI0_IRQn, 0, 0, interruptHandler);
	else InterruptController::DisableChannel(EXTI0_IRQn);
	if(triggerMask & 0x0002)InterruptController::EnableChannel(EXTI1_IRQn, 0, 0, interruptHandler);
	else InterruptController::DisableChannel(EXTI1_IRQn);
	if(triggerMask & 0x0004)InterruptController::EnableChannel(EXTI2_IRQn, 0, 0, interruptHandler);
	else InterruptController::DisableChannel(EXTI2_IRQn);
	if(triggerMask & 0x0008)InterruptController::EnableChannel(EXTI3_IRQn, 0, 0, interruptHandler);
	else InterruptController::DisableChannel(EXTI3_IRQn);
	if(triggerMask & 0x0010)InterruptController::EnableChannel(EXTI4_IRQn, 0, 0, interruptHandler);
	else InterruptController::DisableChannel(EXTI4_IRQn);
	if(triggerMask & 0x03E0)InterruptController::EnableChannel(EXTI9_5_IRQn, 0, 0, interruptHandler);
	else InterruptController::DisableChannel(EXTI9_5_IRQn);
	if(triggerMask & 0xFC00)InterruptController::EnableChannel(EXTI15_10_IRQn, 0, 0, interruptHandler);
	else InterruptController::DisableChannel(EXTI15_10_IRQn);

#ifdef SAMPLING_MANUAL //push-button-trigger
	TIM8->SMCR = TIM_SMCR_TS_0 | TIM_SMCR_TS_1 | TIM_SMCR_TS_2;//External trigger input
	TIM8->SMCR |= TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2;
	TIM8->DIER |= TIM_DIER_TIE;
	InterruptController::EnableChannel(TIM8_TRG_COM_TIM14_IRQn, 2, 0, SamplingManualStart);
	samplingManualToExternalTransit = interruptHandler;
#endif
}

void Sampler::SetupDelayTimer()
{
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM8EN, ENABLE);
	//After-trigger delay timer
	TIM8->CR1 = TIM_CR1_URS;//stop timer too
	TIM8->CNT = 0;
	TIM8->ARR = delayCount;//  / transferSize;
	TIM8->PSC = TIM1->ARR;
	TIM8->CR2 = 0;
	TIM8->EGR = TIM_EGR_UG;
	TIM8->SR &= ~TIM_SR_UIF;
	TIM8->DIER = TIM_DIER_UIE;

	InterruptController::EnableChannel(TIM8_UP_TIM13_IRQn, 2, 0, SamplingFrameCompelte);
}

void Sampler::SetupRegular()
{
	//Sampling scheme:
	//TIM1 overflows with sampling period. Overflow triggers DMA read from sampling port.
	//DMA works in circular mode (so no bursts possible) continuously writing data from GPIO port to
	//sampling ram buffer
	//Once trigger interrupt shoots TIM8 comes in play counting "sampling period" * "delay count" ticks.
	//Once TIM8 overflows and sampled data are sent to host.
	SetupSamplingTimer();
	SetupSamplingDMA(samplingRam, transferCount);
	SetupDelayTimer();
	SetupRegularEXTITrigger(SamplingExternalEventInterrupt);
}

void Sampler::SetupRLE()
{
	//Sampling scheme:
	//Same as regular mode TIM1 provides reference clock for taking samples, but data are being stored in
	//the temporary buffer. Once buffer is full (DMA still works in circular mode) compression functions
	//scans it, writing to final sampling buffer pairs:
	//	1. Value, with zeroed MSB
	//	2. Repeat count for this value, MSB set high

	for(int i = 0; i < MAX_RLE_SAMPLE_COUNT; i++)
	{
		rleTempSamplingRamA[i] = rleTempSamplingRamB[i] = 0;
	}
	rlePtr = 0;
	rleValue = 0;
	rleRepeatCount = 0;
	rleDelayCount = delayCount;
	samplingRam[0] = rleValue;
	rleTailSampling = false;

	SetupSamplingTimer();
	SetupRLESamplingDMA(rleTempSamplingRamA, rleTempSamplingRamB, MAX_RLE_SAMPLE_COUNT);
	SetupRegularEXTITrigger(SamplingRLEExternalEventInterrupt);
}

void Sampler::Start()
{
	if(flags & SUMP_FLAG1_ENABLE_RLE)
	{
		SetupRLE();
		//SetupRegular();
	}
	else
	{
		SetupRegular();
	}
	DMA2->HIFCR = DMA_HIFCR_CTCIF5;
	DMA2_Stream5->CR |= DMA_SxCR_EN;
	TIM1->CR1 |= TIM_CR1_CEN;//enable timer
}

void Sampler::Stop()
{
	DMA2_Stream5->CR &= ~(DMA_SxCR_TCIE | DMA_SxCR_EN);//stop dma
	TIM1->CR1 &= ~TIM_CR1_CEN;//stop sampling timer
}

void Sampler::Arm(InterruptHandler handler)
{
	//SamplingSetCondition(samplingTriggerMask, samplingTriggerValue);
	EXTI->PR = 0xffffffff;//clear pending
	__DSB();
	EXTI->IMR = triggerMask;

	comletionHandler = handler;
}

uint32_t Sampler::ActualTransferCount()
{
	return transferCount - (DMA2_Stream5->NDTR & ~3);
}

uint8_t* Sampler::GetBufferTail()
{
	return (uint8_t*)(samplingRam) + ActualTransferCount() * transferSize;
}

uint32_t Sampler::GetBufferTailSize()
{
	return ActualTransferCount() * transferSize;
}

uint32_t Sampler::GetBufferSize()
{
	return transferCount * transferSize;
}

uint8_t* Sampler::GetBuffer()
{
	return (uint8_t*)samplingRam;
}

//START-----------NDTR---------------------END//
//        NEW               OLD
//
//uint8_t* SamplingGetBuffer(SamplingBufferPart part)
//{
//	switch(part)
//	{
//	case sbpOld:
//		return samplingRam + GetTransferCount();
//	case sbpNew:
//	case sbpTotal:
//		return samplingRam;
//	}
//	//return samplingRam;
//}
//
//uint32_t SamplingGetBufferSize(SamplingBufferPart part)
//{
//	switch(part)
//	{
//	case sbpOld:
//		return samplingBufferSize - GetTransferCount();
//	case sbpNew:
//		return GetTransferCount();
//	case sbpTotal:
//		return samplingBufferSize;
//	}
//}

void SamplingClearBuffer()
{
	for(int i = 0; i < MAX_SAMPLING_RAM; i++)
		samplingRam[i] = 0;
}

static void SamplingFrameCompelte()
{
	TIM1->CR1 &= ~TIM_CR1_CEN;
	TIM8->CR1 &= ~TIM_CR1_CEN;
	TIM8->SR  &= ~TIM_SR_UIF;
	DMA2_Stream5->CR &= ~(DMA_SxCR_TCIE | DMA_SxCR_EN);
	if(comletionHandler != NULL)
		comletionHandler();
}

//uint32_t la_debug[2];
static void SamplingExternalEventInterrupt()
{
	EXTI->PR = 0xffffffff;
	__DSB();
	EXTI->IMR = 0;
	TIM8->CNT = 0;
	TIM8->SR  &= ~TIM_SR_UIF;
	TIM8->CR1 |= TIM_CR1_CEN;
	//la_debug[0] = DMA2_Stream5->NDTR;
	//la_debug[1]++;
}

static void SamplingRLEExternalEventInterrupt()
{
	EXTI->PR = 0xffffffff;
	__DSB();
	EXTI->IMR = 0;
	rleDelayCount = delayCount;
	rleTailSampling = true;
	InterruptController::SetHandler(DMA2_Stream5_IRQn, samplingRLETailFrameInterrupt);
}

static void SamplingManualStart()
{
	TIM8->SR &= ~TIM_SR_TIF;
	TIM8->DIER &= ~TIM_DIER_TIE;
	//call regular handler
	//SamplingExternalEventInterrupt();
	samplingManualToExternalTransit();
}

//#define store ((samplesType*)samplingRam)
//template <class samplesType = uint8_t, int FLAG = RLE_8BIT_FLAG, int MAX_COUNT = RLE_8BIT_MAX_COUNT>
template <class samplesType, uint32_t FLAG, uint32_t MAX_COUNT>
static void SamplingRLEFrameInterrupt()
{
	DMA2->HIFCR = DMA_HIFCR_CTCIF5;

	GPIOD->PUPDR = 0x0004;

	static samplesType * store = (samplesType*)samplingRam;
	samplesType * samples = (samplesType*)((DMA2_Stream5->CR & DMA_SxCR_CT) ? rleTempSamplingRamA : rleTempSamplingRamB);
	//uint16_t * store = (uint16_t*)samplingRam;
	//uint16_t * samples = (uint16_t*)((DMA2_Stream5->CR & DMA_SxCR_CT) ? rleTempSamplingRamA : rleTempSamplingRamB);
	//int start = 0;
	int n = MAX_RLE_SAMPLE_COUNT;
	uint32_t newValue;

	do
	{
		newValue =  *samples++;// & MAX_COUNT;

		if(rleValue == newValue)
		{
			rleRepeatCount++;
			if(MAX_COUNT == rleRepeatCount)//repeat count overflow
			{
				store[rlePtr++] = rleValue;
				store[rlePtr++] = rleRepeatCount | FLAG;
				rleRepeatCount = 0;
				if(rlePtr >= transferCount)
				{
					rlePtr = 0;
				}
			}
		}
		else//change detected
		{
			store[rlePtr++] = rleValue;
			//if(rleRepeatCount != 0)//non zero repeat count
			{
				store[rlePtr++] = rleRepeatCount | FLAG;
			}
			if(rlePtr >= transferCount)
			{
				rlePtr = 0;
			}

			rleRepeatCount = 0;
			rleValue = newValue;
		}
	}
	while(--n);

	GPIOD->PUPDR = 0x0008;
}

template <class samplesType, uint32_t FLAG, uint32_t MAX_COUNT>
static void SamplingRLETailFrameInterrupt()
{
	DMA2->HIFCR = DMA_HIFCR_CTCIF5;

	GPIOD->PUPDR = 0x0004;

	static samplesType * store = (samplesType*)samplingRam;
	samplesType * samples = (samplesType*)((DMA2_Stream5->CR & DMA_SxCR_CT) ? rleTempSamplingRamA : rleTempSamplingRamB);
	//uint16_t * store = (uint16_t*)samplingRam;
	//uint16_t * samples = (uint16_t*)((DMA2_Stream5->CR & DMA_SxCR_CT) ? rleTempSamplingRamA : rleTempSamplingRamB);
	int start = 0;
	uint32_t newValue;

	do
	{
		newValue =  samples[start++];// & MAX_COUNT;

		if(rleValue == newValue)
		{
			rleRepeatCount++;
			if(MAX_COUNT == rleRepeatCount)//repeat count overflow
			{
				store[rlePtr++] = rleValue;
				store[rlePtr++] = rleRepeatCount | FLAG;
				rleRepeatCount = 0;

				rleDelayCount -= 2;
				if(rleDelayCount <= 0)
				{
					SamplingFrameCompelte();
					return;
				}

//				if(rlePtr >= transferCount)
//				{
//					rlePtr = 0;
//				}
			}
		}
		else//change detected
		{
			{
				store[rlePtr++] = rleValue;
				store[rlePtr++] = rleRepeatCount | FLAG;
				rleDelayCount-=2;

//				if(rleRepeatCount != 0)//non zero repeat count
//				{
//					rleDelayCount-=2;
//					store[rlePtr++] = rleRepeatCount | FLAG;
//				}
//				else
//				{
//					rleDelayCount--;
//				}

				if(rleDelayCount <= 0)
				{
					SamplingFrameCompelte();
					return;
				}
			}

//			if(rlePtr >= transferCount)
//			{
//				rlePtr = 0;
//			}

			rleRepeatCount = 0;
			rleValue = newValue;
		}
	}
	while(start != MAX_RLE_SAMPLE_COUNT);

	if(rlePtr >= transferCount)
	{
		rlePtr = 0;
	}

	GPIOD->PUPDR = 0x0008;
}
