#ifndef EGSPLIB_H
#define EGSPLIB_H

#include <stdint.h>
#include <stddef.h>

typedef enum
{
	EGSP_SUCCESS,
	EGSP_FAIL
} EgspResult;

#define EGSP_TRY(X) { if (X == EGSP_FAIL) return EGSP_FAIL; }
#define EGSP_TEST(X) { if (!(X)) return EGSP_FAIL; }

typedef uint8_t* (*EgspFunc)(size_t);
typedef struct
{
	uint8_t* pData;
	size_t offset;
	EgspFunc pFunc;
	void* pHeap;
	size_t heapSize;
	char last;
	int indent;
} EgspLoader;

// Utility
size_t EgspPad(size_t bytes);
void* EgspAlloc(EgspLoader* pLoader, size_t size);
EgspResult EgspFlush(EgspLoader* pLoader);
void EgspSetAlignBytes(size_t bytes);
size_t EgspAlignBytes();
void EgspSetBlockSize(size_t bytes);
size_t EgspBlockSize();

// 64 bit
EgspResult _EgspLoaduint64_t(EgspLoader* pLoader, uint64_t* pVal);
EgspResult _EgspSaveuint64_t(EgspLoader* pLoader, uint64_t* pVal);
EgspResult _EgspLoadint64_t(EgspLoader* pLoader, int64_t* pVal);
EgspResult _EgspSaveint64_t(EgspLoader* pLoader, int64_t* pVal);
EgspResult _EgspLoadDouble(EgspLoader* pLoader, double* pVal);
EgspResult _EgspSaveDouble(EgspLoader* pLoader, double* pVal);

// 32 bit
EgspResult _EgspLoaduint32_t(EgspLoader* pLoader, uint32_t* pVal);
EgspResult _EgspSaveuint32_t(EgspLoader* pLoader, uint32_t* pVal);
EgspResult _EgspLoadint32_t(EgspLoader* pLoader, int32_t* pVal);
EgspResult _EgspSaveint32_t(EgspLoader* pLoader, int32_t* pVal);
EgspResult _EgspLoadfloat(EgspLoader* pLoader, float* pVal);
EgspResult _EgspSavefloat(EgspLoader* pLoader, float* pVal);

// 16 bit
EgspResult _EgspLoaduint16_t(EgspLoader* pLoader, uint16_t* pVal);
EgspResult _EgspSaveuint16_t(EgspLoader* pLoader, uint16_t* pVal);
EgspResult _EgspLoadint16_t(EgspLoader* pLoader, int16_t* pVal);
EgspResult _EgspSaveint16_t(EgspLoader* pLoader, int16_t* pVal);

// 8 bit
EgspResult _EgspLoaduint8_t(EgspLoader* pLoader, uint8_t* pVal);
EgspResult _EgspSaveuint8_t(EgspLoader* pLoader, uint8_t* pVal);
EgspResult _EgspLoadint8_t(EgspLoader* pLoader, uint8_t* pVal);
EgspResult _EgspSaveInt8(EgspLoader* pLoader, int8_t* pVal);

// String
EgspResult _EgspLoadstring(EgspLoader* pLoader, const char** ppString);
EgspResult _EgspSavestring(EgspLoader* pLoader, const char** ppString);

#ifdef EGSP_JSON
// JsonPrint
EgspResult _EgspWriteString(EgspLoader* pLoader, const char* pString);
EgspResult _EgspPrintuint64_t(EgspLoader* pLoader, uint64_t* pVal);
EgspResult _EgspPrintint64_t(EgspLoader* pLoader, int64_t* pVal);
EgspResult _EgspPrintdouble(EgspLoader* pLoader, double* pVal);
EgspResult _EgspPrintuint32_t(EgspLoader* pLoader, uint32_t* pVal);
EgspResult _EgspPrintint32_t(EgspLoader* pLoader, int32_t* pVal);
EgspResult _EgspPrintfloat(EgspLoader* pLoader, float* pVal);
EgspResult _EgspPrintuint16_t(EgspLoader* pLoader, uint16_t* pVal);
EgspResult _EgspPrintint16_t(EgspLoader* pLoader, int16_t* pVal);
EgspResult _EgspPrintuint8_t(EgspLoader* pLoader, uint8_t* pVal);
EgspResult _EgspPrintint8_t(EgspLoader* pLoader, int8_t* pVal);
EgspResult _EgspPrintstring(EgspLoader* pLoader, const char** ppString);

EgspResult _EgspReaduint64_t(EgspLoader* pLoader, uint64_t* pVal);
EgspResult _EgspReadint64_t(EgspLoader* pLoader, int64_t* pVal);
EgspResult _EgspReaddouble(EgspLoader* pLoader, double* pVal);
EgspResult _EgspReaduint32_t(EgspLoader* pLoader, uint32_t* pVal);
EgspResult _EgspReadint32_t(EgspLoader* pLoader, int32_t* pVal);
EgspResult _EgspReadfloat(EgspLoader* pLoader, float* pVal);
EgspResult _EgspReaduint16_t(EgspLoader* pLoader, uint16_t* pVal);
EgspResult _EgspReadint16_t(EgspLoader* pLoader, int16_t* pVal);
EgspResult _EgspReaduint8_t(EgspLoader* pLoader, uint8_t* pVal);
EgspResult _EgspReadint8_t(EgspLoader* pLoader, int8_t* pVal);
EgspResult _EgspReadstring(EgspLoader* pLoader, const char** ppString);
EgspResult _EgspSkipPastChar(EgspLoader* pLoader, char character);
EgspResult _EgspSkipLabel(EgspLoader* pLoader);
#endif // EGSP_JSON
#endif
