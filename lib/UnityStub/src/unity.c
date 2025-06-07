#include "unity.h"

int UnityFailures = 0;

void UnityAssertEqualMemory(const void* expected, const void* actual, size_t len, const char* msg)
{
    if (expected == NULL) {
        if (actual == NULL) {
            return; /* both NULL considered equal */
        }
        const unsigned char* a = (const unsigned char*)actual;
        for (size_t i = 0; i < len; i++) {
            if (a[i] != 0) {
                UNITY_FAIL(msg);
                return;
            }
        }
        return;
    }
    if (actual == NULL || memcmp(expected, actual, len) != 0) {
        UNITY_FAIL(msg);
    }
}
