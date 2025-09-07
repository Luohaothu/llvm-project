// ASan User States 违规检测演示
// 展示违规检测功能的使用方法

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_write_to_readonly() {
    printf("=== 演示1: 写入只读内存 ===\n");
    
    char* buffer = (char*)malloc(100);
    printf("1. 分配100字节内存\n");
    
    // 设置为只读状态
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RO);
    printf("2. 设置为只读状态\n");
    printf("   状态: %s\n", __asan_is_readonly(buffer) ? "只读" : "可写");
    
    // 启用用户状态检测
    __asan_set_detect_user_state(1);
    printf("3. 启用用户状态检测\n");
    
    printf("4. 尝试写入只读内存...\n");
    printf("   (这应该触发ASan违规检测)\n");
    
    // 这会触发违规检测
    buffer[0] = 'X';
    
    // 这行不会被执行
    printf("5. 程序应该已经终止，这行不会显示\n");
    
    free(buffer);
}

void demo_access_destroyed() {
    printf("=== 演示2: 访问已销毁内存 ===\n");
    
    char* buffer = (char*)malloc(100);
    printf("1. 分配100字节内存\n");
    
    // 设置为已销毁状态
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_DEAD);
    printf("2. 设置为已销毁状态\n");
    
    // 启用用户状态检测
    __asan_set_detect_user_state(1);
    printf("3. 启用用户状态检测\n");
    
    printf("4. 尝试读取已销毁内存...\n");
    printf("   (这应该触发ASan违规检测)\n");
    
    // 这会触发违规检测
    volatile char c = buffer[0];
    
    // 这行不会被执行
    printf("5. 程序应该已经终止，这行不会显示\n");
    
    free(buffer);
}

void demo_access_uninitialized() {
    printf("=== 演示3: 访问未初始化内存 ===\n");
    
    char* buffer = (char*)malloc(100);
    printf("1. 分配100字节内存\n");
    
    // 设置为未初始化状态
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_UNINIT);
    printf("2. 设置为未初始化状态\n");
    
    // 启用用户状态检测
    __asan_set_detect_user_state(1);
    printf("3. 启用用户状态检测\n");
    
    printf("4. 尝试读取未初始化内存...\n");
    printf("   (这应该触发ASan违规检测)\n");
    
    // 这会触发违规检测
    volatile char c = buffer[0];
    
    // 这行不会被执行
    printf("5. 程序应该已经终止，这行不会显示\n");
    
    free(buffer);
}

int main(int argc, char* argv[]) {
    printf("ASan User States 违规检测演示\n");
    printf("==============================\n\n");
    
    if (argc != 2) {
        printf("使用方法:\n");
        printf("  %s readonly   - 演示写入只读内存\n", argv[0]);
        printf("  %s destroyed  - 演示访问已销毁内存\n", argv[0]);
        printf("  %s uninit     - 演示访问未初始化内存\n", argv[0]);
        return 1;
    }
    
    if (strcmp(argv[1], "readonly") == 0) {
        demo_write_to_readonly();
    } else if (strcmp(argv[1], "destroyed") == 0) {
        demo_access_destroyed();
    } else if (strcmp(argv[1], "uninit") == 0) {
        demo_access_uninitialized();
    } else {
        printf("错误: 未知的演示类型\n");
        return 1;
    }
    
    return 0;
}