/*
 * nvic.cpp
 *
 *  Created on: 23.12.2009
 *      Author: Flexz
 */

#include "nvic.h"
#include <stdio.h>
#include <string.h>

const int32_t isrCount = 0x100;
//Таблица прерываний должна быть выравнена на 0x100 <BUG>байт</BUG> СЛОВ!!! а не байт
//static InterruptHandler __attribute__((aligned(0x400))) isrTable[isrCount];
#ifdef CFG_LINKER_HAS_ISR_SECTION
//Section ".isr_section" should be located at the start of RAM (or at least aligned to 0x400 for STM32F2XX)
static InterruptHandler __attribute__((section(".isr_section"))) isrTable[isrCount];
#else
static InterruptHandler __attribute__((aligned(0x400)))  isrTable[isrCount];
#endif
//static int32_t DefaultInterruptHandlerHints[isrCount];
//static void* parameterTable[isrCount];

void DefaultHandler()
{
	while(true);
}

void InterruptController::RemapToRam()
{
	//memcpy(isrTable, (void*)0, sizeof(void*)*isrCount);
	for(int i = 0; i < isrCount; i++)
		isrTable[i] = ((InterruptHandler*)0)[i];
	SCB->VTOR = NVIC_VectTab_RAM | ((reinterpret_cast<uint32_t>(isrTable) - 0x20000000));
}

bool InterruptController::SetHandler(IRQn_Type channel, InterruptHandler handler)
{
	//первое STMное прерывание имеет номер 0
	//стандартные кортесовские индексируются в минус (см определение IRQn_Type)
	const uint32_t shift = 16;
	if(channel < isrCount)
	{
		isrTable[channel+shift] = handler;
		return true;
	}
	else
	{
		return false;
	}
}

//static void SetHandler(IRQn_Type channel, InterruptParametericHandler handler, void * parameter)
//{
//	//первое STMное прерывание имеет номер 0
//	//стандартные кортесовские индексируются в минус (см определение IRQn_Type)
//	const uint32_t shift = 16;
//	if(channel < isrCount)
//	{
//		isrTable[channel+shift] = handler;
//		parameterTable[channel+shift] = parameter;
//		return true;
//	}
//	else
//	{
//		printf("isrTable index is out of bounds (%d)\n", channel);
//		return false;
//	}
//}
