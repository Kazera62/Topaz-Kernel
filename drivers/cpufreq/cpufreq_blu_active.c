// SPDX-License-Identifier: GPL-2.0-only
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/cpufreq.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/jiffies.h>
#include <linux/cpumask.h>
#include <linux/cpu.h>

#define POLL_INTERVAL_MS 100  // 100ms interval
#define HIGH_LOAD_THRESHOLD 60  // Load > 60% = naik frekuensi
#define LOW_LOAD_THRESHOLD 20   // Load < 20% = turun frekuensi

static struct workqueue_struct *blu_wq;
static struct delayed_work blu_work;
static struct cpufreq_policy *policy_ptr;

static u64 prev_idle_time = 0, prev_total_time = 0;

static void blu_check_load(struct work_struct *work)
{
	unsigned int cpu = policy_ptr->cpu;
	u64 cur_idle_time, cur_total_time;
	u64 delta_idle, delta_total;
	unsigned int load;

	cur_idle_time = get_cpu_idle_time_us(cpu, &cur_total_time, false);
	delta_idle = cur_idle_time - prev_idle_time;
	delta_total = cur_total_time - prev_total_time;

	if (delta_total == 0) {
		goto out;
	}

	load = 100 * (delta_total - delta_idle) / delta_total;

	pr_debug("blu_active: CPU %u load: %u%%\n", cpu, load);

	if (load > HIGH_LOAD_THRESHOLD && policy_ptr->cur < policy_ptr->max) {
		cpufreq_driver_target(policy_ptr, policy_ptr->max, CPUFREQ_RELATION_H);
	} else if (load < LOW_LOAD_THRESHOLD && policy_ptr->cur > policy_ptr->min) {
		cpufreq_driver_target(policy_ptr, policy_ptr->min, CPUFREQ_RELATION_L);
	}

out:
	prev_idle_time = cur_idle_time;
	prev_total_time = cur_total_time;
	queue_delayed_work(blu_wq, &blu_work, msecs_to_jiffies(POLL_INTERVAL_MS));
}

static int blu_governor_func(struct cpufreq_policy *policy, unsigned int event)
{
	switch (event) {
	case CPUFREQ_GOV_START:
		pr_info("blu_active: Governor started on CPU %u\n", policy->cpu);
		policy_ptr = policy;
		prev_idle_time = 0;
		prev_total_time = 0;

		blu_wq = alloc_workqueue("blu_active_wq", WQ_HIGHPRI, 0);
		if (!blu_wq)
			return -ENOMEM;

		INIT_DELAYED_WORK(&blu_work, blu_check_load);
		queue_delayed_work(blu_wq, &blu_work, msecs_to_jiffies(POLL_INTERVAL_MS));
		break;

	case CPUFREQ_GOV_STOP:
		pr_info("blu_active: Governor stopped\n");
		cancel_delayed_work_sync(&blu_work);
		destroy_workqueue(blu_wq);
		break;

	case CPUFREQ_GOV_LIMITS:
		if (policy->cur < policy->min)
			cpufreq_driver_target(policy, policy->min, CPUFREQ_RELATION_L);
		else if (policy->cur > policy->max)
			cpufreq_driver_target(policy, policy->max, CPUFREQ_RELATION_H);
		break;
	}
	return 0;
}

static struct cpufreq_governor blu_active_gov = {
	.name = "blu_active",
	.governor = blu_governor_func,
	.owner = THIS_MODULE,
	.flags = CPUFREQ_GOV_DYNAMIC_SWITCHING,
};

static int __init blu_init(void)
{
	pr_info("blu_active: Initializing advanced governor\n");
	return cpufreq_register_governor(&blu_active_gov);
}

static void __exit blu_exit(void)
{
	pr_info("blu_active: Exiting advanced governor\n");
	cpufreq_unregister_governor(&blu_active_gov);
}

module_init(blu_init);
module_exit(blu_exit);

MODULE_AUTHOR("PANđøʀᴀ");
MODULE_DESCRIPTION("Blu Active Governor v2 - Responsive CPU Load-based Governor");
MODULE_LICENSE("GPL");