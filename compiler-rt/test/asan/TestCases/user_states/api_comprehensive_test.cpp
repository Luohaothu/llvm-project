// Test all user state API functions
// RUN: %clangxx_asan -O0 %s -o %t
// RUN: %env_asan_opts=user_state_detection=1 %run %t 2>&1 | FileCheck %s

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== API Comprehensive Test ===\n");
    
    char* buffer = (char*)malloc(100);
    
    // Test all convenience functions
    printf("Test 1: Convenience functions\n");
    __asan_mark_allocated(buffer, 100);
    printf("Allocated: %s\n", __asan_is_allocated(buffer) ? "YES" : "NO");
    
    __asan_mark_initialized(buffer, 100);
    printf("Initialized: %s\n", __asan_is_initialized(buffer) ? "YES" : "NO");
    
    __asan_mark_writable(buffer, 100);
    printf("Writable: %s\n", __asan_is_writable(buffer) ? "YES" : "NO");
    
    __asan_mark_tracked(buffer, 100);
    printf("Tracked: %s\n", __asan_is_tracked(buffer) ? "YES" : "NO");
    
    // Test memory_has_state function
    printf("\nTest 2: Memory state checking\n");
    asan_user_state_t test_state = (asan_user_state_t)(ASAN_USER_ALLOCATED | ASAN_USER_STATE_INITIALIZED | ASAN_USER_WRITABLE | ASAN_USER_STATE_TRACKED);
    int has_state = __asan_memory_has_state(buffer, 100, test_state);
    printf("Buffer has expected state: %s\n", has_state ? "YES" : "NO");
    
    // Test recycling
    printf("\nTest 3: Recycling operations\n");
    __asan_mark_recycled(buffer, 100);
    printf("Recycled: %s\n", __asan_is_recycled(buffer) ? "YES" : "NO");
    
    __asan_mark_untracked(buffer, 100);
    printf("Tracked after untracking: %s\n", __asan_is_tracked(buffer) ? "YES" : "NO");
    
    // Test state description for different states
    printf("\nTest 4: State descriptions\n");
    char desc1[256], desc2[256], desc3[256];
    
    __asan_describe_state((asan_user_state_t)ASAN_STATE_FRESH_ALLOC, desc1, sizeof(desc1));
    printf("Fresh allocation: %s\n", desc1);
    
    __asan_describe_state((asan_user_state_t)ASAN_STATE_READY_RW, desc2, sizeof(desc2));
    printf("Ready read-write: %s\n", desc2);
    
    __asan_describe_state((asan_user_state_t)ASAN_STATE_DEAD, desc3, sizeof(desc3));
    printf("Dead state: %s\n", desc3);
    
    free(buffer);
    printf("=== API Test Completed ===\n");
    return 0;
}

// CHECK: === API Comprehensive Test ===
// CHECK: Test 1: Convenience functions
// CHECK: Allocated: YES
// CHECK: Initialized: YES
// CHECK: Writable: YES
// CHECK: Tracked: YES
// CHECK: Test 2: Memory state checking
// CHECK: Buffer has expected state: YES
// CHECK: Test 3: Recycling operations
// CHECK: Recycled: YES
// CHECK: Tracked after untracking: NO
// CHECK: Test 4: State descriptions
// CHECK: Fresh allocation: Lifecycle:Allocated, Init:Uninitialized, Recycle:Active, Access:ReadOnly, Track:NotTracked
// CHECK: Ready read-write: Lifecycle:Allocated, Init:Initialized, Recycle:Active, Access:Writable, Track:NotTracked
// CHECK: Dead state: Lifecycle:Destroyed, Init:Uninitialized, Recycle:Active, Access:ReadOnly, Track:NotTracked
// CHECK: === API Test Completed ===