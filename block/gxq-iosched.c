// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/elevator.h>
#include <linux/blk-mq.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/init.h>

struct gxq_data {
	struct list_head queue;
};

static void gxq_merged_requests(struct request_queue *q, struct request *req, struct request *next)
{
	list_del_init(&next->queuelist);
}

static int gxq_dispatch(struct request_queue *q, int force)
{
	struct gxq_data *nd = q->elevator->elevator_data;

	if (!list_empty(&nd->queue)) {
		struct request *rq;

		rq = list_entry(nd->queue.next, struct request, queuelist);
		list_del_init(&rq->queuelist);
		elv_dispatch_sort(q, rq);
		return 1;
	}
	return 0;
}

static void gxq_add_request(struct request_queue *q, struct request *rq)
{
	struct gxq_data *nd = q->elevator->elevator_data;

	/* Gaming logic: Prioritize READ at front, WRITE at end */
	if (rq_data_dir(rq) == READ)
		list_add(&rq->queuelist, &nd->queue);      // Prioritize
	else
		list_add_tail(&rq->queuelist, &nd->queue); // Normal
}

static int gxq_init_queue(struct request_queue *q, struct elevator_type *e)
{
	struct gxq_data *nd;
	struct elevator_queue *eq;

	eq = elevator_alloc(q, e);
	if (!eq)
		return -ENOMEM;

	nd = kzalloc(sizeof(*nd), GFP_KERNEL);
	if (!nd) {
		kobject_put(&eq->kobj);
		return -ENOMEM;
	}

	INIT_LIST_HEAD(&nd->queue);
	eq->elevator_data = nd;
	q->elevator = eq;

	return 0;
}

static void gxq_exit_queue(struct elevator_queue *e)
{
	struct gxq_data *nd = e->elevator_data;

	kfree(nd);
}

static struct elevator_type elevator_gxq = {
	.ops = {
		.elevator_merge_req_fn	= gxq_merged_requests,
		.elevator_dispatch_fn	= gxq_dispatch,
		.elevator_add_req_fn	= gxq_add_request,
		.elevator_init_fn	= gxq_init_queue,
		.elevator_exit_fn	= gxq_exit_queue,
	},
	.elevator_name = "gxq",
	.elevator_owner = THIS_MODULE,
};

static int __init gxq_init(void)
{
	return elv_register(&elevator_gxq);
}

static void __exit gxq_exit(void)
{
	elv_unregister(&elevator_gxq);
}

module_init(gxq_init);
module_exit(gxq_exit);

MODULE_AUTHOR("PANđøʀᴀ & zera");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Gaming eXtreme Quick I/O scheduler");