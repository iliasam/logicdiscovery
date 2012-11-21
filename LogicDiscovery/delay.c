/*
 * delay.c
 *
 *  Created on: 03.11.2012
 *      Author: user
 */

#include "delay.h"
#include <stdint.h>
#include "stm32f4xx_rcc.h"

volatile uint32_t timeStamp = 0;

void SysTick_Handler(void)
{
	//GPIOD->ODR = GPIOD->ODR ^ 0x1000;
	timeStamp++;
  //TimingDelay_Decrement();
}

void Delay(int ms)
{
	int start = timeStamp;
	while(start + ms > timeStamp)__WFI();
}
