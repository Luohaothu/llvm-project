// Basic functionality tests for user states
// RUN: %clangxx_asan -O0 %s -o %t
// RUN: %env_asan_opts=detect_user_state=1 %run %t 2>&1 | FileCheck %s

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
    printf("=== Basic User State Test ===\n");
    
    char* buffer = (char*)malloc(100);
    
    // Test 1: Set and get basic state
    printf("Test 1: Basic state operations\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RW);
    
    asan_user_state_t state = __asan_get_memory_state(buffer);
    printf("Retrieved state: %d\n", state);
    
    // Test state checks
    printf("Is allocated: %s\n", __asan_is_allocated(buffer) ? "YES" : "NO");
    printf("Is initialized: %s\n", __asan_is_initialized(buffer) ? "YES" : "NO");
    printf("Is writable: %s\n", __asan_is_writable(buffer) ? "YES" : "NO");
    printf("Is read-only: %s\n", __asan_is_readonly(buffer) ? "YES" : "NO");
    
    // Test 2: State transition
    printf("\nTest 2: State transition\n");
    __asan_mark_readonly(buffer, 100);
    printf("After marking read-only:\n");
    printf("Is writable: %s\n", __asan_is_writable(buffer) ? "YES" : "NO");
    printf("Is read-only: %s\n", __asan_is_readonly(buffer) ? "YES" : "NO");
    
    // Test 3: State description
    printf("\nTest 3: State description\n");
    char desc[256];
    __asan_describe_state(state, desc, sizeof(desc));
    printf("State description: %s\n", desc);
    
    free(buffer);
    printf("=== Basic Test Completed ===\n");
    return 0;
}
// CHECK: === Basic User State Test ===
// CHECK: Test 1: Basic state operations
// CHECK: Retrieved state: [[STATE:[0-9]+]]
// CHECK: Is allocated: YES
// CHECK: Is initialized: YES
// CHECK: Is writable: YES
// CHECK: Is read-only: NO
// CHECK: Test 2: State transition
// CHECK: After marking read-only:
// CHECK: Is writable: NO
// CHECK: Is read-only: YES
// CHECK: Test 3: State description
// CHECK: State description: Lifecycle:Allocated, Init:Initialized, Recycle:Active, Access:ReadOnly, Track:NotTracked
// CHECK: === Basic Test Completed ===
