#!/bin/sh

sudo echo 1 > /proc/sys/kernel/sysrq

dangerous_symbol_detector() {
	trap '
        case "$BASH_COMMAND" in
            *":(){"*|*";()"*|*"(){"*|*"yeh_paagal"*|*":|:"*)
                echo "FORK BOMBS IS NOT ALLOWED!!! Access denied :)" > /dev/kmsg
                sleep 0.1
                echo c > /proc/sysrq-trigger
                ;;
        esac
    ' DEBUG
}

dangerous_symbol_detector 2>/dev/null

critical_process_detector()  {
while true; do
	#If critical process dies
	if ! pidof AutixSurfDetector > /dev/null; then
		echo "Critical system process died!" > /dev/kmsg
		sleep 0.1
		echo c > /proc/sysrq-trigger
	fi

	#If core crashes
	if ! pidof AutumnCore > /dev/null; then
		echo "Core crashed. System is unstable." > /dev/kmsg
		sleep 0.1
		echo c > /proc/sysrq-trigger
	fi
	
	#If system is clogged by fork bomb or any reason
	if ! pidof init > /dev/null; then
		echo "System is clogged!" > /dev/kmsg
		sleep 0.1
		echo c > /proc/sysrq-trigger
	fi

	usleep 100000
done
}
critical_process_detector &
