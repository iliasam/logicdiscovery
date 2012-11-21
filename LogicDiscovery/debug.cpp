/*
 * debug.c
 *
 *  Created on: 25.09.2009
 *      Author: Flexz
 */

//#include "bsp/bsp.h"
#include "debug.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>

DebugMsg debugMsg;

//
//int dset_lvl(unsigned char lvl)
//{
//	unsigned char old_lvl = verbose;
//	verbose = lvl;
//	dprintf(DBG_LVL_INFO, "Output level switched to \'%s\' (was \'%s\')\n",
//			get_debug_lvl_string(lvl), get_debug_lvl_string(old_lvl));
//	return old_lvl;
//}
//
//const char* dset_lvl_str(char *lvl)
//{
//	const char* old_lvl = get_debug_lvl_string(verbose);
//	if(lvl == NULL)
//		return old_lvl;
//	for(int i = 0; i < 0x100; i++)
//	{
//		const char *s = get_debug_lvl_string((unsigned char)i);
//		if(s == NULL)
//			continue;
//		if(strcmp(s, lvl) == 0)
//		{
//			dset_lvl(i);
//			return old_lvl;
//		}
//	}
//	dprintf(DBG_LVL_ERROR, "\'%s\' is not a debug level\n", lvl);
//	return old_lvl;
//}

void DebugMsg::operator()(DebugLevel lvl, const char *format, ...)
{
	if(lvl < currentLvl)
	{
		va_list argList;
		va_start(argList, format);

		vprintf(format, argList);

		va_end(argList);
	}
}

void DebugMsg::Print(uint8_t *data, int offset, int count, uint8_t flags)
{
	if(flags & 0x01)
		dprintf(DBG_LVL_VERBOSE, "\nMemory dump (%d):\n", count);
	for(int i = ((offset >> 4) << 4); i < count-offset; i++)
	{
		if(i % 16 == 0)
			dprintf(DBG_LVL_VERBOSE, "\n");

		if(i < offset)
			dprintf(DBG_LVL_VERBOSE, "   ");
		else
			dprintf(DBG_LVL_VERBOSE, "%02x ", data[i - (offset & 0x0f)]);
	}
	if(flags & 0x01)
		dprintf(DBG_LVL_VERBOSE, "\nend of dump.\n");
}

void DebugMsg::Print(DebugLevel lvl, const char *format, ...)
{
	if(lvl < currentLvl)
	{
		va_list argList;
		va_start(argList, format);

		vprintf(format, argList);

		va_end(argList);
	}
}

DebugLevel DebugMsg::SetLevel(DebugLevel lvl)
{
	DebugLevel old_lvl = currentLvl;
	currentLvl = lvl;
	if(lvl > dlSilent)
		dprintf(DBG_LVL_INFO, "Output level switched to \'%s\' (was \'%s\')\n",
				GetLevelStr(lvl), GetLevelStr(old_lvl));
	return old_lvl;
}

const char* DebugMsg::SetLevel(char *lvl)
{
	const char* old_lvl = GetLevelStr(currentLvl);
	if(lvl == NULL)
		return old_lvl;
	for(int i = 0; i < 0x100; i++)
	{
		const char *s = GetLevelStr((DebugLevel)i);
		if(s == NULL)
			continue;
		if(strcmp(s, lvl) == 0)
		{
			SetLevel((DebugLevel)i);
			return old_lvl;
		}
	}
	dprintf(DBG_LVL_ERROR, "\'%s\' is not a debug level\n", lvl);
	return old_lvl;
}

const char* DebugMsg::GetLevelStr(DebugLevel lvl)
{
	switch(lvl)
	{
	case DBG_LVL_ALL:
		return "All";
	case DBG_LVL_FATAL:
		return "Fatal";
	case DBG_LVL_ERROR:
		return "Error";
	case DBG_LVL_WARNING:
		return "Warning";
	case DBG_LVL_NOTICE:
		return "Notice";
	case DBG_LVL_INFO:
		return "Info";
	case DBG_LVL_VERBOSE:
		return "Verbose";
	case DBG_LVL_SILENT:
		return "Silent";
	default: return NULL;
	}
}

#ifdef USE_DEBUG_MSG

typedef struct DbgPostBufferStruct
{
	int count;
	char data[DBG_POST_BUFFER_SIZE];
}DbgPostBufferStruct;

#if DBG_POST_BUFFERS_COUNT == 0
	#error DBG_POST_BUFFERS_COUNT cant be zero
#endif


//static char	postBuffer[DBG_POST_BUFFERS_COUNT][DBG_POST_BUFFER_SIZE];
static DbgPostBufferStruct postBuffer[DBG_POST_BUFFERS_COUNT];
static unsigned char currentBuffer = 0;
static unsigned char verboseLevel = DBG_LVL_ERROR;

void	dbgInit(unsigned char lvl)
{
	for(int i = 0; i < DBG_POST_BUFFERS_COUNT; i++)
	{
		postBuffer[i].data[0] = 0;
		postBuffer[i].count = 0;
	}
	verboseLevel = lvl;
}

void	dbgFlushPostBuffer(void)
{

}

void	dbgSendPostBuffer(void)
{
	unsigned char t = currentBuffer;
	__disable_irq();
	currentBuffer++;
	if(currentBuffer == DBG_POST_BUFFERS_COUNT)
		currentBuffer = 0;
	__enable_irq();
	printf("%s",postBuffer[t].data);
	postBuffer[t].count = 0;
}

unsigned char dbgIsPostBufferEmpty(void)
{
	return postBuffer[currentBuffer].count == 0;
}

void	dbgPostMessage(unsigned char lvl, char *msg)
{
	if(lvl > verboseLevel)
	{
		return;
	}
	int count = strlen(msg);
	if(postBuffer[t].count + count > DBG_POST_BUFFER_SIZE)
	{
		return;
	}
	strncat(postBuffer[t].data, postBuffer[t].count, msg);
	postBuffer[t].count += count;
}

void	dbgSendMessage(unsigned char lvl, char *format, ...)
{
	if(lvl > verboseLevel)
	{
		return;
	}
	int uprintf(const char *format, ...)
	{
		register int *varg = (int *)(&format);
		return print(0, varg);
	}
	//printf("%s",postBuffer[t].data);
}

void	dbgSetVerboseLevel(unsigned char lvl)
{
	verboseLevel = lvl;
}

#endif

