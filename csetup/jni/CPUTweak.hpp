#ifndef __CPU_TWEAK_HPP__
#define __CPU_TWEAK_HPP__

void echo(const char *str, const char *file)
{
	FILE *f = fopen(file, "w");
	if ( f ) 
	{
		fprintf(f, "%s\n", str);
		fclose(f);
	}
}

void tweakCPUandIOSched()
{
	return ;

	echo("384000", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
	echo("1728000", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
	echo("interactive", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
	
	echo("384000", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq");
	echo("1728000", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq");
	echo("interactive", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_governor");

	echo("noop", "/sys/devices/platform/msm_sdcc.1/mmc_host/mmc0/mmc0:0001/block/mmcblk0/queue/scheduler");
	echo("noop", "/sys/devices/platform/msm_sdcc.3/mmc_host/mmc1/mmc1:0001/block/mmcblk1/queue/scheduler");
}

void CPUStartupSetup() 
{
	echo("1", "/sys/module/rpm_resources/enable_low_power/L2_cache");
	echo("1", "/sys/module/rpm_resources/enable_low_power/pxo");
	echo("2", "/sys/module/rpm_resources/enable_low_power/vdd_dig");
	echo("2", "/sys/module/rpm_resources/enable_low_power/vdd_mem");
	echo("1", "/sys/module/rpm_resources/enable_low_power/rpm_cpu");
	echo("1", "/sys/module/pm_8x60/modes/cpu0/power_collapse/suspend_enabled");
	echo("1", "/sys/module/pm_8x60/modes/cpu1/power_collapse/suspend_enabled");
	echo("1", "/sys/module/pm_8x60/modes/cpu0/standalone_power_collapse/suspend_enabled");
	echo("1", "/sys/module/pm_8x60/modes/cpu1/standalone_power_collapse/suspend_enabled");
	echo("1", "/sys/module/pm_8x60/modes/cpu0/power_collapse/idle_enabled");
	echo("1", "/sys/module/pm_8x60/modes/cpu1/power_collapse/idle_enabled");
	echo("1", "/sys/module/pm_8x60/modes/cpu0/standalone_power_collapse/idle_enabled");
	echo("1", "/sys/module/pm_8x60/modes/cpu1/standalone_power_collapse/idle_enabled");
	echo("1", "/sys/module/pm_8660/modes/cpu0/power_collapse/suspend_enabled");
	echo("1", "/sys/module/pm_8660/modes/cpu1/power_collapse/suspend_enabled");
	echo("1", "/sys/module/pm_8660/modes/cpu0/standalone_power_collapse/suspend_enabled");
	echo("1", "/sys/module/pm_8660/modes/cpu1/standalone_power_collapse/suspend_enabled");
	echo("1", "/sys/module/pm_8660/modes/cpu0/power_collapse/idle_enabled");
	echo("1", "/sys/module/pm_8660/modes/cpu1/power_collapse/idle_enabled");
	echo("1", "/sys/module/pm_8660/modes/cpu0/standalone_power_collapse/idle_enabled");
	echo("1", "/sys/module/pm_8660/modes/cpu1/standalone_power_collapse/idle_enabled");
	echo("ondemand", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
	echo("ondemand", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_governor");
	echo("50000", "/sys/devices/system/cpu/cpufreq/ondemand/sampling_rate");
	echo("90", "/sys/devices/system/cpu/cpufreq/ondemand/up_threshold");
	echo("1", "/sys/devices/system/cpu/cpufreq/ondemand/io_is_busy");
	echo("4", "/sys/devices/system/cpu/cpufreq/ondemand/sampling_down_factor");
	echo("intellidemand", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_governor");
	echo("intellidemand", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_governor");
	echo("50000", "/sys/devices/system/cpu/cpufreq/intellidemand/sampling_rate");
	echo("90", "/sys/devices/system/cpu/cpufreq/intellidemand/up_threshold");
	echo("1", "/sys/devices/system/cpu/cpufreq/intellidemand/io_is_busy");
	echo("4", "/sys/devices/system/cpu/cpufreq/intellidemand/sampling_down_factor");
	echo("384000", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq");
	echo("1512000", "/sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq");
	echo("1", "/sys/devices/system/cpu/cpu1/cpufreq/online");
	echo("384000", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_min_freq");
	echo("1512000", "/sys/devices/system/cpu/cpu1/cpufreq/scaling_max_freq");

	// wifi crash fix
	echo("8192", "/proc/sys/vm/min_free_kbytes");
}

#endif
