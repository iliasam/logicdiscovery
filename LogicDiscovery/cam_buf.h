/*
 * cam_buf.h
 *
 *  Created on: 03.11.2012
 *      Author: user
 */

#ifndef CAM_BUF_H_
#define CAM_BUF_H_

#include "ramblocks.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

//const uint32_t camBufferSize = 320*4;
#define CAM_BUFFER_SIZE (320*20)
#define CAM_TRANSFER_SIZE (CAM_BUFFER_SIZE + sizeof(uint32_t))
typedef struct camBuffer_
{
	uint32_t line;
	uint8_t buffer[CAM_BUFFER_SIZE];
}__attribute__((packed)) camBuffer;

extern volatile camBuffer _AHBBSS camBufferPri;
extern volatile camBuffer _AHBBSS camBufferSec;

extern volatile camBuffer _AHBBSS camBufferPriT;
extern volatile camBuffer _AHBBSS camBufferSecT;

extern volatile int camCurrentBuffer;//0 - primary
extern volatile int camPrimaryBufferOut;
extern volatile int camSecondaryBufferOut;

#ifdef __cplusplus
}
#endif

#endif /* CAM_BUF_H_ */
