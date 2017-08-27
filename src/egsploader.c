#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#define EGSP_MAX_FIELD_LENGTH 256
#define EGSP_BUFFER_SIZE (1 << 20)

typedef int(*Processor)(char);
typedef enum
{
	STRUCT_NAME = 0,
	DATA_TYPE,
	VAR_NAME,
	LIST_SIZE,
	COUNT
} FieldType;

typedef enum
{
	ARRAY,
	POINTER,
	ENUM,
	BUFFER,
	DEFAULT
} DataType;

static FILE* s_pCode;
static char s_fields[COUNT][EGSP_MAX_FIELD_LENGTH];
static DataType s_type;
static int s_curField = 0;
static int s_curpos = 0;
static int s_linenum = 0;

static struct {
	char* pBase;
	char* pLoad;
	char* pSave;
	char* pPrint;
	char* pRead;
} s_buffers;

static void ErrorCheck(int condition, const char* text)
{
	if (condition)
	{
		fprintf(stderr, "Line %d: %s\n", s_linenum, text);
		exit(1);
	}
}

static void BeginStruct()
{
	s_buffers.pLoad = s_buffers.pBase;
	s_buffers.pSave = s_buffers.pBase + EGSP_BUFFER_SIZE;
	s_type = DEFAULT;

	//Loader
	s_buffers.pLoad += sprintf(s_buffers.pLoad, 
		"static EgspResult _EgspLoad%s(EgspLoader* pLoader, %s* pVal)\n{\n"
		"\tuint8_t egspNullCheck = 0;\n"
		, s_fields[STRUCT_NAME], s_fields[STRUCT_NAME]);

	//Saver
	s_buffers.pSave += sprintf(s_buffers.pSave, 
		"static EgspResult _EgspSave%s(EgspLoader* pLoader, %s* pVal)\n{\n"
		, s_fields[STRUCT_NAME], s_fields[STRUCT_NAME]);

#ifdef EGSP_JSON
	s_buffers.pPrint = s_buffers.pBase + EGSP_BUFFER_SIZE * 2;
	s_buffers.pRead = s_buffers.pBase + EGSP_BUFFER_SIZE * 3;

	//Printer
	s_buffers.pPrint += sprintf(s_buffers.pPrint, 
		"static EgspResult _EgspPrint%s(EgspLoader* pLoader, %s* pVal)\n{\n"
		"\tEGSP_TRY(_EgspWriteString(pLoader, \"{\"));\n"
		, s_fields[STRUCT_NAME], s_fields[STRUCT_NAME]);

	//Reader
	s_buffers.pRead += sprintf(s_buffers.pRead, 
		"static EgspResult _EgspRead%s(EgspLoader* pLoader, %s* pVal)\n{\n"
		"\tuint8_t egspNullCheck = 0;\n"
		, s_fields[STRUCT_NAME], s_fields[STRUCT_NAME]);
#endif
}

static void EndStruct()
{
	//Loader
	s_buffers.pLoad += sprintf(s_buffers.pLoad, "\treturn EGSP_SUCCESS;\n}\n\n"
		"static EgspResult EgspLoad%s(EgspFunc pLoadFunc, %s* pVal, void* pHeap, size_t heapSize)\n"
		"{\n"
		"\tEgspLoader loader;\n"
		"\tloader.pFunc = pLoadFunc;\n"
		"\tloader.offset = 0;\n"
		"\tloader.pHeap = pHeap;\n"
		"\tloader.heapSize = heapSize;\n"
		"\tEGSP_TEST(loader.pData = loader.pFunc(EgspBlockSize()));\n"
		"\tEGSP_TRY(_EgspLoad%s(&loader, pVal));\n"
		"\treturn EGSP_SUCCESS;\n"
		"}\n\n"
		, s_fields[STRUCT_NAME], s_fields[STRUCT_NAME], s_fields[STRUCT_NAME]);

	//Saver
	s_buffers.pSave += sprintf(s_buffers.pSave, "\treturn EGSP_SUCCESS;\n}\n\n"
		"static EgspResult EgspSave%s(EgspFunc pFlushFunc, %s* pVal, size_t* pHeapRequired)\n"
		"{\n"
		"\tEgspLoader loader;\n"
		"\tloader.pFunc = pFlushFunc;\n"
		"\tloader.offset = 0;\n"
		"\tloader.heapSize = 0;\n"
		"\tEGSP_TEST(loader.pData = loader.pFunc(0));\n"
		"\tEGSP_TRY(_EgspSave%s(&loader, pVal))\n;"
		"\tEGSP_TRY(EgspFlush(&loader));\n"
		"\t*pHeapRequired = loader.heapSize;\n"
		"\treturn EGSP_SUCCESS;\n"
		"}\n\n"
		, s_fields[STRUCT_NAME], s_fields[STRUCT_NAME], s_fields[STRUCT_NAME]);

	fputs(s_buffers.pBase, s_pCode);
	fputs(s_buffers.pBase + EGSP_BUFFER_SIZE, s_pCode);

#ifdef EGSP_JSON
	//Printer
	s_buffers.pPrint += sprintf(s_buffers.pPrint, 
		"\treturn _EgspWriteString(pLoader, \"},\");\n"
		"}\n\n"
		"static EgspResult EgspPrint%s(EgspFunc pFlushFunc, %s* pVal, size_t* pHeapRequired)\n"
		"{\n"
		"\tEgspLoader loader;\n"
		"\tloader.pFunc = pFlushFunc;\n"
		"\tloader.offset = 0;\n"
		"\tloader.heapSize = 0;\n"
		"\tloader.last = 0;\n"
		"\tloader.indent = 0;\n"
		"\tEGSP_TEST(loader.pData = loader.pFunc(0));\n"
		"\tEGSP_TRY(_EgspPrint%s(&loader, pVal));\n"
		"\tEGSP_TRY(EgspFlush(&loader));\n"
		"\t*pHeapRequired = loader.heapSize;\n"
		"\treturn EGSP_SUCCESS;\n"
		"}\n\n"
		, s_fields[STRUCT_NAME], s_fields[STRUCT_NAME], s_fields[STRUCT_NAME]);

	s_buffers.pRead += sprintf(s_buffers.pRead, "\treturn EGSP_SUCCESS;\n}\n\n"
		"static EgspResult EgspRead%s(EgspFunc pLoadFunc, %s* pVal, void* pHeap, size_t heapSize)\n"
		"{\n"
		"\tEgspLoader loader;\n"
		"\tloader.pFunc = pLoadFunc;\n"
		"\tloader.offset = 0;\n"
		"\tloader.pHeap = pHeap;\n"
		"\tloader.heapSize = heapSize;\n"
		"\tEGSP_TEST(loader.pData = loader.pFunc(0));\n"
		"\tEGSP_TRY(_EgspRead%s(&loader, pVal));\n"
		"\treturn EGSP_SUCCESS;\n"
		"}\n\n"
		, s_fields[STRUCT_NAME], s_fields[STRUCT_NAME], s_fields[STRUCT_NAME]);

	fputs(s_buffers.pBase + EGSP_BUFFER_SIZE * 2, s_pCode);
	fputs(s_buffers.pBase + EGSP_BUFFER_SIZE * 3, s_pCode);
#endif
}

static void AddField()
{
	switch (s_type)
	{
	case ARRAY:
		s_buffers.pLoad += sprintf(s_buffers.pLoad, 
			"\tif (pVal->%s = EgspAlloc(pLoader, sizeof(*pVal->%s) * pVal->%s))\n"
			"\t{\n"
			"\t\tfor (size_t i = 0; i < pVal->%s; ++i)\n"
			"\t\t{\n"
			"\t\t\tEGSP_TRY(_EgspLoad%s(pLoader, pVal->%s + i));\n"
			"\t\t}\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[VAR_NAME], s_fields[LIST_SIZE], s_fields[LIST_SIZE]
			, s_fields[DATA_TYPE], s_fields[VAR_NAME]);

		s_buffers.pSave += sprintf(s_buffers.pSave, 
			"\tpLoader->heapSize += EgspPad(sizeof(*pVal->%s)) * pVal->%s;\n"
			"\tfor (size_t i = 0; i < pVal->%s; ++i)\n"
			"\t{\n"
			"\t\tEGSP_TRY(_EgspSave%s(pLoader, pVal->%s + i));\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[LIST_SIZE], s_fields[LIST_SIZE], s_fields[DATA_TYPE]
			, s_fields[VAR_NAME]);
#ifdef EGSP_JSON
		s_buffers.pPrint += sprintf(s_buffers.pPrint, 
			"\tpLoader->heapSize += EgspPad(sizeof(*pVal->%s)) * pVal->%s;\n"
			"\tEGSP_TRY(_EgspWriteString(pLoader, \"\\\"%s\\\":[\"));\n"
			"\tfor (size_t i = 0; i < pVal->%s; ++i)\n"
			"\t{\n"
			"\t\tEGSP_TRY(_EgspPrint%s(pLoader, pVal->%s + i));\n"
			"\t}\n"
			"\tEGSP_TRY(_EgspWriteString(pLoader, \"],\"))\n"
			, s_fields[VAR_NAME], s_fields[LIST_SIZE], s_fields[VAR_NAME], s_fields[LIST_SIZE]
			, s_fields[DATA_TYPE], s_fields[VAR_NAME]);

		s_buffers.pRead += sprintf(s_buffers.pRead,
			"\tif (pVal->%s = EgspAlloc(pLoader, sizeof(*pVal->%s) * pVal->%s))\n"
			"\t{\n"
			"\t\tEGSP_TRY(_EgspSkipPastChar(pLoader, '['));\n"
			"\t\tfor (size_t i = 0; i < pVal->%s; ++i)\n"
			"\t\t{\n"
			"\t\t\tEGSP_TRY(_EgspRead%s(pLoader, pVal->%s + i));\n"
			"\t\t}\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[VAR_NAME], s_fields[LIST_SIZE], s_fields[LIST_SIZE]
			, s_fields[DATA_TYPE], s_fields[VAR_NAME]);
#endif
		break;

	case BUFFER:
		s_buffers.pLoad += sprintf(s_buffers.pLoad,
			"\tif (pVal->%s = EgspAlloc(pLoader, pVal->%s))\n"
			"\t{\n"
			"\t\tEGSP_TRY(_EgspLoadBuffer(pLoader, &pVal->%s, pVal->%s));\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[LIST_SIZE], s_fields[VAR_NAME], s_fields[LIST_SIZE]);

		s_buffers.pSave += sprintf(s_buffers.pSave,
			"\tpLoader->heapSize += EgspPad(pVal->%s);\n"
			"\tEGSP_TRY(_EgspSaveBuffer(pLoader, pVal->%s, pVal->%s));\n"
			,s_fields[LIST_SIZE], s_fields[VAR_NAME], s_fields[LIST_SIZE]);
#ifdef EGSP_JSON
		s_buffers.pPrint += sprintf(s_buffers.pPrint,
			"\tpLoader->heapSize += EgspPad(pVal->%s);\n"
			"\tEGSP_TRY(_EgspWriteString(pLoader, \"\\\"%s\\\":\"));\n"
			"\tEGSP_TRY(_EgspPrintBuffer(pLoader, pVal->%s, pVal->%s));\n"
			, s_fields[LIST_SIZE], s_fields[VAR_NAME], s_fields[VAR_NAME], s_fields[LIST_SIZE]);

		s_buffers.pRead += sprintf(s_buffers.pRead,
			"\tEGSP_TEST(pVal->%s = EgspAlloc(pLoader, pVal->%s))\n"
			"\tEGSP_TRY(_EgspSkipLabel(pLoader));\n"
			"\tEGSP_TRY(_EgspReadBuffer(pLoader, pVal->%s, pVal->%s ));\n"
			, s_fields[VAR_NAME], s_fields[LIST_SIZE], s_fields[VAR_NAME], s_fields[LIST_SIZE]);
#endif
		break;

	case POINTER:
		s_buffers.pLoad += sprintf(s_buffers.pLoad,
			"\tEGSP_TRY(_EgspLoaduint8_t(pLoader, &egspNullCheck));\n"
			"\tif (egspNullCheck)\n"
			"\t{\n"
			"\t\tEGSP_TEST(pVal->%s = EgspAlloc(pLoader, sizeof(%s)))\n"
			"\t\tEGSP_TRY(_EgspLoad%s(pLoader, pVal->%s));\n"
			"\t}\n"
			"\telse\n"
			"\t{\n"
			"\t\tpVal->%s = 0;\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[DATA_TYPE], s_fields[DATA_TYPE], s_fields[VAR_NAME], s_fields[VAR_NAME]);

		s_buffers.pSave += sprintf(s_buffers.pSave,
			"\tif (pVal->%s)\n"
			"\t{\n"
			"\t\tpLoader->heapSize += EgspPad(sizeof(*pVal->%s));\n"
			"\t\tuint8_t nullInd = 1;\n"
			"\t\tEGSP_TRY(_EgspSaveuint8_t(pLoader, &nullInd));\n"
			"\t\tEGSP_TRY(_EgspSave%s(pLoader, pVal->%s));\n"
			"\t}\n"
			"\telse\n"
			"\t{\n"
			"\t\tuint8_t nullInd = 0;\n"
			"\t\tEGSP_TRY(_EgspSaveuint8_t(pLoader, &nullInd));\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[VAR_NAME], s_fields[DATA_TYPE], s_fields[VAR_NAME]);
#ifdef EGSP_JSON
		s_buffers.pPrint += sprintf(s_buffers.pPrint,
			"\tif (pVal->%s)\n"
			"\t{\n"
			"\t\tpLoader->heapSize += EgspPad(sizeof(*pVal->%s));\n"
			"\t\tuint8_t nullInd = 1;\n"
			"\t\tEGSP_TRY(_EgspWriteString(pLoader, \"\\\"%s is not null. Processing\\\":\"));\n"
			"\t\tEGSP_TRY(_EgspPrintuint8_t(pLoader, &nullInd));\n"
			"\t\tEGSP_TRY(_EgspWriteString(pLoader, \"\\\"%s\\\":\"));\n"
			"\t\tEGSP_TRY(_EgspPrint%s(pLoader, pVal->%s))\n"
			"\t}\n"
			"\telse\n"
			"\t{\n"
			"\t\tuint8_t nullInd = 0;\n"
			"\t\tEGSP_TRY(_EgspWriteString(pLoader, \"\\\"%s is null. Skipping.\\\":\"));\n"
			"\t\tEGSP_TRY(_EgspPrintuint8_t(pLoader, &nullInd));\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[VAR_NAME], s_fields[VAR_NAME], s_fields[VAR_NAME], s_fields[DATA_TYPE]
			, s_fields[VAR_NAME], s_fields[VAR_NAME]);

		s_buffers.pRead += sprintf(s_buffers.pRead,
			"\tEGSP_TRY(_EgspSkipLabel(pLoader));\n"
			"\tEGSP_TRY(_EgspReaduint8_t(pLoader, &egspNullCheck));\n"
			"\tif (egspNullCheck)\n"
			"\t{\n"
			"\t\tEGSP_TEST(pVal->%s = EgspAlloc(pLoader, sizeof(*pVal->%s)))\n"
			"\t\tEGSP_TRY(_EgspSkipLabel(pLoader));\n"
			"\t\tEGSP_TRY(_EgspRead%s(pLoader, pVal->%s));\n"
			"\t}\n"
			"\telse\n"
			"\t{\n"
			"\t\tpVal->%s = 0;\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[VAR_NAME], s_fields[DATA_TYPE], s_fields[VAR_NAME], s_fields[VAR_NAME]);
#endif
		break;

	case ENUM:
		s_buffers.pLoad += sprintf(s_buffers.pLoad,
			"\t{\n"
			"\t\tint32_t enumval = 0;\n"
			"\t\tEGSP_TRY(_EgspLoadint32_t(pLoader, &enumval));\n"
			"\t\tpVal->%s = (%s) enumval;\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[DATA_TYPE]);
		s_buffers.pSave += sprintf(s_buffers.pSave, 
			"\t{\n"
			"\t\tint32_t enumval = pVal->%s;\n"
			"\tEGSP_TRY(_EgspSaveint32_t(pLoader, &enumval));\n"
			"\t}\n"
			, s_fields[VAR_NAME]), s_fields[DATA_TYPE];
#ifdef EGSP_JSON
		s_buffers.pPrint += sprintf(s_buffers.pPrint,
			"\t{\n"
			"\t\tint32_t enumval = pVal->%s;\n"
			"\t\tEGSP_TRY(_EgspWriteString(pLoader, \"\\\"%s\\\":\"));\n"
			"\t\tEGSP_TRY(_EgspPrintint32_t(pLoader, &enumval));\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[VAR_NAME]);
		s_buffers.pRead += sprintf(s_buffers.pRead,
			"\t{\n"
			"\t\tint32_t enumval = 0;\n"
			"\t\tEGSP_TRY(_EgspSkipLabel(pLoader));\n"
			"\t\tEGSP_TRY(_EgspReadint32_t(pLoader, &enumval));\n"
			"\t\tpVal->%s = (%s) enumval;\n"
			"\t}\n"
			, s_fields[VAR_NAME], s_fields[DATA_TYPE]);
#endif
		break;

	case DEFAULT:
		s_buffers.pLoad += sprintf(s_buffers.pLoad, "\tEGSP_TRY(_EgspLoad%s(pLoader, &pVal->%s));\n", s_fields[DATA_TYPE], s_fields[VAR_NAME]);
		s_buffers.pSave += sprintf(s_buffers.pSave, "\tEGSP_TRY(_EgspSave%s(pLoader, &pVal->%s));\n", s_fields[DATA_TYPE], s_fields[VAR_NAME]);
#ifdef EGSP_JSON
		s_buffers.pPrint += sprintf(s_buffers.pPrint, 
			"\tEGSP_TRY(_EgspWriteString(pLoader, \"\\\"%s\\\":\"));\n"
			"\tEGSP_TRY(_EgspPrint%s(pLoader, &pVal->%s));\n"
			, s_fields[VAR_NAME], s_fields[DATA_TYPE], s_fields[VAR_NAME]);
		s_buffers.pRead += sprintf(s_buffers.pRead, 
			"\tEGSP_TRY(_EgspSkipLabel(pLoader));\n"
			"\tEGSP_TRY(_EgspRead%s(pLoader, &pVal->%s));\n"
			, s_fields[DATA_TYPE], s_fields[VAR_NAME]);
#endif
	}
	s_type = DEFAULT;
}

static int ProcessStructName(char chr)
{
	if (isspace(chr) || chr == ';')
	{
		return STRUCT_NAME;
	}

	if (chr == '{')
	{
		ErrorCheck(s_curpos == 0, "Expecting struct name");
		s_fields[STRUCT_NAME][s_curpos] = '\0';
		BeginStruct();
		s_curpos = 0;
		return DATA_TYPE;
	}

	ErrorCheck(!isalnum(chr) && chr != '_', "Invalid Structure Name");
	s_fields[STRUCT_NAME][s_curpos++] = chr;
	return STRUCT_NAME;
}

static int ProcessDataType(char chr)
{
	if (chr == '*')
	{
		ErrorCheck(s_curpos == 0, "Data type not specified");
		s_type = POINTER;
		s_fields[DATA_TYPE][s_curpos] = '\0';
		s_curpos = 0;
		return VAR_NAME;
	}

	if (chr == '%')
	{
		s_type = ENUM;
		s_fields[DATA_TYPE][s_curpos] = '\0';
		s_curpos = 0;
		return VAR_NAME;
	}

	if (isspace(chr))
	{
		if (s_curpos == 0)
			return DATA_TYPE;

		s_fields[DATA_TYPE][s_curpos] = '\0';
		s_curpos = 0;
		return VAR_NAME;
	}

	if (chr == '}')
	{
		ErrorCheck(s_curpos != 0, "Unexpected end of struct");
		EndStruct();
		return STRUCT_NAME;
	}

	ErrorCheck(!isalnum(chr) && chr != '_', "Invalid data type");
	s_fields[DATA_TYPE][s_curpos++] = chr;
	return DATA_TYPE;
}

static int ProcessVarName(char chr)
{
	if (chr == '*' && s_curpos == 0)
	{
		s_type = POINTER;
		return VAR_NAME;
	}

	if (chr == '%' && s_curpos == 0)
	{
		s_type = ENUM;
		return VAR_NAME;
	}

	if (isspace(chr))
	{
		return VAR_NAME;
	}

	if (chr == ';')
	{
		ErrorCheck(s_curpos == 0, "Expected variable name");
		s_fields[VAR_NAME][s_curpos] = '\0';
		s_curpos = 0;
		AddField();
		return DATA_TYPE;
	}

	if (chr == '[' || chr == '(')
	{
		ErrorCheck(s_curpos == 0, "Expected variable name");
		s_fields[VAR_NAME][s_curpos] = '\0';
		s_curpos = 0;
		s_type = chr == '[' ? ARRAY : BUFFER;
		return LIST_SIZE;
	}

	ErrorCheck(!isalnum(chr) && chr != '_', "Invalid variable name");
	s_fields[VAR_NAME][s_curpos++] = chr;
	return VAR_NAME;
}

static int ProcessListSize(char chr)
{
	if (chr == ']' || chr == ')')
	{
		ErrorCheck(s_curpos == 0, "Expected list size");
		s_fields[LIST_SIZE][s_curpos] = '\0';
		ErrorCheck(s_type != (chr == ']' ? ARRAY : BUFFER), "Wrong list type");
		return LIST_SIZE;
	}
	if (chr == ';')
	{
		ErrorCheck(s_curpos == 0, "Expected list size");
		AddField();
		s_curpos = 0;
		return DATA_TYPE;
	}

	ErrorCheck(!isalnum(chr) && chr != '_', "Invalid variable name for list size");
	s_fields[LIST_SIZE][s_curpos++] = chr;
	return LIST_SIZE;
}

static Processor s_processors[COUNT] = {
	ProcessStructName,
	ProcessDataType,
	ProcessVarName,
	ProcessListSize
};

static int LoadSchema(const char* filename)
{
	FILE* pFile = 0;
	if (pFile = fopen(filename, "r"))
	{
		int chr = 0;
		while (1)
		{
			int chr = fgetc(pFile);
			if (chr == EOF)
			{
				break;
			}
			if (chr == '\n' || chr == '\r')
			{
				++s_linenum;
			}
			s_curField = s_processors[s_curField](chr);
		}
		fclose(pFile);
		return 0;
	}
	return 1;
}

int main(int argc, char** argv)
{
#ifdef EGSP_JSON
	s_buffers.pBase = (char*)malloc(EGSP_BUFFER_SIZE * 4);
#else
	s_buffers.pBase = (char*)malloc(EGSP_BUFFER_SIZE * 2);
#endif

	if (s_pCode = fopen("egspload.h", "w"))
	{
		fprintf(s_pCode, "// This file is automatically generated by egsploader.\n\n"
			"#ifndef EGSPLOAD_H\n"
			"#define EGSPLOAD_H\n\n"
			"#pragma warning(push)\n"
			"#pragma warning(disable: 4090)\n"
#ifdef EGSP_JSON
			"#define EGSP_JSON\n"
#endif
			"#include \"egsplib.h\"\n\n");

		for (int i = 1; i < argc; ++i)
		{
			if (LoadSchema(argv[i]) != 0)
			{
				fprintf(stderr, "Error Loading %s\n", argv[i]);
				return 1;
			}
		}

		fprintf(s_pCode, "#pragma warning(pop)\n#endif");
		
		fclose(s_pCode);
	}
	free(s_buffers.pBase);
	return 0;
}
