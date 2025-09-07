// ASan User States 简化演示程序
// 展示核心功能的使用方法

#include <sanitizer/asan_interface.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    printf("=== ASan User States 功能演示 ===\n\n");
    
    // 1. 基础状态操作
    printf("1. 基础状态操作演示:\n");
    char* buffer = (char*)malloc(100);
    
    // 设置为就绪读写状态
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RW);
    
    // 检查状态
    printf("   状态值: %d\n", __asan_get_memory_state(buffer));
    printf("   已分配: %s\n", __asan_is_allocated(buffer) ? "YES" : "NO");
    printf("   已初始化: %s\n", __asan_is_initialized(buffer) ? "YES" : "NO");
    printf("   可写: %s\n", __asan_is_writable(buffer) ? "YES" : "NO");
    printf("   只读: %s\n", __asan_is_readonly(buffer) ? "YES" : "NO");
    printf("   被跟踪: %s\n", __asan_is_tracked(buffer) ? "YES" : "NO");
    
    // 2. 状态转换
    printf("\n2. 状态转换演示:\n");
    printf("   转换前: 可写=%s, 只读=%s\n", 
           __asan_is_writable(buffer) ? "YES" : "NO",
           __asan_is_readonly(buffer) ? "YES" : "NO");
    
    __asan_mark_readonly(buffer, 100);
    
    printf("   转换后: 可写=%s, 只读=%s\n", 
           __asan_is_writable(buffer) ? "YES" : "NO",
           __asan_is_readonly(buffer) ? "YES" : "NO");
    
    // 3. 状态描述
    printf("\n3. 状态描述演示:\n");
    char desc[256];
    __asan_describe_state(__asan_get_memory_state(buffer), desc, sizeof(desc));
    printf("   当前状态描述: %s\n", desc);
    
    // 4. 不同状态组合
    printf("\n4. 不同状态组合演示:\n");
    
    const char* state_names[] = {
        "新分配", "未初始化", "就绪读写", "就绪只读", "已回收", "已销毁"
    };
    
    asan_user_state_t states[] = {
        (asan_user_state_t)ASAN_STATE_FRESH_ALLOC,
        (asan_user_state_t)ASAN_STATE_UNINIT,
        (asan_user_state_t)ASAN_STATE_READY_RW,
        (asan_user_state_t)ASAN_STATE_READY_RO,
        (asan_user_state_t)ASAN_STATE_RECYCLED,
        (asan_user_state_t)ASAN_STATE_DEAD
    };
    
    for (int i = 0; i < 6; i++) {
        __asan_set_memory_state(buffer, 100, states[i]);
        __asan_describe_state(states[i], desc, sizeof(desc));
        printf("   %s: %s\n", state_names[i], desc);
    }
    
    // 5. 内存状态检查
    printf("\n5. 内存状态检查演示:\n");
    __asan_set_memory_state(buffer, 100, (asan_user_state_t)ASAN_STATE_READY_RW);
    
    asan_user_state_t target_state = (asan_user_state_t)(
        ASAN_USER_ALLOCATED | ASAN_USER_STATE_INITIALIZED | ASAN_USER_WRITABLE
    );
    
    int has_state = __asan_memory_has_state(buffer, 100, target_state);
    printf("   缓冲区具有目标状态: %s\n", has_state ? "YES" : "NO");
    
    // 6. 回收和跟踪操作
    printf("\n6. 回收和跟踪操作演示:\n");
    printf("   回收前: 已回收=%s\n", __asan_is_recycled(buffer) ? "YES" : "NO");
    
    __asan_mark_recycled(buffer, 100);
    printf("   回收后: 已回收=%s\n", __asan_is_recycled(buffer) ? "YES" : "NO");
    
    __asan_mark_tracked(buffer, 100);
    printf("   跟踪后: 被跟踪=%s\n", __asan_is_tracked(buffer) ? "YES" : "NO");
    
    __asan_mark_untracked(buffer, 100);
    printf("   取消跟踪后: 被跟踪=%s\n", __asan_is_tracked(buffer) ? "YES" : "NO");
    
    free(buffer);
    
    printf("\n=== 演示完成 ===\n");
    printf("所有功能正常工作！\n");
    
    return 0;
}