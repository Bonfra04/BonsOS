#pragma once

#include <stdint.h>
#include <stddef.h>

typedef _Atomic _Bool               atomic_bool;
typedef _Atomic char                atomic_char;
typedef _Atomic signed char         atomic_schar;
typedef _Atomic unsigned char       atomic_uchar;
typedef _Atomic short               atomic_short;
typedef _Atomic unsigned short      atomic_ushort;
typedef _Atomic int                 atomic_int;
typedef _Atomic unsigned int        atomic_uint;
typedef _Atomic long                atomic_long;
typedef _Atomic unsigned long       atomic_ulong;
typedef _Atomic long long           atomic_llong;
typedef _Atomic unsigned long long  atomic_ullong;
typedef _Atomic int_least8_t        atomic_int_least8_t;
typedef _Atomic uint_least8_t       atomic_uint_least8_t;
typedef _Atomic int_least16_t       atomic_int_least16_t;
typedef _Atomic uint_least16_t      atomic_uint_least16_t;
typedef _Atomic int_least32_t       atomic_int_least32_t;
typedef _Atomic uint_least32_t      atomic_uint_least32_t;
typedef _Atomic int_least64_t       atomic_int_least64_t;
typedef _Atomic uint_least64_t      atomic_uint_least64_t;
typedef _Atomic int_fast8_t         atomic_int_fast8_t;
typedef _Atomic uint_fast8_t        atomic_uint_fast8_t;
typedef _Atomic int_fast16_t        atomic_int_fast16_t;
typedef _Atomic uint_fast16_t       atomic_uint_fast16_t;
typedef _Atomic int_fast32_t        atomic_int_fast32_t;
typedef _Atomic uint_fast32_t       atomic_uint_fast32_t;
typedef _Atomic int_fast64_t        atomic_int_fast64_t;
typedef _Atomic uint_fast64_t       atomic_uint_fast64_t;
typedef _Atomic intptr_t            atomic_intptr_t;
typedef _Atomic uintptr_t           atomic_uintptr_t;
typedef _Atomic size_t              atomic_size_t;
typedef _Atomic ptrdiff_t           atomic_ptrdiff_t;
typedef _Atomic intmax_t            atomic_intmax_t;
typedef _Atomic uintmax_t           atomic_uintmax_t;

#define ATOMIC_BOOL_LOCK_FREE       __GCC_ATOMIC_BOOL_LOCK_FREE
#define ATOMIC_CHAR_LOCK_FREE       __GCC_ATOMIC_CHAR_LOCK_FREE
#define ATOMIC_CHAR16_T_LOCK_FREE   __GCC_ATOMIC_CHAR16_T_LOCK_FREE
#define ATOMIC_CHAR32_T_LOCK_FREE   __GCC_ATOMIC_CHAR32_T_LOCK_FREE
#define ATOMIC_WCHAR_T_LOCK_FREE    __GCC_ATOMIC_WCHAR_T_LOCK_FREE
#define ATOMIC_SHORT_LOCK_FREE      __GCC_ATOMIC_SHORT_LOCK_FREE
#define ATOMIC_INT_LOCK_FREE        __GCC_ATOMIC_INT_LOCK_FREE
#define ATOMIC_LONG_LOCK_FREE       __GCC_ATOMIC_LONG_LOCK_FREE
#define ATOMIC_LLONG_LOCK_FREE      __GCC_ATOMIC_LLONG_LOCK_FREE
#define ATOMIC_POINTER_LOCK_FREE    __GCC_ATOMIC_POINTER_LOCK_FREE

typedef enum
{
    memory_order_relaxed = __ATOMIC_RELAXED,
    memory_order_consume = __ATOMIC_CONSUME,
    memory_order_acquire = __ATOMIC_ACQUIRE,
    memory_order_release = __ATOMIC_RELEASE,
    memory_order_acq_rel = __ATOMIC_ACQ_REL,
    memory_order_seq_cst = __ATOMIC_SEQ_CST
} memory_order;

#define atomic_is_lock_free(object) __atomic_is_lock_free(sizeof(*(object)), (object))

#define atomic_store_explicit(where, desired, mem_order)                        \
({                                                                              \
    __auto_type __atomic_store_ptr = (where);                                   \
    __typeof__((void)0, *__atomic_store_ptr) __atomic_store_tmp = (desired);    \
    __atomic_store(__atomic_store_ptr, &__atomic_store_tmp, (mem_order));       \
})

#define atomic_store(where, desired) atomic_store_explicit(where, desired, __ATOMIC_SEQ_CST)

#define atomic_load_explicit(where, mem_order)                          \
({                                                                      \
    __auto_type __atomic_load_ptr = (where);                            \
    __typeof__((void)0, *__atomic_load_ptr) __atomic_load_tmp;          \
    __atomic_load(__atomic_load_ptr, &__atomic_load_tmp, (mem_order));  \
    __atomic_load_tmp;                                                  \
})

#define atomic_load(where) atomic_load_explicit(where, __ATOMIC_SEQ_CST)

#define atomic_exchange_explicit(where, desired, mem_order)                                                 \
({                                                                                                          \
    __auto_type __atomic_exchange_ptr = (where);                                                            \
    __typeof__((void)0, *__atomic_exchange_ptr) __atomic_exchange_val = (desired);                          \
    __typeof__((void)0, *__atomic_exchange_ptr) __atomic_exchange_tmp;                                      \
    __atomic_exchange(__atomic_exchange_ptr, &__atomic_exchange_val, &__atomic_exchange_tmp, (mem_order));  \
    __atomic_exchange_tmp;                                                                                  \
})

#define atomic_exchange(where, desired) atomic_exchange_explicit(where, desired, __ATOMIC_SEQ_CST)

#define atomic_compare_exchange_strong_explicit(where, expected, desired, mo_succ, mo_fail) \
({                                                                                                                                  \
    __auto_type __atomic_compare_exchange_ptr = (where);                                                                            \
    __typeof__((void)0, *__atomic_compare_exchange_ptr) __atomic_compare_exchange_tmp = (desired);                                  \
    __atomic_compare_exchange(__atomic_compare_exchange_ptr, (expected), &__atomic_compare_exchange_tmp, 0, (mo_succ), (mo_fail));  \
})

#define atomic_compare_exchange_strong(where, expected, desired) atomic_compare_exchange_strong_explicit(where, expected, desired, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#define atomic_compare_exchange_weak_explicit(where, expected, desired, mo_succ, mo_fail) \
({                                                                                                                                  \
    __auto_type __atomic_compare_exchange_ptr = (where);                                                                            \
    __typeof__((void)0, *__atomic_compare_exchange_ptr) __atomic_compare_exchange_tmp = (desired);                                  \
    __atomic_compare_exchange(__atomic_compare_exchange_ptr, (expected), &__atomic_compare_exchange_tmp, 1, (mo_succ), (mo_fail));  \
})

#define atomic_compare_exchange_weak(where, expected, desired) atomic_compare_exchange_weak_explicit(where, expected, desired, __ATOMIC_SEQ_CST, __ATOMIC_SEQ_CST)

#define atomic_fetch_add(where, value) __atomic_fetch_add((where), (value), __ATOMIC_SEQ_CST)
#define atomic_fetch_add_explicit(where, value, mem_order) __atomic_fetch_add((where), (value), (mem_order))

#define atomic_fetch_sub(where, value) __atomic_fetch_sub((where), (value), __ATOMIC_SEQ_CST)
#define atomic_fetch_sub_explicit(where, value, mem_order) __atomic_fetch_sub((where), (value), (mem_order))

#define atomic_fetch_or(where, value) __atomic_fetch_or((where), (value), __ATOMIC_SEQ_CST)
#define atomic_fetch_or_explicit(where, value, mem_order) __atomic_fetch_or((where), (value), (mem_order))

#define atomic_fetch_xor(where, value) __atomic_fetch_xor((where), (value), __ATOMIC_SEQ_CST)
#define atomic_fetch_xor_explicit(where, value, mem_order) __atomic_fetch_xor((where), (value), (mem_order))

#define atomic_fetch_and(where, value) __atomic_fetch_and((where), (value), __ATOMIC_SEQ_CST)
#define atomic_fetch_and_explicit(where, value, mem_order) __atomic_fetch_and((where), (value), (mem_order))

typedef _Atomic struct
{
#if __GCC_ATOMIC_TEST_AND_SET_TRUEVAL == 1
    _Bool __val;
#else
    unsigned char __val;
#endif
} atomic_flag;

#define ATOMIC_FLAG_INIT { 0 }

#define atomic_flag_test_and_set(where) __atomic_test_and_set((where), __ATOMIC_SEQ_CST)
#define atomic_flag_test_and_set_explicit(where, mem_order) __atomic_test_and_set((where), (mem_order))

#define atomic_flag_clear(where) __atomic_clear((where), __ATOMIC_SEQ_CST)
#define atomic_flag_clear_explicit(where, mem_order) __atomic_clear((where), (mem_order))

#define ATOMIC_VAR_INIT(value) (value)
#define atomic_init(where, value) atomic_store_explicit(where, value, __ATOMIC_RELAXED)

#define atomic_thread_fence(mem_order) __atomic_thread_fence(mem_order)
#define atomic_signal_fence(mem_order) __atomic_signal_fence(mem_order)

#define kill_dependency(what)                   \
({                                              \
    __auto_type __kill_dependency_tmp = (what); \
    __kill_dependency_tmp;                      \
})
