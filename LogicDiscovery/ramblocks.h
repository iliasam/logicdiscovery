/*
 * ramblocks.h
 *
 *  Created on: 12.10.2012
 *      Author: Flexz
 */

#ifndef RAMBLOCKS_H_
#define RAMBLOCKS_H_


#ifdef STM32F4XX
//by default data goes to CCM RAM (see stm32f4xx_flash.ld), which is not accessible from system bus
//initialized data in 112k SRAM
#define _AHBRAM  __attribute__((section(".ahbram_data")))
#define _AHBRAM1 __attribute__((section(".ahbram_data")))
//zero initialized data in 112k SRAM
#define _AHBBSS  __attribute__((section(".ahbram_bss")))
#define _AHBBSS1 __attribute__((section(".ahbram_bss")))
//non-initialized data in 16k SRAM
#define _AHBRAM2 __attribute__((section(".ahbram2")))
#else
//If CPU has no split memory architecture
#define _AHBRAM
#define _AHBRAM1
#define _AHBBSS
#define _AHBBSS1
#define _AHBRAM2
#endif

#endif /* RAMBLOCKS_H_ */
