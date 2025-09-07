// Performance tests for user states
// RUN: %clangxx_asan -O0 %s -o %t
// RUN: %env_asan_opts=detect_user_state=1 %run %t 2>&1 | FileCheck %s

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define NUM_OPERATIONS 10000
#define ALLOCATION_SIZE 1024

int main() {
    printf("=== Performance Test ===\n");
    
    clock_t start = clock();
    
    for (int i = 0; i < NUM_OPERATIONS; i++) {
        char* buffer = (char*)malloc(ALLOCATION_SIZE);
        
        // Set user state
        __asan_set_memory_state(buffer, ALLOCATION_SIZE, (asan_user_state_t)ASAN_STATE_READY_RW);
        
        // Perform some operations
        asan_user_state_t state = __asan_get_memory_state(buffer);
        
        // State transition
        __asan_mark_readonly(buffer, ALLOCATION_SIZE);
        
        free(buffer);
    }
    
    clock_t end = clock();
    double elapsed = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Completed %d operations in %.3f seconds\n", NUM_OPERATIONS, elapsed);
    printf("Average time per operation: %.3f microseconds\n", 
           (elapsed * 1000000) / NUM_OPERATIONS);
    
    // Performance should be reasonable (less than 100 microseconds per operation)
    if (elapsed < 1.0) {
        printf("Performance test PASSED\n");
    } else {
        printf("Performance test FAILED (too slow)\n");
    }
    
    printf("=== Performance Test Completed ===\n");
    return 0;
}
// CHECK: === Performance Test ===
// CHECK: Completed [[NUM:[0-9]+]] operations in [[TIME:[0-9.]+]] seconds
// CHECK: Average time per operation: [[AVG:[0-9.]+]] microseconds
// CHECK: Performance test PASSED
// CHECK: === Performance Test Completed ===
