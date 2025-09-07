//===-- asan_poisoning.h ----------------------------------------*- C++ -*-===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file is a part of AddressSanitizer, an address sanity checker.
//
// Shadow memory poisoning by ASan RTL and by user application.
//===----------------------------------------------------------------------===//

#ifndef ASAN_POISONING_H
#define ASAN_POISONING_H

#include "asan_interceptors.h"
#include "asan_internal.h"
#include "asan_mapping.h"
#include "sanitizer_common/sanitizer_flags.h"
#include "sanitizer_common/sanitizer_platform.h"

// Forward declarations to avoid header conflicts
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

// User state shadow memory mapping
#define ASAN_USER_STATE_SHADOW_BASE 0x80
#define ASAN_USER_STATE_SHADOW_MAX 0x9F

namespace __asan {

struct PoisonRecord {
  u32 stack_id;
  u32 thread_id;
  uptr begin;
  uptr end;
};

void AddPoisonRecord(const PoisonRecord& new_record);
bool FindPoisonRecord(uptr addr, PoisonRecord& match);

void AcquirePoisonRecords();
void ReleasePoisonRecords();

// Enable/disable memory poisoning.
void SetCanPoisonMemory(bool value);
bool CanPoisonMemory();

// Poisons the shadow memory for "size" bytes starting from "addr".
void PoisonShadow(uptr addr, uptr size, u8 value);

// Poisons the shadow memory for "redzone_size" bytes starting from
// "addr + size".
void PoisonShadowPartialRightRedzone(uptr addr,
                                     uptr size,
                                     uptr redzone_size,
                                     u8 value);

// Fast versions of PoisonShadow and PoisonShadowPartialRightRedzone that
// assume that memory addresses are properly aligned. Use in
// performance-critical code with care.
ALWAYS_INLINE void FastPoisonShadow(uptr aligned_beg, uptr aligned_size,
                                    u8 value) {
  DCHECK(!value || CanPoisonMemory());
#if SANITIZER_FUCHSIA
  __sanitizer_fill_shadow(aligned_beg, aligned_size, value,
                          common_flags()->clear_shadow_mmap_threshold);
#else
  uptr shadow_beg = MEM_TO_SHADOW(aligned_beg);
  uptr shadow_end =
      MEM_TO_SHADOW(aligned_beg + aligned_size - ASAN_SHADOW_GRANULARITY) + 1;
  // FIXME: Page states are different on Windows, so using the same interface
  // for mapping shadow and zeroing out pages doesn't "just work", so we should
  // probably provide higher-level interface for these operations.
  // For now, just memset on Windows.
  if (value || SANITIZER_WINDOWS == 1 ||
      shadow_end - shadow_beg < common_flags()->clear_shadow_mmap_threshold) {
    REAL(memset)((void*)shadow_beg, value, shadow_end - shadow_beg);
  } else {
    uptr page_size = GetPageSizeCached();
    uptr page_beg = RoundUpTo(shadow_beg, page_size);
    uptr page_end = RoundDownTo(shadow_end, page_size);

    if (page_beg >= page_end) {
      REAL(memset)((void *)shadow_beg, 0, shadow_end - shadow_beg);
    } else {
      if (page_beg != shadow_beg) {
        REAL(memset)((void *)shadow_beg, 0, page_beg - shadow_beg);
      }
      if (page_end != shadow_end) {
        REAL(memset)((void *)page_end, 0, shadow_end - page_end);
      }
      ReserveShadowMemoryRange(page_beg, page_end - 1, nullptr);
    }
  }
#endif // SANITIZER_FUCHSIA
}

ALWAYS_INLINE void FastPoisonShadowPartialRightRedzone(
    uptr aligned_addr, uptr size, uptr redzone_size, u8 value) {
  DCHECK(CanPoisonMemory());
  bool poison_partial = flags()->poison_partial;
  u8 *shadow = (u8*)MEM_TO_SHADOW(aligned_addr);
  for (uptr i = 0; i < redzone_size; i += ASAN_SHADOW_GRANULARITY, shadow++) {
    if (i + ASAN_SHADOW_GRANULARITY <= size) {
      *shadow = 0;  // fully addressable
    } else if (i >= size) {
      *shadow =
          (ASAN_SHADOW_GRANULARITY == 128) ? 0xff : value;  // unaddressable
    } else {
      // first size-i bytes are addressable
      *shadow = poison_partial ? static_cast<u8>(size - i) : 0;
    }
  }
}

// Calls __sanitizer::ReleaseMemoryPagesToOS() on
// [MemToShadow(p), MemToShadow(p+size)].
void FlushUnneededASanShadowMemory(uptr p, uptr size);

// User state encoding/decoding functions
u8 encode_user_state_to_shadow(asan_user_state_t state);
asan_user_state_t decode_user_state_from_shadow(u8 shadow_value);
bool is_user_state_shadow(u8 shadow_value);

}  // namespace __asan

#endif  // ASAN_POISONING_H
