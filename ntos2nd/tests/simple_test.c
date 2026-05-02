/*
    ntos2nd - Simple Test Runner
    
    Runs basic kernel functionality tests.
    
    (C) 2025 OakyMacintosh
*/

#include <windows.h>
#include <stdio.h>
#include "../ntos2nd/kernel.h"
#include "../ntos2nd/apibridge.h"
#include "../ntos2nd/undocumented.h"
#include "../ntos2nd/syscall.h"  /* For handle management functions */

static int g_TestCount = 0;
static int g_PassCount = 0;

#define TEST_START(name) printf("[TEST] %s... ", name); g_TestCount++
#define TEST_PASS() printf("PASSED\n"); g_PassCount++
#define TEST_FAIL(msg) printf("FAILED: %s\n", msg)
#define TEST_RESULT() printf("\nResults: %d/%d tests passed\n", g_PassCount, g_TestCount)

void TestKernelInit(void) {
    TEST_START("Kernel Initialization");
    
    NTSTATUS status = NTOS2NDInitialize();
    if (NT_SUCCESS(status)) {
        TEST_PASS();
    } else {
        TEST_FAIL("Init failed");
    }
    
    NTOS2NDShutdown();
}

void TestAPIBridge(void) {
    TEST_START("API Bridge Resolution");
    
    void* addr = NTOS2NDResolveAPI("CreateFileA");
    if (addr != NULL) {
        TEST_PASS();
    } else {
        TEST_FAIL("Could not resolve CreateFileA");
    }
}

void TestUndocumentedAPI(void) {
    TEST_START("Undocumented API Resolution");
    
    NTSTATUS status = NTOS2NDInitializeUndocumentedAPI();
    if (NT_SUCCESS(status)) {
        TEST_PASS();
    } else {
        TEST_FAIL("Failed to init undocumented layer");
    }
    
    void* addr = NTOS2NDResolveUndocumentedAPI("NtClose");
    if (addr != NULL) {
        printf(" (resolved at %p)", addr);
        TEST_PASS();
    } else {
        TEST_FAIL("Could not resolve NtClose");
    }
}

void TestHandleManagement(void) {
    TEST_START("Handle Management");
    
    /* Test handle creation */
    void* testPtr = (void*)0x12345678;
    uint32_t handle = NTOS2NDCreateHandle(NTOS2ND_HANDLE_TYPE_FILE, testPtr);
    
    if (handle != 0 && NTOS2NDIsValidHandle(handle)) {
        if (NTOS2NDGetHandlePointer(handle) == testPtr) {
            NTOS2NDCloseHandle(handle);
            TEST_PASS();
        } else {
            TEST_FAIL("Handle pointer mismatch");
        }
    } else {
        TEST_FAIL("Handle creation failed");
    }
}

int main(void) {
    printf("ntos2nd Test Suite\n");
    printf("==================\n\n");
    
    TestKernelInit();
    TestAPIBridge();
    TestUndocumentedAPI();
    TestHandleManagement();
    
    printf("\n");
    TEST_RESULT();
    
    return g_PassCount == g_TestCount ? 0 : 1;
}