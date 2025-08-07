// SPDX-License-Identifier: GPL-2.0-only
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/tick.h>
#include <linux/jiffies.h>
#include <linux/workqueue.h>

#define HIGH_LOAD_THRESHOLD 80
#define LOW_LOAD_THRESHOLD 20

static void intelid_adjust_freq(struct cpufreq_policy *policy)
{
	unsigned int load = 0;
	unsigned int freq;

	load = get_cpu_idle_time_us(policy->cpu, NULL);

	if (load > HIGH_LOAD_THRESHOLD) {
		freq = policy->max;
	} else if (load < LOW_LOAD_THRESHOLD) {
		freq = policy->min;
	} else {
		freq = (policy->cur + policy->max) / 2;
	}

	pr_info("intelid_v2: load=%u, setting freq=%u\n", load, freq);
	__cpufreq_driver_target(policy, freq, CPUFREQ_RELATION_L);
}

static void intelid_limits(struct cpufreq_policy *policy)
{
	intelid_adjust_freq(policy);
}

static struct cpufreq_governor intelid_gov = {
	.name = "intelid",
	.owner = THIS_MODULE,
	.limits = intelid_limits,
	.flags = CPUFREQ_GOV_DYNAMIC_SWITCHING,
};

static int __init intelid_init(void)
{
	pr_info("intelid_v2: Initializing\n");
	return cpufreq_register_governor(&intelid_gov);
}

static void __exit intelid_exit(void)
{
	cpufreq_unregister_governor(&intelid_gov);
	pr_info("intelid_v2: Unloaded\n");
}

module_init(intelid_init);
module_exit(intelid_exit);

MODULE_AUTHOR("PANđøʀᴀ");
MODULE_DESCRIPTION("intelid_v2 CPUFreq governor (responsive load-based logic)");
MODULE_LICENSE("GPL");