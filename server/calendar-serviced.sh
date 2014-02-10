#dlogutil -v threadtime -f /var/log/calendar-serviced.log -r 1000 -n 10 CALENDAR_SVC &

source /etc/tizen-platform.conf

${TZ_SYS_BIN}/calendar-serviced &
