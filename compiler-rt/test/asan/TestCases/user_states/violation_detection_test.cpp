// Violation detection tests for user states
// RUN: %clangxx_asan -O0 %s -o %t
// RUN: %env_asan_opts=detect_user_state=1 not %run %t 2>&1 | FileCheck %s

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== Violation Detection Test ===\n");
    
    char* buffer = (char*)malloc(100);
    
    // Set buffer to read-only state
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RO);
    
    printf("Buffer set to read-only state\n");
    printf("Attempting to write to read-only memory...\n");
    
    // This should trigger a user state violation
    buffer[0] = 'x';  // Writing to read-only memory
    
    free(buffer);
    printf("This should not be reached\n");
    return 0;
}
// CHECK: === Violation Detection Test ===
// CHECK: Buffer set to read-only state
// CHECK: Attempting to write to read-only memory...
// CHECK: ERROR: AddressSanitizer: user-state-violation
// CHECK: Violation: Writing to read-only memory
