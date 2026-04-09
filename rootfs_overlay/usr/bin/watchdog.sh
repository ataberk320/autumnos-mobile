#!/bin/sh
        
sudo echo 1 > /proc/sys/kernel/sysrq

uptime_format_detector() {
    UPTIME_FILE="/tmp/autumnsys/uptime/autumnuptime0"
    
    while [ ! -f "$UPTIME_FILE" ]; do
        sleep 1
    done

    while true; do
        if [ -f "$UPTIME_FILE" ]; then
            CONTENT=$(cat "$UPTIME_FILE" | tr -d '[:space:]')
            
            if [ -n "$CONTENT" ]; then
                case "$CONTENT" in
                    *[!0-9]*)
						echo 1 > /sys/class/vtconsole/vtcon1/bind  # for enable Glitch panic
                        echo "Invalid uptime format, system is confused: '$CONTENT'" > /dev/kmsg
                        	 
                             sleep 0.1
							 
                             echo c > /proc/sysrq-trigger
                        
                        ;;
                esac
            fi
        fi
        usleep 500000 
    done

}
ram_format_detector() {
	    UPTIME_FILE="/tmp/autumnsys/memory/autumnram0"
	
    while [ ! -f "$UPTIME_FILE" ]; do
        sleep 1
    done

    while true; do
        if [ -f "$UPTIME_FILE" ]; then
            CONTENT=$(cat "$UPTIME_FILE" | tr -d '[:space:]')

            if [ -n "$CONTENT" ]; then
                case "$CONTENT" in
                    *[!0-9]*)
					    echo 1 > /sys/class/vtconsole/vtcon1/bind
                        echo "RAM is crashed fatally: invalid RAM format '$CONTENT'" > /dev/kmsg

                             sleep 0.1
                             echo c > /proc/sysrq-trigger

                        ;;
                esac
            fi
        fi
        usleep 500000
    done
}

sim_status_format_detector() {
    UPTIME_FILE="/tmp/autumnsys/connection/autumnsim0"
    
    while [ ! -f "$UPTIME_FILE" ]; do
        sleep 1
    done

    while true; do
        if [ -f "$UPTIME_FILE" ]; then
            CONTENT=$(cat "$UPTIME_FILE" | tr -d '[:space:]')
            
            if [ -n "$CONTENT" ]; then
                case "$CONTENT" in
                    *[!0-9]*)
						echo 1 > /sys/class/vtconsole/vtcon1/bind  # for enable Glitch panic
                        echo "Invalid SIM card status, system is confused: '$CONTENT'" > /dev/kmsg
                        	 
                             sleep 0.1
							 
                             echo c > /proc/sysrq-trigger
                        
                        ;;
                esac
            fi
        fi
        usleep 500000 
    done

}

disk_format_detector() {
    UPTIME_FILE="/tmp/autumnsys/storage/autumndisk0"
    
    while [ ! -f "$UPTIME_FILE" ]; do
        sleep 1
    done

    while true; do
        if [ -f "$UPTIME_FILE" ]; then
            CONTENT=$(cat "$UPTIME_FILE" | tr -d '[:space:]')
            
            if [ -n "$CONTENT" ]; then
                case "$CONTENT" in
                    *[!0-9]*)
						echo 1 > /sys/class/vtconsole/vtcon1/bind  # for enable Glitch panic
                        echo "Invalid uptime format, system is confused: '$CONTENT'" > /dev/kmsg
                        	 
                             sleep 0.1
							 
                             echo c > /proc/sysrq-trigger
                        
                        ;;
                esac
            fi
        fi
        usleep 500000 
    done

}
critical_process_detector()  {
while true; do
	#If critical process dies
	if ! pidof AutixSurfDetector > /dev/null; then
		echo 1 > /sys/class/vtconsole/vtcon1/bind
		echo "Critical system process died!" > /dev/kmsg
		sleep 0.1
		echo c > /proc/sysrq-trigger
	fi

	#If core crashes
	if ! pidof AutumnCore > /dev/null; then
		echo 1 > /sys/class/vtconsole/vtcon1/bind
		echo "Core crashed. System is unstable." > /dev/kmsg
		sleep 0.1
		echo c > /proc/sysrq-trigger
	fi
	
	#If system is clogged by fork bomb or any reason
	if ! pidof init > /dev/null; then
		echo 1 > /sys/class/vtconsole/vtcon1/bind
		echo "System is clogged!" > /dev/kmsg
		sleep 0.1
		echo c > /proc/sysrq-trigger
	fi

	usleep 100000
done
}
uptime_format_detector &
critical_process_detector &
ram_format_detector &
sim_status_format_detector &
disk_format_detector &
wait
