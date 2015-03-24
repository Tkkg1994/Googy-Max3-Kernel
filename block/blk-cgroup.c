/*
<<<<<<< HEAD
 * BFQ: CGROUPS support.
 *
 * Based on ideas and code from CFQ:
=======
 * Common Block IO controller cgroup interface
 *
 * Based on ideas and code from CFQ, CFS and BFQ:
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
 * Copyright (C) 2003 Jens Axboe <axboe@kernel.dk>
 *
 * Copyright (C) 2008 Fabio Checconi <fabio@gandalf.sssup.it>
 *		      Paolo Valente <paolo.valente@unimore.it>
 *
<<<<<<< HEAD
 * Copyright (C) 2010 Paolo Valente <paolo.valente@unimore.it>
 *
 * Licensed under the GPL-2 as detailed in the accompanying COPYING.BFQ
 * file.
 */

#ifdef CONFIG_CGROUP_BFQIO
static struct bfqio_cgroup bfqio_root_cgroup = {
	.weight = BFQ_DEFAULT_GRP_WEIGHT,
	.ioprio = BFQ_DEFAULT_GRP_IOPRIO,
	.ioprio_class = BFQ_DEFAULT_GRP_CLASS,
};

static inline void bfq_init_entity(struct bfq_entity *entity,
				   struct bfq_group *bfqg)
{
	entity->weight = entity->new_weight;
	entity->orig_weight = entity->new_weight;
	entity->ioprio = entity->new_ioprio;
	entity->ioprio_class = entity->new_ioprio_class;
	entity->parent = bfqg->my_entity;
	entity->sched_data = &bfqg->sched_data;
}

static struct bfqio_cgroup *cgroup_to_bfqio(struct cgroup *cgroup)
{
	return container_of(cgroup_subsys_state(cgroup, bfqio_subsys_id),
			    struct bfqio_cgroup, css);
}

/*
 * Search the bfq_group for bfqd into the hash table (by now only a list)
 * of bgrp.  Must be called under rcu_read_lock().
 */
static struct bfq_group *bfqio_lookup_group(struct bfqio_cgroup *bgrp,
					    struct bfq_data *bfqd)
{
	struct bfq_group *bfqg;
	struct hlist_node *n;
	void *key;

	hlist_for_each_entry_rcu(bfqg, n, &bgrp->group_data, group_node) {
		key = rcu_dereference(bfqg->bfqd);
		if (key == bfqd)
			return bfqg;
=======
 * Copyright (C) 2009 Vivek Goyal <vgoyal@redhat.com>
 * 	              Nauman Rafique <nauman@google.com>
 */
#include <linux/ioprio.h>
#include <linux/seq_file.h>
#include <linux/kdev_t.h>
#include <linux/module.h>
#include <linux/err.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include "blk-cgroup.h"
#include <linux/genhd.h>

#define MAX_KEY_LEN 100

static DEFINE_SPINLOCK(blkio_list_lock);
static LIST_HEAD(blkio_list);

struct blkio_cgroup blkio_root_cgroup = { .weight = 2*BLKIO_WEIGHT_DEFAULT };
EXPORT_SYMBOL_GPL(blkio_root_cgroup);

static struct cgroup_subsys_state *blkiocg_create(struct cgroup *);
static int blkiocg_can_attach(struct cgroup *, struct cgroup_taskset *);
static void blkiocg_attach(struct cgroup *, struct cgroup_taskset *);
static void blkiocg_destroy(struct cgroup *);
static int blkiocg_populate(struct cgroup_subsys *, struct cgroup *);

/* for encoding cft->private value on file */
#define BLKIOFILE_PRIVATE(x, val)	(((x) << 16) | (val))
/* What policy owns the file, proportional or throttle */
#define BLKIOFILE_POLICY(val)		(((val) >> 16) & 0xffff)
#define BLKIOFILE_ATTR(val)		((val) & 0xffff)

struct cgroup_subsys blkio_subsys = {
	.name = "blkio",
	.create = blkiocg_create,
	.can_attach = blkiocg_can_attach,
	.attach = blkiocg_attach,
	.destroy = blkiocg_destroy,
	.populate = blkiocg_populate,
#ifdef CONFIG_BLK_CGROUP
	/* note: blkio_subsys_id is otherwise defined in blk-cgroup.h */
	.subsys_id = blkio_subsys_id,
#endif
	.use_id = 1,
	.module = THIS_MODULE,
};
EXPORT_SYMBOL_GPL(blkio_subsys);

static inline void blkio_policy_insert_node(struct blkio_cgroup *blkcg,
					    struct blkio_policy_node *pn)
{
	list_add(&pn->node, &blkcg->policy_list);
}

static inline bool cftype_blkg_same_policy(struct cftype *cft,
			struct blkio_group *blkg)
{
	enum blkio_policy_id plid = BLKIOFILE_POLICY(cft->private);

	if (blkg->plid == plid)
		return 1;

	return 0;
}

/* Determines if policy node matches cgroup file being accessed */
static inline bool pn_matches_cftype(struct cftype *cft,
			struct blkio_policy_node *pn)
{
	enum blkio_policy_id plid = BLKIOFILE_POLICY(cft->private);
	int fileid = BLKIOFILE_ATTR(cft->private);

	return (plid == pn->plid && fileid == pn->fileid);
}

/* Must be called with blkcg->lock held */
static inline void blkio_policy_delete_node(struct blkio_policy_node *pn)
{
	list_del(&pn->node);
}

/* Must be called with blkcg->lock held */
static struct blkio_policy_node *
blkio_policy_search_node(const struct blkio_cgroup *blkcg, dev_t dev,
		enum blkio_policy_id plid, int fileid)
{
	struct blkio_policy_node *pn;

	list_for_each_entry(pn, &blkcg->policy_list, node) {
		if (pn->dev == dev && pn->plid == plid && pn->fileid == fileid)
			return pn;
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
	}

	return NULL;
}

<<<<<<< HEAD
static inline void bfq_group_init_entity(struct bfqio_cgroup *bgrp,
					 struct bfq_group *bfqg)
{
	struct bfq_entity *entity = &bfqg->entity;

	/*
	 * If the weight of the entity has never been set via the sysfs
	 * interface, then bgrp->weight == 0. In this case we initialize
	 * the weight from the current ioprio value. Otherwise, the group
	 * weight, if set, has priority over the ioprio value.
	 */
	if (bgrp->weight == 0) {
		entity->new_weight = bfq_ioprio_to_weight(bgrp->ioprio);
		entity->new_ioprio = bgrp->ioprio;
	} else {
		if (bgrp->weight < BFQ_MIN_WEIGHT ||
		    bgrp->weight > BFQ_MAX_WEIGHT) {
			printk(KERN_CRIT "bfq_group_init_entity: "
					 "bgrp->weight %d\n", bgrp->weight);
			BUG();
		}
		entity->new_weight = bgrp->weight;
		entity->new_ioprio = bfq_weight_to_ioprio(bgrp->weight);
	}
	entity->orig_weight = entity->weight = entity->new_weight;
	entity->ioprio = entity->new_ioprio;
	entity->ioprio_class = entity->new_ioprio_class = bgrp->ioprio_class;
	entity->my_sched_data = &bfqg->sched_data;
	bfqg->active_entities = 0;
}

static inline void bfq_group_set_parent(struct bfq_group *bfqg,
					struct bfq_group *parent)
{
	struct bfq_entity *entity;

	BUG_ON(parent == NULL);
	BUG_ON(bfqg == NULL);

	entity = &bfqg->entity;
	entity->parent = parent->my_entity;
	entity->sched_data = &parent->sched_data;
}

/**
 * bfq_group_chain_alloc - allocate a chain of groups.
 * @bfqd: queue descriptor.
 * @cgroup: the leaf cgroup this chain starts from.
 *
 * Allocate a chain of groups starting from the one belonging to
 * @cgroup up to the root cgroup.  Stop if a cgroup on the chain
 * to the root has already an allocated group on @bfqd.
 */
static struct bfq_group *bfq_group_chain_alloc(struct bfq_data *bfqd,
					       struct cgroup *cgroup)
{
	struct bfqio_cgroup *bgrp;
	struct bfq_group *bfqg, *prev = NULL, *leaf = NULL;

	for (; cgroup != NULL; cgroup = cgroup->parent) {
		bgrp = cgroup_to_bfqio(cgroup);

		bfqg = bfqio_lookup_group(bgrp, bfqd);
		if (bfqg != NULL) {
			/*
			 * All the cgroups in the path from there to the
			 * root must have a bfq_group for bfqd, so we don't
			 * need any more allocations.
			 */
			break;
		}

		bfqg = kzalloc(sizeof(*bfqg), GFP_ATOMIC);
		if (bfqg == NULL)
			goto cleanup;

		bfq_group_init_entity(bgrp, bfqg);
		bfqg->my_entity = &bfqg->entity;

		if (leaf == NULL) {
			leaf = bfqg;
			prev = leaf;
		} else {
			bfq_group_set_parent(prev, bfqg);
			/*
			 * Build a list of allocated nodes using the bfqd
			 * filed, that is still unused and will be
			 * initialized only after the node will be
			 * connected.
			 */
			prev->bfqd = bfqg;
			prev = bfqg;
		}
	}

	return leaf;

cleanup:
	while (leaf != NULL) {
		prev = leaf;
		leaf = leaf->bfqd;
		kfree(prev);
	}

	return NULL;
}

/**
 * bfq_group_chain_link - link an allocated group chain to a cgroup
 *                        hierarchy.
 * @bfqd: the queue descriptor.
 * @cgroup: the leaf cgroup to start from.
 * @leaf: the leaf group (to be associated to @cgroup).
 *
 * Try to link a chain of groups to a cgroup hierarchy, connecting the
 * nodes bottom-up, so we can be sure that when we find a cgroup in the
 * hierarchy that already as a group associated to @bfqd all the nodes
 * in the path to the root cgroup have one too.
 *
 * On locking: the queue lock protects the hierarchy (there is a hierarchy
 * per device) while the bfqio_cgroup lock protects the list of groups
 * belonging to the same cgroup.
 */
static void bfq_group_chain_link(struct bfq_data *bfqd, struct cgroup *cgroup,
				 struct bfq_group *leaf)
{
	struct bfqio_cgroup *bgrp;
	struct bfq_group *bfqg, *next, *prev = NULL;
	unsigned long flags;

	assert_spin_locked(bfqd->queue->queue_lock);

	for (; cgroup != NULL && leaf != NULL; cgroup = cgroup->parent) {
		bgrp = cgroup_to_bfqio(cgroup);
		next = leaf->bfqd;

		bfqg = bfqio_lookup_group(bgrp, bfqd);
		BUG_ON(bfqg != NULL);

		spin_lock_irqsave(&bgrp->lock, flags);

		rcu_assign_pointer(leaf->bfqd, bfqd);
		hlist_add_head_rcu(&leaf->group_node, &bgrp->group_data);
		hlist_add_head(&leaf->bfqd_node, &bfqd->group_list);

		spin_unlock_irqrestore(&bgrp->lock, flags);

		prev = leaf;
		leaf = next;
	}

	BUG_ON(cgroup == NULL && leaf != NULL);
	if (cgroup != NULL && prev != NULL) {
		bgrp = cgroup_to_bfqio(cgroup);
		bfqg = bfqio_lookup_group(bgrp, bfqd);
		bfq_group_set_parent(prev, bfqg);
	}
}

/**
 * bfq_find_alloc_group - return the group associated to @bfqd in @cgroup.
 * @bfqd: queue descriptor.
 * @cgroup: cgroup being searched for.
 *
 * Return a group associated to @bfqd in @cgroup, allocating one if
 * necessary.  When a group is returned all the cgroups in the path
 * to the root have a group associated to @bfqd.
 *
 * If the allocation fails, return the root group: this breaks guarantees
 * but is a safe fallback.  If this loss becomes a problem it can be
 * mitigated using the equivalent weight (given by the product of the
 * weights of the groups in the path from @group to the root) in the
 * root scheduler.
 *
 * We allocate all the missing nodes in the path from the leaf cgroup
 * to the root and we connect the nodes only after all the allocations
 * have been successful.
 */
static struct bfq_group *bfq_find_alloc_group(struct bfq_data *bfqd,
					      struct cgroup *cgroup)
{
	struct bfqio_cgroup *bgrp = cgroup_to_bfqio(cgroup);
	struct bfq_group *bfqg;

	bfqg = bfqio_lookup_group(bgrp, bfqd);
	if (bfqg != NULL)
		return bfqg;

	bfqg = bfq_group_chain_alloc(bfqd, cgroup);
	if (bfqg != NULL)
		bfq_group_chain_link(bfqd, cgroup, bfqg);
	else
		bfqg = bfqd->root_group;

	return bfqg;
}

/**
 * bfq_bfqq_move - migrate @bfqq to @bfqg.
 * @bfqd: queue descriptor.
 * @bfqq: the queue to move.
 * @entity: @bfqq's entity.
 * @bfqg: the group to move to.
 *
 * Move @bfqq to @bfqg, deactivating it from its old group and reactivating
 * it on the new one.  Avoid putting the entity on the old group idle tree.
 *
 * Must be called under the queue lock; the cgroup owning @bfqg must
 * not disappear (by now this just means that we are called under
 * rcu_read_lock()).
 */
static void bfq_bfqq_move(struct bfq_data *bfqd, struct bfq_queue *bfqq,
			  struct bfq_entity *entity, struct bfq_group *bfqg)
{
	int busy, resume;

	busy = bfq_bfqq_busy(bfqq);
	resume = !RB_EMPTY_ROOT(&bfqq->sort_list);

	BUG_ON(resume && !entity->on_st);
	BUG_ON(busy && !resume && entity->on_st &&
	       bfqq != bfqd->in_service_queue);

	if (busy) {
		BUG_ON(atomic_read(&bfqq->ref) < 2);

		if (!resume)
			bfq_del_bfqq_busy(bfqd, bfqq, 0);
		else
			bfq_deactivate_bfqq(bfqd, bfqq, 0);
	} else if (entity->on_st)
		bfq_put_idle_entity(bfq_entity_service_tree(entity), entity);

	/*
	 * Here we use a reference to bfqg.  We don't need a refcounter
	 * as the cgroup reference will not be dropped, so that its
	 * destroy() callback will not be invoked.
	 */
	entity->parent = bfqg->my_entity;
	entity->sched_data = &bfqg->sched_data;

	if (busy && resume)
		bfq_activate_bfqq(bfqd, bfqq);

	if (bfqd->in_service_queue == NULL && !bfqd->rq_in_driver)
		bfq_schedule_dispatch(bfqd);
}

/**
 * __bfq_bic_change_cgroup - move @bic to @cgroup.
 * @bfqd: the queue descriptor.
 * @bic: the bic to move.
 * @cgroup: the cgroup to move to.
 *
 * Move bic to cgroup, assuming that bfqd->queue is locked; the caller
 * has to make sure that the reference to cgroup is valid across the call.
 *
 * NOTE: an alternative approach might have been to store the current
 * cgroup in bfqq and getting a reference to it, reducing the lookup
 * time here, at the price of slightly more complex code.
 */
static struct bfq_group *__bfq_bic_change_cgroup(struct bfq_data *bfqd,
						 struct bfq_io_cq *bic,
						 struct cgroup *cgroup)
{
	struct bfq_queue *async_bfqq = bic_to_bfqq(bic, 0);
	struct bfq_queue *sync_bfqq = bic_to_bfqq(bic, 1);
	struct bfq_entity *entity;
	struct bfq_group *bfqg;
	struct bfqio_cgroup *bgrp;

	bgrp = cgroup_to_bfqio(cgroup);

	bfqg = bfq_find_alloc_group(bfqd, cgroup);
	if (async_bfqq != NULL) {
		entity = &async_bfqq->entity;

		if (entity->sched_data != &bfqg->sched_data) {
			bic_set_bfqq(bic, NULL, 0);
			bfq_log_bfqq(bfqd, async_bfqq,
				     "bic_change_group: %p %d",
				     async_bfqq, atomic_read(&async_bfqq->ref));
			bfq_put_queue(async_bfqq);
		}
	}

	if (sync_bfqq != NULL) {
		entity = &sync_bfqq->entity;
		if (entity->sched_data != &bfqg->sched_data)
			bfq_bfqq_move(bfqd, sync_bfqq, entity, bfqg);
	}

	return bfqg;
}

/**
 * bfq_bic_change_cgroup - move @bic to @cgroup.
 * @bic: the bic being migrated.
 * @cgroup: the destination cgroup.
 *
 * When the task owning @bic is moved to @cgroup, @bic is immediately
 * moved into its new parent group.
 */
static void bfq_bic_change_cgroup(struct bfq_io_cq *bic,
				  struct cgroup *cgroup)
{
	struct bfq_data *bfqd;
	unsigned long uninitialized_var(flags);

	bfqd = bfq_get_bfqd_locked(&(bic->icq.q->elevator->elevator_data),
				   &flags);
	if (bfqd != NULL) {
		__bfq_bic_change_cgroup(bfqd, bic, cgroup);
		bfq_put_bfqd_unlock(bfqd, &flags);
	}
}

/**
 * bfq_bic_update_cgroup - update the cgroup of @bic.
 * @bic: the @bic to update.
 *
 * Make sure that @bic is enqueued in the cgroup of the current task.
 * We need this in addition to moving bics during the cgroup attach
 * phase because the task owning @bic could be at its first disk
 * access or we may end up in the root cgroup as the result of a
 * memory allocation failure and here we try to move to the right
 * group.
 *
 * Must be called under the queue lock.  It is safe to use the returned
 * value even after the rcu_read_unlock() as the migration/destruction
 * paths act under the queue lock too.  IOW it is impossible to race with
 * group migration/destruction and end up with an invalid group as:
 *   a) here cgroup has not yet been destroyed, nor its destroy callback
 *      has started execution, as current holds a reference to it,
 *   b) if it is destroyed after rcu_read_unlock() [after current is
 *      migrated to a different cgroup] its attach() callback will have
 *      taken care of remove all the references to the old cgroup data.
 */
static struct bfq_group *bfq_bic_update_cgroup(struct bfq_io_cq *bic)
{
	struct bfq_data *bfqd = bic_to_bfqd(bic);
	struct bfq_group *bfqg;
	struct cgroup *cgroup;

	BUG_ON(bfqd == NULL);

	rcu_read_lock();
	cgroup = task_cgroup(current, bfqio_subsys_id);
	bfqg = __bfq_bic_change_cgroup(bfqd, bic, cgroup);
	rcu_read_unlock();

	return bfqg;
}

/**
 * bfq_flush_idle_tree - deactivate any entity on the idle tree of @st.
 * @st: the service tree being flushed.
 */
static inline void bfq_flush_idle_tree(struct bfq_service_tree *st)
{
	struct bfq_entity *entity = st->first_idle;

	for (; entity != NULL; entity = st->first_idle)
		__bfq_deactivate_entity(entity, 0);
}

/**
 * bfq_reparent_leaf_entity - move leaf entity to the root_group.
 * @bfqd: the device data structure with the root group.
 * @entity: the entity to move.
 */
static inline void bfq_reparent_leaf_entity(struct bfq_data *bfqd,
					    struct bfq_entity *entity)
{
	struct bfq_queue *bfqq = bfq_entity_to_bfqq(entity);

	BUG_ON(bfqq == NULL);
	bfq_bfqq_move(bfqd, bfqq, entity, bfqd->root_group);
	return;
}

/**
 * bfq_reparent_active_entities - move to the root group all active
 *                                entities.
 * @bfqd: the device data structure with the root group.
 * @bfqg: the group to move from.
 * @st: the service tree with the entities.
 *
 * Needs queue_lock to be taken and reference to be valid over the call.
 */
static inline void bfq_reparent_active_entities(struct bfq_data *bfqd,
						struct bfq_group *bfqg,
						struct bfq_service_tree *st)
{
	struct rb_root *active = &st->active;
	struct bfq_entity *entity = NULL;

	if (!RB_EMPTY_ROOT(&st->active))
		entity = bfq_entity_of(rb_first(active));

	for (; entity != NULL; entity = bfq_entity_of(rb_first(active)))
		bfq_reparent_leaf_entity(bfqd, entity);

	if (bfqg->sched_data.in_service_entity != NULL)
		bfq_reparent_leaf_entity(bfqd,
			bfqg->sched_data.in_service_entity);

	return;
}

/**
 * bfq_destroy_group - destroy @bfqg.
 * @bgrp: the bfqio_cgroup containing @bfqg.
 * @bfqg: the group being destroyed.
 *
 * Destroy @bfqg, making sure that it is not referenced from its parent.
 */
static void bfq_destroy_group(struct bfqio_cgroup *bgrp, struct bfq_group *bfqg)
{
	struct bfq_data *bfqd;
	struct bfq_service_tree *st;
	struct bfq_entity *entity = bfqg->my_entity;
	unsigned long uninitialized_var(flags);
	int i;

	hlist_del(&bfqg->group_node);

	/*
	 * Empty all service_trees belonging to this group before
	 * deactivating the group itself.
	 */
	for (i = 0; i < BFQ_IOPRIO_CLASSES; i++) {
		st = bfqg->sched_data.service_tree + i;

		/*
		 * The idle tree may still contain bfq_queues belonging
		 * to exited task because they never migrated to a different
		 * cgroup from the one being destroyed now.  No one else
		 * can access them so it's safe to act without any lock.
		 */
		bfq_flush_idle_tree(st);

		/*
		 * It may happen that some queues are still active
		 * (busy) upon group destruction (if the corresponding
		 * processes have been forced to terminate). We move
		 * all the leaf entities corresponding to these queues
		 * to the root_group.
		 * Also, it may happen that the group has an entity
		 * in service, which is disconnected from the active
		 * tree: it must be moved, too.
		 * There is no need to put the sync queues, as the
		 * scheduler has taken no reference.
		 */
		bfqd = bfq_get_bfqd_locked(&bfqg->bfqd, &flags);
		if (bfqd != NULL) {
			bfq_reparent_active_entities(bfqd, bfqg, st);
			bfq_put_bfqd_unlock(bfqd, &flags);
		}
		BUG_ON(!RB_EMPTY_ROOT(&st->active));
		BUG_ON(!RB_EMPTY_ROOT(&st->idle));
	}
	BUG_ON(bfqg->sched_data.next_in_service != NULL);
	BUG_ON(bfqg->sched_data.in_service_entity != NULL);

	/*
	 * We may race with device destruction, take extra care when
	 * dereferencing bfqg->bfqd.
	 */
	bfqd = bfq_get_bfqd_locked(&bfqg->bfqd, &flags);
	if (bfqd != NULL) {
		hlist_del(&bfqg->bfqd_node);
		__bfq_deactivate_entity(entity, 0);
		bfq_put_async_queues(bfqd, bfqg);
		bfq_put_bfqd_unlock(bfqd, &flags);
	}
	BUG_ON(entity->tree != NULL);

	/*
	 * No need to defer the kfree() to the end of the RCU grace
	 * period: we are called from the destroy() callback of our
	 * cgroup, so we can be sure that no one is a) still using
	 * this cgroup or b) doing lookups in it.
	 */
	kfree(bfqg);
}

static void bfq_end_wr_async(struct bfq_data *bfqd)
{
	struct hlist_node *pos, *n;
	struct bfq_group *bfqg;

	hlist_for_each_entry_safe(bfqg, pos, n, &bfqd->group_list, bfqd_node)
		bfq_end_wr_async_queues(bfqd, bfqg);
	bfq_end_wr_async_queues(bfqd, bfqd->root_group);
}

/**
 * bfq_disconnect_groups - disconnect @bfqd from all its groups.
 * @bfqd: the device descriptor being exited.
 *
 * When the device exits we just make sure that no lookup can return
 * the now unused group structures.  They will be deallocated on cgroup
 * destruction.
 */
static void bfq_disconnect_groups(struct bfq_data *bfqd)
{
	struct hlist_node *pos, *n;
	struct bfq_group *bfqg;

	bfq_log(bfqd, "disconnect_groups beginning");
	hlist_for_each_entry_safe(bfqg, pos, n, &bfqd->group_list, bfqd_node) {
		hlist_del(&bfqg->bfqd_node);

		__bfq_deactivate_entity(bfqg->my_entity, 0);

		/*
		 * Don't remove from the group hash, just set an
		 * invalid key.  No lookups can race with the
		 * assignment as bfqd is being destroyed; this
		 * implies also that new elements cannot be added
		 * to the list.
		 */
		rcu_assign_pointer(bfqg->bfqd, NULL);

		bfq_log(bfqd, "disconnect_groups: put async for group %p",
			bfqg);
		bfq_put_async_queues(bfqd, bfqg);
	}
}

static inline void bfq_free_root_group(struct bfq_data *bfqd)
{
	struct bfqio_cgroup *bgrp = &bfqio_root_cgroup;
	struct bfq_group *bfqg = bfqd->root_group;

	bfq_put_async_queues(bfqd, bfqg);

	spin_lock_irq(&bgrp->lock);
	hlist_del_rcu(&bfqg->group_node);
	spin_unlock_irq(&bgrp->lock);

	/*
	 * No need to synchronize_rcu() here: since the device is gone
	 * there cannot be any read-side access to its root_group.
	 */
	kfree(bfqg);
}

static struct bfq_group *bfq_alloc_root_group(struct bfq_data *bfqd, int node)
{
	struct bfq_group *bfqg;
	struct bfqio_cgroup *bgrp;
	int i;

	bfqg = kzalloc_node(sizeof(*bfqg), GFP_KERNEL, node);
	if (bfqg == NULL)
		return NULL;

	bfqg->entity.parent = NULL;
	for (i = 0; i < BFQ_IOPRIO_CLASSES; i++)
		bfqg->sched_data.service_tree[i] = BFQ_SERVICE_TREE_INIT;

	bgrp = &bfqio_root_cgroup;
	spin_lock_irq(&bgrp->lock);
	rcu_assign_pointer(bfqg->bfqd, bfqd);
	hlist_add_head_rcu(&bfqg->group_node, &bgrp->group_data);
	spin_unlock_irq(&bgrp->lock);

	return bfqg;
}

#define SHOW_FUNCTION(__VAR)						\
static u64 bfqio_cgroup_##__VAR##_read(struct cgroup *cgroup,		\
				       struct cftype *cftype)		\
{									\
	struct bfqio_cgroup *bgrp;					\
	u64 ret;							\
									\
	if (!cgroup_lock_live_group(cgroup))				\
		return -ENODEV;						\
									\
	bgrp = cgroup_to_bfqio(cgroup);					\
	spin_lock_irq(&bgrp->lock);					\
	ret = bgrp->__VAR;						\
	spin_unlock_irq(&bgrp->lock);					\
									\
	cgroup_unlock();						\
									\
	return ret;							\
}

SHOW_FUNCTION(weight);
SHOW_FUNCTION(ioprio);
SHOW_FUNCTION(ioprio_class);
#undef SHOW_FUNCTION

#define STORE_FUNCTION(__VAR, __MIN, __MAX)				\
static int bfqio_cgroup_##__VAR##_write(struct cgroup *cgroup,		\
					struct cftype *cftype,		\
					u64 val)			\
{									\
	struct bfqio_cgroup *bgrp;					\
	struct bfq_group *bfqg;						\
	struct hlist_node *n;						\
									\
	if (val < (__MIN) || val > (__MAX))				\
		return -EINVAL;						\
									\
	if (!cgroup_lock_live_group(cgroup))				\
		return -ENODEV;						\
									\
	bgrp = cgroup_to_bfqio(cgroup);					\
									\
	spin_lock_irq(&bgrp->lock);					\
	bgrp->__VAR = (unsigned short)val;				\
	hlist_for_each_entry(bfqg, n, &bgrp->group_data, group_node) {	\
		/*                                                      \
		 * Setting the ioprio_changed flag of the entity        \
		 * to 1 with new_##__VAR == ##__VAR would re-set        \
		 * the value of the weight to its ioprio mapping.       \
		 * Set the flag only if necessary.			\
		 */							\
		if ((unsigned short)val != bfqg->entity.new_##__VAR) {  \
			bfqg->entity.new_##__VAR = (unsigned short)val; \
			/*						\
			 * Make sure that the above new value has been	\
			 * stored in bfqg->entity.new_##__VAR before	\
			 * setting the ioprio_changed flag. In fact,	\
			 * this flag may be read asynchronously (in	\
			 * critical sections protected by a different	\
			 * lock than that held here), and finding this	\
			 * flag set may cause the execution of the code	\
			 * for updating parameters whose value may	\
			 * depend also on bfqg->entity.new_##__VAR (in	\
			 * __bfq_entity_update_weight_prio).		\
			 * This barrier makes sure that the new value	\
			 * of bfqg->entity.new_##__VAR is correctly	\
			 * seen in that code.				\
			 */						\
			smp_wmb();                                      \
			bfqg->entity.ioprio_changed = 1;                \
		}                                                       \
	}								\
	spin_unlock_irq(&bgrp->lock);					\
									\
	cgroup_unlock();						\
									\
	return 0;							\
}

STORE_FUNCTION(weight, BFQ_MIN_WEIGHT, BFQ_MAX_WEIGHT);
STORE_FUNCTION(ioprio, 0, IOPRIO_BE_NR - 1);
STORE_FUNCTION(ioprio_class, IOPRIO_CLASS_RT, IOPRIO_CLASS_IDLE);
#undef STORE_FUNCTION

static struct cftype bfqio_files[] = {
	{
		.name = "weight",
		.read_u64 = bfqio_cgroup_weight_read,
		.write_u64 = bfqio_cgroup_weight_write,
	},
	{
		.name = "ioprio",
		.read_u64 = bfqio_cgroup_ioprio_read,
		.write_u64 = bfqio_cgroup_ioprio_write,
	},
	{
		.name = "ioprio_class",
		.read_u64 = bfqio_cgroup_ioprio_class_read,
		.write_u64 = bfqio_cgroup_ioprio_class_write,
	},
};

static int bfqio_populate(struct cgroup_subsys *subsys, struct cgroup *cgroup)
{
	return cgroup_add_files(cgroup, subsys, bfqio_files,
				ARRAY_SIZE(bfqio_files));
}

static struct cgroup_subsys_state *bfqio_create(struct cgroup *cgroup)
{
	struct bfqio_cgroup *bgrp;

	if (cgroup->parent != NULL) {
		bgrp = kzalloc(sizeof(*bgrp), GFP_KERNEL);
		if (bgrp == NULL)
			return ERR_PTR(-ENOMEM);
	} else
		bgrp = &bfqio_root_cgroup;

	spin_lock_init(&bgrp->lock);
	INIT_HLIST_HEAD(&bgrp->group_data);
	bgrp->ioprio = BFQ_DEFAULT_GRP_IOPRIO;
	bgrp->ioprio_class = BFQ_DEFAULT_GRP_CLASS;

	return &bgrp->css;
}

/*
 * We cannot support shared io contexts, as we have no means to support
 * two tasks with the same ioc in two different groups without major rework
 * of the main bic/bfqq data structures.  By now we allow a task to change
 * its cgroup only if it's the only owner of its ioc; the drawback of this
 * behavior is that a group containing a task that forked using CLONE_IO
 * will not be destroyed until the tasks sharing the ioc die.
 */
static int bfqio_can_attach(struct cgroup *cgroup, struct cgroup_taskset *tset)
=======
struct blkio_cgroup *cgroup_to_blkio_cgroup(struct cgroup *cgroup)
{
	return container_of(cgroup_subsys_state(cgroup, blkio_subsys_id),
			    struct blkio_cgroup, css);
}
EXPORT_SYMBOL_GPL(cgroup_to_blkio_cgroup);

struct blkio_cgroup *task_blkio_cgroup(struct task_struct *tsk)
{
	return container_of(task_subsys_state(tsk, blkio_subsys_id),
			    struct blkio_cgroup, css);
}
EXPORT_SYMBOL_GPL(task_blkio_cgroup);

static inline void
blkio_update_group_weight(struct blkio_group *blkg, unsigned int weight)
{
	struct blkio_policy_type *blkiop;

	list_for_each_entry(blkiop, &blkio_list, list) {
		/* If this policy does not own the blkg, do not send updates */
		if (blkiop->plid != blkg->plid)
			continue;
		if (blkiop->ops.blkio_update_group_weight_fn)
			blkiop->ops.blkio_update_group_weight_fn(blkg->key,
							blkg, weight);
	}
}

static inline void blkio_update_group_bps(struct blkio_group *blkg, u64 bps,
				int fileid)
{
	struct blkio_policy_type *blkiop;

	list_for_each_entry(blkiop, &blkio_list, list) {

		/* If this policy does not own the blkg, do not send updates */
		if (blkiop->plid != blkg->plid)
			continue;

		if (fileid == BLKIO_THROTL_read_bps_device
		    && blkiop->ops.blkio_update_group_read_bps_fn)
			blkiop->ops.blkio_update_group_read_bps_fn(blkg->key,
								blkg, bps);

		if (fileid == BLKIO_THROTL_write_bps_device
		    && blkiop->ops.blkio_update_group_write_bps_fn)
			blkiop->ops.blkio_update_group_write_bps_fn(blkg->key,
								blkg, bps);
	}
}

static inline void blkio_update_group_iops(struct blkio_group *blkg,
			unsigned int iops, int fileid)
{
	struct blkio_policy_type *blkiop;

	list_for_each_entry(blkiop, &blkio_list, list) {

		/* If this policy does not own the blkg, do not send updates */
		if (blkiop->plid != blkg->plid)
			continue;

		if (fileid == BLKIO_THROTL_read_iops_device
		    && blkiop->ops.blkio_update_group_read_iops_fn)
			blkiop->ops.blkio_update_group_read_iops_fn(blkg->key,
								blkg, iops);

		if (fileid == BLKIO_THROTL_write_iops_device
		    && blkiop->ops.blkio_update_group_write_iops_fn)
			blkiop->ops.blkio_update_group_write_iops_fn(blkg->key,
								blkg,iops);
	}
}

/*
 * Add to the appropriate stat variable depending on the request type.
 * This should be called with the blkg->stats_lock held.
 */
static void blkio_add_stat(uint64_t *stat, uint64_t add, bool direction,
				bool sync)
{
	if (direction)
		stat[BLKIO_STAT_WRITE] += add;
	else
		stat[BLKIO_STAT_READ] += add;
	if (sync)
		stat[BLKIO_STAT_SYNC] += add;
	else
		stat[BLKIO_STAT_ASYNC] += add;
}

/*
 * Decrements the appropriate stat variable if non-zero depending on the
 * request type. Panics on value being zero.
 * This should be called with the blkg->stats_lock held.
 */
static void blkio_check_and_dec_stat(uint64_t *stat, bool direction, bool sync)
{
	if (direction) {
		BUG_ON(stat[BLKIO_STAT_WRITE] == 0);
		stat[BLKIO_STAT_WRITE]--;
	} else {
		BUG_ON(stat[BLKIO_STAT_READ] == 0);
		stat[BLKIO_STAT_READ]--;
	}
	if (sync) {
		BUG_ON(stat[BLKIO_STAT_SYNC] == 0);
		stat[BLKIO_STAT_SYNC]--;
	} else {
		BUG_ON(stat[BLKIO_STAT_ASYNC] == 0);
		stat[BLKIO_STAT_ASYNC]--;
	}
}

#ifdef CONFIG_DEBUG_BLK_CGROUP
/* This should be called with the blkg->stats_lock held. */
static void blkio_set_start_group_wait_time(struct blkio_group *blkg,
						struct blkio_group *curr_blkg)
{
	if (blkio_blkg_waiting(&blkg->stats))
		return;
	if (blkg == curr_blkg)
		return;
	blkg->stats.start_group_wait_time = sched_clock();
	blkio_mark_blkg_waiting(&blkg->stats);
}

/* This should be called with the blkg->stats_lock held. */
static void blkio_update_group_wait_time(struct blkio_group_stats *stats)
{
	unsigned long long now;

	if (!blkio_blkg_waiting(stats))
		return;

	now = sched_clock();
	if (time_after64(now, stats->start_group_wait_time))
		stats->group_wait_time += now - stats->start_group_wait_time;
	blkio_clear_blkg_waiting(stats);
}

/* This should be called with the blkg->stats_lock held. */
static void blkio_end_empty_time(struct blkio_group_stats *stats)
{
	unsigned long long now;

	if (!blkio_blkg_empty(stats))
		return;

	now = sched_clock();
	if (time_after64(now, stats->start_empty_time))
		stats->empty_time += now - stats->start_empty_time;
	blkio_clear_blkg_empty(stats);
}

void blkiocg_update_set_idle_time_stats(struct blkio_group *blkg)
{
	unsigned long flags;

	spin_lock_irqsave(&blkg->stats_lock, flags);
	BUG_ON(blkio_blkg_idling(&blkg->stats));
	blkg->stats.start_idle_time = sched_clock();
	blkio_mark_blkg_idling(&blkg->stats);
	spin_unlock_irqrestore(&blkg->stats_lock, flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_set_idle_time_stats);

void blkiocg_update_idle_time_stats(struct blkio_group *blkg)
{
	unsigned long flags;
	unsigned long long now;
	struct blkio_group_stats *stats;

	spin_lock_irqsave(&blkg->stats_lock, flags);
	stats = &blkg->stats;
	if (blkio_blkg_idling(stats)) {
		now = sched_clock();
		if (time_after64(now, stats->start_idle_time))
			stats->idle_time += now - stats->start_idle_time;
		blkio_clear_blkg_idling(stats);
	}
	spin_unlock_irqrestore(&blkg->stats_lock, flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_idle_time_stats);

void blkiocg_update_avg_queue_size_stats(struct blkio_group *blkg)
{
	unsigned long flags;
	struct blkio_group_stats *stats;

	spin_lock_irqsave(&blkg->stats_lock, flags);
	stats = &blkg->stats;
	stats->avg_queue_size_sum +=
			stats->stat_arr[BLKIO_STAT_QUEUED][BLKIO_STAT_READ] +
			stats->stat_arr[BLKIO_STAT_QUEUED][BLKIO_STAT_WRITE];
	stats->avg_queue_size_samples++;
	blkio_update_group_wait_time(stats);
	spin_unlock_irqrestore(&blkg->stats_lock, flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_avg_queue_size_stats);

void blkiocg_set_start_empty_time(struct blkio_group *blkg)
{
	unsigned long flags;
	struct blkio_group_stats *stats;

	spin_lock_irqsave(&blkg->stats_lock, flags);
	stats = &blkg->stats;

	if (stats->stat_arr[BLKIO_STAT_QUEUED][BLKIO_STAT_READ] ||
			stats->stat_arr[BLKIO_STAT_QUEUED][BLKIO_STAT_WRITE]) {
		spin_unlock_irqrestore(&blkg->stats_lock, flags);
		return;
	}

	/*
	 * group is already marked empty. This can happen if cfqq got new
	 * request in parent group and moved to this group while being added
	 * to service tree. Just ignore the event and move on.
	 */
	if(blkio_blkg_empty(stats)) {
		spin_unlock_irqrestore(&blkg->stats_lock, flags);
		return;
	}

	stats->start_empty_time = sched_clock();
	blkio_mark_blkg_empty(stats);
	spin_unlock_irqrestore(&blkg->stats_lock, flags);
}
EXPORT_SYMBOL_GPL(blkiocg_set_start_empty_time);

void blkiocg_update_dequeue_stats(struct blkio_group *blkg,
			unsigned long dequeue)
{
	blkg->stats.dequeue += dequeue;
}
EXPORT_SYMBOL_GPL(blkiocg_update_dequeue_stats);
#else
static inline void blkio_set_start_group_wait_time(struct blkio_group *blkg,
					struct blkio_group *curr_blkg) {}
static inline void blkio_end_empty_time(struct blkio_group_stats *stats) {}
#endif

void blkiocg_update_io_add_stats(struct blkio_group *blkg,
			struct blkio_group *curr_blkg, bool direction,
			bool sync)
{
	unsigned long flags;

	spin_lock_irqsave(&blkg->stats_lock, flags);
	blkio_add_stat(blkg->stats.stat_arr[BLKIO_STAT_QUEUED], 1, direction,
			sync);
	blkio_end_empty_time(&blkg->stats);
	blkio_set_start_group_wait_time(blkg, curr_blkg);
	spin_unlock_irqrestore(&blkg->stats_lock, flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_io_add_stats);

void blkiocg_update_io_remove_stats(struct blkio_group *blkg,
						bool direction, bool sync)
{
	unsigned long flags;

	spin_lock_irqsave(&blkg->stats_lock, flags);
	blkio_check_and_dec_stat(blkg->stats.stat_arr[BLKIO_STAT_QUEUED],
					direction, sync);
	spin_unlock_irqrestore(&blkg->stats_lock, flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_io_remove_stats);

void blkiocg_update_timeslice_used(struct blkio_group *blkg, unsigned long time,
				unsigned long unaccounted_time)
{
	unsigned long flags;

	spin_lock_irqsave(&blkg->stats_lock, flags);
	blkg->stats.time += time;
#ifdef CONFIG_DEBUG_BLK_CGROUP
	blkg->stats.unaccounted_time += unaccounted_time;
#endif
	spin_unlock_irqrestore(&blkg->stats_lock, flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_timeslice_used);

/*
 * should be called under rcu read lock or queue lock to make sure blkg pointer
 * is valid.
 */
void blkiocg_update_dispatch_stats(struct blkio_group *blkg,
				uint64_t bytes, bool direction, bool sync)
{
	struct blkio_group_stats_cpu *stats_cpu;
	unsigned long flags;

	/*
	 * Disabling interrupts to provide mutual exclusion between two
	 * writes on same cpu. It probably is not needed for 64bit. Not
	 * optimizing that case yet.
	 */
	local_irq_save(flags);

	stats_cpu = this_cpu_ptr(blkg->stats_cpu);

	u64_stats_update_begin(&stats_cpu->syncp);
	stats_cpu->sectors += bytes >> 9;
	blkio_add_stat(stats_cpu->stat_arr_cpu[BLKIO_STAT_CPU_SERVICED],
			1, direction, sync);
	blkio_add_stat(stats_cpu->stat_arr_cpu[BLKIO_STAT_CPU_SERVICE_BYTES],
			bytes, direction, sync);
	u64_stats_update_end(&stats_cpu->syncp);
	local_irq_restore(flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_dispatch_stats);

void blkiocg_update_completion_stats(struct blkio_group *blkg,
	uint64_t start_time, uint64_t io_start_time, bool direction, bool sync)
{
	struct blkio_group_stats *stats;
	unsigned long flags;
	unsigned long long now = sched_clock();

	spin_lock_irqsave(&blkg->stats_lock, flags);
	stats = &blkg->stats;
	if (time_after64(now, io_start_time))
		blkio_add_stat(stats->stat_arr[BLKIO_STAT_SERVICE_TIME],
				now - io_start_time, direction, sync);
	if (time_after64(io_start_time, start_time))
		blkio_add_stat(stats->stat_arr[BLKIO_STAT_WAIT_TIME],
				io_start_time - start_time, direction, sync);
	spin_unlock_irqrestore(&blkg->stats_lock, flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_completion_stats);

/*  Merged stats are per cpu.  */
void blkiocg_update_io_merged_stats(struct blkio_group *blkg, bool direction,
					bool sync)
{
	struct blkio_group_stats_cpu *stats_cpu;
	unsigned long flags;

	/*
	 * Disabling interrupts to provide mutual exclusion between two
	 * writes on same cpu. It probably is not needed for 64bit. Not
	 * optimizing that case yet.
	 */
	local_irq_save(flags);

	stats_cpu = this_cpu_ptr(blkg->stats_cpu);

	u64_stats_update_begin(&stats_cpu->syncp);
	blkio_add_stat(stats_cpu->stat_arr_cpu[BLKIO_STAT_CPU_MERGED], 1,
				direction, sync);
	u64_stats_update_end(&stats_cpu->syncp);
	local_irq_restore(flags);
}
EXPORT_SYMBOL_GPL(blkiocg_update_io_merged_stats);

/*
 * This function allocates the per cpu stats for blkio_group. Should be called
 * from sleepable context as alloc_per_cpu() requires that.
 */
int blkio_alloc_blkg_stats(struct blkio_group *blkg)
{
	/* Allocate memory for per cpu stats */
	blkg->stats_cpu = alloc_percpu(struct blkio_group_stats_cpu);
	if (!blkg->stats_cpu)
		return -ENOMEM;
	return 0;
}
EXPORT_SYMBOL_GPL(blkio_alloc_blkg_stats);

void blkiocg_add_blkio_group(struct blkio_cgroup *blkcg,
		struct blkio_group *blkg, void *key, dev_t dev,
		enum blkio_policy_id plid)
{
	unsigned long flags;

	spin_lock_irqsave(&blkcg->lock, flags);
	spin_lock_init(&blkg->stats_lock);
	rcu_assign_pointer(blkg->key, key);
	blkg->blkcg_id = css_id(&blkcg->css);
	hlist_add_head_rcu(&blkg->blkcg_node, &blkcg->blkg_list);
	blkg->plid = plid;
	spin_unlock_irqrestore(&blkcg->lock, flags);
	/* Need to take css reference ? */
	cgroup_path(blkcg->css.cgroup, blkg->path, sizeof(blkg->path));
	blkg->dev = dev;
}
EXPORT_SYMBOL_GPL(blkiocg_add_blkio_group);

static void __blkiocg_del_blkio_group(struct blkio_group *blkg)
{
	hlist_del_init_rcu(&blkg->blkcg_node);
	blkg->blkcg_id = 0;
}

/*
 * returns 0 if blkio_group was still on cgroup list. Otherwise returns 1
 * indicating that blk_group was unhashed by the time we got to it.
 */
int blkiocg_del_blkio_group(struct blkio_group *blkg)
{
	struct blkio_cgroup *blkcg;
	unsigned long flags;
	struct cgroup_subsys_state *css;
	int ret = 1;

	rcu_read_lock();
	css = css_lookup(&blkio_subsys, blkg->blkcg_id);
	if (css) {
		blkcg = container_of(css, struct blkio_cgroup, css);
		spin_lock_irqsave(&blkcg->lock, flags);
		if (!hlist_unhashed(&blkg->blkcg_node)) {
			__blkiocg_del_blkio_group(blkg);
			ret = 0;
		}
		spin_unlock_irqrestore(&blkcg->lock, flags);
	}

	rcu_read_unlock();
	return ret;
}
EXPORT_SYMBOL_GPL(blkiocg_del_blkio_group);

/* called under rcu_read_lock(). */
struct blkio_group *blkiocg_lookup_group(struct blkio_cgroup *blkcg, void *key)
{
	struct blkio_group *blkg;
	struct hlist_node *n;
	void *__key;

	hlist_for_each_entry_rcu(blkg, n, &blkcg->blkg_list, blkcg_node) {
		__key = blkg->key;
		if (__key == key)
			return blkg;
	}

	return NULL;
}
EXPORT_SYMBOL_GPL(blkiocg_lookup_group);

static void blkio_reset_stats_cpu(struct blkio_group *blkg)
{
	struct blkio_group_stats_cpu *stats_cpu;
	int i, j, k;
	/*
	 * Note: On 64 bit arch this should not be an issue. This has the
	 * possibility of returning some inconsistent value on 32bit arch
	 * as 64bit update on 32bit is non atomic. Taking care of this
	 * corner case makes code very complicated, like sending IPIs to
	 * cpus, taking care of stats of offline cpus etc.
	 *
	 * reset stats is anyway more of a debug feature and this sounds a
	 * corner case. So I am not complicating the code yet until and
	 * unless this becomes a real issue.
	 */
	for_each_possible_cpu(i) {
		stats_cpu = per_cpu_ptr(blkg->stats_cpu, i);
		stats_cpu->sectors = 0;
		for(j = 0; j < BLKIO_STAT_CPU_NR; j++)
			for (k = 0; k < BLKIO_STAT_TOTAL; k++)
				stats_cpu->stat_arr_cpu[j][k] = 0;
	}
}

static int
blkiocg_reset_stats(struct cgroup *cgroup, struct cftype *cftype, u64 val)
{
	struct blkio_cgroup *blkcg;
	struct blkio_group *blkg;
	struct blkio_group_stats *stats;
	struct hlist_node *n;
	uint64_t queued[BLKIO_STAT_TOTAL];
	int i;
#ifdef CONFIG_DEBUG_BLK_CGROUP
	bool idling, waiting, empty;
	unsigned long long now = sched_clock();
#endif

	blkcg = cgroup_to_blkio_cgroup(cgroup);
	spin_lock_irq(&blkcg->lock);
	hlist_for_each_entry(blkg, n, &blkcg->blkg_list, blkcg_node) {
		spin_lock(&blkg->stats_lock);
		stats = &blkg->stats;
#ifdef CONFIG_DEBUG_BLK_CGROUP
		idling = blkio_blkg_idling(stats);
		waiting = blkio_blkg_waiting(stats);
		empty = blkio_blkg_empty(stats);
#endif
		for (i = 0; i < BLKIO_STAT_TOTAL; i++)
			queued[i] = stats->stat_arr[BLKIO_STAT_QUEUED][i];
		memset(stats, 0, sizeof(struct blkio_group_stats));
		for (i = 0; i < BLKIO_STAT_TOTAL; i++)
			stats->stat_arr[BLKIO_STAT_QUEUED][i] = queued[i];
#ifdef CONFIG_DEBUG_BLK_CGROUP
		if (idling) {
			blkio_mark_blkg_idling(stats);
			stats->start_idle_time = now;
		}
		if (waiting) {
			blkio_mark_blkg_waiting(stats);
			stats->start_group_wait_time = now;
		}
		if (empty) {
			blkio_mark_blkg_empty(stats);
			stats->start_empty_time = now;
		}
#endif
		spin_unlock(&blkg->stats_lock);

		/* Reset Per cpu stats which don't take blkg->stats_lock */
		blkio_reset_stats_cpu(blkg);
	}

	spin_unlock_irq(&blkcg->lock);
	return 0;
}

static void blkio_get_key_name(enum stat_sub_type type, dev_t dev, char *str,
				int chars_left, bool diskname_only)
{
	snprintf(str, chars_left, "%d:%d", MAJOR(dev), MINOR(dev));
	chars_left -= strlen(str);
	if (chars_left <= 0) {
		printk(KERN_WARNING
			"Possibly incorrect cgroup stat display format");
		return;
	}
	if (diskname_only)
		return;
	switch (type) {
	case BLKIO_STAT_READ:
		strlcat(str, " Read", chars_left);
		break;
	case BLKIO_STAT_WRITE:
		strlcat(str, " Write", chars_left);
		break;
	case BLKIO_STAT_SYNC:
		strlcat(str, " Sync", chars_left);
		break;
	case BLKIO_STAT_ASYNC:
		strlcat(str, " Async", chars_left);
		break;
	case BLKIO_STAT_TOTAL:
		strlcat(str, " Total", chars_left);
		break;
	default:
		strlcat(str, " Invalid", chars_left);
	}
}

static uint64_t blkio_fill_stat(char *str, int chars_left, uint64_t val,
				struct cgroup_map_cb *cb, dev_t dev)
{
	blkio_get_key_name(0, dev, str, chars_left, true);
	cb->fill(cb, str, val);
	return val;
}


static uint64_t blkio_read_stat_cpu(struct blkio_group *blkg,
			enum stat_type_cpu type, enum stat_sub_type sub_type)
{
	int cpu;
	struct blkio_group_stats_cpu *stats_cpu;
	u64 val = 0, tval;

	for_each_possible_cpu(cpu) {
		unsigned int start;
		stats_cpu  = per_cpu_ptr(blkg->stats_cpu, cpu);

		do {
			start = u64_stats_fetch_begin(&stats_cpu->syncp);
			if (type == BLKIO_STAT_CPU_SECTORS)
				tval = stats_cpu->sectors;
			else
				tval = stats_cpu->stat_arr_cpu[type][sub_type];
		} while(u64_stats_fetch_retry(&stats_cpu->syncp, start));

		val += tval;
	}

	return val;
}

static uint64_t blkio_get_stat_cpu(struct blkio_group *blkg,
		struct cgroup_map_cb *cb, dev_t dev, enum stat_type_cpu type)
{
	uint64_t disk_total, val;
	char key_str[MAX_KEY_LEN];
	enum stat_sub_type sub_type;

	if (type == BLKIO_STAT_CPU_SECTORS) {
		val = blkio_read_stat_cpu(blkg, type, 0);
		return blkio_fill_stat(key_str, MAX_KEY_LEN - 1, val, cb, dev);
	}

	for (sub_type = BLKIO_STAT_READ; sub_type < BLKIO_STAT_TOTAL;
			sub_type++) {
		blkio_get_key_name(sub_type, dev, key_str, MAX_KEY_LEN, false);
		val = blkio_read_stat_cpu(blkg, type, sub_type);
		cb->fill(cb, key_str, val);
	}

	disk_total = blkio_read_stat_cpu(blkg, type, BLKIO_STAT_READ) +
			blkio_read_stat_cpu(blkg, type, BLKIO_STAT_WRITE);

	blkio_get_key_name(BLKIO_STAT_TOTAL, dev, key_str, MAX_KEY_LEN, false);
	cb->fill(cb, key_str, disk_total);
	return disk_total;
}

/* This should be called with blkg->stats_lock held */
static uint64_t blkio_get_stat(struct blkio_group *blkg,
		struct cgroup_map_cb *cb, dev_t dev, enum stat_type type)
{
	uint64_t disk_total;
	char key_str[MAX_KEY_LEN];
	enum stat_sub_type sub_type;

	if (type == BLKIO_STAT_TIME)
		return blkio_fill_stat(key_str, MAX_KEY_LEN - 1,
					blkg->stats.time, cb, dev);
#ifdef CONFIG_DEBUG_BLK_CGROUP
	if (type == BLKIO_STAT_UNACCOUNTED_TIME)
		return blkio_fill_stat(key_str, MAX_KEY_LEN - 1,
					blkg->stats.unaccounted_time, cb, dev);
	if (type == BLKIO_STAT_AVG_QUEUE_SIZE) {
		uint64_t sum = blkg->stats.avg_queue_size_sum;
		uint64_t samples = blkg->stats.avg_queue_size_samples;
		if (samples)
			do_div(sum, samples);
		else
			sum = 0;
		return blkio_fill_stat(key_str, MAX_KEY_LEN - 1, sum, cb, dev);
	}
	if (type == BLKIO_STAT_GROUP_WAIT_TIME)
		return blkio_fill_stat(key_str, MAX_KEY_LEN - 1,
					blkg->stats.group_wait_time, cb, dev);
	if (type == BLKIO_STAT_IDLE_TIME)
		return blkio_fill_stat(key_str, MAX_KEY_LEN - 1,
					blkg->stats.idle_time, cb, dev);
	if (type == BLKIO_STAT_EMPTY_TIME)
		return blkio_fill_stat(key_str, MAX_KEY_LEN - 1,
					blkg->stats.empty_time, cb, dev);
	if (type == BLKIO_STAT_DEQUEUE)
		return blkio_fill_stat(key_str, MAX_KEY_LEN - 1,
					blkg->stats.dequeue, cb, dev);
#endif

	for (sub_type = BLKIO_STAT_READ; sub_type < BLKIO_STAT_TOTAL;
			sub_type++) {
		blkio_get_key_name(sub_type, dev, key_str, MAX_KEY_LEN, false);
		cb->fill(cb, key_str, blkg->stats.stat_arr[type][sub_type]);
	}
	disk_total = blkg->stats.stat_arr[type][BLKIO_STAT_READ] +
			blkg->stats.stat_arr[type][BLKIO_STAT_WRITE];
	blkio_get_key_name(BLKIO_STAT_TOTAL, dev, key_str, MAX_KEY_LEN, false);
	cb->fill(cb, key_str, disk_total);
	return disk_total;
}

static int blkio_policy_parse_and_set(char *buf,
	struct blkio_policy_node *newpn, enum blkio_policy_id plid, int fileid)
{
	struct gendisk *disk = NULL;
	char *s[4], *p, *major_s = NULL, *minor_s = NULL;
	unsigned long major, minor;
	int i = 0, ret = -EINVAL;
	int part;
	dev_t dev;
	u64 temp;

	memset(s, 0, sizeof(s));

	while ((p = strsep(&buf, " ")) != NULL) {
		if (!*p)
			continue;

		s[i++] = p;

		/* Prevent from inputing too many things */
		if (i == 3)
			break;
	}

	if (i != 2)
		goto out;

	p = strsep(&s[0], ":");
	if (p != NULL)
		major_s = p;
	else
		goto out;

	minor_s = s[0];
	if (!minor_s)
		goto out;

	if (strict_strtoul(major_s, 10, &major))
		goto out;

	if (strict_strtoul(minor_s, 10, &minor))
		goto out;

	dev = MKDEV(major, minor);

	if (strict_strtoull(s[1], 10, &temp))
		goto out;

	/* For rule removal, do not check for device presence. */
	if (temp) {
		disk = get_gendisk(dev, &part);
		if (!disk || part) {
			ret = -ENODEV;
			goto out;
		}
	}

	newpn->dev = dev;

	switch (plid) {
	case BLKIO_POLICY_PROP:
		if ((temp < BLKIO_WEIGHT_MIN && temp > 0) ||
		     temp > BLKIO_WEIGHT_MAX)
			goto out;

		newpn->plid = plid;
		newpn->fileid = fileid;
		newpn->val.weight = temp;
		break;
	case BLKIO_POLICY_THROTL:
		switch(fileid) {
		case BLKIO_THROTL_read_bps_device:
		case BLKIO_THROTL_write_bps_device:
			newpn->plid = plid;
			newpn->fileid = fileid;
			newpn->val.bps = temp;
			break;
		case BLKIO_THROTL_read_iops_device:
		case BLKIO_THROTL_write_iops_device:
			if (temp > THROTL_IOPS_MAX)
				goto out;

			newpn->plid = plid;
			newpn->fileid = fileid;
			newpn->val.iops = (unsigned int)temp;
			break;
		}
		break;
	default:
		BUG();
	}
	ret = 0;
out:
	put_disk(disk);
	return ret;
}

unsigned int blkcg_get_weight(struct blkio_cgroup *blkcg,
			      dev_t dev)
{
	struct blkio_policy_node *pn;
	unsigned long flags;
	unsigned int weight;

	spin_lock_irqsave(&blkcg->lock, flags);

	pn = blkio_policy_search_node(blkcg, dev, BLKIO_POLICY_PROP,
				BLKIO_PROP_weight_device);
	if (pn)
		weight = pn->val.weight;
	else
		weight = blkcg->weight;

	spin_unlock_irqrestore(&blkcg->lock, flags);

	return weight;
}
EXPORT_SYMBOL_GPL(blkcg_get_weight);

uint64_t blkcg_get_read_bps(struct blkio_cgroup *blkcg, dev_t dev)
{
	struct blkio_policy_node *pn;
	unsigned long flags;
	uint64_t bps = -1;

	spin_lock_irqsave(&blkcg->lock, flags);
	pn = blkio_policy_search_node(blkcg, dev, BLKIO_POLICY_THROTL,
				BLKIO_THROTL_read_bps_device);
	if (pn)
		bps = pn->val.bps;
	spin_unlock_irqrestore(&blkcg->lock, flags);

	return bps;
}

uint64_t blkcg_get_write_bps(struct blkio_cgroup *blkcg, dev_t dev)
{
	struct blkio_policy_node *pn;
	unsigned long flags;
	uint64_t bps = -1;

	spin_lock_irqsave(&blkcg->lock, flags);
	pn = blkio_policy_search_node(blkcg, dev, BLKIO_POLICY_THROTL,
				BLKIO_THROTL_write_bps_device);
	if (pn)
		bps = pn->val.bps;
	spin_unlock_irqrestore(&blkcg->lock, flags);

	return bps;
}

unsigned int blkcg_get_read_iops(struct blkio_cgroup *blkcg, dev_t dev)
{
	struct blkio_policy_node *pn;
	unsigned long flags;
	unsigned int iops = -1;

	spin_lock_irqsave(&blkcg->lock, flags);
	pn = blkio_policy_search_node(blkcg, dev, BLKIO_POLICY_THROTL,
				BLKIO_THROTL_read_iops_device);
	if (pn)
		iops = pn->val.iops;
	spin_unlock_irqrestore(&blkcg->lock, flags);

	return iops;
}

unsigned int blkcg_get_write_iops(struct blkio_cgroup *blkcg, dev_t dev)
{
	struct blkio_policy_node *pn;
	unsigned long flags;
	unsigned int iops = -1;

	spin_lock_irqsave(&blkcg->lock, flags);
	pn = blkio_policy_search_node(blkcg, dev, BLKIO_POLICY_THROTL,
				BLKIO_THROTL_write_iops_device);
	if (pn)
		iops = pn->val.iops;
	spin_unlock_irqrestore(&blkcg->lock, flags);

	return iops;
}

/* Checks whether user asked for deleting a policy rule */
static bool blkio_delete_rule_command(struct blkio_policy_node *pn)
{
	switch(pn->plid) {
	case BLKIO_POLICY_PROP:
		if (pn->val.weight == 0)
			return 1;
		break;
	case BLKIO_POLICY_THROTL:
		switch(pn->fileid) {
		case BLKIO_THROTL_read_bps_device:
		case BLKIO_THROTL_write_bps_device:
			if (pn->val.bps == 0)
				return 1;
			break;
		case BLKIO_THROTL_read_iops_device:
		case BLKIO_THROTL_write_iops_device:
			if (pn->val.iops == 0)
				return 1;
		}
		break;
	default:
		BUG();
	}

	return 0;
}

static void blkio_update_policy_rule(struct blkio_policy_node *oldpn,
					struct blkio_policy_node *newpn)
{
	switch(oldpn->plid) {
	case BLKIO_POLICY_PROP:
		oldpn->val.weight = newpn->val.weight;
		break;
	case BLKIO_POLICY_THROTL:
		switch(newpn->fileid) {
		case BLKIO_THROTL_read_bps_device:
		case BLKIO_THROTL_write_bps_device:
			oldpn->val.bps = newpn->val.bps;
			break;
		case BLKIO_THROTL_read_iops_device:
		case BLKIO_THROTL_write_iops_device:
			oldpn->val.iops = newpn->val.iops;
		}
		break;
	default:
		BUG();
	}
}

/*
 * Some rules/values in blkg have changed. Propagate those to respective
 * policies.
 */
static void blkio_update_blkg_policy(struct blkio_cgroup *blkcg,
		struct blkio_group *blkg, struct blkio_policy_node *pn)
{
	unsigned int weight, iops;
	u64 bps;

	switch(pn->plid) {
	case BLKIO_POLICY_PROP:
		weight = pn->val.weight ? pn->val.weight :
				blkcg->weight;
		blkio_update_group_weight(blkg, weight);
		break;
	case BLKIO_POLICY_THROTL:
		switch(pn->fileid) {
		case BLKIO_THROTL_read_bps_device:
		case BLKIO_THROTL_write_bps_device:
			bps = pn->val.bps ? pn->val.bps : (-1);
			blkio_update_group_bps(blkg, bps, pn->fileid);
			break;
		case BLKIO_THROTL_read_iops_device:
		case BLKIO_THROTL_write_iops_device:
			iops = pn->val.iops ? pn->val.iops : (-1);
			blkio_update_group_iops(blkg, iops, pn->fileid);
			break;
		}
		break;
	default:
		BUG();
	}
}

/*
 * A policy node rule has been updated. Propagate this update to all the
 * block groups which might be affected by this update.
 */
static void blkio_update_policy_node_blkg(struct blkio_cgroup *blkcg,
				struct blkio_policy_node *pn)
{
	struct blkio_group *blkg;
	struct hlist_node *n;

	spin_lock(&blkio_list_lock);
	spin_lock_irq(&blkcg->lock);

	hlist_for_each_entry(blkg, n, &blkcg->blkg_list, blkcg_node) {
		if (pn->dev != blkg->dev || pn->plid != blkg->plid)
			continue;
		blkio_update_blkg_policy(blkcg, blkg, pn);
	}

	spin_unlock_irq(&blkcg->lock);
	spin_unlock(&blkio_list_lock);
}

static int blkiocg_file_write(struct cgroup *cgrp, struct cftype *cft,
 				       const char *buffer)
{
	int ret = 0;
	char *buf;
	struct blkio_policy_node *newpn, *pn;
	struct blkio_cgroup *blkcg;
	int keep_newpn = 0;
	enum blkio_policy_id plid = BLKIOFILE_POLICY(cft->private);
	int fileid = BLKIOFILE_ATTR(cft->private);

	buf = kstrdup(buffer, GFP_KERNEL);
	if (!buf)
		return -ENOMEM;

	newpn = kzalloc(sizeof(*newpn), GFP_KERNEL);
	if (!newpn) {
		ret = -ENOMEM;
		goto free_buf;
	}

	ret = blkio_policy_parse_and_set(buf, newpn, plid, fileid);
	if (ret)
		goto free_newpn;

	blkcg = cgroup_to_blkio_cgroup(cgrp);

	spin_lock_irq(&blkcg->lock);

	pn = blkio_policy_search_node(blkcg, newpn->dev, plid, fileid);
	if (!pn) {
		if (!blkio_delete_rule_command(newpn)) {
			blkio_policy_insert_node(blkcg, newpn);
			keep_newpn = 1;
		}
		spin_unlock_irq(&blkcg->lock);
		goto update_io_group;
	}

	if (blkio_delete_rule_command(newpn)) {
		blkio_policy_delete_node(pn);
		kfree(pn);
		spin_unlock_irq(&blkcg->lock);
		goto update_io_group;
	}
	spin_unlock_irq(&blkcg->lock);

	blkio_update_policy_rule(pn, newpn);

update_io_group:
	blkio_update_policy_node_blkg(blkcg, newpn);

free_newpn:
	if (!keep_newpn)
		kfree(newpn);
free_buf:
	kfree(buf);
	return ret;
}

static void
blkio_print_policy_node(struct seq_file *m, struct blkio_policy_node *pn)
{
	switch(pn->plid) {
		case BLKIO_POLICY_PROP:
			if (pn->fileid == BLKIO_PROP_weight_device)
				seq_printf(m, "%u:%u\t%u\n", MAJOR(pn->dev),
					MINOR(pn->dev), pn->val.weight);
			break;
		case BLKIO_POLICY_THROTL:
			switch(pn->fileid) {
			case BLKIO_THROTL_read_bps_device:
			case BLKIO_THROTL_write_bps_device:
				seq_printf(m, "%u:%u\t%llu\n", MAJOR(pn->dev),
					MINOR(pn->dev), pn->val.bps);
				break;
			case BLKIO_THROTL_read_iops_device:
			case BLKIO_THROTL_write_iops_device:
				seq_printf(m, "%u:%u\t%u\n", MAJOR(pn->dev),
					MINOR(pn->dev), pn->val.iops);
				break;
			}
			break;
		default:
			BUG();
	}
}

/* cgroup files which read their data from policy nodes end up here */
static void blkio_read_policy_node_files(struct cftype *cft,
			struct blkio_cgroup *blkcg, struct seq_file *m)
{
	struct blkio_policy_node *pn;

	if (!list_empty(&blkcg->policy_list)) {
		spin_lock_irq(&blkcg->lock);
		list_for_each_entry(pn, &blkcg->policy_list, node) {
			if (!pn_matches_cftype(cft, pn))
				continue;
			blkio_print_policy_node(m, pn);
		}
		spin_unlock_irq(&blkcg->lock);
	}
}

static int blkiocg_file_read(struct cgroup *cgrp, struct cftype *cft,
				struct seq_file *m)
{
	struct blkio_cgroup *blkcg;
	enum blkio_policy_id plid = BLKIOFILE_POLICY(cft->private);
	int name = BLKIOFILE_ATTR(cft->private);

	blkcg = cgroup_to_blkio_cgroup(cgrp);

	switch(plid) {
	case BLKIO_POLICY_PROP:
		switch(name) {
		case BLKIO_PROP_weight_device:
			blkio_read_policy_node_files(cft, blkcg, m);
			return 0;
		default:
			BUG();
		}
		break;
	case BLKIO_POLICY_THROTL:
		switch(name){
		case BLKIO_THROTL_read_bps_device:
		case BLKIO_THROTL_write_bps_device:
		case BLKIO_THROTL_read_iops_device:
		case BLKIO_THROTL_write_iops_device:
			blkio_read_policy_node_files(cft, blkcg, m);
			return 0;
		default:
			BUG();
		}
		break;
	default:
		BUG();
	}

	return 0;
}

static int blkio_read_blkg_stats(struct blkio_cgroup *blkcg,
		struct cftype *cft, struct cgroup_map_cb *cb,
		enum stat_type type, bool show_total, bool pcpu)
{
	struct blkio_group *blkg;
	struct hlist_node *n;
	uint64_t cgroup_total = 0;

	rcu_read_lock();
	hlist_for_each_entry_rcu(blkg, n, &blkcg->blkg_list, blkcg_node) {
		if (blkg->dev) {
			if (!cftype_blkg_same_policy(cft, blkg))
				continue;
			if (pcpu)
				cgroup_total += blkio_get_stat_cpu(blkg, cb,
						blkg->dev, type);
			else {
				spin_lock_irq(&blkg->stats_lock);
				cgroup_total += blkio_get_stat(blkg, cb,
						blkg->dev, type);
				spin_unlock_irq(&blkg->stats_lock);
			}
		}
	}
	if (show_total)
		cb->fill(cb, "Total", cgroup_total);
	rcu_read_unlock();
	return 0;
}

/* All map kind of cgroup file get serviced by this function */
static int blkiocg_file_read_map(struct cgroup *cgrp, struct cftype *cft,
				struct cgroup_map_cb *cb)
{
	struct blkio_cgroup *blkcg;
	enum blkio_policy_id plid = BLKIOFILE_POLICY(cft->private);
	int name = BLKIOFILE_ATTR(cft->private);

	blkcg = cgroup_to_blkio_cgroup(cgrp);

	switch(plid) {
	case BLKIO_POLICY_PROP:
		switch(name) {
		case BLKIO_PROP_time:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_TIME, 0, 0);
		case BLKIO_PROP_sectors:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_CPU_SECTORS, 0, 1);
		case BLKIO_PROP_io_service_bytes:
			return blkio_read_blkg_stats(blkcg, cft, cb,
					BLKIO_STAT_CPU_SERVICE_BYTES, 1, 1);
		case BLKIO_PROP_io_serviced:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_CPU_SERVICED, 1, 1);
		case BLKIO_PROP_io_service_time:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_SERVICE_TIME, 1, 0);
		case BLKIO_PROP_io_wait_time:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_WAIT_TIME, 1, 0);
		case BLKIO_PROP_io_merged:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_CPU_MERGED, 1, 1);
		case BLKIO_PROP_io_queued:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_QUEUED, 1, 0);
#ifdef CONFIG_DEBUG_BLK_CGROUP
		case BLKIO_PROP_unaccounted_time:
			return blkio_read_blkg_stats(blkcg, cft, cb,
					BLKIO_STAT_UNACCOUNTED_TIME, 0, 0);
		case BLKIO_PROP_dequeue:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_DEQUEUE, 0, 0);
		case BLKIO_PROP_avg_queue_size:
			return blkio_read_blkg_stats(blkcg, cft, cb,
					BLKIO_STAT_AVG_QUEUE_SIZE, 0, 0);
		case BLKIO_PROP_group_wait_time:
			return blkio_read_blkg_stats(blkcg, cft, cb,
					BLKIO_STAT_GROUP_WAIT_TIME, 0, 0);
		case BLKIO_PROP_idle_time:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_IDLE_TIME, 0, 0);
		case BLKIO_PROP_empty_time:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_EMPTY_TIME, 0, 0);
#endif
		default:
			BUG();
		}
		break;
	case BLKIO_POLICY_THROTL:
		switch(name){
		case BLKIO_THROTL_io_service_bytes:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_CPU_SERVICE_BYTES, 1, 1);
		case BLKIO_THROTL_io_serviced:
			return blkio_read_blkg_stats(blkcg, cft, cb,
						BLKIO_STAT_CPU_SERVICED, 1, 1);
		default:
			BUG();
		}
		break;
	default:
		BUG();
	}

	return 0;
}

static int blkio_weight_write(struct blkio_cgroup *blkcg, u64 val)
{
	struct blkio_group *blkg;
	struct hlist_node *n;
	struct blkio_policy_node *pn;

	if (val < BLKIO_WEIGHT_MIN || val > BLKIO_WEIGHT_MAX)
		return -EINVAL;

	spin_lock(&blkio_list_lock);
	spin_lock_irq(&blkcg->lock);
	blkcg->weight = (unsigned int)val;

	hlist_for_each_entry(blkg, n, &blkcg->blkg_list, blkcg_node) {
		pn = blkio_policy_search_node(blkcg, blkg->dev,
				BLKIO_POLICY_PROP, BLKIO_PROP_weight_device);
		if (pn)
			continue;

		blkio_update_group_weight(blkg, blkcg->weight);
	}
	spin_unlock_irq(&blkcg->lock);
	spin_unlock(&blkio_list_lock);
	return 0;
}

static u64 blkiocg_file_read_u64 (struct cgroup *cgrp, struct cftype *cft) {
	struct blkio_cgroup *blkcg;
	enum blkio_policy_id plid = BLKIOFILE_POLICY(cft->private);
	int name = BLKIOFILE_ATTR(cft->private);

	blkcg = cgroup_to_blkio_cgroup(cgrp);

	switch(plid) {
	case BLKIO_POLICY_PROP:
		switch(name) {
		case BLKIO_PROP_weight:
			return (u64)blkcg->weight;
		}
		break;
	default:
		BUG();
	}
	return 0;
}

static int
blkiocg_file_write_u64(struct cgroup *cgrp, struct cftype *cft, u64 val)
{
	struct blkio_cgroup *blkcg;
	enum blkio_policy_id plid = BLKIOFILE_POLICY(cft->private);
	int name = BLKIOFILE_ATTR(cft->private);

	blkcg = cgroup_to_blkio_cgroup(cgrp);

	switch(plid) {
	case BLKIO_POLICY_PROP:
		switch(name) {
		case BLKIO_PROP_weight:
			return blkio_weight_write(blkcg, val);
		}
		break;
	default:
		BUG();
	}

	return 0;
}

struct cftype blkio_files[] = {
	{
		.name = "weight_device",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_weight_device),
		.read_seq_string = blkiocg_file_read,
		.write_string = blkiocg_file_write,
		.max_write_len = 256,
	},
	{
		.name = "weight",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_weight),
		.read_u64 = blkiocg_file_read_u64,
		.write_u64 = blkiocg_file_write_u64,
	},
	{
		.name = "time",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_time),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "sectors",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_sectors),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "io_service_bytes",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_io_service_bytes),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "io_serviced",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_io_serviced),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "io_service_time",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_io_service_time),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "io_wait_time",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_io_wait_time),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "io_merged",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_io_merged),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "io_queued",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_io_queued),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "reset_stats",
		.write_u64 = blkiocg_reset_stats,
	},
#ifdef CONFIG_BLK_DEV_THROTTLING
	{
		.name = "throttle.read_bps_device",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_THROTL,
				BLKIO_THROTL_read_bps_device),
		.read_seq_string = blkiocg_file_read,
		.write_string = blkiocg_file_write,
		.max_write_len = 256,
	},

	{
		.name = "throttle.write_bps_device",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_THROTL,
				BLKIO_THROTL_write_bps_device),
		.read_seq_string = blkiocg_file_read,
		.write_string = blkiocg_file_write,
		.max_write_len = 256,
	},

	{
		.name = "throttle.read_iops_device",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_THROTL,
				BLKIO_THROTL_read_iops_device),
		.read_seq_string = blkiocg_file_read,
		.write_string = blkiocg_file_write,
		.max_write_len = 256,
	},

	{
		.name = "throttle.write_iops_device",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_THROTL,
				BLKIO_THROTL_write_iops_device),
		.read_seq_string = blkiocg_file_read,
		.write_string = blkiocg_file_write,
		.max_write_len = 256,
	},
	{
		.name = "throttle.io_service_bytes",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_THROTL,
				BLKIO_THROTL_io_service_bytes),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "throttle.io_serviced",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_THROTL,
				BLKIO_THROTL_io_serviced),
		.read_map = blkiocg_file_read_map,
	},
#endif /* CONFIG_BLK_DEV_THROTTLING */

#ifdef CONFIG_DEBUG_BLK_CGROUP
	{
		.name = "avg_queue_size",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_avg_queue_size),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "group_wait_time",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_group_wait_time),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "idle_time",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_idle_time),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "empty_time",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_empty_time),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "dequeue",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_dequeue),
		.read_map = blkiocg_file_read_map,
	},
	{
		.name = "unaccounted_time",
		.private = BLKIOFILE_PRIVATE(BLKIO_POLICY_PROP,
				BLKIO_PROP_unaccounted_time),
		.read_map = blkiocg_file_read_map,
	},
#endif
};

static int blkiocg_populate(struct cgroup_subsys *subsys, struct cgroup *cgroup)
{
	return cgroup_add_files(cgroup, subsys, blkio_files,
				ARRAY_SIZE(blkio_files));
}

static void blkiocg_destroy(struct cgroup *cgroup)
{
	struct blkio_cgroup *blkcg = cgroup_to_blkio_cgroup(cgroup);
	unsigned long flags;
	struct blkio_group *blkg;
	void *key;
	struct blkio_policy_type *blkiop;
	struct blkio_policy_node *pn, *pntmp;

	rcu_read_lock();
	do {
		spin_lock_irqsave(&blkcg->lock, flags);

		if (hlist_empty(&blkcg->blkg_list)) {
			spin_unlock_irqrestore(&blkcg->lock, flags);
			break;
		}

		blkg = hlist_entry(blkcg->blkg_list.first, struct blkio_group,
					blkcg_node);
		key = rcu_dereference(blkg->key);
		__blkiocg_del_blkio_group(blkg);

		spin_unlock_irqrestore(&blkcg->lock, flags);

		/*
		 * This blkio_group is being unlinked as associated cgroup is
		 * going away. Let all the IO controlling policies know about
		 * this event.
		 */
		spin_lock(&blkio_list_lock);
		list_for_each_entry(blkiop, &blkio_list, list) {
			if (blkiop->plid != blkg->plid)
				continue;
			blkiop->ops.blkio_unlink_group_fn(key, blkg);
		}
		spin_unlock(&blkio_list_lock);
	} while (1);

	list_for_each_entry_safe(pn, pntmp, &blkcg->policy_list, node) {
		blkio_policy_delete_node(pn);
		kfree(pn);
	}

	free_css_id(&blkio_subsys, &blkcg->css);
	rcu_read_unlock();
	if (blkcg != &blkio_root_cgroup)
		kfree(blkcg);
}

static struct cgroup_subsys_state *blkiocg_create(struct cgroup *cgroup)
{
	struct blkio_cgroup *blkcg;
	struct cgroup *parent = cgroup->parent;

	if (!parent) {
		blkcg = &blkio_root_cgroup;
		goto done;
	}

	blkcg = kzalloc(sizeof(*blkcg), GFP_KERNEL);
	if (!blkcg)
		return ERR_PTR(-ENOMEM);

	blkcg->weight = BLKIO_WEIGHT_DEFAULT;
done:
	spin_lock_init(&blkcg->lock);
	INIT_HLIST_HEAD(&blkcg->blkg_list);

	INIT_LIST_HEAD(&blkcg->policy_list);
	return &blkcg->css;
}

/*
 * We cannot support shared io contexts, as we have no mean to support
 * two tasks with the same ioc in two different groups without major rework
 * of the main cic data structures.  For now we allow a task to change
 * its cgroup only if it's the only owner of its ioc.
 */
static int blkiocg_can_attach(struct cgroup *cgrp, struct cgroup_taskset *tset)
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
{
	struct task_struct *task;
	struct io_context *ioc;
	int ret = 0;

<<<<<<< HEAD
	cgroup_taskset_for_each(task, cgroup, tset) {
		/* task_lock() is needed to avoid races with exit_io_context() */
		task_lock(task);
		ioc = task->io_context;
		if (ioc != NULL && atomic_read(&ioc->nr_tasks) > 1)
			/*
			 * ioc == NULL means that the task is either too
			 * young or exiting: if it has still no ioc the
			 * ioc can't be shared, if the task is exiting the
			 * attach will fail anyway, no matter what we
			 * return here.
			 */
=======
	/* task_lock() is needed to avoid races with exit_io_context() */
	cgroup_taskset_for_each(task, cgrp, tset) {
		task_lock(task);
		ioc = task->io_context;
		if (ioc && atomic_read(&ioc->nr_tasks) > 1)
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
			ret = -EINVAL;
		task_unlock(task);
		if (ret)
			break;
	}
<<<<<<< HEAD

	return ret;
}

static void bfqio_attach(struct cgroup *cgroup, struct cgroup_taskset *tset)
{
	struct task_struct *task;
	struct io_context *ioc;
	struct io_cq *icq;
	struct hlist_node *n;

	/*
	 * IMPORTANT NOTE: The move of more than one process at a time to a
	 * new group has not yet been tested.
	 */
	cgroup_taskset_for_each(task, cgroup, tset) {
		ioc = get_task_io_context(task, GFP_ATOMIC, NUMA_NO_NODE);
		if (ioc) {
			/*
			 * Handle cgroup change here.
			 */
			rcu_read_lock();
			hlist_for_each_entry_rcu(icq, n, &ioc->icq_list, ioc_node)
				if (!strncmp(
					icq->q->elevator->type->elevator_name,
					"bfq", ELV_NAME_MAX))
					bfq_bic_change_cgroup(icq_to_bic(icq),
							      cgroup);
			rcu_read_unlock();
=======
	return ret;
}

static void blkiocg_attach(struct cgroup *cgrp, struct cgroup_taskset *tset)
{
	struct task_struct *task;
	struct io_context *ioc;

	cgroup_taskset_for_each(task, cgrp, tset) {
		/* we don't lose anything even if ioc allocation fails */
		ioc = get_task_io_context(task, GFP_ATOMIC, NUMA_NO_NODE);
		if (ioc) {
			ioc_cgroup_changed(ioc);
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
			put_io_context(ioc);
		}
	}
}

<<<<<<< HEAD
static void bfqio_destroy(struct cgroup *cgroup)
{
	struct bfqio_cgroup *bgrp = cgroup_to_bfqio(cgroup);
	struct hlist_node *n, *tmp;
	struct bfq_group *bfqg;

	/*
	 * Since we are destroying the cgroup, there are no more tasks
	 * referencing it, and all the RCU grace periods that may have
	 * referenced it are ended (as the destruction of the parent
	 * cgroup is RCU-safe); bgrp->group_data will not be accessed by
	 * anything else and we don't need any synchronization.
	 */
	hlist_for_each_entry_safe(bfqg, n, tmp, &bgrp->group_data, group_node)
		bfq_destroy_group(bgrp, bfqg);

	BUG_ON(!hlist_empty(&bgrp->group_data));

	kfree(bgrp);
}

struct cgroup_subsys bfqio_subsys = {
	.name = "bfqio",
	.create = bfqio_create,
	.can_attach = bfqio_can_attach,
	.attach = bfqio_attach,
	.destroy = bfqio_destroy,
	.populate = bfqio_populate,
	.subsys_id = bfqio_subsys_id,
};
#else
static inline void bfq_init_entity(struct bfq_entity *entity,
				   struct bfq_group *bfqg)
{
	entity->weight = entity->new_weight;
	entity->orig_weight = entity->new_weight;
	entity->ioprio = entity->new_ioprio;
	entity->ioprio_class = entity->new_ioprio_class;
	entity->sched_data = &bfqg->sched_data;
}

static inline struct bfq_group *
bfq_bic_update_cgroup(struct bfq_io_cq *bic)
{
	struct bfq_data *bfqd = bic_to_bfqd(bic);
	return bfqd->root_group;
}

static inline void bfq_bfqq_move(struct bfq_data *bfqd,
				 struct bfq_queue *bfqq,
				 struct bfq_entity *entity,
				 struct bfq_group *bfqg)
{
}

static void bfq_end_wr_async(struct bfq_data *bfqd)
{
	bfq_end_wr_async_queues(bfqd, bfqd->root_group);
}

static inline void bfq_disconnect_groups(struct bfq_data *bfqd)
{
	bfq_put_async_queues(bfqd, bfqd->root_group);
}

static inline void bfq_free_root_group(struct bfq_data *bfqd)
{
	kfree(bfqd->root_group);
}

static struct bfq_group *bfq_alloc_root_group(struct bfq_data *bfqd, int node)
{
	struct bfq_group *bfqg;
	int i;

	bfqg = kmalloc_node(sizeof(*bfqg), GFP_KERNEL | __GFP_ZERO, node);
	if (bfqg == NULL)
		return NULL;

	for (i = 0; i < BFQ_IOPRIO_CLASSES; i++)
		bfqg->sched_data.service_tree[i] = BFQ_SERVICE_TREE_INIT;

	return bfqg;
}
#endif
=======
void blkio_policy_register(struct blkio_policy_type *blkiop)
{
	spin_lock(&blkio_list_lock);
	list_add_tail(&blkiop->list, &blkio_list);
	spin_unlock(&blkio_list_lock);
}
EXPORT_SYMBOL_GPL(blkio_policy_register);

void blkio_policy_unregister(struct blkio_policy_type *blkiop)
{
	spin_lock(&blkio_list_lock);
	list_del_init(&blkiop->list);
	spin_unlock(&blkio_list_lock);
}
EXPORT_SYMBOL_GPL(blkio_policy_unregister);

static int __init init_cgroup_blkio(void)
{
	return cgroup_load_subsys(&blkio_subsys);
}

static void __exit exit_cgroup_blkio(void)
{
	cgroup_unload_subsys(&blkio_subsys);
}

module_init(init_cgroup_blkio);
module_exit(exit_cgroup_blkio);
MODULE_LICENSE("GPL");
>>>>>>> dd443260309c9cabf13b8e4fe17420c7ebfabcea
