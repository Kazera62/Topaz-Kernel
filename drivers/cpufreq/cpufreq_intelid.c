// SPDX-License-Identifier: GPL-2.0-only
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/init.h>

static void intelid_limits(struct cpufreq_policy *policy)
{
	unsigned int target_freq = (policy->cur + policy->max) / 2;
	pr_debug("intelid: setting freq to %u kHz\n", target_freq);
	__cpufreq_driver_target(policy, target_freq, CPUFREQ_RELATION_H);
}

static struct cpufreq_governor intelid_gov = {
	.name = "intelid",
	.owner = THIS_MODULE,
	.limits = intelid_limits,
	.flags = CPUFREQ_GOV_DYNAMIC_SWITCHING,
};

static int __init intelid_init(void)
{
	return cpufreq_register_governor(&intelid_gov);
}

static void __exit intelid_exit(void)
{
	cpufreq_unregister_governor(&intelid_gov);
}

module_init(intelid_init);
module_exit(intelid_exit);

MODULE_AUTHOR("PANđøʀᴀ");
MODULE_DESCRIPTION("Intelid CPUFreq governor (dynamic mid-frequency policy)");
MODULE_LICENSE("GPL");
