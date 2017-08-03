#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>

// Data type definitions. These would usually sit in a header file
typedef enum
{
	FIRST_VAL,
	SECOND_VAL,
	THIRD_VAL,
	FOURTH_VAL
} TestEnum;

typedef struct
{
	uint64_t dummy;
	uint32_t dummy2;
} InnerStruct;

typedef struct
{
	uint32_t testint;
	float testfloat;
	int16_t testsigned;
	uint32_t structcount;
	InnerStruct* teststruct;
	InnerStruct* pointerstruct;
	InnerStruct* nullstruct;
	InnerStruct inlinestruct;
	TestEnum testenum;
	const char* TestString;
} TestStruct;

#include "egspload.h"

// Control Variables
uint8_t buffer[1<<20];
static size_t count = 0;
FILE* s_pFile;

// TestData
TestStruct testdata;
TestStruct output;
InnerStruct testarray[4];
char teststring[1024];

// LoadFunc for testing multiple blocks
uint8_t* LoadFunc(size_t size)
{
	uint8_t* pData = buffer + (count++ * EgspBlockSize());
	return pData;
}

// Flush func for writing to file to verify Json output
uint8_t* FlushFunc(size_t size)
{
	fwrite(buffer, 1, size, s_pFile);
	return buffer;
}

void SetupTestData()
{
	sprintf(teststring,
		"I loved a maid as fair as \"summer\"\\ with sunlight in her hair\n"
		"\tI loved a maid as red as \"autumn\"\\ with sunset in her hair\n"
		"\t\tI loved a maid as white as \"winter\"/ with moonglow in her hair\r"
		"\t\t\tI loved a maid as spry as \"springtime\"/ with blossoms in her hair\r");

	testarray[0].dummy = 1111;
	testarray[1].dummy = 2222;
	testarray[2].dummy = 3333;

	testdata.testint = 32;
	testdata.testsigned = -23;
	testdata.testfloat = 45.678f;
	testdata.structcount = 3;
	testdata.teststruct = testarray;
	testdata.nullstruct = NULL;
	testdata.pointerstruct = &testarray[1];
	testdata.inlinestruct.dummy = 5678;
	testdata.TestString = teststring;
	testdata.testenum = SECOND_VAL;
}

void Reset()
{
	count = 0;
	s_pFile = 0;
	memset(&output, 0, sizeof(output));
}

void VerifyOutput()
{
	assert(output.testint == testdata.testint);
	assert(output.testsigned == testdata.testsigned);
	assert(output.testfloat == testdata.testfloat);
	assert(output.structcount == testdata.structcount);
	assert(output.teststruct[0].dummy == testdata.teststruct[0].dummy);
	assert(output.teststruct[1].dummy == testdata.teststruct[1].dummy);
	assert(output.teststruct[2].dummy == testdata.teststruct[2].dummy);
	assert(output.pointerstruct->dummy == testdata.pointerstruct->dummy);
	assert(output.inlinestruct.dummy == testdata.inlinestruct.dummy);
	assert(strcmp(output.TestString, testdata.TestString) == 0);
	assert(output.testenum == testdata.testenum);
	assert(output.nullstruct == testdata.nullstruct);
}

int main(int argc, char** argv)
{
	size_t heapSize;
	SetupTestData();
	// 3 is a great number for tests because it is odd, prime
	// and causes the most amount of work to be done. Horrible
	// from an optimization standpoint though.
	EgspSetBlockSize(3);

	Reset();
	EgspSaveTestStruct(LoadFunc, &testdata, &heapSize);

	Reset();
	void* pHeap = malloc(heapSize);
	EgspLoadTestStruct(LoadFunc, &output, pHeap, heapSize);
	VerifyOutput();
	free(pHeap);

#ifdef EGSP_JSON
	Reset();
	s_pFile = fopen("Test.json", "wb");
	EgspPrintTestStruct(FlushFunc, &testdata, &heapSize);
	fclose(s_pFile);

	Reset();
	EgspPrintTestStruct(LoadFunc, &testdata, &heapSize);

	Reset();
	pHeap = malloc(heapSize);
	EgspReadTestStruct(LoadFunc, &output, pHeap, heapSize);
	VerifyOutput();
	free(pHeap);
#endif
	return 0;
}
