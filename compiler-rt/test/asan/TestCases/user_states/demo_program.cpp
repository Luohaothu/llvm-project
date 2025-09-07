// ASan User States 功能演示程序
// 展示所有用户状态功能的使用方法

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demonstrate_basic_operations() {
    printf("=== 基础操作演示 ===\n");
    
    // 分配内存
    char* buffer = (char*)malloc(100);
    printf("1. 分配了100字节内存\n");
    
    // 设置初始状态：已分配 + 可读写
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RW);
    printf("2. 设置状态为 READY_RW (已分配+已初始化+可读写)\n");
    
    // 检查状态
    asan_user_state_t state = __asan_get_memory_state(buffer);
    printf("3. 获取状态值: %d (二进制: ", state);
    for (int i = 7; i >= 0; i--) {
        printf("%d", (state >> i) & 1);
    }
    printf(")\n");
    
    // 使用便利函数检查状态
    printf("4. 使用便利函数检查:\n");
    printf("   - 是否已分配: %s\n", __asan_is_allocated(buffer) ? "YES" : "NO");
    printf("   - 是否已初始化: %s\n", __asan_is_initialized(buffer) ? "YES" : "NO");
    printf("   - 是否可写: %s\n", __asan_is_writable(buffer) ? "YES" : "NO");
    printf("   - 是否只读: %s\n", __asan_is_readonly(buffer) ? "YES" : "NO");
    printf("   - 是否被跟踪: %s\n", __asan_is_tracked(buffer) ? "YES" : "NO");
    
    // 状态转换
    printf("\n5. 状态转换演示:\n");
    printf("   - 转换为只读状态...\n");
    __asan_mark_readonly(buffer, 100);
    printf("   - 转换后: 可写=%s, 只读=%s\n", 
           __asan_is_writable(buffer) ? "YES" : "NO",
           __asan_is_readonly(buffer) ? "YES" : "NO");
    
    // 获取状态描述
    char desc[256];
    __asan_describe_state(__asan_get_memory_state(buffer), desc, sizeof(desc));
    printf("6. 状态描述: %s\n", desc);
    
    free(buffer);
    printf("7. 释放内存\n\n");
}

void demonstrate_memory_lifecycle() {
    printf("=== 内存生命周期演示 ===\n");
    
    char* buffer = (char*)malloc(100);
    
    // 阶段1: 新分配
    printf("阶段1: 新分配内存\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_FRESH_ALLOC);
    char desc[256];
    __asan_describe_state(__asan_get_memory_state(buffer), desc, sizeof(desc));
    printf("状态: %s\n", desc);
    
    // 阶段2: 初始化
    printf("\n阶段2: 初始化内存\n");
    // 先设置为可写状态
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RW);
    // 然后进行初始化
    memset(buffer, 0, 100);  // 实际初始化
    __asan_describe_state(__asan_get_memory_state(buffer), desc, sizeof(desc));
    printf("状态: %s\n", desc);
    
    // 阶段3: 标记为只读
    printf("\n阶段3: 标记为只读\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RO);
    __asan_describe_state(__asan_get_memory_state(buffer), desc, sizeof(desc));
    printf("状态: %s\n", desc);
    
    // 阶段4: 回收
    printf("\n阶段4: 标记为回收\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_RECYCLED);
    __asan_describe_state(__asan_get_memory_state(buffer), desc, sizeof(desc));
    printf("状态: %s\n", desc);
    
    // 阶段5: 销毁
    printf("\n阶段5: 销毁内存\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_DEAD);
    __asan_describe_state(__asan_get_memory_state(buffer), desc, sizeof(desc));
    printf("状态: %s\n", desc);
    
    free(buffer);
    printf("\n");
}

void demonstrate_violation_detection() {
    printf("=== 违规检测演示 ===\n");
    printf("注意: 这个测试会触发 ASan 违规检测并终止程序\n");
    printf("这是预期行为，证明违规检测功能正常工作\n\n");
    
    char* buffer = (char*)malloc(100);
    
    // 设置为只读状态
    printf("1. 设置内存为只读状态\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RO);
    printf("2. 状态: 只读\n");
    
    // 尝试写入 - 这会触发违规检测
    printf("3. 尝试写入只读内存...\n");
    printf("   这应该触发 ASan 违规检测\n");
    
    // 启用用户状态检测
    __asan_set_detect_user_state(1);
    
    // 这行代码会触发违规检测
    buffer[0] = 'X';  // 写入只读内存
    
    // 这行不会被执行
    printf("4. 这行不会被执行，因为程序已经终止\n");
    
    free(buffer);
}

void demonstrate_state_combinations() {
    printf("=== 状态组合演示 ===\n");
    
    char* buffer = (char*)malloc(100);
    
    // 演示不同的状态组合
    struct {
        const char* name;
        asan_user_state_t state;
    } test_states[] = {
        {"新分配", (asan_user_state_t)ASAN_STATE_FRESH_ALLOC},
        {"未初始化", (asan_user_state_t)ASAN_STATE_UNINIT},
        {"就绪读写", (asan_user_state_t)ASAN_STATE_READY_RW},
        {"就绪只读", (asan_user_state_t)ASAN_STATE_READY_RO},
        {"已回收", (asan_user_state_t)ASAN_STATE_RECYCLED},
        {"已销毁", (asan_user_state_t)ASAN_STATE_DEAD},
        {"已跟踪", (asan_user_state_t)ASAN_STATE_TRACKED_RW}
    };
    
    for (int i = 0; i < sizeof(test_states) / sizeof(test_states[0]); i++) {
        printf("%d. %s:\n", i + 1, test_states[i].name);
        __asan_set_memory_state(buffer, 100, test_states[i].state);
        
        char desc[256];
        __asan_describe_state(__asan_get_memory_state(buffer), desc, sizeof(desc));
        printf("   状态值: %d\n", test_states[i].state);
        printf("   描述: %s\n", desc);
        
        // 显示各个标志位
        asan_user_state_t state = __asan_get_memory_state(buffer);
        printf("   标志位: ");
        if (state & ASAN_USER_ALLOCATED) printf("已分配 ");
        if (state & ASAN_USER_STATE_INITIALIZED) printf("已初始化 ");
        if (state & ASAN_USER_RESERVED) printf("已回收 ");
        if (state & ASAN_USER_WRITABLE) printf("可写 ");
        if (state & ASAN_USER_STATE_TRACKED) printf("已跟踪 ");
        printf("\n\n");
    }
    
    free(buffer);
}

int main() {
    printf("ASan User States 功能演示\n");
    printf("==========================\n\n");
    
    // 演示基础操作
    demonstrate_basic_operations();
    
    // 演示内存生命周期
    demonstrate_memory_lifecycle();
    
    // 演示状态组合
    demonstrate_state_combinations();
    
    // 演示违规检测 (这会终止程序)
    // demonstrate_violation_detection();
    
    printf("=== 演示完成 ===\n");
    printf("要测试违规检测，请单独运行: ./violation_demo\n");
    
    return 0;
}