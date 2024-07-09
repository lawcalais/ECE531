#!/bin/sh

DAEMON_NAME = "DAEMON_NAME"

start(){
	printf "starting $DAEMON_NAME: "
	echo "OK"
}

stop() {
	printf "Stopping $DAEMON_NAME: "
	echo "OK" 
}

restart() {
	stop 
	start
}

case "$1" in
	start)
	start 
	;;
	stop)
	stop
	;;
	restart|reload)
	restart
	;;
	*)

	echo "Usage: $0 {start|stop|restart}"
	exit 1
esac

exit $?
