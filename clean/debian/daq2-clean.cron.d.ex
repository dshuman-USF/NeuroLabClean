#
# Regular cron jobs for the daq2-clean package
#
0 4	* * *	root	[ -x /usr/bin/daq2-clean_maintenance ] && /usr/bin/daq2-clean_maintenance
