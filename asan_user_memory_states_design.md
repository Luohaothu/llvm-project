# ASan用户自定义内存状态管理方案设计

## 目录

1. [概述](#1-概述)
2. [状态设计架构](#2-状态设计架构)
3. [API接口设计](#3-api接口设计)
4. [实现细节](#4-实现细节)
5. [使用示例](#5-使用示例)
6. [测试方案](#6-测试方案)
7. [性能考虑](#7-性能考虑)
8. [集成路线图](#8-集成路线图)
9. [风险和限制](#9-风险和限制)
10. [结论](#10-结论)

## 1. 概述

### 1.1 项目背景
AddressSanitizer (ASan) 是一个强大的内存错误检测工具，通过shadow memory机制跟踪内存状态。目前ASan使用固定的标记值来表示不同的内存状态（如堆红区、栈红区、已释放内存等），但缺乏让用户自定义内存状态的能力。

### 1.2 设计目标
- 允许用户定义和管理自定义的内存访问状态
- 提供细粒度的内存状态控制能力
- 保持与现有ASan系统的完全兼容性
- 最小化对现有性能的影响
- 提供直观的API接口和错误报告

### 1.3 应用场景
- 自定义内存池管理
- 对象生命周期跟踪
- 权限控制和访问管理
- 调试和性能分析

## 2. 状态设计架构

### 2.1 内存状态定义
用户内存包含以下几种状态，采用位标志设计：

```cpp
// 用户内存状态标志位定义
typedef enum {
    // 生命周期状态
    ASAN_USER_ALLOCATED   = 0x01,  // 0=已销毁, 1=已开辟
    
    // 初始化状态
    ASAN_USER_STATE_INITIALIZED = 0x02,  // 0=未初始化, 1=已初始化
    
    // 回收状态
    ASAN_USER_RESERVED    = 0x04,  // 0=未回收, 1=已回收
    
    // 访问权限状态
    ASAN_USER_WRITABLE      = 0x08,  // 0=只读, 1=可写
    
    // 跟踪状态
    ASAN_USER_STATE_TRACKED     = 0x10,  // 0=未跟踪, 1=跟踪
    
    // 保留位
    ASAN_USER_STATE_RESERVED1   = 0x20,
    ASAN_USER_STATE_RESERVED2   = 0x40,
    ASAN_USER_STATE_RESERVED3   = 0x80
} asan_user_state_t;
```

### 2.2 状态设计特点
- **位标志设计**: 使用单个bit表示互斥状态（如开辟-销毁、只读-可写）
- **可扩展性**: 预留保留位用于未来扩展
- **高效性**: 位运算操作，性能开销最小
- **兼容性**: 不影响现有ASan功能

## 3. API接口设计

### 3.1 状态查询宏定义

```cpp
// 生命周期状态解释
#define ASAN_IS_ALLOCATED(state)    ((state) & ASAN_USER_ALLOCATED)
#define ASAN_IS_DESTROYED(state)    (!((state) & ASAN_USER_ALLOCATED))

// 初始化状态解释  
#define ASAN_IS_INITIALIZED(state)  ((state) & ASAN_USER_STATE_INITIALIZED)
#define ASAN_IS_UNINITIALIZED(state) (!((state) & ASAN_USER_STATE_INITIALIZED))

// 回收状态解释
#define ASAN_IS_RECYCLED(state)     ((state) & ASAN_USER_RESERVED)
#define ASAN_IS_NOT_RECYCLED(state) (!((state) & ASAN_USER_RESERVED))

// 访问权限状态解释
#define ASAN_IS_WRITABLE(state)     ((state) & ASAN_USER_WRITABLE)
#define ASAN_IS_READONLY(state)     (!((state) & ASAN_USER_WRITABLE))

// 跟踪状态解释
#define ASAN_IS_TRACKED(state)      ((state) & ASAN_USER_STATE_TRACKED)
#define ASAN_IS_NOT_TRACKED(state)  (!((state) & ASAN_USER_STATE_TRACKED))
```

### 3.2 状态设置宏定义

```cpp
// 设置生命周期状态
#define ASAN_SET_ALLOCATED(state)   ((state) |= ASAN_USER_ALLOCATED)
#define ASAN_SET_DESTROYED(state)   ((state) &= ~ASAN_USER_ALLOCATED)

// 设置初始化状态
#define ASAN_SET_INITIALIZED(state)   ((state) |= ASAN_USER_STATE_INITIALIZED)  
#define ASAN_SET_UNINITIALIZED(state) ((state) &= ~ASAN_USER_STATE_INITIALIZED)

// 设置回收状态
#define ASAN_SET_RECYCLED(state)     ((state) |= ASAN_USER_RESERVED)
#define ASAN_SET_NOT_RECYCLED(state) ((state) &= ~ASAN_USER_RESERVED)

// 设置访问权限状态
#define ASAN_SET_WRITABLE(state)     ((state) |= ASAN_USER_WRITABLE)
#define ASAN_SET_READONLY(state)     ((state) &= ~ASAN_USER_WRITABLE)

// 设置跟踪状态
#define ASAN_SET_TRACKED(state)      ((state) |= ASAN_USER_STATE_TRACKED)
#define ASAN_SET_NOT_TRACKED(state)  ((state) &= ~ASAN_USER_STATE_TRACKED)
```

### 3.3 核心API函数

```cpp
// 基础状态操作
void __asan_set_memory_state(void* addr, size_t size, asan_user_state_t state);
asan_user_state_t __asan_get_memory_state(void* addr);
bool __asan_memory_has_state(void* addr, size_t size, asan_user_state_t state);

// 便利的状态查询函数
bool __asan_is_allocated(void* addr);
bool __asan_is_destroyed(void* addr);
bool __asan_is_initialized(void* addr);
bool __asan_is_recycled(void* addr);
bool __asan_is_writable(void* addr);
bool __asan_is_readonly(void* addr);
bool __asan_is_tracked(void* addr);

// 状态转换函数
void __asan_mark_allocated(void* addr, size_t size);
void __asan_mark_destroyed(void* addr, size_t size);
void __asan_mark_initialized(void* addr, size_t size);
void __asan_mark_recycled(void* addr, size_t size);
void __asan_mark_writable(void* addr, size_t size);
void __asan_mark_readonly(void* addr, size_t size);
void __asan_mark_tracked(void* addr, size_t size);
void __asan_mark_untracked(void* addr, size_t size);

// 状态描述（调试用）
const char* __asan_describe_state(asan_user_state_t state, char* buffer, size_t buffer_size);
```

### 3.4 预定义状态组合

```cpp
// 常用状态组合
#define ASAN_STATE_FRESH_ALLOC      (ASAN_USER_ALLOCATED)  
// 已开辟，未初始化，未回收，只读，未跟踪

#define ASAN_STATE_UNINIT (ASAN_USER_ALLOCATED | ASAN_USER_WRITABLE)
// 已开辟，未初始化，等待写入

#define ASAN_STATE_READY_RW         (ASAN_USER_ALLOCATED | ASAN_USER_STATE_INITIALIZED | ASAN_USER_WRITABLE)  
// 已开辟，已初始化，可写

#define ASAN_STATE_READY_RO         (ASAN_USER_ALLOCATED | ASAN_USER_STATE_INITIALIZED)  
// 已开辟，已初始化，只读

#define ASAN_STATE_RECYCLED            (ASAN_USER_ALLOCATED | ASAN_USER_RESERVED)  
// 已开辟，已回收

#define ASAN_STATE_DEAD             (0x00)  
// 已销毁，未初始化，未回收，只读，未跟踪

#define ASAN_STATE_TRACKED_RW       (ASAN_STATE_READY_RW | ASAN_USER_STATE_TRACKED)  
// 可读写且跟踪
```

## 4. 实现细节

### 4.1 Shadow Memory映射方案

#### 4.1.1 现有ASan魔数值分析
现有ASan已占用的shadow值：
- `0x00`: 正常内存（未poisoned）
- `0x01-0x07`: 部分可访问内存
- `0xac`: kAsanArrayCookieMagic
- `0xbb`: kAsanIntraObjectRedzone  
- `0xca, 0xcb`: kAsanAllocaLeftMagic, kAsanAllocaRightMagic
- `0xf1-0xf3, 0xf5-0xf9, 0xfa, 0xfc-0xfe`: 各种ASan内部魔数

#### 4.1.2 用户状态映射
选择未被ASan占用的连续区间 `0x80-0x9F` (32个值)，直接进行数学映射：

```cpp
// 用户状态shadow memory映射
#define ASAN_USER_STATE_SHADOW_BASE 0x80
#define ASAN_USER_STATE_SHADOW_MAX  0x9F

// 将用户状态编码到shadow memory (直接数学映射)
static inline u8 encode_user_state_to_shadow(asan_user_state_t state) {
    // 限制状态值在有效范围内 (0-31)，然后加上基址
    return ASAN_USER_STATE_SHADOW_BASE + ((u8)state & 0x1F);
}

// 从shadow memory解码用户状态 (直接数学映射)
static inline asan_user_state_t decode_user_state_from_shadow(u8 shadow_value) {
    // 检查是否在用户状态范围内
    if (shadow_value < ASAN_USER_STATE_SHADOW_BASE || 
        shadow_value > ASAN_USER_STATE_SHADOW_MAX) {
        return 0;  // 不是用户状态
    }
    return (asan_user_state_t)(shadow_value - ASAN_USER_STATE_SHADOW_BASE);
}

// 检查是否为用户状态shadow值
static inline bool is_user_state_shadow(u8 shadow_value) {
    return shadow_value >= ASAN_USER_STATE_SHADOW_BASE && 
           shadow_value <= ASAN_USER_STATE_SHADOW_MAX;
}
```

### 4.2 核心实现函数

```cpp
// 设置内存区域的用户状态
void __asan_set_memory_state(void* addr, size_t size, asan_user_state_t state) {
    uptr a = (uptr)addr;
    u8 shadow_value = encode_user_state_to_shadow(state);
    
    // 使用现有的PoisonShadow机制
    PoisonShadow(a, size, shadow_value);
    
    // 记录状态变更（如果启用了poison_history）
    if (flags()->poison_history_size > 0) {
        PoisonRecord record = {a, size, shadow_value, GET_CURRENT_PC(), 
                              StackDepotPut(*stack)};
        AddPoisonRecord(record);
    }
}

// 获取地址的用户状态
asan_user_state_t __asan_get_memory_state(void* addr) {
    uptr a = (uptr)addr;
    u8* shadow_addr = (u8*)MemToShadow(a);
    u8 shadow_value = *shadow_addr;
    
    return decode_user_state_from_shadow(shadow_value);
}

// 检查内存区域是否具有指定状态
bool __asan_memory_has_state(void* addr, size_t size, asan_user_state_t state) {
    uptr a = (uptr)addr;
    u8 expected_shadow = encode_user_state_to_shadow(state);
    
    for (uptr i = 0; i < size; ++i) {
        u8* shadow_addr = (u8*)MemToShadow(a + i);
        if (*shadow_addr != expected_shadow) {
            return false;
        }
    }
    return true;
}
```

### 4.3 用户状态检测机制

#### 4.3.1 利用现有ASan插桩
通过扩展现有的`__asan_load*`和`__asan_store*`函数实现用户状态检测：

```cpp
// 快速用户状态检测函数
static inline bool FastCheckUserStateAccess(uptr addr, bool is_write) {
    u8 shadow_value = *(volatile u8*)MemToShadow(addr);
    
    // 快速范围检查 - 使用编译时常量
    if (shadow_value < ASAN_USER_STATE_SHADOW_BASE || 
        shadow_value > ASAN_USER_STATE_SHADOW_MAX) {
        return true;  // 不是用户状态，快速返回
    }
    
    // 解码状态并检查
    asan_user_state_t state = (asan_user_state_t)(shadow_value - ASAN_USER_STATE_SHADOW_BASE);
    
    // 位运算检查
    if (!(state & ASAN_USER_ALLOCATED)) return false;  // 已销毁
    if (!(state & ASAN_USER_STATE_INITIALIZED)) return false;  // 未初始化
    if (state & ASAN_USER_RESERVED) return false;  // 已回收
    if (is_write && !(state & ASAN_USER_WRITABLE)) return false;  // 只读写入
    
    return true;
}
```

#### 4.3.2 修改现有ASan访存函数

**重要发现**：ASan中的小尺寸load/store函数（1, 2, 4, 8, 16字节）并没有转发到loadN/storeN函数上，而是通过宏 `ASAN_MEMORY_ACCESS_CALLBACK` 生成独立的实现。

##### 4.3.2.1 修改小尺寸访存函数

需要修改 `asan_rtl.cpp` 中的 `ASAN_MEMORY_ACCESS_CALLBACK_BODY` 宏定义：

```cpp
// 在 asan_rtl.cpp 中修改 ASAN_MEMORY_ACCESS_CALLBACK_BODY 宏
#define ASAN_MEMORY_ACCESS_CALLBACK_BODY(type, is_write, size, exp_arg, fatal) \
  /* 新增：用户状态检测 */                                                \
  if (ShouldCheckUserState() && !FastCheckUserStateAccess(addr, is_write)) { \
    ReportUserStateViolation(addr, size, is_write, *(u8*)MemToShadow(addr)); \
    return;                                                                 \
  }                                                                          \
                                                                             \
  /* 原有的ASan检测逻辑 */                                                  \
  uptr sp = MEM_TO_SHADOW(addr);                                            \
  uptr s = size <= ASAN_SHADOW_GRANULARITY ? *reinterpret_cast<u8 *>(sp)      \
                                           : *reinterpret_cast<u16 *>(sp);    \
  if (UNLIKELY(s)) {                                                         \
    if (UNLIKELY(size >= ASAN_SHADOW_GRANULARITY ||                           \
                 ((s8)((addr & (ASAN_SHADOW_GRANULARITY - 1)) + size - 1)) >= \
                     (s8)s)) {                                                \
      ReportGenericErrorWrapper(addr, is_write, size, exp_arg, fatal);        \
    }                                                                        \
  }
```

##### 4.3.2.2 修改变长访存函数

同时需要修改 `__asan_loadN` 和 `__asan_storeN` 函数：

```cpp
// 修改 __asan_loadN 函数
void __asan_loadN(uptr addr, uptr size) {
    // 新增：用户状态检测（检查首尾和采样点）
    if (ShouldCheckUserState()) {
        // 检查第一个和最后一个字节
        if (!FastCheckUserStateAccess(addr, false) ||
            !FastCheckUserStateAccess(addr + size - 1, false)) {
            ReportUserStateViolation(addr, size, false, *(u8*)MemToShadow(addr));
            return;
        }
        
        // 对于大块内存，采样检查中间位置
        if (size > 16) {
            const uptr stride = 1024; // 1KB采样间隔
            for (uptr i = stride; i < size - 1; i += stride) {
                if (!FastCheckUserStateAccess(addr + i, false)) {
                    ReportUserStateViolation(addr + i, 1, false, 
                                             *(u8*)MemToShadow(addr + i));
                    return;
                }
            }
        }
    }
    
    // 原有的ASan检测逻辑
    if ((addr = __asan_region_is_poisoned(addr, size))) {
        GET_CALLER_PC_BP_SP;
        ReportGenericError(pc, bp, sp, addr, false, size, 0, true);
    }
}

// 修改 __asan_storeN 函数
void __asan_storeN(uptr addr, uptr size) {
    // 新增：用户状态检测（检查首尾和采样点）
    if (ShouldCheckUserState()) {
        // 检查第一个和最后一个字节
        if (!FastCheckUserStateAccess(addr, true) ||
            !FastCheckUserStateAccess(addr + size - 1, true)) {
            ReportUserStateViolation(addr, size, true, *(u8*)MemToShadow(addr));
            return;
        }
        
        // 对于大块内存，采样检查中间位置
        if (size > 16) {
            const uptr stride = 1024; // 1KB采样间隔
            for (uptr i = stride; i < size - 1; i += stride) {
                if (!FastCheckUserStateAccess(addr + i, true)) {
                    ReportUserStateViolation(addr + i, 1, true, 
                                             *(u8*)MemToShadow(addr + i));
                    return;
                }
            }
        }
    }
    
    // 原有的ASan检测逻辑
    if ((addr = __asan_region_is_poisoned(addr, size))) {
        GET_CALLER_PC_BP_SP;
        ReportGenericError(pc, bp, sp, addr, true, size, 0, true);
    }
}
```

##### 4.3.2.3 修改汇编优化函数

如果存在汇编优化的实现（如 `asan_rtl_x86_64.S`），也需要相应修改，或者在编译时禁用汇编优化，强制使用C++实现。

### 4.4 运行时控制

#### 4.4.1 添加运行时开关

```cpp
// 在 asan_flags.cpp 中添加
ASAN_FLAG(bool, user_state_detection, false,
          "Enable user-defined memory state detection")

// 在 asan_flags.h 中添加
DECLARE_FLAG(bool, user_state_detection);

// 检查是否启用用户状态检测
static inline bool ShouldCheckUserState() {
    return flags()->user_state_detection;
}
```

#### 4.4.2 启用方式

```bash
# 编译时启用
export ASAN_OPTIONS=detect_user_state=1
clang -fsanitize=address source.cpp -o program

# 运行时启用
export ASAN_OPTIONS=detect_user_state=1
./program
```

### 4.5 错误报告增强

```cpp
// 用户状态违规报告函数
void ReportUserStateViolation(uptr addr, uptr access_size, bool is_write,
                              u8 shadow_value) {
    asan_user_state_t current_state = decode_user_state_from_shadow(shadow_value);
    char state_desc[256];
    __asan_describe_state(current_state, state_desc, sizeof(state_desc));
    
    Printf("ERROR: AddressSanitizer: user-state-violation on address %p\n", addr);
    Printf("  %s of size %zu at %p\n", is_write ? "WRITE" : "READ", access_size, addr);
    Printf("  Current memory state: %s (shadow=0x%02x)\n", state_desc, shadow_value);
    
    // 检查具体的违规类型
    if (ASAN_IS_DESTROYED(current_state)) {
        Printf("  Violation: Accessing destroyed memory\n");
    } else if (ASAN_IS_READONLY(current_state) && is_write) {
        Printf("  Violation: Writing to read-only memory\n");
    } else if (ASAN_IS_UNINITIALIZED(current_state)) {
        Printf("  Violation: Accessing uninitialized memory\n");
    } else if (ASAN_IS_RECYCLED(current_state)) {
        Printf("  Violation: Accessing recycled memory\n");
    }
    
    // 打印调用栈
    GET_STACK_TRACE(kStackTraceMax, kStackTraceMax);
    stack.Print();
}
```

## 5. 使用示例

### 5.1 基础使用示例

```cpp
#include <sanitizer/asan_interface.h>

int main() {
    // 分配内存
    char* buffer = malloc(1024);
    
    // 1. 设置为已开辟状态
    asan_user_state_t state = 0;
    ASAN_SET_ALLOCATED(state);
    __asan_set_memory_state(buffer, 1024, state);
    
    // 2. 初始化内存
    memset(buffer, 0, 1024);
    ASAN_SET_INITIALIZED(state);
    ASAN_SET_WRITABLE(state);
    __asan_set_memory_state(buffer, 1024, state);
    
    // 3. 使用内存
    strcpy(buffer, "Hello, World!");
    
    // 4. 转换为只读状态
    ASAN_SET_READONLY(state);
    __asan_set_memory_state(buffer, 1024, state);
    
    // 5. 尝试写入只读内存（将触发错误）
    // buffer[0] = 'h';  // 这将被检测为违规
    
    // 6. 标记为已回收
    ASAN_SET_RECYCLED(state);
    __asan_set_memory_state(buffer, 1024, state);
    
    // 7. 最终销毁
    ASAN_SET_DESTROYED(state);
    __asan_set_memory_state(buffer, 1024, state);
    
    free(buffer);
    return 0;
}
```

### 5.2 自定义内存池示例

```cpp
typedef struct {
    void* memory;
    size_t size;
    asan_user_state_t state;
} memory_pool_block_t;

class CustomMemoryPool {
private:
    memory_pool_block_t* blocks;
    size_t block_count;
    
public:
    void* allocate(size_t size) {
        memory_pool_block_t* block = find_free_block(size);
        if (!block) return nullptr;
        
        // 设置为已分配和可写状态
        block->state = 0;
        ASAN_SET_ALLOCATED(block->state);
        ASAN_SET_WRITABLE(block->state);
        ASAN_SET_TRACKED(block->state);  // 启用跟踪
        
        __asan_set_memory_state(block->memory, block->size, block->state);
        return block->memory;
    }
    
    void deallocate(void* ptr) {
        memory_pool_block_t* block = find_block(ptr);
        if (!block) return;
        
        // 标记为已回收，只读状态
        ASAN_SET_RECYCLED(block->state);
        ASAN_SET_READONLY(block->state);
        
        __asan_set_memory_state(block->memory, block->size, block->state);
    }
    
    bool is_valid_access(void* ptr, size_t size, bool is_write) {
        asan_user_state_t state = __asan_get_memory_state(ptr);
        
        // 检查是否已销毁
        if (ASAN_IS_DESTROYED(state)) {
            return false;
        }
        
        // 检查写访问是否被允许
        if (is_write && ASAN_IS_READONLY(state)) {
            return false;
        }
        
        // 检查是否已被回收
        if (ASAN_IS_RECYCLED(state)) {
            return false;
        }
        
        return true;
    }
};
```

### 5.3 对象生命周期管理示例

```cpp
template<typename T>
class TrackedObject {
private:
    T* object;
    
public:
    TrackedObject() {
        object = new T;
        
        // 设置初始状态：已分配，未初始化，可写，跟踪
        asan_user_state_t state = 0;
        ASAN_SET_ALLOCATED(state);
        ASAN_SET_WRITABLE(state);
        ASAN_SET_TRACKED(state);
        
        __asan_set_memory_state(object, sizeof(T), state);
    }
    
    void initialize() {
        asan_user_state_t state = __asan_get_memory_state(object);
        ASAN_SET_INITIALIZED(state);
        __asan_set_memory_state(object, sizeof(T), state);
    }
    
    void make_readonly() {
        asan_user_state_t state = __asan_get_memory_state(object);
        ASAN_SET_READONLY(state);
        __asan_set_memory_state(object, sizeof(T), state);
    }
    
    ~TrackedObject() {
        // 标记为已销毁
        asan_user_state_t state = 0;
        ASAN_SET_DESTROYED(state);
        __asan_set_memory_state(object, sizeof(T), state);
        
        delete object;
    }
    
    T* get() { 
        // 检查访问合法性
        asan_user_state_t state = __asan_get_memory_state(object);
        if (ASAN_IS_DESTROYED(state) || ASAN_IS_RECYCLED(state)) {
            // 报告错误或抛出异常
            abort();
        }
        return object; 
    }
};
```

## 6. 测试方案

### 6.1 单元测试

```cpp
// 基础状态操作测试
TEST(AsanUserStatesTest, BasicStateOperations) {
    void* ptr = malloc(100);
    
    // 测试状态设置和查询
    __asan_set_memory_state(ptr, 100, ASAN_STATE_READY_RW);
    
    EXPECT_TRUE(__asan_is_allocated(ptr));
    EXPECT_TRUE(__asan_is_initialized(ptr));
    EXPECT_TRUE(__asan_is_writable(ptr));
    EXPECT_FALSE(__asan_is_destroyed(ptr));
    EXPECT_FALSE(__asan_is_readonly(ptr));
    
    // 测试状态转换
    __asan_mark_readonly(ptr, 100);
    EXPECT_TRUE(__asan_is_readonly(ptr));
    EXPECT_FALSE(__asan_is_writable(ptr));
    
    free(ptr);
}

// 互反状态测试
TEST(AsanUserStatesTest, MutuallyExclusiveStates) {
    void* ptr = malloc(100);
    
    // 设置为已分配
    __asan_mark_allocated(ptr, 100);
    EXPECT_TRUE(__asan_is_allocated(ptr));
    EXPECT_FALSE(__asan_is_destroyed(ptr));
    
    // 转换为已销毁
    __asan_mark_destroyed(ptr, 100);
    EXPECT_FALSE(__asan_is_allocated(ptr));
    EXPECT_TRUE(__asan_is_destroyed(ptr));
    
    free(ptr);
}

// 状态描述测试
TEST(AsanUserStatesTest, StateDescription) {
    char buffer[512];
    asan_user_state_t state = ASAN_STATE_READY_RW | ASAN_USER_STATE_TRACKED;
    
    const char* desc = __asan_describe_state(state, buffer, sizeof(buffer));
    
    EXPECT_STREQ(desc, "Lifecycle:Allocated, Init:Initialized, Recycle:Active, Access:Writable, Track:Tracked");
}
```

### 6.2 集成测试

```cpp
// 错误检测测试
TEST(AsanUserStatesTest, ErrorDetection) {
    void* ptr = malloc(100);
    
    // 设置为只读
    __asan_mark_readonly(ptr, 100);
    
    // 尝试写入应该触发错误
    EXPECT_DEATH({
        char* cptr = (char*)ptr;
        cptr[0] = 'x';  // 应该被检测为写入只读内存
    }, "user-state-violation.*Writing to read-only memory");
    
    free(ptr);
}

// 状态一致性测试
TEST(AsanUserStatesTest, StateConsistency) {
    void* ptr = malloc(1000);
    
    // 设置整个区域为相同状态
    __asan_set_memory_state(ptr, 1000, ASAN_STATE_READY_RO);
    
    // 检查区域内所有地址的状态
    EXPECT_TRUE(__asan_memory_has_state(ptr, 1000, ASAN_STATE_READY_RO));
    
    // 部分区域状态更改
    __asan_set_memory_state((char*)ptr + 100, 200, ASAN_STATE_READY_RW);
    
    // 验证状态边界
    EXPECT_TRUE(__asan_memory_has_state(ptr, 100, ASAN_STATE_READY_RO));
    EXPECT_TRUE(__asan_memory_has_state((char*)ptr + 100, 200, ASAN_STATE_READY_RW));
    EXPECT_TRUE(__asan_memory_has_state((char*)ptr + 300, 700, ASAN_STATE_READY_RO));
    
    free(ptr);
}
```

### 6.3 性能测试

```cpp
// 性能基准测试
TEST(AsanUserStatesTest, PerformanceBenchmark) {
    const size_t num_allocations = 10000;
    const size_t allocation_size = 1024;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < num_allocations; ++i) {
        void* ptr = malloc(allocation_size);
        
        // 设置用户状态
        __asan_set_memory_state(ptr, allocation_size, ASAN_STATE_READY_RW);
        
        // 模拟状态查询
        asan_user_state_t state = __asan_get_memory_state(ptr);
        EXPECT_TRUE(ASAN_IS_ALLOCATED(state));
        
        // 状态转换
        __asan_mark_readonly(ptr, allocation_size);
        
        free(ptr);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // 确保性能在可接受范围内（具体阈值需要根据实际情况确定）
    EXPECT_LT(duration.count(), 100000);  // 100ms for 10k operations
}
```

## 7. 性能考虑

### 7.1 内存开销
- **Shadow Memory**: 每个用户状态占用1字节shadow memory，与现有ASan相同
- **状态注册**: 用户状态枚举和宏定义不占用运行时内存
- **历史记录**: 如果启用poison_history，每次状态变更会记录历史（可选）

### 7.2 运行时开销
- **状态设置**: 与现有`PoisonShadow`函数相同的开销
- **状态查询**: 单次内存访问，开销极小
- **位运算**: 状态检查和设置使用位运算，CPU开销很小

### 7.3 优化策略
1. **批量操作**: 对大内存区域的状态设置使用批量操作
2. **缓存友好**: 利用现有shadow memory的缓存特性
3. **编译时优化**: 状态检查宏可以被编译器内联优化
4. **可选功能**: 用户状态功能可以通过编译标志控制启用/禁用

### 7.4 性能优化技巧

```cpp
// 快速路径优化
static inline bool FastCheckUserStateAccess(uptr addr, bool is_write) {
    // 单次内存访问获取shadow值
    u8 shadow_value = *(volatile u8*)MemToShadow(addr);
    
    // 快速范围检查 - 使用编译时常量
    if (shadow_value < ASAN_USER_STATE_SHADOW_BASE || 
        shadow_value > ASAN_USER_STATE_SHADOW_MAX) {
        return true;  // 不是用户状态，快速返回
    }
    
    // 解码状态并检查
    asan_user_state_t state = (asan_user_state_t)(shadow_value - ASAN_USER_STATE_SHADOW_BASE);
    
    // 位运算检查
    if (!(state & ASAN_USER_ALLOCATED)) return false;  // 已销毁
    if (!(state & ASAN_USER_STATE_INITIALIZED)) return false;  // 未初始化
    if (state & ASAN_USER_RESERVED) return false;  // 已回收
    if (is_write && !(state & ASAN_USER_WRITABLE)) return false;  // 只读写入
    
    return true;
}

// 条件编译优化
#ifdef ASAN_USER_STATES_ENABLED
    #define USER_STATE_CHECK(addr, size, is_write) \
        if (ShouldCheckUserState() && !FastCheckUserStateAccess(addr, is_write)) { \
            ReportUserStateViolation(addr, size, is_write, *(u8*)MemToShadow(addr)); \
            return; \
        }
#else
    #define USER_STATE_CHECK(addr, size, is_write) do {} while(0)
#endif
```

## 8. 集成路线图

### 8.1 第一阶段：核心功能实现
- [ ] 定义用户状态枚举和宏
- [ ] 实现基础API函数
- [ ] 扩展shadow memory编码/解码
- [ ] 基础单元测试

### 8.2 第二阶段：错误检测增强
- [ ] 增强错误报告系统
- [ ] 实现访问权限检查
- [ ] 添加状态违规检测
- [ ] 错误检测测试用例

### 8.3 第三阶段：API完善
- [ ] 添加便利函数和宏
- [ ] 实现状态描述功能
- [ ] 添加批量操作支持
- [ ] API文档和示例

### 8.4 第四阶段：性能优化
- [ ] 性能基准测试
- [ ] 优化关键路径
- [ ] 内存使用优化
- [ ] 编译时开关支持

### 8.5 第五阶段：集成测试
- [ ] 大规模集成测试
- [ ] 与现有ASan功能兼容性测试
- [ ] 真实应用场景测试
- [ ] 文档和用户指南

## 9. 风险和限制

### 9.1 兼容性风险
- **Shadow Memory空间**: 需要确保用户状态值不与现有ASan magic值冲突
- **API稳定性**: 新增API需要保持向后兼容

### 9.2 性能风险
- **频繁状态变更**: 大量状态变更操作可能影响性能
- **内存碎片**: 细粒度状态管理可能导致shadow memory访问模式变化

### 9.3 使用限制
- **并发安全**: 状态变更操作不是线程安全的，需要用户层同步
- **内存对齐**: 状态设置遵循ASan的内存对齐要求
- **调试模式**: 某些功能可能只在调试模式下可用

### 9.4 实现限制
- **编译器依赖**: 需要支持ASan的编译器版本
- **平台支持**: 依赖于ASan的跨平台支持
- **内存消耗**: 增加的shadow memory使用

## 10. 结论

本方案为ASan提供了强大的用户自定义内存状态管理能力，通过位标志设计实现了高效的状态表示和操作。方案的主要优势包括：

1. **完全兼容**: 利用现有ASan插桩，不影响任何现有功能
2. **性能可控**: 通过运行时开关控制，禁用时零开销
3. **实现简单**: 只需要修改几个现有函数，添加少量代码
4. **易于部署**: 可以作为ASan的扩展功能直接使用
5. **可扩展性**: 预留保留位，支持未来功能扩展

通过分阶段实施，可以逐步构建完整的用户内存状态管理系统，为内存安全检测和调试提供更加精细的控制能力。该方案通过扩展现有ASan插桩函数的方式，实现了用户状态检测功能，避免了复杂的编译器修改，是最实用的实现方案。