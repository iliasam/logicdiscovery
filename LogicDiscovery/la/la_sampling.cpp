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

static void SamplingFrameCompelte();
static void SamplingExternalEventInterrupt();
static void SamplingManualStart();

static InterruptHandler comletionHandler = NULL;
uint8_t _AHBBSS samplingRam[MAX_SAMPLING_RAM];

void Sampler::Setup()
{
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM1EN, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_TIM8EN, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1ENR_DMA2EN, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2ENR_SYSCFGEN, ENABLE);

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

	TIM8->DIER = 0;
	TIM8->SR &= ~TIM_SR_UIF;
	//TIM1_UP -> DMA2, Ch6, Stream5
	//DMA should be stopped before this point
	DMA2_Stream5->CR = (DMA_SxCR_CHSEL_1 | DMA_SxCR_CHSEL_2) | dmaSize | DMA_SxCR_MINC | DMA_SxCR_CIRC;
	//DMA2_Stream5->CR = D<>| dmaSize | DMA_SxCR_MINC | DMA_SxCR_CIRC;
	DMA2_Stream5->M0AR = (uint32_t)samplingRam;
#ifdef SAMPLING_FSMC
	DMA2_Stream5->PAR  = (uint32_t)FSMC_ADDR;
#else
	DMA2_Stream5->PAR  = (uint32_t)&(SAMPLING_PORT->IDR);
#endif
	DMA2_Stream5->NDTR = transferCount;// / transferSize;
	DMA2_Stream5->FCR = DMA_SxFCR_DMDIS | DMA_SxFCR_FTH;

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

	if(triggerMask & 0x0001)InterruptController::EnableChannel(EXTI0_IRQn, 0, 0, SamplingExternalEventInterrupt);
	else InterruptController::DisableChannel(EXTI0_IRQn);
	if(triggerMask & 0x0002)InterruptController::EnableChannel(EXTI1_IRQn, 0, 0, SamplingExternalEventInterrupt);
	else InterruptController::DisableChannel(EXTI1_IRQn);
	if(triggerMask & 0x0004)InterruptController::EnableChannel(EXTI2_IRQn, 0, 0, SamplingExternalEventInterrupt);
	else InterruptController::DisableChannel(EXTI2_IRQn);
	if(triggerMask & 0x0008)InterruptController::EnableChannel(EXTI3_IRQn, 0, 0, SamplingExternalEventInterrupt);
	else InterruptController::DisableChannel(EXTI3_IRQn);
	if(triggerMask & 0x0010)InterruptController::EnableChannel(EXTI4_IRQn, 0, 0, SamplingExternalEventInterrupt);
	else InterruptController::DisableChannel(EXTI4_IRQn);
	if(triggerMask & 0x03E0)InterruptController::EnableChannel(EXTI9_5_IRQn, 0, 0, SamplingExternalEventInterrupt);
	else InterruptController::DisableChannel(EXTI9_5_IRQn);
	if(triggerMask & 0xFC00)InterruptController::EnableChannel(EXTI15_10_IRQn, 0, 0, SamplingExternalEventInterrupt);
	else InterruptController::DisableChannel(EXTI15_10_IRQn);

#ifdef SAMPLING_MANUAL
	TIM8->SMCR = TIM_SMCR_TS_0 | TIM_SMCR_TS_1 | TIM_SMCR_TS_2;//External trigger input
	TIM8->SMCR |= TIM_SMCR_SMS_1 | TIM_SMCR_SMS_2;
	TIM8->DIER |= TIM_DIER_TIE;
	InterruptController::EnableChannel(TIM8_TRG_COM_TIM14_IRQn, 2, 0, SamplingManualStart);
#endif
}

void Sampler::Start()
{
	Setup();
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
	return samplingRam + ActualTransferCount() * transferSize;
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
	return samplingRam;
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

static void SamplingManualStart()
{
	TIM8->SR &= ~TIM_SR_TIF;
	TIM8->DIER &= ~TIM_DIER_TIE;
	//call regular handler
	SamplingExternalEventInterrupt();
}
