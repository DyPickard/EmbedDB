#ifndef UNITY_H
#define UNITY_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef __cplusplus
extern "C" {
#endif

extern int UnityFailures;
void setUp(void);
void tearDown(void);

#define UNITY_BEGIN() (UnityFailures = 0)
#define UNITY_END() (UnityFailures)

#define UNITY_FAIL(msg) do { printf("FAIL: %s\n", msg); UnityFailures++; } while(0)

#define RUN_TEST(fn) do { \
    printf("Running %s...\n", #fn); \
    setUp(); \
    fn(); \
    tearDown(); \
} while (0)

#define TEST_ASSERT_TRUE(condition) if(!(condition)) UNITY_FAIL(#condition)
#define TEST_ASSERT(condition) TEST_ASSERT_TRUE(condition)
#define TEST_ASSERT_NOT_NULL_MESSAGE(p,msg) if((p)==NULL){UNITY_FAIL(msg);} 
#define TEST_ASSERT_NULL_MESSAGE(p,msg) if((p)!=NULL){UNITY_FAIL(msg);} 
#define TEST_ASSERT_EQUAL_INT_MESSAGE(exp,act,msg) if((int)(exp)!=(int)(act)){printf("%s Expected %d Was %d\n",msg,(int)(exp),(int)(act)); UnityFailures++;}
#define TEST_ASSERT_EQUAL_INT(exp,act) TEST_ASSERT_EQUAL_INT_MESSAGE(exp,act,#exp " != " #act)
#define TEST_ASSERT_EQUAL(exp,act) TEST_ASSERT_EQUAL_INT_MESSAGE(exp,act,#exp " != " #act)
#define TEST_ASSERT_EQUAL_FLOAT(exp,act) TEST_ASSERT_EQUAL_FLOAT_MESSAGE(exp,act,#exp " != " #act)
#define TEST_ASSERT_EQUAL_UINT32(exp,act) TEST_ASSERT_EQUAL_UINT32_MESSAGE(exp,act,#exp " != " #act)
#define TEST_ASSERT_EQUAL_INT8_MESSAGE(exp,act,msg) TEST_ASSERT_EQUAL_INT_MESSAGE(exp,act,msg)
#define TEST_ASSERT_EQUAL_INT16_MESSAGE(exp,act,msg) TEST_ASSERT_EQUAL_INT_MESSAGE(exp,act,msg)
#define TEST_ASSERT_EQUAL_INT32_MESSAGE(exp,act,msg) if((int32_t)(exp)!=(int32_t)(act)){printf("%s Expected %d Was %d\n",msg,(int32_t)(exp),(int32_t)(act)); UnityFailures++;}
#define TEST_ASSERT_EQUAL_INT32(exp,act) TEST_ASSERT_EQUAL_INT32_MESSAGE(exp,act,#exp " != " #act)
#define TEST_ASSERT_EQUAL_UINT_MESSAGE(exp,act,msg) if((unsigned)(exp)!=(unsigned)(act)){printf("%s Expected %u Was %u\n",msg,(unsigned)(exp),(unsigned)(act)); UnityFailures++;}
#define TEST_ASSERT_EQUAL_UINT8_MESSAGE(exp,act,msg) TEST_ASSERT_EQUAL_UINT_MESSAGE(exp,act,msg)
#define TEST_ASSERT_EQUAL_UINT16_MESSAGE(exp,act,msg) TEST_ASSERT_EQUAL_UINT_MESSAGE(exp,act,msg)
#define TEST_ASSERT_EQUAL_UINT32_MESSAGE(exp,act,msg) if((uint32_t)(exp)!=(uint32_t)(act)){printf("%s Expected %u Was %u\n",msg,(uint32_t)(exp),(uint32_t)(act)); UnityFailures++;}
#define TEST_ASSERT_GREATER_OR_EQUAL_UINT32_MESSAGE(exp,act,msg) if((uint32_t)(act) < (uint32_t)(exp)){printf("%s Expected >=%u Was %u\n",msg,(uint32_t)(exp),(uint32_t)(act)); UnityFailures++;}
#define TEST_ASSERT_LESS_OR_EQUAL_UINT32_MESSAGE(exp,act,msg) if((uint32_t)(act) > (uint32_t)(exp)){printf("%s Expected <=%u Was %u\n",msg,(uint32_t)(exp),(uint32_t)(act)); UnityFailures++;}
#define TEST_ASSERT_EQUAL_FLOAT_MESSAGE(exp,act,msg) if(fabs((double)(exp)-(double)(act))>1e-6){printf("%s Expected %f Was %f\n",msg,(double)(exp),(double)(act)); UnityFailures++;}
#define TEST_ASSERT_EQUAL_MEMORY_MESSAGE(exp,act,len,msg) if(memcmp((exp),(act),(len))){UNITY_FAIL(msg);} 
#define TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE(exp,act,len,msg) TEST_ASSERT_EQUAL_MEMORY_MESSAGE(exp,act,len,msg)
#define TEST_ASSERT_EACH_EQUAL_CHAR_MESSAGE(exp,act,len,msg) TEST_ASSERT_EQUAL_MEMORY_MESSAGE(exp,act,len,msg)
#define TEST_ASSERT_EQUAL_UINT32_ARRAY_MESSAGE(exp,act,len,msg) if(memcmp((exp),(act),(len)*sizeof(uint32_t))){UNITY_FAIL(msg);}
#define TEST_ASSERT_EQUAL_INT8_ARRAY_MESSAGE(exp,act,len,msg) if(memcmp((exp),(act),(len)*sizeof(int8_t))){UNITY_FAIL(msg);}
#define TEST_ASSERT_EQUAL_MEMORY(exp,act,len) TEST_ASSERT_EQUAL_MEMORY_MESSAGE(exp,act,len,"Memory not equal")

#ifdef __cplusplus
}
#endif
#endif
