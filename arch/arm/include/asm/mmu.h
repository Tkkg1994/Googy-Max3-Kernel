#ifndef __ARM_MMU_H
#define __ARM_MMU_H

#ifdef CONFIG_MMU

typedef struct {
#ifdef CONFIG_CPU_HAS_ASID
<<<<<<< HEAD
	atomic64_t	id;
#endif
	unsigned int kvm_seq;
=======
	u64 id;
#endif
	unsigned int kvm_seq;
	unsigned long	sigpage;
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
} mm_context_t;

#ifdef CONFIG_CPU_HAS_ASID
#define ASID_BITS	8
#define ASID_MASK	((~0ULL) << ASID_BITS)
<<<<<<< HEAD
#define ASID(mm)	((mm)->context.id.counter & ~ASID_MASK)
=======
#define ASID(mm)	((mm)->context.id & ~ASID_MASK)
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
#else
#define ASID(mm)	(0)
#endif

#else

/*
 * From nommu.h:
 *  Copyright (C) 2002, David McCullough <davidm@snapgear.com>
 *  modified for 2.6 by Hyok S. Choi <hyok.choi@samsung.com>
 */
typedef struct {
<<<<<<< HEAD
	unsigned long	end_brk;
=======
	unsigned long		end_brk;
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
} mm_context_t;

#endif

/*
 * switch_mm() may do a full cache flush over the context switch,
 * so enable interrupts over the context switch to avoid high
 * latency.
 */
<<<<<<< HEAD
#ifndef CONFIG_CPU_HAS_ASID
#define __ARCH_WANT_INTERRUPTS_ON_CTXSW
#endif
=======
#define __ARCH_WANT_INTERRUPTS_ON_CTXSW
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea

#endif
