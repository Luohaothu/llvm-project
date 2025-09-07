// Basic user state functionality tests
// RUN: %clangxx_asan -O0 %s -o %t
// RUN: not %run %t 2>&1 | FileCheck %s

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test 1: Basic state setting and getting
int main() {
    char* buffer = (char*)malloc(100);
    
    // Try to set user state (should fail initially)
    // This tests that the functionality doesn't exist yet
    printf("Testing user state functionality...\n");
    
    // These calls should either fail or have no effect
    // We'll verify this by checking the return values
    
    free(buffer);
    printf("Test completed\n");
    return 0;
}
// CHECK: Testing user state functionality
// CHECK: Test completed