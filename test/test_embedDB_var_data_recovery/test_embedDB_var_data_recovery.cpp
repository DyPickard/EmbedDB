/******************************************************************************/
/**
 * @file        test_embedDB_var_data_recovery.cpp
 * @author      EmbedDB Team (See Authors.md)
 * @brief       Test EmbedDB variable length data recovery.
 * @copyright   Copyright 2024
 *              EmbedDB Team
 * @par Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 * @par 1.Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *
 * @par 2.Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *
 * @par 3.Neither the name of the copyright holder nor the names of its contributors
 *  may be used to endorse or promote products derived from this software without
 *  specific prior written permission.
 *
 * @par THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */
/******************************************************************************/

#ifdef DIST
#include "embedDB.h"
#else
#include "embedDB/embedDB.h"
#include "embedDBUtility.h"
#endif

#if defined(MEMBOARD)
#include "memboardTestSetup.h"
#endif

#if defined(MEGA)
#include "megaTestSetup.h"
#endif

#if defined(DUE)
#include "dueTestSetup.h"
#endif

#ifdef ARDUINO
#include "SDFileInterface.h"
#define getFileInterface getSDInterface
#define setupFile setupSDFile
#define tearDownFile tearDownSDFile
#define DATA_FILE_PATH "dataFile.bin"
#define INDEX_FILE_PATH "indexFile.bin"
#define VAR_DATA_FILE_PATH "varFile.bin"
#else
#include "desktopFileInterface.h"
#define DATA_FILE_PATH "build/artifacts/dataFile.bin"
#define INDEX_FILE_PATH "build/artifacts/indexFile.bin"
#define VAR_DATA_FILE_PATH "build/artifacts/varFile.bin"
/* On the desktop platform, there is a file interface which simulates "erasing" by writing out all 1's to the location in the file ot be erased */
#define MOCK_ERASE_INTERFACE
#endif

#include "unity.h"
#define UNITY_SUPPORT_64

embedDBState *state;

void setupEmbedDB() {
    state = (embedDBState *)malloc(sizeof(embedDBState));
    state->keySize = 4;
    state->dataSize = 4;
    state->pageSize = 512;
    state->bufferSizeInBlocks = 4;
    state->numSplinePoints = 8;
    state->buffer = calloc(1, state->pageSize * state->bufferSizeInBlocks);

    // Set active rules
    #define MAX_RULES 0
    state->rules = (activeRule**)calloc(MAX_RULES, sizeof(activeRule*));
    state->numRules = 0;

    TEST_ASSERT_NOT_NULL_MESSAGE(state->buffer, "Failed to allocate EmbedDB buffer.");

/* configure EmbedDB storage */
#ifdef MOCK_ERASE_INTERFACE
    state->fileInterface = getMockEraseFileInterface();
#else
    state->fileInterface = getFileInterface();
#endif
    char dataPath[] = DATA_FILE_PATH, varPath[] = VAR_DATA_FILE_PATH;
    state->dataFile = setupFile(dataPath);
    state->varFile = setupFile(varPath);

    state->numDataPages = 64;
    state->numVarPages = 76;
    state->eraseSizeInPages = 4;
    state->parameters = EMBEDDB_USE_VDATA | EMBEDDB_RESET_DATA;
    state->compareKey = int32Comparator;
    state->compareData = int32Comparator;
    int8_t result = embedDBInit(state, 1);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(0, result, "EmbedDB did not initialize correctly.");
}

void initalizeEmbedDBFromFile(void) {
    state = (embedDBState *)malloc(sizeof(embedDBState));
    state->keySize = 4;
    state->dataSize = 4;
    state->pageSize = 512;
    state->bufferSizeInBlocks = 4;
    state->numSplinePoints = 8;
    state->buffer = calloc(1, state->pageSize * state->bufferSizeInBlocks);
    TEST_ASSERT_NOT_NULL_MESSAGE(state->buffer, "Failed to allocate EmbedDB buffer.");

/* configure EmbedDB storage */
#ifdef MOCK_ERASE_INTERFACE
    state->fileInterface = getMockEraseFileInterface();
#else
    state->fileInterface = getFileInterface();
#endif
    char dataPath[] = DATA_FILE_PATH, varPath[] = VAR_DATA_FILE_PATH;
    state->dataFile = setupFile(dataPath);
    state->varFile = setupFile(varPath);

    state->numDataPages = 64;
    state->numVarPages = 76;
    state->eraseSizeInPages = 4;
    state->parameters = EMBEDDB_USE_VDATA;
    state->compareKey = int32Comparator;
    state->compareData = int32Comparator;
    int8_t result = embedDBInit(state, 1);
    TEST_ASSERT_EQUAL_INT8_MESSAGE(0, result, "Second EmbedDB did not initialize correctly.");
}

void tearDownEmbedDB() {
    embedDBClose(state);
    tearDownFile(state->dataFile);
    tearDownFile(state->varFile);
}

void setUp() {
    setupEmbedDB();
}

void tearDown() {
    free(state->buffer);
    free(state->fileInterface);
    free(state);
}

void insertRecords(int32_t numberOfRecords, int32_t startingKey, int32_t startingData) {
    int32_t key = startingKey;
    int32_t data = startingData;
    char variableData[13] = "Hello World!";
    for (int32_t i = 0; i < numberOfRecords; i++) {
        key += 1;
        data += 1;
        int8_t insertResult = embedDBPutVar(state, &key, &data, variableData, 13);
        TEST_ASSERT_EQUAL_INT8_MESSAGE(0, insertResult, "EmbedDB failed to insert data.");
    }
}

void embedDB_variable_data_page_numbers_are_correct() {
    insertRecords(1429, 1444, 64);
    /* Number of records * average data size % page size */
    uint32_t numberOfPagesExpected = 69;
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(numberOfPagesExpected - 1, state->nextVarPageId, "EmbedDB next variable data logical page number is incorrect.");
    uint32_t pageNumber;
    void *buffer = (int8_t *)state->buffer + state->pageSize * EMBEDDB_VAR_READ_BUFFER(state->parameters);
    for (uint32_t i = 0; i < numberOfPagesExpected - 1; i++) {
        readVariablePage(state, i);
        memcpy(&pageNumber, buffer, sizeof(id_t));
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(i, pageNumber, "EmbedDB variable data did not have the correct page number.");
    }
    tearDownEmbedDB();
}

void embedDB_variable_data_reloads_with_no_data_correctly() {
    tearDownEmbedDB();
    tearDown();
    initalizeEmbedDBFromFile();
    TEST_ASSERT_EQUAL_INT8_MESSAGE(8, state->variableDataHeaderSize, "EmbedDB variableDataHeaderSize did not have the correct value after initializing variable data from a file with no records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, state->currentVarLoc, "EmbedDB currentVarLoc did not have the correct value after initializing variable data from a file with no records.");
    uint64_t expectedMinVarRecordId = UINT64_MAX;
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expectedMinVarRecordId, &state->minVarRecordId, sizeof(uint64_t), "EmbedDB minVarRecordId did not have the correct value after initializing variable data from a file with no records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(76, state->numAvailVarPages, "EmbedDB numAvailVarPages did not have the correct value after initializing variable data from a file with no records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, state->nextVarPageId, "EmbedDB nextVarPageId did not have the correct value after initializing variable data from a file with no records.");
    tearDownEmbedDB();
}

void embedDB_variable_data_reloads_correctly_when_variable_records_are_written_but_no_data_records_are_written() {
    insertRecords(30, 100, 10);
    tearDownEmbedDB();
    tearDown();
    initalizeEmbedDBFromFile();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8, state->currentVarLoc, "EmbedDB currentVarLoc did not have the correct value after initializing variable data from a file with one page of records.");
    uint64_t expectedMinVarRecordId = UINT64_MAX;
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expectedMinVarRecordId, &state->minVarRecordId, sizeof(uint64_t), "EmbedDB minVarRecordId did not have the correct value after initializing variable data from a file with one page of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(76, state->numAvailVarPages, "EmbedDB numAvailVarPages did not have the correct value after initializing variable data from a file with one page of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(0, state->nextVarPageId, "EmbedDB nextVarPageId did not have the correct value after initializing variable data from a file with one page of records.");
    tearDownEmbedDB();
}

void embedDB_variable_data_reloads_with_one_page_of_data_correctly() {
    /* Insert records into state */
    insertRecords(43, 100, 10);

    /* Tear down this wall - Some American Politician */
    tearDownEmbedDB();
    tearDown();
    initalizeEmbedDBFromFile();

    /* Check that the state was setup correctly */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1032, state->currentVarLoc, "EmbedDB currentVarLoc did not have the correct value after initializing variable data from a file with one page of records.");
    uint32_t expectedMinVarRecordId = 101;
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expectedMinVarRecordId, &state->minVarRecordId, sizeof(uint32_t), "EmbedDB minVarRecordId did not have the correct value after initializing variable data from a file with one page of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(74, state->numAvailVarPages, "EmbedDB numAvailVarPages did not have the correct value after initializing variable data from a file with one page of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, state->nextVarPageId, "EmbedDB nextVarPageId did not have the correct value after initializing variable data from a file with one page of records.");
}

void embedDB_variable_data_reloads_with_sixteen_pages_of_data_correctly() {
    insertRecords(337, 1648, 10);
    tearDownEmbedDB();
    tearDown();
    initalizeEmbedDBFromFile();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(8200, state->currentVarLoc, "EmbedDB currentVarLoc did not have the correct value after initializing variable data from a file with one page of records.");
    uint64_t expectedMinVarRecordId = 1649;
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expectedMinVarRecordId, &state->minVarRecordId, sizeof(uint64_t), "EmbedDB minVarRecordId did not have the correct value after initializing variable data from a file with one page of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(60, state->numAvailVarPages, "EmbedDB numAvailVarPages did not have the correct value after initializing variable data from a file with one page of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(16, state->nextVarPageId, "EmbedDB nextVarPageId did not have the correct value after initializing variable data from a file with one page of records.");
    tearDownEmbedDB();
}

void embedDB_variable_data_reloads_with_fifty_three_pages_of_data_correctly() {
    insertRecords(2227, 100, 10);
    tearDownEmbedDB();
    tearDown();
    initalizeEmbedDBFromFile();
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(15368, state->currentVarLoc, "EmbedDB currentVarLoc did not have the correct value after initializing variable data from a file with 106 pages of records.");
    uint32_t expectedMinVarRecordId = 803;
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expectedMinVarRecordId, &state->minVarRecordId, sizeof(uint32_t), "EmbedDB minVarRecordId did not have the correct value after initializing variable data from a file with 106 pages of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(2, state->numAvailVarPages, "EmbedDB numAvailVarPages did not have the correct value after initializing variable data from a file with 106 pages of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(106, state->nextVarPageId, "EmbedDB nextVarPageId did not have the correct value after initializing variable data from a file with 106 pages of records.");
    tearDownEmbedDB();
}

void embedDB_variable_data_reloads_and_queries_with_thirty_one_pages_of_data_correctly() {
    int32_t key = 1000;
    int32_t data = 10;
    insertRecords(651, key, data);
    embedDBFlush(state);
    tearDownEmbedDB();
    tearDown();
    initalizeEmbedDBFromFile();
    int32_t recordData = 0;
    char variableData[13] = "Hello World!";
    char variableDataBuffer[13];
    char message[100];
    embedDBVarDataStream *stream = NULL;
    key = 1001;
    data = 11;
    /* Records inserted before reload */
    for (int i = 0; i < 650; i++) {
        int8_t getResult = embedDBGetVar(state, &key, &recordData, &stream);
        snprintf(message, 100, "EmbedDB get encountered an error fetching the data for key %li.", key);
        TEST_ASSERT_EQUAL_INT8_MESSAGE(0, getResult, message);
        uint32_t streamBytesRead = 0;
        snprintf(message, 100, "EmbedDB get var returned null stream for key %li.", key);
        TEST_ASSERT_NOT_NULL_MESSAGE(stream, message);
        streamBytesRead = embedDBVarDataStreamRead(state, stream, variableDataBuffer, 13);
        snprintf(message, 100, "EmbedDB get did not return correct data for a record inserted before reloading (key %li).", key);
        TEST_ASSERT_EQUAL_INT32_MESSAGE(data, recordData, message);
        TEST_ASSERT_EQUAL_UINT32_MESSAGE(13, streamBytesRead, "EmbedDB var data stream did not read the correct number of bytes.");
        snprintf(message, 100, "EmbedDB get var did not return the correct variable data for key %li.", key);
        TEST_ASSERT_EQUAL_MEMORY_MESSAGE(variableData, variableDataBuffer, 13, message);
        key++;
        data++;
        free(stream);
    }
    tearDownEmbedDB();
}

void embedDB_variable_data_reloads_and_queries_with_two_hundred_forty_seven_pages_of_data_correctly() {
    /* Insert records and restart state */
    int32_t key = 6798;
    int32_t data = 13467895;
    insertRecords(5187, key, data);
    embedDBFlush(state);
    tearDownEmbedDB();
    tearDown();
    initalizeEmbedDBFromFile();

    /* Check that the state was setup correctly */
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(9736, state->currentVarLoc, "EmbedDB currentVarLoc did not have the correct value after initializing variable data from a file with one page of records.");
    uint32_t expectedMinVarRecordId = 10441;
    TEST_ASSERT_EQUAL_MEMORY_MESSAGE(&expectedMinVarRecordId, &state->minVarRecordId, sizeof(uint32_t), "EmbedDB minVarRecordId did not have the correct value after initializing variable data from a file with one page of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(1, state->numAvailVarPages, "EmbedDB numAvailVarPages did not have the correct value after initializing variable data from a file with one page of records.");
    TEST_ASSERT_EQUAL_UINT32_MESSAGE(247, state->nextVarPageId, "EmbedDB nextVarPageId did not have the correct value after initializing variable data from a file with one page of records.");

    /* Query records */
    int32_t recordData = 0;
    char variableData[13] = "Hello World!";
    char variableDataBuffer[13];
    char message[120];
    embedDBVarDataStream *stream = NULL;
    key = 9487;
    data = 13470584;
    /* Records inserted before reload */
    for (int i = 0; i < 2499; i++) {
        int8_t getResult = embedDBGetVar(state, &key, &recordData, &stream);
        if (i > 953) {
            snprintf(message, 120, "EmbedDB get encountered an error fetching the data for key %li.", key);
            TEST_ASSERT_EQUAL_INT8_MESSAGE(0, getResult, message);
            snprintf(message, 120, "EmbedDB get did not return correct data for a record inserted before reloading (key %li).", key);
            TEST_ASSERT_EQUAL_INT32_MESSAGE(data, recordData, message);
            snprintf(message, 120, "EmbedDB get var returned null stream for key %li.", key);
            TEST_ASSERT_NOT_NULL_MESSAGE(stream, message);
            uint32_t streamBytesRead = embedDBVarDataStreamRead(state, stream, variableDataBuffer, 13);
            TEST_ASSERT_EQUAL_UINT32_MESSAGE(13, streamBytesRead, "EmbedDB var data stream did not read the correct number of bytes.");
            snprintf(message, 120, "EmbedDB get var did not return the correct variable data for key %li.", key);
            TEST_ASSERT_EQUAL_MEMORY_MESSAGE(variableData, variableDataBuffer, 13, message);
            free(stream);
        } else {
            snprintf(message, 120, "EmbedDB get encountered an error fetching the data for key %li. The var data was not detected as being overwritten.", key);
            TEST_ASSERT_EQUAL_INT8_MESSAGE(1, getResult, message);
            snprintf(message, 120, "EmbedDB get did not return correct data for a record inserted before reloading (key %li).", key);
            TEST_ASSERT_EQUAL_INT32_MESSAGE(data, recordData, message);
            snprintf(message, 120, "EmbedDB get var did not return null stream for key %li when it should have no variable data.", key);
            TEST_ASSERT_NULL_MESSAGE(stream, message);
        }
        key++;
        data++;
    }
    tearDownEmbedDB();
}

int runUnityTests() {
    UNITY_BEGIN();
    RUN_TEST(embedDB_variable_data_page_numbers_are_correct);
    RUN_TEST(embedDB_variable_data_reloads_with_no_data_correctly);
    RUN_TEST(embedDB_variable_data_reloads_correctly_when_variable_records_are_written_but_no_data_records_are_written);
    RUN_TEST(embedDB_variable_data_reloads_with_one_page_of_data_correctly);
    RUN_TEST(embedDB_variable_data_reloads_with_sixteen_pages_of_data_correctly);
    RUN_TEST(embedDB_variable_data_reloads_with_fifty_three_pages_of_data_correctly);
    RUN_TEST(embedDB_variable_data_reloads_and_queries_with_thirty_one_pages_of_data_correctly);
    RUN_TEST(embedDB_variable_data_reloads_and_queries_with_two_hundred_forty_seven_pages_of_data_correctly);
    return UNITY_END();
}

#ifdef ARDUINO

void setup() {
    delay(2000);
    setupBoard();
    runUnityTests();
}

void loop() {}

#else

int main() {
    return runUnityTests();
}

#endif
