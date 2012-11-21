/*
 * debug.h
 *
 *  Created on: 25.09.2009
 *      Author: Flexz
 */

#ifndef DEBUG_H_
#define DEBUG_H_

//#include "bsp.h"
#include <stdint.h>

#define DBG_POST_BUFFERS_COUNT	2
#define DBG_POST_BUFFER_SIZE	512
//no messages
#define DBG_LVL_SILENT	0x00

#define DBG_LVL_FATAL	0x04
#define DBG_LVL_ERROR	0x08
#define DBG_LVL_WARNING	0x0C
#define DBG_LVL_NOTICE	0x10
#define DBG_LVL_INFO	0x20
#define DBG_LVL_VERBOSE	0x80
//all messages
#define DBG_LVL_ALL		0xff

enum DebugLevel
{
	dlSilent = 0x00,
	dlFatal = 0x04,
	dlError	= 0x08,
	dlWarning = 0x0C,
	dlNotice = 0x10,
	dlInfo = 0x20,
	dlVerbose  = 0x80,
	dlAll = 0xff
};

class DebugMsg
{
private:
	DebugLevel currentLvl;
public:
	DebugMsg()
	{
		currentLvl = dlWarning;
	}

	void Print(DebugLevel lvl, const char *format, ...);
	void Print(uint8_t *data, int offset, int count, uint8_t flags = 0x01);
	void operator()(DebugLevel lvl, const char *format, ...);
	DebugLevel SetLevel(DebugLevel lvl);
	const char* SetLevel(char *lvl);
	const char* GetLevelStr();
	const char* GetLevelStr(DebugLevel lvl);
};

extern DebugMsg debugMsg;

#ifdef USE_DEBUG_MSG

//Инициализация
void	dbgInit(unsigned char lvl);

//Помещает сообщение в буффер отправки и сразу отдает управление.
//Данную функцию следует использовать только на тех участках кода, где
//скорость выполнения критична и не может затормаживаться медленным выводом в порт
//Буфер отправляется при вызове dbgSendPostBuffer
void	dbgPostMessage(unsigned char lvl, char *msg);
//#define DBGPRINT(lvl, ...)

//Отправляет накопленный посредством dbgPostMessage буффер.
//Возвращает управление только после отправки
void	dbgSendPostBuffer(void);

//Отправляет сообщение сразу.
//Возвращает управление только после отправки
void	dbgSendMessage(unsigned char lvl, char *msg);

void	dbgSetVerboseLevel(unsigned char lvl);

#else

#define dbgInit()
#define dbgPostMessage(A,B)
#define dbgSendPostBuffer()
#define dbgSendMessage(A,B)

#endif

#endif /* DEBUG_H_ */
