// SPDX-License-Identifier: GPL-2.0
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/delay.h>

struct blu_policy_data {
	struct cpufreq_policy *policy;
	struct task_struct *thread;
	bool should_run;
};

static int blu_thread_fn(void *data)
{
	struct blu_policy_data *pdata = data;
	unsigned int cpu = pdata->policy->cpu;
	u64 cur_idle_time, prev_idle_time = 0;
	u64 cur_total_time, prev_total_time = 0;
	unsigned int freq;

	while (!kthread_should_stop()) {
		cur_idle_time = get_cpu_idle_time_us(cpu, &cur_total_time);

		if (cur_total_time != prev_total_time) {
			u64 delta_idle = cur_idle_time - prev_idle_time;
			u64 delta_total = cur_total_time - prev_total_time;
			u64 usage = 100 - (delta_idle * 100 / delta_total);

			if (usage > 80)
				freq = pdata->policy->max;
			else if (usage < 20)
				freq = pdata->policy->min;
			else
				freq = (pdata->policy->max + pdata->policy->min) / 2;

			cpufreq_driver_target(pdata->policy, freq, CPUFREQ_RELATION_L);
			prev_idle_time = cur_idle_time;
			prev_total_time = cur_total_time;
		}

		msleep(50);
	}

	return 0;
}

static int blu_init(struct cpufreq_policy *policy)
{
	struct blu_policy_data *pdata;

	pdata = kzalloc(sizeof(*pdata), GFP_KERNEL);
	if (!pdata)
		return -ENOMEM;

	pdata->policy = policy;
	pdata->should_run = true;

	pdata->thread = kthread_run(blu_thread_fn, pdata, "blu_cpu_thread/%u", policy->cpu);
	if (IS_ERR(pdata->thread)) {
		kfree(pdata);
		return PTR_ERR(pdata->thread);
	}

	policy->governor_data = pdata;
	return 0;
}

static void blu_exit(struct cpufreq_policy *policy)
{
	struct blu_policy_data *pdata = policy->governor_data;

	if (pdata && pdata->thread)
		kthread_stop(pdata->thread);

	kfree(pdata);
}

static void blu_limits(struct cpufreq_policy *policy)
{
	// Optional: implement jika ingin merespon perubahan batas frekuensi
}

static struct cpufreq_governor blu_active_gov = {
	.name = "blu_active",
	.owner = THIS_MODULE,
	.init = blu_init,
	.exit = blu_exit,
	.limits = blu_limits,
};

static int __init blu_active_init(void)
{
	return cpufreq_register_governor(&blu_active_gov);
}

static void __exit blu_active_exit(void)
{
	cpufreq_unregister_governor(&blu_active_gov);
}

module_init(blu_active_init);
module_exit(blu_active_exit);

MODULE_AUTHOR("PANđøʀᴀ");
MODULE_DESCRIPTION("blu_active CPUFreq Governor");
MODULE_LICENSE("GPL");