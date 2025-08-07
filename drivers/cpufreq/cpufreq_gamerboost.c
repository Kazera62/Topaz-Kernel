// SPDX-License-Identifier: GPL-2.0-only
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/sched/signal.h>
#include <linux/sched/loadavg.h>
#include <linux/workqueue.h>

#define BOOST_THRESHOLD 15
#define IDLE_THRESHOLD 10

static void gamerboost_limits(struct cpufreq_policy *policy)
{
	unsigned int load = (avenrun[0] * 100) >> FSHIFT;
	unsigned int target_freq;

	/* Check for RT tasks or high-priority foreground tasks */
	bool rt_task_active = false;
	struct task_struct *g;

	rcu_read_lock();
	for_each_process(g) {
		if (task_is_running(g) &&
		    (rt_task(g) || (g->prio <= MAX_RT_PRIO + 20))) {
			rt_task_active = true;
			break;
		}
	}
	rcu_read_unlock();

	if (rt_task_active || load >= BOOST_THRESHOLD) {
		target_freq = policy->max;
	} else if (load <= IDLE_THRESHOLD) {
		target_freq = (policy->cur + policy->min) / 2;
	} else {
		target_freq = (policy->cur + policy->max) / 2;
	}

	pr_debug("gamerboost_extreme: load=%u%%, rt_task=%d, setting freq=%u\n",
		 load, rt_task_active, target_freq);

	__cpufreq_driver_target(policy, target_freq, CPUFREQ_RELATION_H);
}

static struct cpufreq_governor gamerboost_gov = {
	.name = "gamerboost_extreme",
	.owner = THIS_MODULE,
	.limits = gamerboost_limits,
	.flags = CPUFREQ_GOV_DYNAMIC_SWITCHING,
};

static int __init gamerboost_init(void)
{
	pr_info("GamerBoost Extreme Governor Initialized\n");
	return cpufreq_register_governor(&gamerboost_gov);
}

static void __exit gamerboost_exit(void)
{
	cpufreq_unregister_governor(&gamerboost_gov);
	pr_info("GamerBoost Extreme Governor Unloaded\n");
}

module_init(gamerboost_init);
module_exit(gamerboost_exit);

MODULE_AUTHOR("PANđøʀᴀ");
MODULE_DESCRIPTION("GamerBoost Extreme Governor - Smart + Reactive Gaming");
MODULE_LICENSE("GPL");