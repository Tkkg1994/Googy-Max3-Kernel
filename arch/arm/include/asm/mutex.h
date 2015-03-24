/*
 * arch/arm/include/asm/mutex.h
 *
 * ARM optimized mutex locking primitives
 *
 * Please look into asm-generic/mutex-xchg.h for a formal definition.
 */
#ifndef _ASM_MUTEX_H
#define _ASM_MUTEX_H
<<<<<<< HEAD

=======
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
/*
 * On pre-ARMv6 hardware this results in a swp-based implementation,
 * which is the most efficient. For ARMv6+, we have exclusive memory
 * accessors and use atomic_dec to avoid the extra xchg operations
 * on the locking slowpaths.
 */
#if __LINUX_ARM_ARCH__ < 6
<<<<<<< HEAD
/* On pre-ARMv6 hardware the swp based implementation is the most efficient. */
=======
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
#include <asm-generic/mutex-xchg.h>
#else
#include <asm-generic/mutex-dec.h>
#endif
<<<<<<< HEAD
#endif
=======
#endif	/* _ASM_MUTEX_H */
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
