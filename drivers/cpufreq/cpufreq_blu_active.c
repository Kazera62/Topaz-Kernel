// SPDX-License-Identifier: GPL-2.0-only
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/init.h>

static void blu_active_limits(struct cpufreq_policy *policy)
{
	pr_debug("blu_active: setting freq to %u kHz\n", policy->cur);
	__cpufreq_driver_target(policy, policy->cur, CPUFREQ_RELATION_L);
}

static struct cpufreq_governor blu_active_gov = {
	.name = "blu_active",
	.owner = THIS_MODULE,
	.limits = blu_active_limits,
	.flags = CPUFREQ_GOV_DYNAMIC_SWITCHING,
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
MODULE_DESCRIPTION("Blu Active CPUFreq governor (dynamic + lightweight)");
MODULE_LICENSE("GPL");
