// Test different violation types
// RUN: %clangxx_asan -O0 %s -o %t1 -DTEST_DESTROYED
// RUN: %env_asan_opts=user_state_detection=1 not %run %t1 2>&1 | FileCheck %s --check-prefix=CHECK-DESTROYED
// RUN: %clangxx_asan -O0 %s -o %t2 -DTEST_UNINITIALIZED
// RUN: %env_asan_opts=user_state_detection=1 not %run %t2 2>&1 | FileCheck %s --check-prefix=CHECK-UNINITIALIZED
// RUN: %clangxx_asan -O0 %s -o %t3 -DTEST_RECYCLED
// RUN: %env_asan_opts=user_state_detection=1 not %run %t3 2>&1 | FileCheck %s --check-prefix=CHECK-RECYCLED

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    char* buffer = (char*)malloc(100);
    __asan_set_detect_user_state(1);
    
    #ifdef TEST_DESTROYED
    printf("=== Testing Destroyed Memory Access ===\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_DEAD);
    printf("Attempting to access destroyed memory...\n");
    volatile char c = buffer[0]; // Read from destroyed memory
    
    #elif defined(TEST_UNINITIALIZED)
    printf("=== Testing Uninitialized Memory Access ===\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_UNINIT);
    printf("Attempting to access uninitialized memory...\n");
    volatile char c = buffer[0]; // Read from uninitialized memory
    
    #elif defined(TEST_RECYCLED)
    printf("=== Testing Recycled Memory Access ===\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_RECYCLED);
    printf("Attempting to access recycled memory...\n");
    volatile char c = buffer[0]; // Read from recycled memory
    
    #else
    printf("No test case defined\n");
    #endif
    
    free(buffer);
    printf("This should not be reached\n");
    return 0;
}

// CHECK-DESTROYED: === Testing Destroyed Memory Access ===
// CHECK-DESTROYED: Attempting to access destroyed memory...
// CHECK-DESTROYED: ERROR: AddressSanitizer: user-state-violation
// CHECK-DESTROYED: Violation: Accessing destroyed memory

// CHECK-UNINITIALIZED: === Testing Uninitialized Memory Access ===
// CHECK-UNINITIALIZED: Attempting to access uninitialized memory...
// CHECK-UNINITIALIZED: ERROR: AddressSanitizer: user-state-violation
// CHECK-UNINITIALIZED: Violation: Accessing uninitialized memory

// CHECK-RECYCLED: === Testing Recycled Memory Access ===
// CHECK-RECYCLED: Attempting to access recycled memory...
// CHECK-RECYCLED: ERROR: AddressSanitizer: user-state-violation
// CHECK-RECYCLED: Violation: Accessing recycled memory