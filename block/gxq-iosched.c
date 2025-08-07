// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/elevator.h>
#include <linux/bio.h>
#include <linux/blkdev.h>
#include <linux/slab.h>
#include <linux/init.h>

struct gxq_data {
	struct list_head queue;
};

static void gxq_merged_requests(struct request_queue *q, struct request *rq,
				 struct request *next)
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

	// Gaming logic: prioritas READ saat game, tulis di idle
	if (rq_data_dir(rq) == READ) {
		list_add(&rq->queuelist, &nd->queue); // Prioritas depan
	} else {
		list_add_tail(&rq->queuelist, &nd->queue); // Belakang
	}
}

static struct elevator_type elevator_gxq = {
	.ops = {
		.elevator_merge_req_fn		= gxq_merged_requests,
		.elevator_dispatch_fn		= gxq_dispatch,
		.elevator_add_req_fn		= gxq_add_request,
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