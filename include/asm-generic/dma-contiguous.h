#ifndef ASM_DMA_CONTIGUOUS_H
#define ASM_DMA_CONTIGUOUS_H

#ifdef __KERNEL__
#ifdef CONFIG_CMA

#include <linux/device.h>
#include <linux/dma-contiguous.h>

static inline struct cma *dev_get_cma_area(struct device *dev)
{
	if (dev && dev->cma_area)
		return dev->cma_area;
<<<<<<< HEAD
	return dma_contiguous_default_area;
=======
	return dma_contiguous_def_area;
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
}

static inline void dev_set_cma_area(struct device *dev, struct cma *cma)
{
	if (dev)
		dev->cma_area = cma;
<<<<<<< HEAD
	if (!dev || !dma_contiguous_default_area)
		dma_contiguous_default_area = cma;
=======
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
}

#endif
#endif

#endif
