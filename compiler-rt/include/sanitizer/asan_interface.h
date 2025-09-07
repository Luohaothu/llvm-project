//===-- sanitizer/asan_interface.h ------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer (ASan).
//
// Public interface header.
//===----------------------------------------------------------------------===//
#ifndef SANITIZER_ASAN_INTERFACE_H
#define SANITIZER_ASAN_INTERFACE_H

#include <sanitizer/common_interface_defs.h>

#ifdef __cplusplus
extern "C" {
#endif
/// Marks a memory region (<c>[addr, addr+size)</c>) as unaddressable.
///
/// This memory must be previously allocated by your program. Instrumented
/// code is forbidden from accessing addresses in this region until it is
/// unpoisoned. This function is not guaranteed to poison the entire region -
/// it could poison only a subregion of <c>[addr, addr+size)</c> due to ASan
/// alignment restrictions.
///
/// \note This function is not thread-safe because no two threads can poison or
/// unpoison memory in the same memory region simultaneously.
///
/// \param addr Start of memory region.
/// \param size Size of memory region.
void SANITIZER_CDECL __asan_poison_memory_region(void const volatile *addr,
                                                 size_t size);

/// Marks a memory region (<c>[addr, addr+size)</c>) as addressable.
///
/// This memory must be previously allocated by your program. Accessing
/// addresses in this region is allowed until this region is poisoned again.
/// This function could unpoison a super-region of <c>[addr, addr+size)</c> due
/// to ASan alignment restrictions.
///
/// \note This function is not thread-safe because no two threads can
/// poison or unpoison memory in the same memory region simultaneously.
///
/// \param addr Start of memory region.
/// \param size Size of memory region.
void SANITIZER_CDECL __asan_unpoison_memory_region(void const volatile *addr,
                                                   size_t size);

// Macros provided for convenience.
#ifdef __has_feature
#if __has_feature(address_sanitizer)
#define ASAN_DEFINE_REGION_MACROS
#endif
#elif defined(__SANITIZE_ADDRESS__)
#define ASAN_DEFINE_REGION_MACROS
#endif

#ifdef ASAN_DEFINE_REGION_MACROS
/// Marks a memory region as unaddressable.
///
/// \note Macro provided for convenience; defined as a no-op if ASan is not
/// enabled.
///
/// \param addr Start of memory region.
/// \param size Size of memory region.
#define ASAN_POISON_MEMORY_REGION(addr, size)                                  \
  __asan_poison_memory_region((addr), (size))

/// Marks a memory region as addressable.
///
/// \note Macro provided for convenience; defined as a no-op if ASan is not
/// enabled.
///
/// \param addr Start of memory region.
/// \param size Size of memory region.
#define ASAN_UNPOISON_MEMORY_REGION(addr, size)                                \
  __asan_unpoison_memory_region((addr), (size))
#else
#define ASAN_POISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#define ASAN_UNPOISON_MEMORY_REGION(addr, size) ((void)(addr), (void)(size))
#endif
#undef ASAN_DEFINE_REGION_MACROS

/// Checks if an address is poisoned.
///
/// Returns 1 if <c><i>addr</i></c> is poisoned (that is, 1-byte read/write
/// access to this address would result in an error report from ASan).
/// Otherwise returns 0.
///
/// \param addr Address to check.
///
/// \retval 1 Address is poisoned.
/// \retval 0 Address is not poisoned.
int SANITIZER_CDECL __asan_address_is_poisoned(void const volatile *addr);

/// Checks if a region is poisoned.
///
/// If at least one byte in <c>[beg, beg+size)</c> is poisoned, returns the
/// address of the first such byte. Otherwise returns 0.
///
/// \param beg Start of memory region.
/// \param size Start of memory region.
/// \returns Address of first poisoned byte.
void *SANITIZER_CDECL __asan_region_is_poisoned(void *beg, size_t size);

/// Describes an address (useful for calling from the debugger).
///
/// Prints the description of <c><i>addr</i></c>.
///
/// \param addr Address to describe.
void SANITIZER_CDECL __asan_describe_address(void *addr);

/// Checks if an error has been or is being reported (useful for calling from
/// the debugger to get information about an ASan error).
///
/// Returns 1 if an error has been (or is being) reported. Otherwise returns 0.
///
/// \returns 1 if an error has been (or is being) reported. Otherwise returns
/// 0.
int SANITIZER_CDECL __asan_report_present(void);

/// Gets the PC (program counter) register value of an ASan error (useful for
/// calling from the debugger).
///
/// Returns PC if an error has been (or is being) reported.
/// Otherwise returns 0.
///
/// \returns PC value.
void *SANITIZER_CDECL __asan_get_report_pc(void);

/// Gets the BP (base pointer) register value of an ASan error (useful for
/// calling from the debugger).
///
/// Returns BP if an error has been (or is being) reported.
/// Otherwise returns 0.
///
/// \returns BP value.
void *SANITIZER_CDECL __asan_get_report_bp(void);

/// Gets the SP (stack pointer) register value of an ASan error (useful for
/// calling from the debugger).
///
/// If an error has been (or is being) reported, returns SP.
/// Otherwise returns 0.
///
/// \returns SP value.
void *SANITIZER_CDECL __asan_get_report_sp(void);

/// Gets the address of the report buffer of an ASan error (useful for calling
/// from the debugger).
///
/// Returns the address of the report buffer if an error has been (or is being)
/// reported. Otherwise returns 0.
///
/// \returns Address of report buffer.
void *SANITIZER_CDECL __asan_get_report_address(void);

/// Gets access type of an ASan error (useful for calling from the debugger).
///
/// Returns access type (read or write) if an error has been (or is being)
/// reported. Otherwise returns 0.
///
/// \returns Access type (0 = read, 1 = write).
int SANITIZER_CDECL __asan_get_report_access_type(void);

/// Gets access size of an ASan error (useful for calling from the debugger).
///
/// Returns access size if an error has been (or is being) reported. Otherwise
/// returns 0.
///
/// \returns Access size in bytes.
size_t SANITIZER_CDECL __asan_get_report_access_size(void);

/// Gets the bug description of an ASan error (useful for calling from a
/// debugger).
///
/// \returns Returns a bug description if an error has been (or is being)
/// reported - for example, "heap-use-after-free". Otherwise returns an empty
/// string.
const char *SANITIZER_CDECL __asan_get_report_description(void);

/// Gets information about a pointer (useful for calling from the debugger).
///
/// Returns the category of the given pointer as a constant string.
/// Possible return values are <c>global</c>, <c>stack</c>, <c>stack-fake</c>,
/// <c>heap</c>, <c>heap-invalid</c>, <c>shadow-low</c>, <c>shadow-gap</c>,
/// <c>shadow-high</c>, and <c>unknown</c>.
///
/// If the return value is <c>global</c> or <c>stack</c>, tries to also return
/// the variable name, address, and size. If the return value is <c>heap</c>,
/// tries to return the chunk address and size. <c><i>name</i></c> should point
/// to an allocated buffer of size <c><i>name_size</i></c>.
///
/// \param addr Address to locate.
/// \param name Buffer to store the variable's name.
/// \param name_size Size in bytes of the variable's name buffer.
/// \param[out] region_address Address of the region.
/// \param[out] region_size Size of the region in bytes.
///
/// \returns Returns the category of the given pointer as a constant string.
const char *SANITIZER_CDECL __asan_locate_address(void *addr, char *name,
                                                  size_t name_size,
                                                  void **region_address,
                                                  size_t *region_size);

/// Gets the allocation stack trace and thread ID for a heap address (useful
/// for calling from the debugger).
///
/// Stores up to <c><i>size</i></c> frames in <c><i>trace</i></c>. Returns
/// the number of stored frames or 0 on error.
///
/// \param addr A heap address.
/// \param trace A buffer to store the stack trace.
/// \param size Size in bytes of the trace buffer.
/// \param[out] thread_id The thread ID of the address.
///
/// \returns Returns the number of stored frames or 0 on error.
size_t SANITIZER_CDECL __asan_get_alloc_stack(void *addr, void **trace,
                                              size_t size, int *thread_id);

/// Gets the free stack trace and thread ID for a heap address (useful for
/// calling from the debugger).
///
/// Stores up to <c><i>size</i></c> frames in <c><i>trace</i></c>. Returns
/// the number of stored frames or 0 on error.
///
/// \param addr A heap address.
/// \param trace A buffer to store the stack trace.
/// \param size Size in bytes of the trace buffer.
/// \param[out] thread_id The thread ID of the address.
///
/// \returns Returns the number of stored frames or 0 on error.
size_t SANITIZER_CDECL __asan_get_free_stack(void *addr, void **trace,
                                             size_t size, int *thread_id);

/// Gets the current shadow memory mapping (useful for calling from the
/// debugger).
///
/// \param[out] shadow_scale Shadow scale value.
/// \param[out] shadow_offset Offset value.
void SANITIZER_CDECL __asan_get_shadow_mapping(size_t *shadow_scale,
                                               size_t *shadow_offset);

/// This is an internal function that is called to report an error. However,
/// it is still a part of the interface because you might want to set a
/// breakpoint on this function in the debugger.
///
/// \param pc <c><i>pc</i></c> value of the ASan error.
/// \param bp <c><i>bp</i></c> value of the ASan error.
/// \param sp <c><i>sp</i></c> value of the ASan error.
/// \param addr Address of the ASan error.
/// \param is_write True if the error is a write error; false otherwise.
/// \param access_size Size of the memory access of the ASan error.
void SANITIZER_CDECL __asan_report_error(void *pc, void *bp, void *sp,
                                         void *addr, int is_write,
                                         size_t access_size);

// Deprecated. Call __sanitizer_set_death_callback instead.
void SANITIZER_CDECL __asan_set_death_callback(void (*callback)(void));

/// Sets the callback function to be called during ASan error reporting.
///
/// The callback provides a string pointer to the report.
///
/// \param callback User-provided function.
void SANITIZER_CDECL
__asan_set_error_report_callback(void (*callback)(const char *));

/// User-provided callback on ASan errors.
///
/// You can provide a function that would be called immediately when ASan
/// detects an error. This is useful in cases when ASan detects an error but
/// your program crashes before the ASan report is printed.
void SANITIZER_CDECL __asan_on_error(void);

/// Prints accumulated statistics to <c>stderr</c> (useful for calling from the
/// debugger).
void SANITIZER_CDECL __asan_print_accumulated_stats(void);

/// User-provided default option settings.
///
/// You can provide your own implementation of this function to return a string
/// containing ASan runtime options (for example,
/// <c>verbosity=1:halt_on_error=0</c>).
///
/// \returns Default options string.
const char *SANITIZER_CDECL __asan_default_options(void);

// The following two functions facilitate garbage collection in presence of
// ASan's fake stack.

/// Gets an opaque handler to the current thread's fake stack.
///
/// Returns an opaque handler to be used by
/// <c>__asan_addr_is_in_fake_stack()</c>. Returns NULL if the current thread
/// does not have a fake stack.
///
/// \returns An opaque handler to the fake stack or NULL.
void *SANITIZER_CDECL __asan_get_current_fake_stack(void);

/// Checks if an address belongs to a given fake stack.
///
/// If <c><i>fake_stack</i></c> is non-NULL and <c><i>addr</i></c> belongs to a
/// fake frame in <c><i>fake_stack</i></c>, returns the address of the real
/// stack that corresponds to the fake frame and sets <c><i>beg</i></c> and
/// <c><i>end</i></c> to the boundaries of this fake frame. Otherwise returns
/// NULL and does not touch <c><i>beg</i></c> and <c><i>end</i></c>.
///
/// If <c><i>beg</i></c> or <c><i>end</i></c> are NULL, they are not touched.
///
/// \note This function can be called from a thread other than the owner of
/// <c><i>fake_stack</i></c>, but the owner thread needs to be alive.
///
/// \param fake_stack An opaque handler to a fake stack.
/// \param addr Address to test.
/// \param[out] beg Beginning of fake frame.
/// \param[out] end End of fake frame.
/// \returns Stack address or NULL.
void *SANITIZER_CDECL __asan_addr_is_in_fake_stack(void *fake_stack, void *addr,
                                                   void **beg, void **end);

/// Performs shadow memory cleanup of the current thread's stack before a
/// function marked with the <c>[[noreturn]]</c> attribute is called.
///
/// To avoid false positives on the stack, must be called before no-return
/// functions like <c>_exit()</c> and <c>execl()</c>.
void SANITIZER_CDECL __asan_handle_no_return(void);

/// Update allocation stack trace for the given allocation to the current stack
/// trace. Returns 1 if successful, 0 if not.
int SANITIZER_CDECL __asan_update_allocation_context(void *addr);

// User-defined memory state flags
typedef enum {
    ASAN_USER_ALLOCATED = 0x01,    // 0=destroyed, 1=allocated
    ASAN_USER_STATE_INITIALIZED = 0x02,  // 0=uninitialized, 1=initialized
    ASAN_USER_RESERVED = 0x04,      // 0=not recycled, 1=recycled
    ASAN_USER_WRITABLE = 0x08,      // 0=read-only, 1=writable
    ASAN_USER_STATE_TRACKED = 0x10, // 0=not tracked, 1=tracked
    ASAN_USER_STATE_RESERVED1 = 0x20,
    ASAN_USER_STATE_RESERVED2 = 0x40,
    ASAN_USER_STATE_RESERVED3 = 0x80
} asan_user_state_t;

// State checking macros
#define ASAN_IS_ALLOCATED(state) ((state) & ASAN_USER_ALLOCATED)
#define ASAN_IS_DESTROYED(state) (!((state) & ASAN_USER_ALLOCATED))
#define ASAN_IS_INITIALIZED(state) ((state) & ASAN_USER_STATE_INITIALIZED)
#define ASAN_IS_UNINITIALIZED(state) (!((state) & ASAN_USER_STATE_INITIALIZED))
#define ASAN_IS_RECYCLED(state) ((state) & ASAN_USER_RESERVED)
#define ASAN_IS_NOT_RECYCLED(state) (!((state) & ASAN_USER_RESERVED))
#define ASAN_IS_WRITABLE(state) ((state) & ASAN_USER_WRITABLE)
#define ASAN_IS_READONLY(state) (!((state) & ASAN_USER_WRITABLE))
#define ASAN_IS_TRACKED(state) ((state) & ASAN_USER_STATE_TRACKED)
#define ASAN_IS_NOT_TRACKED(state) (!((state) & ASAN_USER_STATE_TRACKED))

// State setting macros
#define ASAN_SET_ALLOCATED(state) ((state) |= ASAN_USER_ALLOCATED)
#define ASAN_SET_DESTROYED(state) ((state) &= ~ASAN_USER_ALLOCATED)
#define ASAN_SET_INITIALIZED(state) ((state) |= ASAN_USER_STATE_INITIALIZED)
#define ASAN_SET_UNINITIALIZED(state) ((state) &= ~ASAN_USER_STATE_INITIALIZED)
#define ASAN_SET_RECYCLED(state) ((state) |= ASAN_USER_RESERVED)
#define ASAN_SET_NOT_RECYCLED(state) ((state) &= ~ASAN_USER_RESERVED)
#define ASAN_SET_WRITABLE(state) ((state) |= ASAN_USER_WRITABLE)
#define ASAN_SET_READONLY(state) ((state) &= ~ASAN_USER_WRITABLE)
#define ASAN_SET_TRACKED(state) ((state) |= ASAN_USER_STATE_TRACKED)
#define ASAN_SET_NOT_TRACKED(state) ((state) &= ~ASAN_USER_STATE_TRACKED)

// Predefined state combinations
#define ASAN_STATE_FRESH_ALLOC (ASAN_USER_ALLOCATED)
#define ASAN_STATE_UNINIT (ASAN_USER_ALLOCATED | ASAN_USER_WRITABLE)
#define ASAN_STATE_READY_RW (ASAN_USER_ALLOCATED | ASAN_USER_STATE_INITIALIZED | ASAN_USER_WRITABLE)
#define ASAN_STATE_READY_RO (ASAN_USER_ALLOCATED | ASAN_USER_STATE_INITIALIZED)
#define ASAN_STATE_RECYCLED (ASAN_USER_ALLOCATED | ASAN_USER_RESERVED)
#define ASAN_STATE_DEAD (0x00)
#define ASAN_STATE_TRACKED_RW (ASAN_STATE_READY_RW | ASAN_USER_STATE_TRACKED)

// User state shadow memory mapping
#define ASAN_USER_STATE_SHADOW_BASE 0x80
#define ASAN_USER_STATE_SHADOW_MAX 0x9F

// User state API functions
    // Basic state operations
    void SANITIZER_CDECL __asan_set_memory_state(void* addr, size_t size, asan_user_state_t state);
    asan_user_state_t SANITIZER_CDECL __asan_get_memory_state(void* addr);
    int SANITIZER_CDECL __asan_memory_has_state(void* addr, size_t size, asan_user_state_t state);
    
    // Convenience state query functions
    int SANITIZER_CDECL __asan_is_allocated(void* addr);
    int SANITIZER_CDECL __asan_is_destroyed(void* addr);
    int SANITIZER_CDECL __asan_is_initialized(void* addr);
    int SANITIZER_CDECL __asan_is_recycled(void* addr);
    int SANITIZER_CDECL __asan_is_writable(void* addr);
    int SANITIZER_CDECL __asan_is_readonly(void* addr);
    int SANITIZER_CDECL __asan_is_tracked(void* addr);
    
    // State transition functions
    void SANITIZER_CDECL __asan_mark_allocated(void* addr, size_t size);
    void SANITIZER_CDECL __asan_mark_destroyed(void* addr, size_t size);
    void SANITIZER_CDECL __asan_mark_initialized(void* addr, size_t size);
    void SANITIZER_CDECL __asan_mark_recycled(void* addr, size_t size);
    void SANITIZER_CDECL __asan_mark_writable(void* addr, size_t size);
    void SANITIZER_CDECL __asan_mark_readonly(void* addr, size_t size);
    void SANITIZER_CDECL __asan_mark_tracked(void* addr, size_t size);
    void SANITIZER_CDECL __asan_mark_untracked(void* addr, size_t size);
    
    // State description (for debugging)
    const char* SANITIZER_CDECL __asan_describe_state(asan_user_state_t state, char* buffer, size_t buffer_size);
    
    // Runtime control
    void SANITIZER_CDECL __asan_set_detect_user_state(int enabled);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // SANITIZER_ASAN_INTERFACE_H
