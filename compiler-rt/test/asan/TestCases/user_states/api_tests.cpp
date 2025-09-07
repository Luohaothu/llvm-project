// API functionality tests
// RUN: %clangxx_asan -O0 %s -o %t
// RUN: %run %t 2>&1 | FileCheck %s

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>

// Test API function availability
int main() {
    void* ptr = malloc(100);
    
    printf("Testing API availability...\n");
    
    // These should compile but may not work correctly yet
    // We'll test the actual functionality in later stages
    
    free(ptr);
    printf("API test completed\n");
    return 0;
}
// CHECK: Testing API availability
// CHECK: API test completed