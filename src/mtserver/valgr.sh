#!/bin/sh
#
# /etc/init.d/ufdb
#
# stop/start the UFDB daemons
#
# All *nix flavors have different mechanisms for
# stop/start scripts to give feedback and it is not supported.
# So this script tries to give a simple and usable feedback
# with the echo command only.
#
# This script should be in /etc/init.d, /sbin/init.d or equivalent.
# From rc3.d there should be symbolic links to this script.
# Suggested names to use in rc3.d are K02ufdb and S50ufdb.
#
# $Id: ufdb.sh,v 1.6 2005/11/01 19:27:26 root Exp root $

SQUIDUSER=squid
SQUIDDIR="/local/squid"

who=`whoami`

msg=""


case "$1" in
	start)
		msg="Starting UFDB daemon"
		if [ "$who" = root ]
		then
		   su $SQUIDUSER -c "$SQUIDDIR/bin/ufdbguardd -c $SQUIDDIR/etc/ufdbGuard.conf"
		else
		   $SQUIDDIR/bin/ufdbguardd -c $SQUIDDIR/etc/ufdbGuard.conf
	        fi
		exitcode=$?
		# delay to prevent startup retries by ufdbgclient/squid
		sleep 1		
		;;
	stop)
		msg="Shutting down UFDB daemon"
		PIDS=`ps -ef | grep valgrind | grep -v grep | awk '{ print $2 }' `
		exitcode=0
		if [ "$PIDS" != "" ]
		then
		   kill -TERM $PIDS
		   exitcode=$?
		   # sleep 1
		   # kill -TERM $PIDS 2>/dev/null
	        fi
		;;
	kill)
		msg="Killing UFDB daemon"
		PIDS=`ps -ef | grep valgrind | grep -v grep | awk '{ print $2 }' `
		if [ "$PIDS" != "" ]
		then
		   kill -KILL $PIDS
	        fi
		exitcode=$?
		;;
	reconfig)
		PIDS=`ps -ef | grep valgrind | grep -v grep | awk '{ print $2 }' `
		if [ "$PIDS" != "" ]
		then
		   echo "Sending HUP signal to UFDB daemon to reconfigure"
		   kill -HUP $PIDS
	        fi
		exitcode=0
		;;
	restart)
		$0 stop
		$0 start
		exitcode=$?
		;;
	status)
		echo "Checking for UFDB daemon"
		ps -ef | grep ufdb | grep -v grep
		exitcode=$?
		;;
	*)
		echo "Usage: $0 {start|stop|status|restart|reconfig|kill}"
		exit 1
		;;
esac

if [ "$msg" != "" ]
then
   if [ $exitcode -eq 0 ]
   then
      echo "$msg OK"
   else
      echo "$msg FAIL"
   fi
fi

exit $exitcode

