// SPDX-License-Identifier: GPL-2.0
#include <linux/module.h>
#include <linux/cpufreq.h>
#include <linux/init.h>
#include <linux/cpu.h>

static int gaming_cpufreq_init(struct cpufreq_policy *policy)
{
    pr_info("Gaming governor: set CPU%d to max freq %u\n", policy->cpu, policy->max);

    // Paksa min dan max ke frekuensi maksimal
    policy->min = policy->max;
    policy->max = policy->max;

    cpufreq_update_policy(policy->cpu);

    return 0;
}

static void gaming_cpufreq_exit(struct cpufreq_policy *policy)
{
    pr_info("Gaming governor: exit CPU%d\n", policy->cpu);
}

static struct cpufreq_governor cpufreq_gaming_gov = {
    .name = "gaming", // <= 15 karakter
    .init = gaming_cpufreq_init,
    .exit = gaming_cpufreq_exit,
    .owner = THIS_MODULE,
};

static int __init cpufreq_gaming_gov_init(void)
{
    return cpufreq_register_governor(&cpufreq_gaming_gov);
}

static void __exit cpufreq_gaming_gov_exit(void)
{
    cpufreq_unregister_governor(&cpufreq_gaming_gov);
}

module_init(cpufreq_gaming_gov_init);
module_exit(cpufreq_gaming_gov_exit);

MODULE_AUTHOR("Kazera62");
MODULE_DESCRIPTION("Gaming CPUFreq Governor - Max Performance");
MODULE_LICENSE("GPL");
