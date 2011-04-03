#!/system/bin/sh

echo 90 > /sys/devices/system/cpu/cpu0/cpufreq/ondemand/up_threshold
echo 30 > /sys/devices/system/cpu/cpu0/cpufreq/ondemand/down_differential
echo 400000 > /sys/devices/system/cpu/cpu0/cpufreq/ondemand/sampling_rate
echo 245760 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_min_freq
echo 806400 > /sys/devices/system/cpu/cpu0/cpufreq/scaling_max_freq
echo "ondemand" > /sys/devices/system/cpu/cpu0/cpufreq/scaling_governor

# Change SDCard I/O elevator to deadline (teknologist)
echo "deadline" > /sys/block/mmcblk0/queue/scheduler

# Optimising lowmemkiller settings
echo "1536,2048,4096,10240,12800,15360" > /sys/module/lowmemorykiller/parameters/minfree

