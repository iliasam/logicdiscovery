/*
 * la_sampling.h
 *
 *  Created on: 11.11.2012
 *      Author: user
 */

#ifndef LA_SAMPLING_H_
#define LA_SAMPLING_H_

#include "nvic.h"

#define MAX_SAMPLING_RAM (24*1024)
//#define SAMPLING_FSMC
#define FSMC_ADDR (0x60000000)
//Port A: used for USB_FS, non-usable
//Port B: B2 is BOOT1 pin with 10k pull-up
//Port C: C0 nEnable of STMPS2141STR. C3 output form mic.
//Port D: D5 has pull-up and led, connected nFault of STMPS2141STR. D12-15 connected to color LEDs <- looks safe
//Port E: E0,E1 - active low from LIS302DL, non-usable
#define SAMPLING_PORT GPIOD
//enable manual trigger by user-button
#define SAMPLING_MANUAL

class Sampler
{
private:
	uint32_t transferCount;
	uint32_t delayCount;
	uint32_t triggerMask;
	uint32_t triggerValue;
	uint16_t flags;
	uint16_t period;

	int transferSize;

	void Setup();
	uint32_t ActualTransferCount();
public:
	void SetBufferSize(uint32_t value){transferCount = value;}
	void SetDelayCount(uint32_t value){delayCount = value;}
	void SetTriggerMask(uint32_t value){triggerMask = value;}
	void SetTriggerValue(uint32_t value){triggerValue = value;}
	void SetFlags(uint32_t value){flags = value;}
	void SetSamplingPeriod(uint32_t value){period = value;}

	void Start();
	void Stop();
	void Arm(InterruptHandler handler);

	int GetBytesPerTransfer(){return transferSize;}
	//cyclic buffer handling
	uint8_t* GetBufferTail();
	uint32_t GetBufferTailSize();
	uint32_t GetBufferSize();
	uint8_t* GetBuffer();
};

extern Sampler sampler;

void SamplingSetupTimer(uint32_t period);
void SamplingSetupBuffer(uint32_t bufferSize, uint32_t delayCount);
//start streaming capture, trigger not armed
void SamplingStart();
void SamplingStop();
void SamplingSetCondition(uint32_t mask, uint32_t value);
void SamplingSetFlags(uint16_t flags);
void SamplingSetTriggerMask(uint32_t mask);
void SamplingSetTriggerValue(uint32_t value);
//arm trigger
void SamplingTriggerEnable(InterruptHandler handler);

enum SamplingBufferPart
{
	sbpOld,
	sbpNew,
	sbpTotal
};

uint8_t* SamplingGetBuffer(SamplingBufferPart part);
uint32_t SamplingGetBufferSize(SamplingBufferPart part);

extern uint32_t la_debug[2];

void SamplingClearBuffer();


#endif /* LA_SAMPLING_H_ */
