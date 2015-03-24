/*
 *  linux/arch/arm/lib/ucmpdi2.S
 *
 *  Author:	Nicolas Pitre
 *  Created:	Oct 19, 2005
 *  Copyright:	Monta Vista Software, Inc.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 */

#include <linux/linkage.h>
<<<<<<< HEAD
#include <asm/assembler.h>
=======
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea

#ifdef __ARMEB__
#define xh r0
#define xl r1
#define yh r2
#define yl r3
#else
#define xl r0
#define xh r1
#define yl r2
#define yh r3
#endif

ENTRY(__ucmpdi2)

	cmp	xh, yh
	cmpeq	xl, yl
	movlo	r0, #0
	moveq	r0, #1
	movhi	r0, #2
<<<<<<< HEAD
	ret	lr
=======
	mov	pc, lr
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea

ENDPROC(__ucmpdi2)

#ifdef CONFIG_AEABI

ENTRY(__aeabi_ulcmp)

	cmp	xh, yh
	cmpeq	xl, yl
	movlo	r0, #-1
	moveq	r0, #0
	movhi	r0, #1
<<<<<<< HEAD
	ret	lr
=======
	mov	pc, lr
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea

ENDPROC(__aeabi_ulcmp)

#endif

