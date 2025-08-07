// SPDX-License-Identifier: GPL-2.0
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/mutex.h>
#include <linux/workqueue.h>
#include <linux/sched/cputime.h>
#include <linux/delay.h>
#include <linux/jiffies.h>

static struct cpufreq_governor eco_intel_gov;

struct eco_intel_cpuinfo {
	struct cpufreq_policy *policy;
	struct delayed_work work;
};

static void eco_intel_adjust_freq(struct work_struct *work)
{
	struct eco_intel_cpuinfo *ci = container_of(work, struct eco_intel_cpuinfo, work.work);
	struct cpufreq_policy *policy = ci->policy;
	unsigned int min_freq = policy->min;
	unsigned int max_freq = policy->max;
	unsigned int target_freq;

	// Logika ultra hemat daya:
	// Jika idle banyak, turunin freq. Kalau aktif naik dikit aja.
	unsigned int load = cpufreq_quick_get(policy->cpu);
	if (!load)
		load = min_freq;

	// Paling hemat: ambil 1/4 dari max freq saat idle
	if (load < max_freq / 4)
		target_freq = min_freq;
	else if (load < max_freq / 2)
		target_freq = (min_freq + max_freq) / 4;
	else
		target_freq = (min_freq + max_freq) / 2;

	cpufreq_driver_target(policy, target_freq, CPUFREQ_RELATION_L);
	schedule_delayed_work(&ci->work, msecs_to_jiffies(300)); // Cek setiap 300ms
}

static int eco_intel_start(struct cpufreq_policy *policy)
{
	struct eco_intel_cpuinfo *ci;

	ci = kzalloc(sizeof(*ci), GFP_KERNEL);
	if (!ci)
		return -ENOMEM;

	ci->policy = policy;
	policy->governor_data = ci;

	INIT_DELAYED_WORK(&ci->work, eco_intel_adjust_freq);
	schedule_delayed_work(&ci->work, msecs_to_jiffies(300));
	return 0;
}

static void eco_intel_stop(struct cpufreq_policy *policy)
{
	struct eco_intel_cpuinfo *ci = policy->governor_data;
	if (!ci)
		return;

	cancel_delayed_work_sync(&ci->work);
	kfree(ci);
}

static struct cpufreq_governor eco_intel_gov = {
	.name = "eco_intel",
	.owner = THIS_MODULE,
	.init = eco_intel_start,
	.exit = eco_intel_stop,
	.dynamic_switching = true,
};

static int __init eco_intel_init(void)
{
	return cpufreq_register_governor(&eco_intel_gov);
}
module_init(eco_intel_init);

static void __exit eco_intel_exit(void)
{
	cpufreq_unregister_governor(&eco_intel_gov);
}
module_exit(eco_intel_exit);

MODULE_AUTHOR("PANđøʀᴀ");
MODULE_DESCRIPTION("eco_intel - ultra power saving CPU governor");
MODULE_LICENSE("GPL");