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


#endif