/*
 * Copyright (c) 2012, Sergey Parshin, qrck@mail.ru
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the <organization> nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

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
