#ifndef __ASM_ARM_SUSPEND_H
#define __ASM_ARM_SUSPEND_H

<<<<<<< HEAD
struct sleep_save_sp {
	u32 *save_ptr_stash;
	u32 save_ptr_stash_phys;
};

=======
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
extern void cpu_resume(void);
extern int cpu_suspend(unsigned long, int (*)(unsigned long));

#endif
