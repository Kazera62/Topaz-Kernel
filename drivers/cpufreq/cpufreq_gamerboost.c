// SPDX-License-Identifier: GPL-2.0-only
#include <linux/cpufreq.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/tick.h>
#include <linux/sched.h>
#include <linux/sched/loadavg.h>

#define BOOST_THRESHOLD 15
#define IDLE_THRESHOLD 10

static void gamerboost_limits(struct cpufreq_policy *policy)
{
        unsigned int load = (avenrun[0] * 100) >> FSHIFT;
        unsigned int target_freq;

        if (load >= BOOST_THRESHOLD) {
                target_freq = policy->max;
        } else if (load <= IDLE_THRESHOLD) {
                target_freq = (policy->cur + policy->min) / 2;
        } else {
                target_freq = (policy->cur + policy->max) / 2;
        }

        pr_debug("gamerboost: load=%u%%, setting freq=%u\n", load, target_freq);
        __cpufreq_driver_target(policy, target_freq, CPUFREQ_RELATION_H);
}

static struct cpufreq_governor gamerboost_gov = {
        .name = "gamerboost",
        .owner = THIS_MODULE,
        .limits = gamerboost_limits,
        .flags = CPUFREQ_GOV_DYNAMIC_SWITCHING,
};

static int __init gamerboost_init(void)
{
        return cpufreq_register_governor(&gamerboost_gov);
}

static void __exit gamerboost_exit(void)
{
        cpufreq_unregister_governor(&gamerboost_gov);
}

module_init(gamerboost_init);
module_exit(gamerboost_exit);

MODULE_AUTHOR("PANđøʀᴀ");
MODULE_DESCRIPTION("Extreme Gaming Smart CPUFreq governor");
MODULE_LICENSE("GPL");