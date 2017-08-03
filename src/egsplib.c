#include "egsplib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define EGSP_NUMERIC_BUFFER_LENGTH 256

static size_t EGSP_BLOCK_SIZE = 4096;
static size_t ALIGN_BYTES = 2;


// Utility
size_t EgspPad(size_t bytes)
{
	return bytes + (ALIGN_BYTES - (bytes % ALIGN_BYTES)) % ALIGN_BYTES;
}

void* EgspAlloc(EgspLoader* pLoader, size_t size)
{
	size_t padded = EgspPad(size);
	if (padded > pLoader->heapSize)
	{
		assert(0 && "Buffer overflow");
		return 0;
	}

	pLoader->heapSize -= padded;
	return (uint8_t*)pLoader->pHeap + pLoader->heapSize;
}

EgspResult EgspFlush(EgspLoader* pLoader)
{
	EgspResult retval = (pLoader->pData = pLoader->pFunc(pLoader->offset)) ? EGSP_SUCCESS : EGSP_FAIL;
	pLoader->offset = 0;
	return retval;
}

static EgspResult CheckOverFlow(EgspLoader* pLoader)
{
	if (pLoader->offset + 1 >= EGSP_BLOCK_SIZE)
	{
		EGSP_TEST(pLoader->pData = pLoader->pFunc(pLoader->offset));
		pLoader->offset = 0;
	}
	return EGSP_SUCCESS;
}

void EgspSetAlignBytes(size_t bytes)
{
	ALIGN_BYTES = bytes;
}

size_t EgspAlignBytes()
{
	return ALIGN_BYTES;
}

void EgspSetBlockSize(size_t bytes)
{
	EGSP_BLOCK_SIZE = bytes;
}

size_t EgspBlockSize()
{
	return EGSP_BLOCK_SIZE;
}

// 64 bit
EgspResult _EgspLoaduint64_t(EgspLoader* pLoader, uint64_t* pVal)
{
	*pVal = 0;
	for (int i = 0; i < 8; ++i)
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		*pVal <<= 8;
		*pVal += (pLoader->pData)[pLoader->offset++];
	}
	return EGSP_SUCCESS;
}

EgspResult _EgspSaveuint64_t(EgspLoader* pLoader, uint64_t* pVal)
{
	for (int i = 7; i >= 0; --i)
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		pLoader->pData[pLoader->offset++] = (uint8_t)((*pVal >> (8 * i)) & 0xFF);
	}
	return EGSP_SUCCESS;
}

EgspResult _EgspLoadint64_t(EgspLoader* pLoader, int64_t* pVal)
{
	return _EgspLoaduint64_t(pLoader, (uint64_t*)pVal);
}

EgspResult _EgspSaveint64_t(EgspLoader* pLoader, int64_t* pVal)
{
	return _EgspSaveuint64_t(pLoader, (uint64_t*)pVal);
}

EgspResult _EgspLoadDouble(EgspLoader* pLoader, double* pVal)
{
	return _EgspLoaduint64_t(pLoader, (uint64_t*)pVal);
}

EgspResult _EgspSaveDouble(EgspLoader* pLoader, double* pVal)
{
	return _EgspSaveuint64_t(pLoader, (uint64_t*)pVal);
}

// 32 bit
EgspResult _EgspLoaduint32_t(EgspLoader* pLoader, uint32_t* pVal)
{
	*pVal = 0;
	for (int i = 0; i < 4; ++i)
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		*pVal <<= 8;
		*pVal += (pLoader->pData)[pLoader->offset++];
	}
	return EGSP_SUCCESS;
}

EgspResult _EgspSaveuint32_t(EgspLoader* pLoader, uint32_t* pVal)
{
	for (int i = 3; i >= 0; --i)
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		pLoader->pData[pLoader->offset++] = (uint8_t)((*pVal >> (8 * i)) & 0xFF);
	}
	return EGSP_SUCCESS;
}

EgspResult _EgspLoadint32_t(EgspLoader* pLoader, int32_t* pVal)
{
	return _EgspLoaduint32_t(pLoader, (uint32_t*)pVal);
}

EgspResult _EgspSaveint32_t(EgspLoader* pLoader, int32_t* pVal)
{
	return _EgspSaveuint32_t(pLoader, (uint32_t*)pVal);
}


EgspResult _EgspLoadfloat(EgspLoader* pLoader, float* pVal)
{
	return _EgspLoaduint32_t(pLoader, (uint32_t*)pVal);
}

EgspResult _EgspSavefloat(EgspLoader* pLoader, float* pVal)
{
	return _EgspSaveuint32_t(pLoader, (uint32_t*)pVal);
}

// 16 bit
EgspResult _EgspLoaduint16_t(EgspLoader* pLoader, uint16_t* pVal)
{
	*pVal = 0;
	for (int i = 0; i < 2; ++i)
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		*pVal <<= 8;
		*pVal += (pLoader->pData)[pLoader->offset++];
	}
	return EGSP_SUCCESS;
}

EgspResult _EgspSaveuint16_t(EgspLoader* pLoader, uint16_t* pVal)
{
	for (int i = 1; i >= 0; --i)
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		pLoader->pData[pLoader->offset++] = (uint8_t)((*pVal >> (8 * i)) & 0xFF);
	}
	return EGSP_SUCCESS;
}

EgspResult _EgspLoadint16_t(EgspLoader* pLoader, int16_t* pVal)
{
	return _EgspLoaduint16_t(pLoader, (uint16_t*)pVal);
}

EgspResult _EgspSaveint16_t(EgspLoader* pLoader, int16_t* pVal)
{
	return _EgspSaveuint16_t(pLoader, (uint16_t*)pVal);
}

// 8 bit
EgspResult _EgspLoaduint8_t(EgspLoader* pLoader, uint8_t* pVal)
{
	EGSP_TRY(CheckOverFlow(pLoader));
	*pVal = pLoader->pData[pLoader->offset++];
	return EGSP_SUCCESS;
}

EgspResult _EgspSaveuint8_t(EgspLoader* pLoader, uint8_t* pVal)
{

	EGSP_TRY(CheckOverFlow(pLoader));
	pLoader->pData[pLoader->offset++] = *pVal;
	return EGSP_SUCCESS;
}

EgspResult _EgspLoadint8_t(EgspLoader* pLoader, uint8_t* pVal)
{
	return _EgspLoaduint8_t(pLoader, (uint8_t*)pVal);
}

EgspResult _EgspSaveInt8(EgspLoader* pLoader, int8_t* pVal)
{
	return _EgspSaveuint8_t(pLoader, (uint8_t*)pVal);
}

// String
EgspResult _EgspLoadstring(EgspLoader* pLoader, const char** ppString)
{
	uint32_t length = 0;
	EGSP_TRY(_EgspLoaduint32_t(pLoader, &length));
	EGSP_TRY(CheckOverFlow(pLoader));

	char* pbuffer = EgspAlloc(pLoader, length + 1);
	size_t pos = 0;
	for (size_t remain = EGSP_BLOCK_SIZE - pLoader->offset; remain < length - pos; pos += remain)
	{
		memcpy(pbuffer + pos, pLoader->pData + pLoader->offset, remain);
		EGSP_TEST(pLoader->pData = pLoader->pFunc(EGSP_BLOCK_SIZE));
		pLoader->offset = 0;
		remain = EGSP_BLOCK_SIZE;
	}
	memcpy(pbuffer + pos, pLoader->pData + pLoader->offset, length - pos);
	pLoader->offset += length - pos;
	pbuffer[length] = '\0';
	*ppString = pbuffer;

	return EGSP_SUCCESS;
}

EgspResult _EgspSavestring(EgspLoader* pLoader, const char** ppString)
{
	uint32_t length = (uint32_t)strlen(*ppString);
	pLoader->heapSize += EgspPad(length + 1);

	EGSP_TRY(_EgspSaveuint32_t(pLoader, &length));
	EGSP_TRY(CheckOverFlow(pLoader));

	size_t pos = 0;
	for (size_t remain = EGSP_BLOCK_SIZE - pLoader->offset; remain < length - pos; pos += remain)
	{
		memcpy(pLoader->pData + pLoader->offset, *ppString + pos, remain);
		EGSP_TEST(pLoader->pData = pLoader->pFunc(EGSP_BLOCK_SIZE));
		pLoader->offset = 0;
		remain = EGSP_BLOCK_SIZE;
	}

	memcpy(pLoader->pData + pLoader->offset, *ppString + pos, length - pos);
	pLoader->offset += length - pos;
	return EGSP_SUCCESS;
}

#ifdef EGSP_JSON
static EgspResult _EgspWriteChar(EgspLoader* pLoader, char chr)
{
	EGSP_TRY(CheckOverFlow(pLoader));
	pLoader->pData[pLoader->offset++] = chr;
	return EGSP_SUCCESS;
}

static EgspResult _EgspNewLine(EgspLoader* pLoader)
{
	EGSP_TRY(_EgspWriteChar(pLoader, '\n'));
	for (int i = 0; i < pLoader->indent; ++i)
	{
		EGSP_TRY(_EgspWriteChar(pLoader, '\t'));
	}
	return EGSP_SUCCESS;
}

EgspResult _EgspWriteString(EgspLoader* pLoader, const char* pString)
{
	size_t length = strlen(pString);
	for (size_t i = 0; i < length; ++i)
	{
		char cur = pString[i];

		switch (pLoader->last)
		{
		case '{':
		case '[':
			++pLoader->indent;
			EGSP_TRY(_EgspNewLine(pLoader));
			break;
		case ',':
			if (cur != ']' && cur != '}')
			{
				EGSP_TRY(_EgspWriteChar(pLoader, pLoader->last));
				EGSP_TRY(_EgspNewLine(pLoader));
			}
			break;
		default:
			if (cur == '[' || cur == '{')
			{
				EGSP_TRY(_EgspNewLine(pLoader));
			}
		}

		switch (cur)
		{
		case '{':
		case '[':
			EGSP_TRY(_EgspWriteChar(pLoader, cur));
			break;
		case '}':
		case ']':
			--pLoader->indent;
			EGSP_TRY(_EgspNewLine(pLoader));
			EGSP_TRY(_EgspWriteChar(pLoader, cur));
			break;
		case ':':
			EGSP_TRY(_EgspWriteChar(pLoader, ':'));
			EGSP_TRY(_EgspWriteChar(pLoader, ' '));
			break;
		case ',':
			break;
		default:
			EGSP_TRY(_EgspWriteChar(pLoader, cur));
		}
		pLoader->last = cur;
	}
	return EGSP_SUCCESS;
}

static EgspResult _EgspReadChar(EgspLoader* pLoader, size_t* pPos, char** pChar, char chr)
{
	if (*pPos >= ALIGN_BYTES)
	{
		EGSP_TEST(*pChar = EgspAlloc(pLoader, ALIGN_BYTES));
		*pPos = 0;
	}
	(*pChar)[(*pPos)++] = chr;
	return EGSP_SUCCESS;
}

EgspResult _EgspGetField(EgspLoader* pLoader, char* buffer)
{
	char chr = '\0';
	size_t pos = 0;
	while (chr != ',' && chr != '}' && chr != ']')
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		chr = pLoader->pData[pLoader->offset++];
		buffer[pos++] = chr;
	}
	buffer[pos] = '\0';
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintuint64_t(EgspLoader* pLoader, uint64_t* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%llu,", (unsigned long long)*pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReaduint64_t(EgspLoader* pLoader, uint64_t* pVal)
{
	long long val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%llu", &val);
	*pVal = val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintint64_t(EgspLoader* pLoader, int64_t* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%lld,", (long long)*pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReadint64_t(EgspLoader* pLoader, int64_t* pVal)
{
	long long val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%lld", &val);
	*pVal = val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintdouble(EgspLoader* pLoader, double* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%f,", *pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReaddouble(EgspLoader* pLoader, double* pVal)
{
	double val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%lf", &val);
	*pVal = val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintuint32_t(EgspLoader* pLoader, uint32_t* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%u,", *pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReaduint32_t(EgspLoader* pLoader, uint32_t* pVal)
{
	unsigned int val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%u", &val);
	*pVal = val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintint32_t(EgspLoader* pLoader, int32_t* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%d,", *pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReadint32_t(EgspLoader* pLoader, int32_t* pVal)
{
	int val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%d", &val);
	*pVal = val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintfloat(EgspLoader* pLoader, float* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%f,", *pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReadfloat(EgspLoader* pLoader, float* pVal)
{
	float val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%f", &val);
	*pVal = val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintuint16_t(EgspLoader* pLoader, uint16_t* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%u,", *pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReaduint16_t(EgspLoader* pLoader, uint16_t* pVal)
{
	unsigned int val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%u", &val);
	*pVal = (uint16_t)val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintint16_t(EgspLoader* pLoader, int16_t* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%d,", *pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReadint16_t(EgspLoader* pLoader, int16_t* pVal)
{
	int val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%d", &val);
	*pVal = (int16_t)val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintuint8_t(EgspLoader* pLoader, uint8_t* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%u,", *pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReaduint8_t(EgspLoader* pLoader, uint8_t* pVal)
{
	unsigned int val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%u", &val);
	*pVal = (uint8_t)val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintint8_t(EgspLoader* pLoader, int8_t* pVal)
{
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	sprintf(buffer, "%d,", *pVal);
	return _EgspWriteString(pLoader, buffer);
}

EgspResult _EgspReadint8_t(EgspLoader* pLoader, int8_t* pVal)
{
	int val = 0;
	char buffer[EGSP_NUMERIC_BUFFER_LENGTH];
	EGSP_TRY(_EgspGetField(pLoader, buffer));
	sscanf(buffer, "%d", &val);
	*pVal = (int8_t)val;
	return EGSP_SUCCESS;
}

EgspResult _EgspPrintstring(EgspLoader* pLoader, const char** ppString)
{
	uint32_t length = (uint32_t)strlen(*ppString);
	pLoader->heapSize += EgspPad(length + 1);
	EGSP_TRY(_EgspWriteChar(pLoader, '"'));
	for (const char* pChr = *ppString; *pChr != '\0'; ++pChr)
	{
		// List from http://json.org/. Unicode(\u) not supported.
		switch (*pChr)
		{
		case '"':
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			EGSP_TRY(_EgspWriteChar(pLoader, '"'));
			break;
		case '\n':
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			EGSP_TRY(_EgspWriteChar(pLoader, 'n'));
			break;
		case '\r':
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			EGSP_TRY(_EgspWriteChar(pLoader, 'r'));
			break;
		case '\t':
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			EGSP_TRY(_EgspWriteChar(pLoader, 't'));
			break;
		case '\b':
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			EGSP_TRY(_EgspWriteChar(pLoader, 'b'));
			break;
		case '\\':
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			break;
		case '/':
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			EGSP_TRY(_EgspWriteChar(pLoader, '/'));
			break;
		case '\f':
			EGSP_TRY(_EgspWriteChar(pLoader, '\\'));
			EGSP_TRY(_EgspWriteChar(pLoader, 'f'));
			break;
		default:
			EGSP_TRY(_EgspWriteChar(pLoader, *pChr));
		}
	}
	EGSP_TRY(_EgspWriteChar(pLoader, '"'));
	EGSP_TRY(_EgspWriteString(pLoader, ","));
	return EGSP_SUCCESS;
}

// Pretty sure this is some sort of interview question
static void _EgspReverseBlocks(uint8_t* pFirst, uint8_t* pLast)
{
	while (pFirst < pLast)
	{
		for (size_t i = 0; i < ALIGN_BYTES; ++i)
		{
			char tmp = pFirst[i];
			pFirst[i] = pLast[i];
			pLast[i] = tmp;
		}
		pFirst += ALIGN_BYTES;
		pLast -= ALIGN_BYTES;
	}
}

EgspResult _EgspReadstring(EgspLoader* pLoader, const char** ppString)
{
	char chr = '\0';
	char* pBuffer = 0;
	while (chr != '"')
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		chr = pLoader->pData[pLoader->offset++];
	}

	// Bookmark this spot. Allocations happen in reverse order.
	uint8_t* pFirst = (uint8_t*)pLoader->pHeap + pLoader->heapSize - ALIGN_BYTES;
	size_t pos = ALIGN_BYTES;
	chr = '\0';
	while (1)
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		chr = pLoader->pData[pLoader->offset++];
		if (chr == '"')
		{
			break;
		}
		else if (chr == '\\')
		{
			EGSP_TRY(CheckOverFlow(pLoader));
			// List from http://json.org/. Unicode(\u) not supported.
			switch (pLoader->pData[pLoader->offset++])
			{
			case '"':
				EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '"'));
				break;
			case 'n':
				EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '\n'));
				break;
			case 'r':
				EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '\r'));
				break;
			case 't':
				EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '\t'));
				break;
			case 'b':
				EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '\b'));
				break;
			case '\\':
				EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '\\'));
				break;
			case '/':
				EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '/'));
				break;
			case 'f':
				EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '\f'));
				break;
			default:
				return EGSP_FAIL;
			}
		}
		else
		{
			EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, chr));
		}
	}
	EGSP_TRY(_EgspReadChar(pLoader, &pos, &pBuffer, '\0'));
	_EgspReverseBlocks(pBuffer, pFirst);
	*ppString = pBuffer;
	return EGSP_SUCCESS;
}

EgspResult _EgspSkipLabel(EgspLoader* pLoader)
{
	char chr = '\0';
	size_t pos = 0;
	while (chr != ':')
	{
		EGSP_TRY(CheckOverFlow(pLoader));
		chr = pLoader->pData[pLoader->offset++];
	}
	return EGSP_SUCCESS;
}

#endif